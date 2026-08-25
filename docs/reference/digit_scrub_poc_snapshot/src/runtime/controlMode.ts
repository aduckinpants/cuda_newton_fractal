import type { ControlMode, RuntimeAdapterName, UiParam } from "../ast/types";

function numericParam(param: Pick<UiParam, "type">): boolean {
  return param.type === "int" || param.type === "float" || param.type === "double";
}

export function hasUsableSliderRange(param: Pick<UiParam, "type" | "runtimeBound" | "min" | "max" | "step">): boolean {
  if (!numericParam(param) || param.runtimeBound !== true) return false;
  const min = Number(param.min);
  const max = Number(param.max);
  const step = Number(param.step);
  if (!Number.isFinite(min) || !Number.isFinite(max) || !Number.isFinite(step)) return false;
  if (max <= min || step <= 0) return false;
  return step <= max - min;
}

export function controlModeForParam(options: {
  adapterKind: RuntimeAdapterName;
  connected: boolean;
  runtimeBound?: boolean;
  safeToScrub: boolean;
}): ControlMode {
  void options.safeToScrub;
  if (options.adapterKind === "broker" && options.connected && options.runtimeBound === true) {
    return "live_control";
  }
  return "rebuild_materialize";
}
