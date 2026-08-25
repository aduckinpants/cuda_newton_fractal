import { describe, expect, it } from "vitest";
import { controlModeForParam, hasUsableSliderRange } from "./controlMode";

describe("controlModeForParam", () => {
  it("treats broker-proven scrub-safe params as live control", () => {
    expect(
      controlModeForParam({
        adapterKind: "broker",
        connected: true,
        runtimeBound: true,
        safeToScrub: true,
      }),
    ).toBe("live_control");
  });

  it("treats broker-proven enum/text params as live control even when they are not scrub-safe", () => {
    expect(
      controlModeForParam({
        adapterKind: "broker",
        connected: true,
        runtimeBound: true,
        safeToScrub: false,
      }),
    ).toBe("live_control");
  });

  it("keeps disconnected broker params in rebuild/materialize mode", () => {
    expect(
      controlModeForParam({
        adapterKind: "broker",
        connected: false,
        runtimeBound: true,
        safeToScrub: true,
      }),
    ).toBe("rebuild_materialize");
  });
});

describe("hasUsableSliderRange", () => {
  it("accepts broker-bound numeric params with valid min/max/step", () => {
    expect(
      hasUsableSliderRange({
        type: "float",
        runtimeBound: true,
        min: "0",
        max: "1",
        step: "0.01",
      }),
    ).toBe(true);
  });

  it("falls back when the range is incomplete", () => {
    expect(
      hasUsableSliderRange({
        type: "float",
        runtimeBound: true,
        min: "0",
        max: undefined,
        step: "0.01",
      }),
    ).toBe(false);
  });

  it("falls back for unbound numeric params", () => {
    expect(
      hasUsableSliderRange({
        type: "float",
        runtimeBound: false,
        min: "0",
        max: "1",
        step: "0.01",
      }),
    ).toBe(false);
  });
});
