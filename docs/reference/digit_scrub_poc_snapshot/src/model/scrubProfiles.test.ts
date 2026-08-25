import { describe, expect, it } from "vitest";
import { scrub, scrubDigitDelta, SCRUB_PROFILES } from "./scrubProfiles";
import type { ScrubProfileName } from "./types";

const names = Object.keys(SCRUB_PROFILES) as ScrubProfileName[];

describe("scrub profiles", () => {
  it("are deterministic", () => {
    for (const name of names) {
      expect(scrub(42, 100, {}, name)).toBe(scrub(42, 100, {}, name));
    }
  });

  it("are monotonic for normal drag input", () => {
    for (const name of names) {
      const a = scrub(4, 1, {}, name);
      const b = scrub(20, 1, {}, name);
      const c = scrub(80, 1, {}, name);
      expect(a).toBeLessThanOrEqual(b);
      expect(b).toBeLessThanOrEqual(c);
    }
  });

  it("honors modifier scaling", () => {
    expect(Math.abs(scrub(40, 1, { shiftKey: true }, "linear"))).toBeGreaterThan(
      Math.abs(scrub(40, 1, {}, "linear")),
    );
    expect(Math.abs(scrub(40, 1, { altKey: true }, "linear"))).toBeLessThan(
      Math.abs(scrub(40, 1, {}, "linear")),
    );
  });

  it("returns integer digit deltas for exact application", () => {
    expect(scrubDigitDelta(8, {}, "linear")).toBe(1);
    expect(scrubDigitDelta(-8, {}, "linear")).toBe(-1);
    expect(scrubDigitDelta(1, {}, "fineLinear")).toBe(0);
  });
});
