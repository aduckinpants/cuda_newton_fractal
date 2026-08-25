import type { DigitScrubConfig, DigitValue, PlaceDigit, RationalValue, Sign } from "./types";

const DIGITS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

type EditClamp = Pick<DigitScrubConfig, "min" | "max" | "precision">;

function assertBase(base: number): void {
  if (!Number.isInteger(base) || base < 2 || base > DIGITS.length) {
    throw new Error(`base must be an integer between 2 and ${DIGITS.length}`);
  }
}

function digitToValue(ch: string, base: number): number {
  const n = DIGITS.indexOf(ch.toUpperCase());
  if (n < 0 || n >= base) {
    throw new Error(`digit ${ch} is invalid for base ${base}`);
  }
  return n;
}

function valueToDigit(n: number): string {
  if (!Number.isInteger(n) || n < 0 || n >= DIGITS.length) {
    throw new Error(`invalid digit value ${n}`);
  }
  return DIGITS[n];
}

function trimLeadingZeros(digits: number[]): number[] {
  let i = 0;
  while (i < digits.length - 1 && digits[i] === 0) i++;
  return digits.slice(i);
}

function trimTrailingZeros(digits: number[]): number[] {
  let end = digits.length;
  while (end > 0 && digits[end - 1] === 0) end--;
  return digits.slice(0, end);
}

function isZeroDigits(intDigits: number[], fracDigits: number[]): boolean {
  return intDigits.every((d) => d === 0) && fracDigits.every((d) => d === 0);
}

function powBigInt(base: number, exp: number): bigint {
  if (exp < 0) throw new Error(`negative bigint exponent ${exp}`);
  let out = 1n;
  const b = BigInt(base);
  for (let i = 0; i < exp; i++) out *= b;
  return out;
}

function gcd(a: bigint, b: bigint): bigint {
  let x = a < 0n ? -a : a;
  let y = b < 0n ? -b : b;
  while (y !== 0n) {
    const next = x % y;
    x = y;
    y = next;
  }
  return x || 1n;
}

function normalizeRational(numerator: bigint, denominator: bigint): RationalValue {
  if (denominator === 0n) throw new Error("rational denominator cannot be zero");
  if (numerator === 0n) return { numerator: 0n, denominator: 1n };

  let n = numerator;
  let d = denominator;
  if (d < 0n) {
    n = -n;
    d = -d;
  }

  const divisor = gcd(n, d);
  return { numerator: n / divisor, denominator: d / divisor };
}

function digitsToBigInt(digits: number[], base: number): bigint {
  let out = 0n;
  const b = BigInt(base);
  for (const digit of digits) {
    out = out * b + BigInt(digit);
  }
  return out;
}

function toScaledMantissa(value: DigitValue, targetScale: number): bigint {
  if (targetScale < value.fracDigits.length) {
    throw new Error("targetScale cannot truncate fractional digits");
  }
  const raw = digitsToBigInt([...value.intDigits, ...value.fracDigits], value.base);
  const scaled = raw * powBigInt(value.base, targetScale - value.fracDigits.length);
  return value.sign === -1 ? -scaled : scaled;
}

function fromScaledMantissa(mantissa: bigint, base: number, scale: number): DigitValue {
  assertBase(base);
  if (scale < 0) throw new Error("scale cannot be negative");

  let sign: Sign = 1;
  let mag = mantissa;
  if (mag < 0n) {
    sign = -1;
    mag = -mag;
  }

  const digitsLsd: number[] = [];
  const b = BigInt(base);
  if (mag === 0n) {
    digitsLsd.push(0);
  } else {
    while (mag > 0n) {
      digitsLsd.push(Number(mag % b));
      mag /= b;
    }
  }

  while (digitsLsd.length <= scale) {
    digitsLsd.push(0);
  }

  const digits = digitsLsd.reverse();
  const split = digits.length - scale;
  const intDigits = trimLeadingZeros(split > 0 ? digits.slice(0, split) : [0]);
  const fracDigits = trimTrailingZeros(scale > 0 ? digits.slice(split) : []);

  if (isZeroDigits(intDigits, fracDigits)) {
    return { base, sign: 1, intDigits: [0], fracDigits: [] };
  }
  return { base, sign, intDigits, fracDigits };
}

