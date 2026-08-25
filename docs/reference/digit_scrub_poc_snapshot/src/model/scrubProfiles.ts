import type { ScrubModifiers, ScrubProfileName } from "./types";

type ScrubProfile = {
  name: ScrubProfileName;
  unitsFromPixels: (deltaPx: number) => number;
};

export const SCRUB_PROFILES: Record<ScrubProfileName, ScrubProfile> = {
  linear: {
    name: "linear",
    unitsFromPixels: (deltaPx) => deltaPx / 8,
  },
  fineLinear: {
    name: "fineLinear",
    unitsFromPixels: (deltaPx) => deltaPx / 28,
  },
  log: {
    name: "log",
    unitsFromPixels: (deltaPx) => Math.sign(deltaPx) * Math.log1p(Math.abs(deltaPx) / 4),
  },
  cubic: {
    name: "cubic",
    unitsFromPixels: (deltaPx) => {
      const scaled = Math.abs(deltaPx) / 16;
      return Math.sign(deltaPx) * scaled * scaled * scaled;
    },
  },
};

function modifierScale(modifiers: ScrubModifiers = {}): number {
  let scale = 1;
  if (modifiers.shiftKey) scale *= 10;
  if (modifiers.altKey) scale *= 0.1;
  if (modifiers.ctrlKey || modifiers.metaKey) scale *= 0.25;
  return scale;
}

export function scrub(
  deltaPx: number,
  placeWeight: number,
  modifiers: ScrubModifiers,
  profileName: ScrubProfileName,
): number {
  const profile = SCRUB_PROFILES[profileName];
  return profile.unitsFromPixels(deltaPx) * modifierScale(modifiers) * placeWeight;
}

export function scrubDigitDelta(
  deltaPx: number,
  modifiers: ScrubModifiers,
  profileName: ScrubProfileName,
): number {
  const scalarUnits = SCRUB_PROFILES[profileName].unitsFromPixels(deltaPx) * modifierScale(modifiers);
  return scalarUnits < 0 ? Math.ceil(scalarUnits) : Math.floor(scalarUnits);
}
