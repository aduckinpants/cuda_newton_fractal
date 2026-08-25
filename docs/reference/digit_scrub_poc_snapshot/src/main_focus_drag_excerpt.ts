// Reference-only excerpt copied from src/main.ts lines 2962-3140 of the source working tree.

  for (const button of app.querySelectorAll<HTMLButtonElement>(".digit-cell")) {
    const paramId = String(button.dataset.paramId);
    const place = Number(button.dataset.place);
    button.addEventListener("click", () => {
      focusedPlaces = { ...focusedPlaces, [paramId]: place };
      residualUnits = { ...residualUnits, [paramId]: 0 };
      focusAfterRender = { paramId, place };
      render();
    });
    button.addEventListener("wheel", (event) => {
      const param = paramsById.get(paramId);
      if (!param) return;
      event.preventDefault();
      const nextFinePlace = nextFinerPlaceOnWheel(param, place, event.deltaY);
      if (nextFinePlace !== null && nextFinePlace !== place) {
        focusedPlaces = { ...focusedPlaces, [paramId]: nextFinePlace };
        residualUnits = { ...residualUnits, [paramId]: 0 };
        focusAfterRender = { paramId, place: nextFinePlace };
        render();
        return;
      }
      focusedPlaces = { ...focusedPlaces, [paramId]: place };
      applyDigitDelta(param, place, event.deltaY < 0 ? 1 : -1, "wheel");
      residualUnits = { ...residualUnits, [paramId]: 0 };
      render();
    });
    button.addEventListener("keydown", (event) => {
      const param = paramsById.get(paramId);
      if (!param) return;
      if (event.key === "ArrowRight" || event.key === "ArrowUp") {
        event.preventDefault();
        applyDigitDelta(param, place, event.shiftKey ? 10 : 1, "keyboard");
        focusAfterRender = { paramId, place };
        render();
      }
      if (event.key === "ArrowLeft" || event.key === "ArrowDown") {
        event.preventDefault();
        applyDigitDelta(param, place, event.shiftKey ? -10 : -1, "keyboard");
        focusAfterRender = { paramId, place };
        render();
      }
    });
    button.addEventListener("pointerdown", (event) => {
      event.preventDefault();
      focusedPlaces = { ...focusedPlaces, [paramId]: place };
      dragState = { paramId, place, pointerId: event.pointerId, startX: event.clientX, appliedUnits: 0 };
      focusAfterRender = { paramId, place };
      render();
    });
  }

  for (const input of app.querySelectorAll<HTMLInputElement>("[data-testid^='residual-rail-']")) {
    input.addEventListener("input", () => {
      const param = paramsById.get(String(input.dataset.paramId));
      if (!param) return;
      const focused = focusedPlaces[param.id] ?? 0;
      const nextUnits = Number(input.value);
      const currentUnits = residualUnits[param.id] ?? 0;
      residualUnits = { ...residualUnits, [param.id]: nextUnits };
      applyDigitDelta(param, focused - 1, nextUnits - currentUnits, "input");
      render();
    });
  }

  for (const input of app.querySelectorAll<HTMLInputElement>("[data-testid^='slider-'], [data-testid^='number-']")) {
    input.addEventListener("input", () => {
      const param = paramsById.get(String(input.dataset.paramId));
      if (!param) return;
      setDraftValue(param, input.value, "input");
      if (paramControlMode(param) === "live_control" && widgetForParam(param) === "slider_number") {
        if (!syncLiveControlUi(param)) render();
      } else {
        render();
      }
    });
  }

  for (const input of app.querySelectorAll<HTMLInputElement>("[data-testid^='checkbox-']")) {
    input.addEventListener("change", () => {
      const param = paramsById.get(String(input.dataset.paramId));
      if (!param) return;
      setDraftValue(param, input.checked, "input");
      if (paramControlMode(param) === "live_control" && !syncLiveControlUi(param)) render();
      else if (paramControlMode(param) !== "live_control") render();
    });
  }

  for (const select of app.querySelectorAll<HTMLSelectElement>("[data-testid^='select-']")) {
    select.addEventListener("change", () => {
      const param = paramsById.get(String(select.dataset.paramId));
      if (!param) return;
      setDraftValue(param, select.value, "input");
      if (paramControlMode(param) === "live_control" && !syncLiveControlUi(param)) render();
      else if (paramControlMode(param) !== "live_control") render();
    });
  }

  for (const input of app.querySelectorAll<HTMLInputElement>("[data-testid^='text-']")) {
    input.addEventListener("change", () => {
      const param = paramsById.get(String(input.dataset.paramId));
      if (!param) return;
      setDraftValue(param, input.value, "input");
      if (paramControlMode(param) === "live_control" && !syncLiveControlUi(param)) render();
      else if (paramControlMode(param) !== "live_control") render();
    });
  }

  const active = focusAfterRender
    ? app.querySelector<HTMLButtonElement>(`.digit-cell[data-param-id="${CSS.escape(focusAfterRender.paramId)}"][data-place="${focusAfterRender.place}"]`)
    : null;
  if (active) {
    focusAfterRender = null;
    active.focus({ preventScroll: true });
  }
}

window.addEventListener("pointerup", () => {
  const wasGraphPanning = graphDragState !== null;
  if (columnDragState && workbenchColumnWidths) persistWorkbenchColumnWidths(workbenchColumnWidths);
  dragState = null;
  splitterDragState = null;
  graphDragState = null;
  columnDragState = null;
  if (wasGraphPanning) render();
});

window.addEventListener("mouseup", () => {
  if (graphDragState?.pointerId !== -1) return;
  graphDragState = null;
  render();
});

window.addEventListener("pointercancel", () => {
  const wasGraphPanning = graphDragState !== null;
  dragState = null;
  splitterDragState = null;
  graphDragState = null;
  columnDragState = null;
  if (wasGraphPanning) render();
});

window.addEventListener(
  "click",
  (event) => {
    const target = event.target instanceof HTMLElement ? event.target : null;
    if (!target?.closest("[data-testid='close-probe-results']")) return;
    probeResultsDismissed = true;
    brokerProbeResults = [];
    probingBrokers = false;
    render();
  },
  { capture: true },
);

window.addEventListener("pointermove", (event) => {
  if (columnDragState && event.pointerId === columnDragState.pointerId) {
    event.preventDefault();
    const delta = event.clientX - columnDragState.startX;
    const index = columnDragState.splitterIndex;
    const next = [...columnDragState.startWidths] as [number, number, number, number];
    const leftMin = MIN_WORKBENCH_COLUMNS[index];
    const rightMin = MIN_WORKBENCH_COLUMNS[index + 1];
    const pairTotal = columnDragState.startWidths[index] + columnDragState.startWidths[index + 1];
    next[index] = Math.max(leftMin, Math.min(pairTotal - rightMin, columnDragState.startWidths[index] + delta));
    next[index + 1] = pairTotal - next[index];
    workbenchColumnWidths = clampWorkbenchColumns(next);
    persistWorkbenchColumnWidths(workbenchColumnWidths);
    render();
    return;
  }
  if (splitterDragState && event.pointerId === splitterDragState.pointerId) {
    event.preventDefault();
    setPatchPanelHeight(splitterDragState.startHeight - (event.clientY - splitterDragState.startY));
    render();
    return;
  }
  if (graphDragState && event.pointerId === graphDragState.pointerId) {
    event.preventDefault();
    graphPan = {