function bigIntToDigits(magnitude: bigint, base: number): number[] {
  assertBase(base);
  if (magnitude < 0n) throw new Error("magnitude must be non-negative");
  if (magnitude === 0n) return [0];

  const digitsLsd: number[] = [];
  const b = BigInt(base);
  let mag = magnitude;
  while (mag > 0n) {
    digitsLsd.push(Number(mag % b));
    mag /= b;
  }
  return digitsLsd.reverse();
}

export function parseValue(text: string, base = 10): DigitValue {
  assertBase(base);
  let raw = text.trim().replace(/_/g, "");
  if (!raw) raw = "0";

  let sign: Sign = 1;
  if (raw[0] === "-") {
    sign = -1;
    raw = raw.slice(1);
  } else if (raw[0] === "+") {
    raw = raw.slice(1);
  }
  if (!raw || raw === ".") raw = "0";

  const parts = raw.split(".");
  if (parts.length > 2) throw new Error(`invalid numeric value ${text}`);

  const intRaw = parts[0] || "0";
  const fracRaw = parts[1] || "";
  const intDigits = [...intRaw].map((ch) => digitToValue(ch, base));
  const fracDigits = [...fracRaw].map((ch) => digitToValue(ch, base));

  return normalizeDigits({ base, sign, intDigits, fracDigits });
}

export function normalizeDigits(value: DigitValue): DigitValue {
  assertBase(value.base);
  const scale = Math.max(0, value.fracDigits.length);

  let mantissa = 0n;
  for (let i = 0; i < value.intDigits.length; i++) {
    const place = value.intDigits.length - 1 - i;
    mantissa += BigInt(value.intDigits[i]) * powBigInt(value.base, place + scale);
  }
  for (let i = 0; i < value.fracDigits.length; i++) {
    const place = -1 - i;
    mantissa += BigInt(value.fracDigits[i]) * powBigInt(value.base, place + scale);
  }
  if (value.sign === -1) mantissa = -mantissa;

  let normalized = fromScaledMantissa(mantissa, value.base, scale);
  normalized.intDigits = trimLeadingZeros(normalized.intDigits);
  normalized.fracDigits = trimTrailingZeros(normalized.fracDigits);
  if (isZeroDigits(normalized.intDigits, normalized.fracDigits)) {
    normalized = { ...normalized, sign: 1, intDigits: [0], fracDigits: [] };
  }
  return normalized;
}

export function formatValue(value: DigitValue): string {
  const normalized = normalizeDigits(value);
  const intText = normalized.intDigits.map(valueToDigit).join("") || "0";
  const fracText = normalized.fracDigits.map(valueToDigit).join("");
  const signText = normalized.sign === -1 ? "-" : "";
  return `${signText}${intText}${fracText ? `.${fracText}` : ""}`;
}

export function digitValueToRational(value: DigitValue): RationalValue {
  const normalized = normalizeDigits(value);
  const scale = normalized.fracDigits.length;
  const magnitude = digitsToBigInt([...normalized.intDigits, ...normalized.fracDigits], normalized.base);
  const signedMagnitude = normalized.sign === -1 ? -magnitude : magnitude;
  return normalizeRational(signedMagnitude, powBigInt(normalized.base, scale));
}

export function digitValueFromRational(rational: RationalValue, base: number, precision: number): DigitValue {
  assertBase(base);
  const fractionalPlaces = Math.max(0, Math.floor(precision));
  const normalized = normalizeRational(rational.numerator, rational.denominator);

  let sign: Sign = 1;
  let magnitude = normalized.numerator;
  if (magnitude < 0n) {
    sign = -1;
    magnitude = -magnitude;
  }

  const intPart = magnitude / normalized.denominator;
  let remainder = magnitude % normalized.denominator;
  const intDigits = bigIntToDigits(intPart, base);
  const fracDigits: number[] = [];
  const b = BigInt(base);

  for (let i = 0; i < fractionalPlaces && remainder !== 0n; i++) {
    remainder *= b;
    fracDigits.push(Number(remainder / normalized.denominator));
    remainder %= normalized.denominator;
  }

  return normalizeDigits({ base, sign, intDigits, fracDigits });
}

