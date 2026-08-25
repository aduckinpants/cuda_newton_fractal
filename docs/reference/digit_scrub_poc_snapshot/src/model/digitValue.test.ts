import { describe, expect, it } from "vitest";
import {
  applyPlaceDelta,
  clamp,
  compare,
  compareRational,
  convertBase,
  decomposePlaces,
  digitValueFromRational,
  digitValueToRational,
  formatValue,
  normalizeDigits,
  parseValue,
  projectToNumber,
} from "./digitValue";

describe("digit value model", () => {
  it("normalizes zero, sign, leading zeros, and trailing fractional zeros", () => {
    expect(formatValue(parseValue("-000.0000"))).toBe("0");
    expect(formatValue(parseValue("00012.3400"))).toBe("12.34");
    expect(formatValue(parseValue("-00012.3400"))).toBe("-12.34");
  });

  it("normalizes carry from out-of-range digit arrays", () => {
    const value = normalizeDigits({
      base: 10,
      sign: 1,
      intDigits: [9],
      fracDigits: [9, 10],
    });
    expect(formatValue(value)).toBe("10");
  });

  it("increments and decrements exact places", () => {
    expect(formatValue(applyPlaceDelta(parseValue("1.234"), 0, 2))).toBe("3.234");
    expect(formatValue(applyPlaceDelta(parseValue("1.234"), -3, 5))).toBe("1.239");
  });

  it("borrows through fractional digits", () => {
    expect(formatValue(applyPlaceDelta(parseValue("10.000"), -3, -1, { precision: 3 }))).toBe("9.999");
  });

  it("carries from 9.999 to 10.000 through the thousandths place", () => {
    expect(formatValue(applyPlaceDelta(parseValue("9.999"), -3, 1, { precision: 3 }))).toBe("10");
  });

  it("compares and clamps exact decimal strings", () => {
    expect(compare(parseValue("1.20"), parseValue("1.2"))).toBe(0);
    expect(formatValue(clamp(parseValue("12.5"), { min: "0", max: "10" }))).toBe("10");
    expect(formatValue(clamp(parseValue("-12.5"), { min: "-3", max: "10" }))).toBe("-3");
  });

  it("decomposes visible places as a flat place-indexed list", () => {
    const places = decomposePlaces(parseValue("12.3"), 2);
    expect(places.map((p) => [p.place, p.digit])).toEqual([
      [1, 1],
      [0, 2],
      [-1, 3],
      [-2, 0],
    ]);
  });

  it("supports base 2 and base 16 in the model", () => {
    expect(formatValue(applyPlaceDelta(parseValue("1111", 2), 0, 1))).toBe("10000");
    expect(formatValue(applyPlaceDelta(parseValue("F.F", 16), -1, 1, { precision: 1 }))).toBe("10");
  });

  it("converts finite values across arbitrary integer bases", () => {
    const hex = convertBase(parseValue("10.5"), 16, 6);
    expect(formatValue(hex)).toBe("A.8");
    expect(formatValue(convertBase(hex, 10, 6))).toBe("10.5");

    const binary = convertBase(parseValue("10.5"), 2, 6);
    expect(formatValue(binary)).toBe("1010.1");
    expect(formatValue(convertBase(binary, 10, 6))).toBe("10.5");
  });

  it("keeps a rational source stable when a display base cannot terminate", () => {
    const exact = digitValueToRational(parseValue("9.999"));
    const hexView = digitValueFromRational(exact, 16, 6);

    expect(formatValue(hexView)).toBe("9.FFBE76");
    expect(compareRational(exact, digitValueToRational(hexView))).toBe(1);
    expect(formatValue(digitValueFromRational(exact, 10, 6))).toBe("9.999");
  });

  it("keeps exact text stable beyond JavaScript number precision", () => {
    const exact = "123456789012345678901234567890.12345678901234567891";
    const value = parseValue(exact);
    expect(formatValue(value)).toBe(exact);
    expect(Number.isFinite(projectToNumber(value))).toBe(true);
    expect(String(projectToNumber(value))).not.toBe(exact);
  });
});
