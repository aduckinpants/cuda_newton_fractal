#pragma once

#include "explaino_sidecar_controller.h"

bool SidecarAutoDemoControllerPoliciesMatch(
    const SidecarAutoDemoControllerPolicy& left,
    const SidecarAutoDemoControllerPolicy& right);

bool ShouldRefreshExplainoSidecarState(
    bool dirty,
    bool sidecarStateValid,
    const SidecarAutoDemoControllerPolicy* cachedPolicy,
    const SidecarAutoDemoControllerPolicy& requestedPolicy);