export function convertBase(value: DigitValue, targetBase: number, precision: number): DigitValue {
  return digitValueFromRational(digitValueToRational(value), targetBase, precision);
}

export function compareRational(a: RationalValue, b: RationalValue): -1 | 0 | 1 {
  const av = a.numerator * b.denominator;
  const bv = b.numerator * a.denominator;
  return av < bv ? -1 : av > bv ? 1 : 0;
}

export function decomposePlaces(value: DigitValue, precision: number): PlaceDigit[] {
  const normalized = normalizeDigits(value);
  const maxPlace = Math.max(0, normalized.intDigits.length - 1);
  const minPlace = -Math.max(0, precision);
  const out: PlaceDigit[] = [];

  for (let place = maxPlace; place >= minPlace; place--) {
    out.push({
      place,
      digit: digitAtPlace(normalized, place),
      label: `${normalized.base}^${place}`,
      kind: place >= 0 ? "integer" : "fractional",
    });
  }
  return out;
}

export function digitAtPlace(value: DigitValue, place: number): number {
  const normalized = normalizeDigits(value);
  if (place >= 0) {
    const index = normalized.intDigits.length - 1 - place;
    return index >= 0 && index < normalized.intDigits.length ? normalized.intDigits[index] : 0;
  }
  const fracIndex = -place - 1;
  return fracIndex >= 0 && fracIndex < normalized.fracDigits.length ? normalized.fracDigits[fracIndex] : 0;
}

export function applyPlaceDelta(
  value: DigitValue,
  place: number,
  digitDelta: number,
  config?: EditClamp,
): DigitValue {
  if (!Number.isInteger(place)) throw new Error("place must be an integer");
  if (!Number.isInteger(digitDelta)) throw new Error("digitDelta must be an integer");
  if (digitDelta === 0) return clamp(value, config);

  const targetScale = Math.max(value.fracDigits.length, place < 0 ? -place : 0, config?.precision ?? 0);
  const current = toScaledMantissa(value, targetScale);
  const deltaExp = place + targetScale;
  const delta = BigInt(digitDelta) * powBigInt(value.base, deltaExp);
  return clamp(fromScaledMantissa(current + delta, value.base, targetScale), config);
}

export function compare(a: DigitValue, b: DigitValue): -1 | 0 | 1 {
  if (a.base !== b.base) throw new Error("cannot compare values with different bases");
  const targetScale = Math.max(a.fracDigits.length, b.fracDigits.length);
  const av = toScaledMantissa(a, targetScale);
  const bv = toScaledMantissa(b, targetScale);
  return av < bv ? -1 : av > bv ? 1 : 0;
}

export function clamp(value: DigitValue, config?: Pick<DigitScrubConfig, "min" | "max">): DigitValue {
  let out = normalizeDigits(value);
  if (config?.min !== undefined) {
    const min = parseValue(config.min, out.base);
    if (compare(out, min) < 0) out = min;
  }
  if (config?.max !== undefined) {
    const max = parseValue(config.max, out.base);
    if (compare(out, max) > 0) out = max;
  }
  return out;
}

export function projectToNumber(value: DigitValue): number {
  const normalized = normalizeDigits(value);
  if (normalized.base === 10) return Number(formatValue(normalized));

  let total = 0;
  for (let i = 0; i < normalized.intDigits.length; i++) {
    const place = normalized.intDigits.length - 1 - i;
    total += normalized.intDigits[i] * normalized.base ** place;
  }
  for (let i = 0; i < normalized.fracDigits.length; i++) {
    total += normalized.fracDigits[i] * normalized.base ** (-1 - i);
  }
  return normalized.sign * total;
}

export function placeWeight(base: number, place: number): number {
  assertBase(base);
  return base ** place;
}
