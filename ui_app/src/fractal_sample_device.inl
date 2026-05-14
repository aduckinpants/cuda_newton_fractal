    // Precision tier dispatch: resolved eval mode determines arithmetic width.
    bool useFP64 = (render.resolved_eval.backend == NumericBackend::float64);

    int maxIter = max(1, params.max_iter);
    float eps = fmaxf(1e-12f, params.epsilon);
    double epsD = fmax(1.0e-14, (double)params.epsilon);

    int it = 0;
    float pAbs = 0.0f;
    bool converged = false;
    bool escaped = false;

    Cx z{0.0f, 0.0f};
    Cx cConst{0.0f, 0.0f};

    FractalType ft = view.fractal_type;
    const bool isExplainoComposedVariant =
        ft == FractalType::explaino_ripple ||
        ft == FractalType::explaino_splice ||
        ft == FractalType::explaino_vortex ||
        ft == FractalType::explaino_tension;
    const bool hasExplainoComposedPerturbation =
        params.ripple_amplitude != 0.0f ||
        params.splice_offset != 0.0f ||
        params.vortex_strength != 0.0f ||
        params.tension_strength != 0.0f;
    const bool hasExplainoBalanceVoidPerturbation =
        params.balance_void != 0.0f ||
        params.symmetry_tension != 0.0f ||
        params.field_curvature != 0.0f;
    // Zero-axis Explaino variants must collapse to the baseline Explaino path exactly.
    if (isExplainoComposedVariant && !hasExplainoComposedPerturbation) {
        ft = FractalType::explaino;
    }
    if (ft == FractalType::explaino_balance_void && !hasExplainoBalanceVoidPerturbation) {
        ft = FractalType::explaino;
    }
    if (ft == FractalType::newton) {
        if (useFP64) {
            Cxd zd = coordD;
            double pAbsD = 0.0;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; k++) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                if (dAbs2 < 1e-30) break;
                Cxd step = cxd_div(P, dP);
                zd = cxd_sub(zd, step);
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = coord;
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; k++) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                if (dAbs2 < 1e-20f) break;
                Cx step = cx_div(P, dP);
                z = cx_sub(z, step);
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino || ft == FractalType::explaino_dual || ft == FractalType::explaino_mult) {
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                if (dAbs2 < 1e-30) break;
                Cxd step = cxd_div(P, dP);
                zd = cxd_sub(zd, cxd_scale(step, dampD));
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                if (dAbs2 < 1e-20f) break;
                Cx step = cx_div(P, dP);
                z = cx_sub(z, cx_scale(step, userDamp));
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino_balance_void) {
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        const bool useExplicitRoots = params.explaino_root_count > 0;
        const int polynomialRootCount = ResolvePolynomialRootCount(params.poly_kind);
        const int rootCount = useExplicitRoots ? params.explaino_root_count : polynomialRootCount;
        int bestIt_balance_void = 0;
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double bestPD_balance_void = 1.0e30;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < bestPD_balance_void) { bestPD_balance_void = pAbsD; bestIt_balance_void = it; }
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                Cxd newtonStep = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                double stepMag = sqrt(fmax(0.0, cxd_abs2(newtonStep)));
                double damp = (double)userDamp / (1.0 + 0.25 * stepMag);

                Cxd bias = {0.0, 0.0};
                if (rootCount >= 2) {
                    int idxNearest = 0;
                    int idxSecond = 1;
                    double best1 = 1.0e30;
                    double best2 = 1.0e30;
                    for (int rootIndex = 0; rootIndex < rootCount; ++rootIndex) {
                        Cx root{};
                        if (useExplicitRoots) {
                            root = {params.explaino_roots[rootIndex].x, params.explaino_roots[rootIndex].y};
                        } else {
                            root = unit_root_k(rootIndex, polynomialRootCount);
                        }
                        double dx = zd.x - (double)root.x;
                        double dy = zd.y - (double)root.y;
                        double d2 = dx * dx + dy * dy;
                        if (d2 < best1) {
                            best2 = best1;
                            idxSecond = idxNearest;
                            best1 = d2;
                            idxNearest = rootIndex;
                        } else if (d2 < best2) {
                            best2 = d2;
                            idxSecond = rootIndex;
                        }
                    }

                    Cx rootA{};
                    Cx rootB{};
                    if (useExplicitRoots) {
                        rootA = {params.explaino_roots[idxNearest].x, params.explaino_roots[idxNearest].y};
                        rootB = {params.explaino_roots[idxSecond].x, params.explaino_roots[idxSecond].y};
                    } else {
                        rootA = unit_root_k(idxNearest, polynomialRootCount);
                        rootB = unit_root_k(idxSecond, polynomialRootCount);
                    }

                    double midX = 0.5 * ((double)rootA.x + (double)rootB.x);
                    double midY = 0.5 * ((double)rootA.y + (double)rootB.y);
                    double axisX = (double)rootB.x - (double)rootA.x;
                    double axisY = (double)rootB.y - (double)rootA.y;
                    double axisLen = sqrt(axisX * axisX + axisY * axisY);
                    if (axisLen > 1e-30) {
                        double offsetX = zd.x - midX;
                        double offsetY = zd.y - midY;
                        Cxd balanceBias = {
                            (midX - zd.x) * (double)params.balance_void * 0.20,
                            (midY - zd.y) * (double)params.balance_void * 0.20};
                        double axisHatX = axisX / axisLen;
                        double axisHatY = axisY / axisLen;
                        double along = offsetX * axisHatX + offsetY * axisHatY;
                        double perpX = offsetX - along * axisHatX;
                        double perpY = offsetY - along * axisHatY;
                        Cxd symmetryBias = {
                            -perpX * (double)params.symmetry_tension * 0.25,
                            -perpY * (double)params.symmetry_tension * 0.25};
                        Cxd curvatureBias = {0.0, 0.0};
                        double radialLen = sqrt(offsetX * offsetX + offsetY * offsetY);
                        if (radialLen > 1e-30) {
                            double curvatureScale = 0.20 * (double)params.field_curvature * axisLen;
                            curvatureBias = {
                                (-offsetY / radialLen) * curvatureScale,
                                (offsetX / radialLen) * curvatureScale};
                        }
                        bias = cxd_add(balanceBias, cxd_add(symmetryBias, curvatureBias));
                    }
                }

                zd = cxd_add(cxd_sub(zd, cxd_scale(newtonStep, damp)), cxd_scale(bias, damp));
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            float bestPF_balance_void = 1.0e30f;
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < bestPF_balance_void) { bestPF_balance_void = pAbs; bestIt_balance_void = it; }
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                Cx newtonStep = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(newtonStep)));
                float damp = userDamp / (1.0f + 0.25f * stepMag);

                Cx bias = {0.0f, 0.0f};
                if (rootCount >= 2) {
                    int idxNearest = 0;
                    int idxSecond = 1;
                    float best1 = 1.0e30f;
                    float best2 = 1.0e30f;
                    for (int rootIndex = 0; rootIndex < rootCount; ++rootIndex) {
                        Cx root{};
                        if (useExplicitRoots) {
                            root = {params.explaino_roots[rootIndex].x, params.explaino_roots[rootIndex].y};
                        } else {
                            root = unit_root_k(rootIndex, polynomialRootCount);
                        }
                        float dx = z.x - root.x;
                        float dy = z.y - root.y;
                        float d2 = dx * dx + dy * dy;
                        if (d2 < best1) {
                            best2 = best1;
                            idxSecond = idxNearest;
                            best1 = d2;
                            idxNearest = rootIndex;
                        } else if (d2 < best2) {
                            best2 = d2;
                            idxSecond = rootIndex;
                        }
                    }

                    Cx rootA{};
                    Cx rootB{};
                    if (useExplicitRoots) {
                        rootA = {params.explaino_roots[idxNearest].x, params.explaino_roots[idxNearest].y};
                        rootB = {params.explaino_roots[idxSecond].x, params.explaino_roots[idxSecond].y};
                    } else {
                        rootA = unit_root_k(idxNearest, polynomialRootCount);
                        rootB = unit_root_k(idxSecond, polynomialRootCount);
                    }

                    float midX = 0.5f * (rootA.x + rootB.x);
                    float midY = 0.5f * (rootA.y + rootB.y);
                    float axisX = rootB.x - rootA.x;
                    float axisY = rootB.y - rootA.y;
                    float axisLen = sqrtf(axisX * axisX + axisY * axisY);
                    if (axisLen > 1e-20f) {
                        float offsetX = z.x - midX;
                        float offsetY = z.y - midY;
                        Cx balanceBias = {
                            (midX - z.x) * params.balance_void * 0.20f,
                            (midY - z.y) * params.balance_void * 0.20f};
                        float axisHatX = axisX / axisLen;
                        float axisHatY = axisY / axisLen;
                        float along = offsetX * axisHatX + offsetY * axisHatY;
                        float perpX = offsetX - along * axisHatX;
                        float perpY = offsetY - along * axisHatY;
                        Cx symmetryBias = {
                            -perpX * params.symmetry_tension * 0.25f,
                            -perpY * params.symmetry_tension * 0.25f};
                        Cx curvatureBias = {0.0f, 0.0f};
                        float radialLen = sqrtf(offsetX * offsetX + offsetY * offsetY);
                        if (radialLen > 1e-20f) {
                            float curvatureScale = 0.20f * params.field_curvature * axisLen;
                            curvatureBias = {
                                (-offsetY / radialLen) * curvatureScale,
                                (offsetX / radialLen) * curvatureScale};
                        }
                        bias = cx_add(balanceBias, cx_add(symmetryBias, curvatureBias));
                    }
                }

                z = cx_add(cx_sub(z, cx_scale(newtonStep, damp)), cx_scale(bias, damp));
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            it = bestIt_balance_void;
            if (polynomialRootCount > 0) {
                if (useFP64) {
                    Cxd zd = {(double)z.x, (double)z.y};
                    int idx = NearestRootIndexUnitRoots(zd, polynomialRootCount);
                    z = unit_root_k(idx, polynomialRootCount);
                } else {
                    int idx = NearestRootIndexUnitRoots(z, polynomialRootCount);
                    z = unit_root_k(idx, polynomialRootCount);
                }
            } else if (params.explaino_root_count > 0) {
                if (useFP64) {
                    Cxd zd = {(double)z.x, (double)z.y};
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                } else {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_fp) {
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                Cxd step = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                double stepMag = sqrt(fmax(0.0, cxd_abs2(step)));
                double damp = dampD / (1.0 + stepMag);
                zd = cxd_sub(zd, cxd_scale(step, damp));
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                Cx step = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(step)));
                float damp = userDamp / (1.0f + stepMag);
                z = cx_sub(z, cx_scale(step, damp));
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (useFP64) {
                Cxd zd = {(double)z.x, (double)z.y};
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(zd, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            } else {
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_y) {
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        z = explaino_warp_start(coord, seed, phase, strength);
        Cx zPrev = z;

        float bestP = 1.0e30f;
        int bestIt = 0;
        Cx bestZ = z;

        for (; it < maxIter; ++it) {
            float localPhase = phase + 0.07f * (float)it;
            Cx zW = explaino_warp_start(z, seed, localPhase, strength * 0.30f);

            Cx P, dP;

            float coeffs[5];
            #pragma unroll
            for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];

            poly_eval_real_coeffs_deg4(coeffs, zW, &P, &dP);

            pAbs = cx_abs(P);
            if (pAbs < bestP) {
                bestP = pAbs;
                bestIt = it;
                bestZ = zW;
            }
            if (pAbs < eps) {
                z = zW;
                break;
            }

            float dAbs2 = cx_abs2(dP);
            Cx step = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);

            float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(step)));
            float damp = 0.90f * userDamp / (1.0f + stepMag);
            Cx newtonW = cx_sub(zW, cx_scale(step, damp));

            float mix = 0.78f;
            Cx zNext = cx_add(cx_scale(z, 1.0f - mix), cx_scale(newtonW, mix));

            Cx vel = cx_sub(z, zPrev);
            zNext = cx_add(zNext, cx_scale(vel, 0.10f));
            zNext = cx_add(zNext, cx_scale(coord, 0.045f));

            zPrev = z;
            z = zNext;

            float r2 = cx_abs2(z);
            float k = 1.0f / sqrtf(1.0f + r2 * 0.0625f);
            z = cx_scale(z, 4.0f * k);

            if (!isfinite(z.x) || !isfinite(z.y)) {
                z = {0.0f, 0.0f};
                break;
            }
        }

        converged = (pAbs < eps);
        if (!converged) {
            z = bestZ;
            it = bestIt;
            // Snap to nearest root for meaningful basin assignment (matches
            // explaino_fp / explaino_joy pattern).
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (nRoots > 0) {
                int idx = NearestRootIndexUnitRoots(z, nRoots);
                z = unit_root_k(idx, nRoots);
                pAbs = 0.0f;
            } else if (params.explaino_root_count > 0) {
                int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                pAbs = 0.0f;
            } else {
                pAbs = bestP;
            }
            converged = true;
        }
    } else if (ft == FractalType::nova || ft == FractalType::explaino_nova) {
        // Nova (V1): z_{n+1} = z_n - alpha * f(z_n)/f'(z_n) + c
        z = {0.0f, 0.0f};
        cConst = coord;

        float alpha = params.nova_alpha;
        if (!(alpha > 0.0f) || !(alpha <= 2.0f) || !isfinite(alpha)) {
            escaped = true;
        } else if (useFP64) {
            Cxd zd = {0.0, 0.0};
            Cxd cConstD = coordD;
            double alphaD = (double)alpha;
            double pAbsD = 0.0;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; k++) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) { converged = true; break; }
                double dAbs2 = cxd_abs2(dP);
                // Nova: when derivative is zero, skip Newton step but still apply +c.
                if (dAbs2 >= 1e-30) {
                    Cxd step = cxd_div(P, dP);
                    zd = cxd_sub(zd, cxd_scale(step, alphaD));
                }
                zd = cxd_add(zd, cConstD);
                if (!isfinite(zd.x) || !isfinite(zd.y)) { escaped = true; break; }
                if (cxd_abs2(zd) > 4.0) { escaped = true; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
        } else {
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; k++) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < eps) { converged = true; break; }
                float dAbs2 = cx_abs2(dP);
                // Nova: when derivative is zero, skip Newton step but still apply +c.
                if (dAbs2 >= 1e-20f) {
                    Cx step = cx_div(P, dP);
                    z = cx_sub(z, cx_scale(step, alpha));
                }
                z = cx_add(z, cConst);
                if (!isfinite(z.x) || !isfinite(z.y)) { escaped = true; break; }
                if (cx_abs2(z) > 4.0f) { escaped = true; break; }
            }
        }
    } else if (ft == FractalType::explaino_halley) {
        // Halley's method: z_{n+1} = z - 2 f(z) f'(z) / (2 f'(z)^2 - f(z) f''(z))
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            for (; it < maxIter; ++it) {
                Cxd P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2_d(coeffs, zd, &P, &dP, &d2P);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                Cxd dp2 = cxd_mul(dP, dP);
                Cxd fd2 = cxd_mul(P, d2P);
                Cxd denom = cxd_sub(cxd_scale(dp2, 2.0), fd2);
                double denomAbs2 = cxd_abs2(denom);
                if (denomAbs2 < 1e-30) break;
                Cxd numer = cxd_scale(cxd_mul(P, dP), 2.0);
                Cxd step = cxd_div(numer, denom);
                zd = cxd_sub(zd, cxd_scale(step, dampD));
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                Cx P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2(coeffs, z, &P, &dP, &d2P);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                Cx dp2 = cx_mul(dP, dP);
                Cx fd2 = cx_mul(P, d2P);
                Cx denom = cx_sub(cx_scale(dp2, 2.0f), fd2);
                float denomAbs2 = cx_abs2(denom);
                if (denomAbs2 < 1e-20f) break;
                Cx numer = cx_scale(cx_mul(P, dP), 2.0f);
                Cx step = cx_div(numer, denom);
                z = cx_sub(z, cx_scale(step, userDamp));
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino_phoenix) {
        // Explaino-Phoenix: seeded Newton with previous-z memory term.
        // z_{n+1} = z_n - damp * P(z)/P'(z) + p * z_{n-1}
        // Uses adaptive damping and orbit pullback like standard explaino.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        int bestIt_phx = 0;
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd pConstD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double bestPD_phx = 1.0e30;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < bestPD_phx) { bestPD_phx = pAbsD; bestIt_phx = it; }
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                Cxd step = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                double stepMag = sqrt(fmax(0.0, cxd_abs2(step)));
                double damp = dampD / (1.0 + stepMag);
                Cxd zNext = cxd_add(cxd_sub(zd, cxd_scale(step, damp)), cxd_mul(pConstD, zPrevD));
                zPrevD = zd;
                zd = zNext;
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
            float bestPF_phx = 1.0e30f;
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < bestPF_phx) { bestPF_phx = pAbs; bestIt_phx = it; }
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                Cx step = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(step)));
                float damp = userDamp / (1.0f + stepMag);
                Cx zNext = cx_add(cx_sub(z, cx_scale(step, damp)), cx_mul(pConst, zPrev));
                zPrev = z;
                z = zNext;
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            it = bestIt_phx;
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (useFP64) {
                Cxd zd = {(double)z.x, (double)z.y};
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(zd, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            } else {
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_joy) {
        // Explaino-Joy: coupled root-critical Newton.
        // z_{n+1} = z_n - damp * [(1-gamma)*P/P' + gamma*P'/P''] + phoenix_p * z_{n-1}
        // Uses adaptive damping and orbit pullback like standard explaino.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float joyCoupling = params.joy_coupling;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        int bestIt_joy = 0;
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd pConstD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double gammaD = (double)joyCoupling;
            double oneMinusGammaD = 1.0 - gammaD;
            double bestPD_joy = 1.0e30;
            for (; it < maxIter; ++it) {
                Cxd P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2_d(coeffs, zd, &P, &dP, &d2P);
                pAbsD = cxd_abs(P);
                if (pAbsD < bestPD_joy) { bestPD_joy = pAbsD; bestIt_joy = it; }
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                Cxd newtonStep = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                Cxd joyStep = {0.0, 0.0};
                double d2Abs2 = cxd_abs2(d2P);
                if (d2Abs2 > 1e-30) {
                    joyStep = cxd_div(dP, d2P);
                }
                Cxd combinedStep = cxd_add(
                    cxd_scale(newtonStep, oneMinusGammaD),
                    cxd_scale(joyStep, gammaD));
                double stepMag = sqrt(fmax(0.0, cxd_abs2(combinedStep)));
                double damp = dampD / (1.0 + stepMag);
                Cxd zNext = cxd_add(
                    cxd_sub(zd, cxd_scale(combinedStep, damp)),
                    cxd_mul(pConstD, zPrevD));
                zPrevD = zd;
                zd = zNext;
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
            float oneMinusGamma = 1.0f - joyCoupling;
            float bestPF_joy = 1.0e30f;
            for (; it < maxIter; ++it) {
                Cx P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2(coeffs, z, &P, &dP, &d2P);
                pAbs = cx_abs(P);
                if (pAbs < bestPF_joy) { bestPF_joy = pAbs; bestIt_joy = it; }
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                Cx newtonStep = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                Cx joyStep = {0.0f, 0.0f};
                float d2Abs2 = cx_abs2(d2P);
                if (d2Abs2 > 1e-20f) {
                    joyStep = cx_div(dP, d2P);
                }
                Cx combinedStep = cx_add(
                    cx_scale(newtonStep, oneMinusGamma),
                    cx_scale(joyStep, joyCoupling));
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(combinedStep)));
                float damp = userDamp / (1.0f + stepMag);
                Cx zNext = cx_add(
                    cx_sub(z, cx_scale(combinedStep, damp)),
                    cx_mul(pConst, zPrev));
                zPrev = z;
                z = zNext;
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            it = bestIt_joy;
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (useFP64) {
                Cxd zd = {(double)z.x, (double)z.y};
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(zd, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            } else {
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_fold) {
        // Explaino-Fold: Burning-Ship-style abs-value folding applied to Newton step.
        // step_folded = (|Re(P/P')|, |Im(P/P')|)
        // combined = (1-alpha)*step_newton + alpha*step_folded
        // z_{n+1} = z - damp * combined + mu * z_{n-1}
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float foldAlpha = params.fold_coupling;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        int bestIt_fold = 0;
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd pConstD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double alphaD = (double)foldAlpha;
            double oneMinusAlphaD = 1.0 - alphaD;
            double bestPD_fold = 1.0e30;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < bestPD_fold) { bestPD_fold = pAbsD; bestIt_fold = it; }
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                Cxd newtonStep = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                Cxd foldedStep = {fabs(newtonStep.x), fabs(newtonStep.y)};
                Cxd combinedStep = cxd_add(
                    cxd_scale(newtonStep, oneMinusAlphaD),
                    cxd_scale(foldedStep, alphaD));
                double stepMag = sqrt(fmax(0.0, cxd_abs2(combinedStep)));
                double damp = dampD / (1.0 + stepMag);
                Cxd zNext = cxd_add(
                    cxd_sub(zd, cxd_scale(combinedStep, damp)),
                    cxd_mul(pConstD, zPrevD));
                zPrevD = zd;
                zd = zNext;
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
            float oneMinusAlpha = 1.0f - foldAlpha;
            float bestPF_fold = 1.0e30f;
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < bestPF_fold) { bestPF_fold = pAbs; bestIt_fold = it; }
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                Cx newtonStep = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                Cx foldedStep = {fabsf(newtonStep.x), fabsf(newtonStep.y)};
                Cx combinedStep = cx_add(
                    cx_scale(newtonStep, oneMinusAlpha),
                    cx_scale(foldedStep, foldAlpha));
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(combinedStep)));
                float damp = userDamp / (1.0f + stepMag);
                Cx zNext = cx_add(
                    cx_sub(z, cx_scale(combinedStep, damp)),
                    cx_mul(pConst, zPrev));
                zPrev = z;
                z = zNext;
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            it = bestIt_fold;
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (useFP64) {
                Cxd zd = {(double)z.x, (double)z.y};
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(zd, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            } else {
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_bell) {
        // Explaino-Bell: Measurement-reaction decomposition of Newton step.
        // Project step onto P-phase direction (measurement/bulk) and orthogonal (reaction/spin).
        // Attenuate measurement channel by beta; reaction channel passes through.
        // combined = (1-beta)*s_parallel + s_perp = s - beta*s_parallel
        // z_{n+1} = z - damp * combined + mu * z_{n-1}
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float bellBeta = params.bell_coupling;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        int bestIt_bell = 0;
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd pConstD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double betaD = (double)bellBeta;
            double bestPD_bell = 1.0e30;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < bestPD_bell) { bestPD_bell = pAbsD; bestIt_bell = it; }
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                Cxd newtonStep = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                // Decompose step by P-phase: parallel = projection onto P direction
                double pMag = fmax(1e-30, pAbsD);
                Cxd pHat = {P.x / pMag, P.y / pMag};
                // dot(step, pHat) = Re(step * conj(pHat))
                double dotPar = newtonStep.x * pHat.x + newtonStep.y * pHat.y;
                Cxd sParallel = {dotPar * pHat.x, dotPar * pHat.y};
                // combined = step - beta * sParallel (attenuate measurement channel)
                Cxd combinedStep = {newtonStep.x - betaD * sParallel.x,
                                    newtonStep.y - betaD * sParallel.y};
                double stepMag = sqrt(fmax(0.0, cxd_abs2(combinedStep)));
                double damp = dampD / (1.0 + stepMag);
                Cxd zNext = cxd_add(
                    cxd_sub(zd, cxd_scale(combinedStep, damp)),
                    cxd_mul(pConstD, zPrevD));
                zPrevD = zd;
                zd = zNext;
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
            float bestPF_bell = 1.0e30f;
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < bestPF_bell) { bestPF_bell = pAbs; bestIt_bell = it; }
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                Cx newtonStep = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                // Decompose step by P-phase
                float pMag = fmaxf(1e-20f, pAbs);
                Cx pHat = {P.x / pMag, P.y / pMag};
                float dotPar = newtonStep.x * pHat.x + newtonStep.y * pHat.y;
                Cx sParallel = {dotPar * pHat.x, dotPar * pHat.y};
                Cx combinedStep = {newtonStep.x - bellBeta * sParallel.x,
                                   newtonStep.y - bellBeta * sParallel.y};
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(combinedStep)));
                float damp = userDamp / (1.0f + stepMag);
                Cx zNext = cx_add(
                    cx_sub(z, cx_scale(combinedStep, damp)),
                    cx_mul(pConst, zPrev));
                zPrev = z;
                z = zNext;
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            it = bestIt_bell;
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (useFP64) {
                Cxd zd = {(double)z.x, (double)z.y};
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(zd, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            } else {
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_ripple ||
               ft == FractalType::explaino_splice ||
               ft == FractalType::explaino_vortex ||
               ft == FractalType::explaino_tension) {
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float rippleA = params.ripple_amplitude;
        float V = params.vortex_strength;
        float T = params.tension_strength;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        int nRootsForPull = params.explaino_root_count;
        int bestIt_composed = 0;
        const bool useSplice = params.splice_offset != 0.0f;
        const float kTwoPI = 6.2831853071795864f;
        const float kRipplePeriod = 8.0f;
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd pConstD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double ampD = (double)rippleA;
            double Vd = (double)V;
            double Td = (double)T;
            double bestPD_composed = 1.0e30;
            float coeffsA[5], coeffsB[5];
            #pragma unroll
            for (int k = 0; k < 5; ++k) {
                coeffsA[k] = params.poly_coeffs[k];
                coeffsB[k] = params.poly_coeffs_b[k];
            }
            for (; it < maxIter; ++it) {
                const float* activeCoeffs = (useSplice && (it % 2) != 0) ? coeffsB : coeffsA;
                Cxd P, dP;
                poly_eval_real_coeffs_deg4_d(activeCoeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < bestPD_composed) { bestPD_composed = pAbsD; bestIt_composed = it; }
                if (useSplice) {
                    Cxd PA, dPA;
                    poly_eval_real_coeffs_deg4_d(coeffsA, zd, &PA, &dPA);
                    double paAbs = cxd_abs(PA);
                    if (paAbs < epsD) { pAbsD = paAbs; break; }
                } else if (pAbsD < epsD) {
                    break;
                }
                double dAbs2 = cxd_abs2(dP);
                Cxd newtonStep = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                double stepMag = sqrt(fmax(0.0, cxd_abs2(newtonStep)));

                Cxd composedStep = newtonStep;
                if (Vd > 0.0 && stepMag > 1e-30) {
                    double theta = atan2(newtonStep.y, newtonStep.x);
                    double angle = Vd * theta;
                    double cosA = cos(angle);
                    double sinA = sin(angle);
                    composedStep = {newtonStep.x * cosA - newtonStep.y * sinA,
                                    newtonStep.x * sinA + newtonStep.y * cosA};
                }

                Cxd kick = {0.0, 0.0};
                if (ampD > 0.0 && stepMag > 1e-30) {
                    Cxd nHat = {-newtonStep.y / stepMag, newtonStep.x / stepMag};
                    double dpArg = atan2(dP.y, dP.x);
                    double wave = ampD * sin((double)kTwoPI * (double)it / (double)kRipplePeriod + dpArg);
                    kick = {nHat.x * wave, nHat.y * wave};
                }

                Cxd pull = {0.0, 0.0};
                if (Td > 0.0 && nRootsForPull >= 2) {
                    int idxNearest = 0;
                    double best1 = 1e30;
                    for (int r = 0; r < nRootsForPull; ++r) {
                        double dx = zd.x - (double)params.explaino_roots[r].x;
                        double dy = zd.y - (double)params.explaino_roots[r].y;
                        double d2 = dx * dx + dy * dy;
                        if (d2 < best1) { best1 = d2; idxNearest = r; }
                    }
                    double best2 = 1e30;
                    int idx2 = (idxNearest == 0) ? 1 : 0;
                    for (int r = 0; r < nRootsForPull; ++r) {
                        if (r == idxNearest) continue;
                        double dx = zd.x - (double)params.explaino_roots[r].x;
                        double dy = zd.y - (double)params.explaino_roots[r].y;
                        double d2 = dx * dx + dy * dy;
                        if (d2 < best2) { best2 = d2; idx2 = r; }
                    }
                    double fx = (double)params.explaino_roots[idx2].x - zd.x;
                    double fy = (double)params.explaino_roots[idx2].y - zd.y;
                    double dist2 = fx * fx + fy * fy;
                    if (dist2 > 1e-20) {
                        pull = {Td * fx / dist2, Td * fy / dist2};
                    }
                }

                double damp = dampD / (1.0 + stepMag);
                Cxd zNext = {
                    zd.x - composedStep.x * damp + kick.x + pull.x + pConstD.x * zPrevD.x - pConstD.y * zPrevD.y,
                    zd.y - composedStep.y * damp + kick.y + pull.y + pConstD.x * zPrevD.y + pConstD.y * zPrevD.x
                };
                zPrevD = zd;
                zd = zNext;
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
            float bestPF_composed = 1.0e30f;
            float coeffsA[5], coeffsB[5];
            #pragma unroll
            for (int k = 0; k < 5; ++k) {
                coeffsA[k] = params.poly_coeffs[k];
                coeffsB[k] = params.poly_coeffs_b[k];
            }
            for (; it < maxIter; ++it) {
                const float* activeCoeffs = (useSplice && (it % 2) != 0) ? coeffsB : coeffsA;
                Cx P, dP;
                poly_eval_real_coeffs_deg4(activeCoeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < bestPF_composed) { bestPF_composed = pAbs; bestIt_composed = it; }
                if (useSplice) {
                    Cx PA, dPA;
                    poly_eval_real_coeffs_deg4(coeffsA, z, &PA, &dPA);
                    float paAbs = cx_abs(PA);
                    if (paAbs < eps) { pAbs = paAbs; break; }
                } else if (pAbs < eps) {
                    break;
                }
                float dAbs2 = cx_abs2(dP);
                Cx newtonStep = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(newtonStep)));

                Cx composedStep = newtonStep;
                if (V > 0.0f && stepMag > 1e-20f) {
                    float theta = atan2f(newtonStep.y, newtonStep.x);
                    float angle = V * theta;
                    float cosA = cosf(angle);
                    float sinA = sinf(angle);
                    composedStep = {newtonStep.x * cosA - newtonStep.y * sinA,
                                    newtonStep.x * sinA + newtonStep.y * cosA};
                }

                Cx kick = {0.0f, 0.0f};
                if (rippleA > 0.0f && stepMag > 1e-20f) {
                    Cx nHat = {-newtonStep.y / stepMag, newtonStep.x / stepMag};
                    float dpArg = atan2f(dP.y, dP.x);
                    float wave = rippleA * sinf(kTwoPI * (float)it / kRipplePeriod + dpArg);
                    kick = {nHat.x * wave, nHat.y * wave};
                }

                Cx pull = {0.0f, 0.0f};
                if (T > 0.0f && nRootsForPull >= 2) {
                    int idxNearest = 0;
                    float best1 = 1e30f;
                    for (int r = 0; r < nRootsForPull; ++r) {
                        float dx = z.x - params.explaino_roots[r].x;
                        float dy = z.y - params.explaino_roots[r].y;
                        float d2 = dx * dx + dy * dy;
                        if (d2 < best1) { best1 = d2; idxNearest = r; }
                    }
                    float best2 = 1e30f;
                    int idx2 = (idxNearest == 0) ? 1 : 0;
                    for (int r = 0; r < nRootsForPull; ++r) {
                        if (r == idxNearest) continue;
                        float dx = z.x - params.explaino_roots[r].x;
                        float dy = z.y - params.explaino_roots[r].y;
                        float d2 = dx * dx + dy * dy;
                        if (d2 < best2) { best2 = d2; idx2 = r; }
                    }
                    float fx = params.explaino_roots[idx2].x - z.x;
                    float fy = params.explaino_roots[idx2].y - z.y;
                    float dist2 = fx * fx + fy * fy;
                    if (dist2 > 1e-20f) {
                        pull = {T * fx / dist2, T * fy / dist2};
                    }
                }

                float damp = userDamp / (1.0f + stepMag);
                Cx zNext = cx_add(
                    cx_add(cx_add(cx_sub(z, cx_scale(composedStep, damp)), kick), pull),
                    cx_mul(pConst, zPrev));
                zPrev = z;
                z = zNext;
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            it = bestIt_composed;
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (useFP64) {
                Cxd zd = {(double)z.x, (double)z.y};
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(zd, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            } else {
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_ripple) {
        // Explaino-Ripple: Standing-wave perturbation perpendicular to Newton step.
        // After computing Newton step s = P/P', adds sinusoidal kick normal to s:
        //   nHat = i*s/|s|  (90-degree rotation of step direction)
        //   kick = A * sin(2*pi*n/T + arg(P')) * nHat
        //   z_{n+1} = z - damp*s + kick + mu * z_{n-1}
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float rippleA = params.ripple_amplitude;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        int bestIt_ripple = 0;
        const float kTwoPI = 6.2831853071795864f;
        const float kRipplePeriod = 8.0f;
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd pConstD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double ampD = (double)rippleA;
            double bestPD_ripple = 1.0e30;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < bestPD_ripple) { bestPD_ripple = pAbsD; bestIt_ripple = it; }
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                Cxd newtonStep = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                double stepMag = sqrt(fmax(0.0, cxd_abs2(newtonStep)));
                // Standing-wave kick perpendicular to step direction
                Cxd kick = {0.0, 0.0};
                if (ampD > 0.0 && stepMag > 1e-30) {
                    Cxd nHat = {-newtonStep.y / stepMag, newtonStep.x / stepMag};
                    double dpArg = atan2(dP.y, dP.x);
                    double wave = ampD * sin((double)kTwoPI * (double)it / (double)kRipplePeriod + dpArg);
                    kick = {nHat.x * wave, nHat.y * wave};
                }
                double damp = dampD / (1.0 + stepMag);
                Cxd zNext = {
                    zd.x - newtonStep.x * damp + kick.x + pConstD.x * zPrevD.x - pConstD.y * zPrevD.y,
                    zd.y - newtonStep.y * damp + kick.y + pConstD.x * zPrevD.y + pConstD.y * zPrevD.x
                };
                zPrevD = zd;
                zd = zNext;
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
            float bestPF_ripple = 1.0e30f;
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < bestPF_ripple) { bestPF_ripple = pAbs; bestIt_ripple = it; }
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                Cx newtonStep = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(newtonStep)));
                // Standing-wave kick perpendicular to step direction
                Cx kick = {0.0f, 0.0f};
                if (rippleA > 0.0f && stepMag > 1e-20f) {
                    Cx nHat = {-newtonStep.y / stepMag, newtonStep.x / stepMag};
                    float dpArg = atan2f(dP.y, dP.x);
                    float wave = rippleA * sinf(kTwoPI * (float)it / kRipplePeriod + dpArg);
                    kick = {nHat.x * wave, nHat.y * wave};
                }
                float damp = userDamp / (1.0f + stepMag);
                Cx zNext = cx_add(
                    cx_add(cx_sub(z, cx_scale(newtonStep, damp)), kick),
                    cx_mul(pConst, zPrev));
                zPrev = z;
                z = zNext;
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            it = bestIt_ripple;
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (useFP64) {
                Cxd zd = {(double)z.x, (double)z.y};
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(zd, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            } else {
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_splice) {
        // Explaino-Splice: Alternating-polynomial interference.
        // Even iterations use P_A (params.poly_coeffs), odd use P_B (params.poly_coeffs_b).
        // Convergence is tested against P_A (primary polynomial).
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        int bestIt_splice = 0;
        float coeffsA[5], coeffsB[5];
        #pragma unroll
        for (int k = 0; k < 5; ++k) { coeffsA[k] = params.poly_coeffs[k]; coeffsB[k] = params.poly_coeffs_b[k]; }
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd pConstD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double bestPD_splice = 1.0e30;
            for (; it < maxIter; ++it) {
                // Pick polynomial based on even/odd iteration
                const float* activeCoeffs = (it % 2 == 0) ? coeffsA : coeffsB;
                Cxd P, dP;
                poly_eval_real_coeffs_deg4_d(activeCoeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < bestPD_splice) { bestPD_splice = pAbsD; bestIt_splice = it; }
                // Convergence check against primary polynomial P_A
                {
                    Cxd PA, dPA;
                    poly_eval_real_coeffs_deg4_d(coeffsA, zd, &PA, &dPA);
                    double paAbs = cxd_abs(PA);
                    if (paAbs < epsD) { pAbsD = paAbs; break; }
                }
                double dAbs2 = cxd_abs2(dP);
                Cxd step = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                double stepMag = sqrt(fmax(0.0, cxd_abs2(step)));
                double damp = dampD / (1.0 + stepMag);
                Cxd zNext = {
                    zd.x - step.x * damp + pConstD.x * zPrevD.x - pConstD.y * zPrevD.y,
                    zd.y - step.y * damp + pConstD.x * zPrevD.y + pConstD.y * zPrevD.x
                };
                zPrevD = zd;
                zd = zNext;
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
            float bestPF_splice = 1.0e30f;
            for (; it < maxIter; ++it) {
                const float* activeCoeffs = (it % 2 == 0) ? coeffsA : coeffsB;
                Cx P, dP;
                poly_eval_real_coeffs_deg4(activeCoeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < bestPF_splice) { bestPF_splice = pAbs; bestIt_splice = it; }
                // Convergence check against primary polynomial P_A
                {
                    Cx PA, dPA;
                    poly_eval_real_coeffs_deg4(coeffsA, z, &PA, &dPA);
                    float paAbs = cx_abs(PA);
                    if (paAbs < eps) { pAbs = paAbs; break; }
                }
                float dAbs2 = cx_abs2(dP);
                Cx step = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(step)));
                float damp = userDamp / (1.0f + stepMag);
                Cx zNext = cx_add(
                    cx_sub(z, cx_scale(step, damp)),
                    cx_mul(pConst, zPrev));
                zPrev = z;
                z = zNext;
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            it = bestIt_splice;
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (useFP64) {
                Cxd zd = {(double)z.x, (double)z.y};
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(zd, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            } else {
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_vortex) {
        // Explaino-Vortex: Self-referential spin — Newton step rotated by V * arg(step).
        //   step = P/P'
        //   theta = atan2(step.y, step.x)
        //   rotated = step * exp(i * V * theta)
        //   z_{n+1} = z - damp * rotated + mu * z_{n-1}
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float V = params.vortex_strength;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        int bestIt_vortex = 0;
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd pConstD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double Vd = (double)V;
            double bestPD_vortex = 1.0e30;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < bestPD_vortex) { bestPD_vortex = pAbsD; bestIt_vortex = it; }
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                Cxd newtonStep = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                double stepMag = sqrt(fmax(0.0, cxd_abs2(newtonStep)));
                // Self-referential rotation by V * arg(step)
                Cxd rotStep = newtonStep;
                if (Vd > 0.0 && stepMag > 1e-30) {
                    double theta = atan2(newtonStep.y, newtonStep.x);
                    double angle = Vd * theta;
                    double cosA = cos(angle);
                    double sinA = sin(angle);
                    rotStep = {newtonStep.x * cosA - newtonStep.y * sinA,
                               newtonStep.x * sinA + newtonStep.y * cosA};
                }
                double damp = dampD / (1.0 + stepMag);
                Cxd zNext = {
                    zd.x - rotStep.x * damp + pConstD.x * zPrevD.x - pConstD.y * zPrevD.y,
                    zd.y - rotStep.y * damp + pConstD.x * zPrevD.y + pConstD.y * zPrevD.x
                };
                zPrevD = zd;
                zd = zNext;
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
            float bestPF_vortex = 1.0e30f;
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < bestPF_vortex) { bestPF_vortex = pAbs; bestIt_vortex = it; }
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                Cx newtonStep = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(newtonStep)));
                // Self-referential rotation by V * arg(step)
                Cx rotStep = newtonStep;
                if (V > 0.0f && stepMag > 1e-20f) {
                    float theta = atan2f(newtonStep.y, newtonStep.x);
                    float angle = V * theta;
                    float cosA = cosf(angle);
                    float sinA = sinf(angle);
                    rotStep = {newtonStep.x * cosA - newtonStep.y * sinA,
                               newtonStep.x * sinA + newtonStep.y * cosA};
                }
                float damp = userDamp / (1.0f + stepMag);
                Cx zNext = cx_add(
                    cx_sub(z, cx_scale(rotStep, damp)),
                    cx_mul(pConst, zPrev));
                zPrev = z;
                z = zNext;
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            it = bestIt_vortex;
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (useFP64) {
                Cxd zd = {(double)z.x, (double)z.y};
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(zd, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            } else {
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_tension) {
        // Explaino-Tension: Newton step + weak pull toward second-closest root.
        //   step = P/P'
        //   r_far = second_closest_root(z)
        //   pull  = T * (r_far - z) / |r_far - z|^2
        //   z_{n+1} = z - damp * step + pull + mu * z_{n-1}
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float T = params.tension_strength;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        int nRootsForPull = params.explaino_root_count;
        int bestIt_tension = 0;
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd pConstD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double Td = (double)T;
            double bestPD_tension = 1.0e30;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < bestPD_tension) { bestPD_tension = pAbsD; bestIt_tension = it; }
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                Cxd newtonStep = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                double stepMag = sqrt(fmax(0.0, cxd_abs2(newtonStep)));
                double damp = dampD / (1.0 + stepMag);
                // Compute pull toward second-closest root
                Cxd pull = {0.0, 0.0};
                if (Td > 0.0 && nRootsForPull >= 2) {
                    // Find nearest and second-nearest root
                    double best1 = 1e30, best2 = 1e30;
                    int idx2 = 0;
                    for (int r = 0; r < nRootsForPull; ++r) {
                        double dx = zd.x - (double)params.explaino_roots[r].x;
                        double dy = zd.y - (double)params.explaino_roots[r].y;
                        double d2 = dx*dx + dy*dy;
                        if (d2 < best1) { best2 = best1; idx2 = r; best1 = d2; }
                        else if (d2 < best2) { best2 = d2; idx2 = r; }
                    }
                    // Fix: re-scan to get true second-nearest
                    int idxNearest = 0;
                    best1 = 1e30;
                    for (int r = 0; r < nRootsForPull; ++r) {
                        double dx = zd.x - (double)params.explaino_roots[r].x;
                        double dy = zd.y - (double)params.explaino_roots[r].y;
                        double d2 = dx*dx + dy*dy;
                        if (d2 < best1) { best1 = d2; idxNearest = r; }
                    }
                    best2 = 1e30; idx2 = (idxNearest == 0) ? 1 : 0;
                    for (int r = 0; r < nRootsForPull; ++r) {
                        if (r == idxNearest) continue;
                        double dx = zd.x - (double)params.explaino_roots[r].x;
                        double dy = zd.y - (double)params.explaino_roots[r].y;
                        double d2 = dx*dx + dy*dy;
                        if (d2 < best2) { best2 = d2; idx2 = r; }
                    }
                    double fx = (double)params.explaino_roots[idx2].x - zd.x;
                    double fy = (double)params.explaino_roots[idx2].y - zd.y;
                    double dist2 = fx*fx + fy*fy;
                    if (dist2 > 1e-20) {
                        pull = {Td * fx / dist2, Td * fy / dist2};
                    }
                }
                Cxd zNext = {
                    zd.x - newtonStep.x * damp + pull.x + pConstD.x * zPrevD.x - pConstD.y * zPrevD.y,
                    zd.y - newtonStep.y * damp + pull.y + pConstD.x * zPrevD.y + pConstD.y * zPrevD.x
                };
                zPrevD = zd;
                zd = zNext;
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
            float bestPF_tension = 1.0e30f;
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < bestPF_tension) { bestPF_tension = pAbs; bestIt_tension = it; }
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                Cx newtonStep = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(newtonStep)));
                float damp = userDamp / (1.0f + stepMag);
                // Compute pull toward second-closest root
                Cx pull = {0.0f, 0.0f};
                if (T > 0.0f && nRootsForPull >= 2) {
                    int idxNearest = 0;
                    float best1 = 1e30f;
                    for (int r = 0; r < nRootsForPull; ++r) {
                        float dx = z.x - params.explaino_roots[r].x;
                        float dy = z.y - params.explaino_roots[r].y;
                        float d2 = dx*dx + dy*dy;
                        if (d2 < best1) { best1 = d2; idxNearest = r; }
                    }
                    float best2 = 1e30f;
                    int idx2 = (idxNearest == 0) ? 1 : 0;
                    for (int r = 0; r < nRootsForPull; ++r) {
                        if (r == idxNearest) continue;
                        float dx = z.x - params.explaino_roots[r].x;
                        float dy = z.y - params.explaino_roots[r].y;
                        float d2 = dx*dx + dy*dy;
                        if (d2 < best2) { best2 = d2; idx2 = r; }
                    }
                    float fx = params.explaino_roots[idx2].x - z.x;
                    float fy = params.explaino_roots[idx2].y - z.y;
                    float dist2 = fx*fx + fy*fy;
                    if (dist2 > 1e-20f) {
                        pull = {T * fx / dist2, T * fy / dist2};
                    }
                }
                Cx zNext = cx_add(
                    cx_add(cx_sub(z, cx_scale(newtonStep, damp)), pull),
                    cx_mul(pConst, zPrev));
                zPrev = z;
                z = zNext;
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            it = bestIt_tension;
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (useFP64) {
                Cxd zd = {(double)z.x, (double)z.y};
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(zd, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            } else {
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_transcendental) {
        // Explaino-Transcendental: seeded Newton applied to transcendental functions.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);

        TranscendentalFunc tf = params.transcendental_func;
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            for (; it < maxIter; ++it) {
                Cxd F, dF;
                if (tf == TranscendentalFunc::f_sin) {
                    double sx = sin(zd.x), cx_ = cos(zd.x);
                    double shy = sinh(zd.y), chy = cosh(zd.y);
                    F = {sx * chy, cx_ * shy};
                    dF = {cx_ * chy, -sx * shy};
                } else if (tf == TranscendentalFunc::f_exp_minus_1) {
                    double ex = exp(zd.x);
                    double cy_ = cos(zd.y), sy_ = sin(zd.y);
                    F = {ex * cy_ - 1.0, ex * sy_};
                    dF = {ex * cy_, ex * sy_};
                } else {
                    double chx = cosh(zd.x), shx = sinh(zd.x);
                    double cy_ = cos(zd.y), sy_ = sin(zd.y);
                    F = {chx * cy_, shx * sy_};
                    dF = {shx * cy_, chx * sy_};
                }
                pAbsD = cxd_abs(F);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dF);
                if (dAbs2 < 1e-30) break;
                Cxd step = cxd_div(F, dF);
                zd = cxd_sub(zd, cxd_scale(step, dampD));
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                Cx F, dF;
                if (tf == TranscendentalFunc::f_sin) {
                    float sx = sinf(z.x), cx_ = cosf(z.x);
                    float shy = sinhf(z.y), chy = coshf(z.y);
                    F = {sx * chy, cx_ * shy};
                    dF = {cx_ * chy, -sx * shy};
                } else if (tf == TranscendentalFunc::f_exp_minus_1) {
                    float ex = expf(z.x);
                    float cy_ = cosf(z.y), sy_ = sinf(z.y);
                    F = {ex * cy_ - 1.0f, ex * sy_};
                    dF = {ex * cy_, ex * sy_};
                } else {
                    float chx = coshf(z.x), shx = sinhf(z.x);
                    float cy_ = cosf(z.y), sy_ = sinf(z.y);
                    F = {chx * cy_, shx * sy_};
                    dF = {shx * cy_, chx * sy_};
                }
                pAbs = cx_abs(F);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dF);
                if (dAbs2 < 1e-20f) break;
                Cx step = cx_div(F, dF);
                z = cx_sub(z, cx_scale(step, userDamp));
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino_inertial) {
        // Explaino-Inertial: seeded Newton with previous-step momentum.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float beta = params.momentum_beta;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd zPrev2D = zd;
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double betaD = (double)beta;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                if (dAbs2 < 1e-30) break;
                Cxd newtonStep = cxd_div(P, dP);
                Cxd momentum = cxd_sub(zPrevD, zPrev2D);
                Cxd zNext = cxd_add(cxd_sub(zd, cxd_scale(newtonStep, dampD)),
                                    cxd_scale(momentum, betaD));
                zPrev2D = zPrevD;
                zPrevD = zd;
                zd = zNext;
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx zPrev2 = z;
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                if (dAbs2 < 1e-20f) break;
                Cx newtonStep = cx_div(P, dP);
                Cx momentum = cx_sub(zPrev, zPrev2);
                Cx zNext = cx_add(cx_sub(z, cx_scale(newtonStep, userDamp)),
                                  cx_scale(momentum, beta));
                zPrev2 = zPrev;
                zPrev = z;
                z = zNext;
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino_julia) {
        // Explaino-Julia bridge: escape-time z^2+c with warp start and seeded Julia constant.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        z = explaino_warp_start(coord, seed, phase, strength);

        // Julia constant c from the first seeded root (falls back to classic if no roots).
        Cx cJ = (params.explaino_root_count > 0)
            ? Cx{params.explaino_roots[0].x, params.explaino_roots[0].y}
            : Cx{-0.7f, 0.27015f};

        for (; it < maxIter; ++it) {
            Cx z2 = cx_mul(z, z);
            z = cx_add(z2, cJ);

            if (cx_abs2(z) > 4.0f) {
                escaped = true;
                break;
            }
            if (!isfinite(z.x) || !isfinite(z.y)) {
                escaped = true;
                break;
            }
        }
    } else if (ft == FractalType::explaino_lambda) {
        // Explaino-Lambda bridge: escape-time logistic map with warp start.
        // z_{n+1} = lambda * z * (1 - z), seeded z from explaino polynomial surface.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        z = explaino_warp_start(coord, seed, phase, strength);

        Cx lambdaC{params.lambda_real, params.lambda_imag};

        for (; it < maxIter; ++it) {
            Cx oneMinusZ{1.0f - z.x, -z.y};
            z = cx_mul(lambdaC, cx_mul(z, oneMinusZ));

            if (cx_abs2(z) > 4.0f) {
                escaped = true;
                break;
            }
            if (!isfinite(z.x) || !isfinite(z.y)) {
                escaped = true;
                break;
            }
        }
    } else if (ft == FractalType::explaino_rational_escape) {
        // Explaino-Rational-Escape: escape-time iteration of Laurent polynomial P(z)/z^3.
        // z_{n+1} = P(z) / z^3 where P is the deg-4 seed polynomial.
        // Decomposes to: c0/z^3 + c1/z^2 + c2/z + c3 + c4*z (mixed positive/negative powers).
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        z = explaino_warp_start(coord, seed, phase, strength);

        float coeffs[5];
        #pragma unroll
        for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];

        for (; it < maxIter; ++it) {
            float zAbs2 = cx_abs2(z);
            if (zAbs2 < 1e-20f) {
                // At pole: treat as escaped (division by zero)
                escaped = true;
                break;
            }

            // Evaluate P(z)
            Cx P, dP;
            poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);

            // Compute z^3 = z * z * z
            Cx z2 = cx_mul(z, z);
            Cx z3 = cx_mul(z2, z);

            // z_{n+1} = P(z) / z^3
            z = cx_div(P, z3);

            if (cx_abs2(z) > 10000.0f) {
                escaped = true;
                break;
            }
            if (!isfinite(z.x) || !isfinite(z.y)) {
                escaped = true;
                break;
            }
        }
    } else if (ft == FractalType::explaino_rational) {
        // Explaino-Rational: Newton on f(z) = P(z) + cluster_radius/z.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float ratAlpha = params.explaino_cluster_radius;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double ratAlphaD = (double)ratAlpha;
            for (; it < maxIter; ++it) {
                double zAbs2 = cxd_abs2(zd);
                if (zAbs2 < 1e-30) break;
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                Cxd zInv = cxd_div({1.0, 0.0}, zd);
                Cxd F = cxd_add(P, cxd_scale(zInv, ratAlphaD));
                Cxd zInv2 = cxd_mul(zInv, zInv);
                Cxd dF = cxd_sub(dP, cxd_scale(zInv2, ratAlphaD));
                pAbsD = cxd_abs(F);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dF);
                if (dAbs2 < 1e-30) break;
                Cxd step = cxd_div(F, dF);
                zd = cxd_sub(zd, cxd_scale(step, dampD));
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                float zAbs2 = cx_abs2(z);
                if (zAbs2 < 1e-20f) break;
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                Cx zInv = cx_div({1.0f, 0.0f}, z);
                Cx F = cx_add(P, cx_scale(zInv, ratAlpha));
                Cx zInv2 = cx_mul(zInv, zInv);
                Cx dF = cx_sub(dP, cx_scale(zInv2, ratAlpha));
                pAbs = cx_abs(F);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dF);
                if (dAbs2 < 1e-20f) break;
                Cx step = cx_div(F, dF);
                z = cx_sub(z, cx_scale(step, userDamp));
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::multicorn) {
        EscapeTimeDirectState<Cx> state = InitEscapeTimeDirectState(ft, coord);
        const Cx lambdaConst{params.lambda_real, params.lambda_imag};
        const Cx phoenixP{params.phoenix_p_real, params.phoenix_p_imag};

        for (; it < maxIter; ++it) {
            StepEscapeTimeDirectState(ft, params.multibrot_power_float, params.multibrot_power, lambdaConst, phoenixP, &state);
            z = state.z;

            if (cx_abs2(state.z) > DirectEscapeTimeRadiusSquared<float>()) {
                escaped = true;
                break;
            }
            if (!isfinite(state.z.x) || !isfinite(state.z.y) || !isfinite(state.z_prev.x) || !isfinite(state.z_prev.y)) {
                escaped = true;
                break;
            }
        }
    } else if (ft == FractalType::halley) {
        // Standalone Halley's method on standard polynomials.
        if (useFP64) {
            Cxd zd = coordD;
            double pAbsD = 0.0;
            for (; it < maxIter; ++it) {
                Cxd P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2_d(coeffs, zd, &P, &dP, &d2P);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                Cxd dp2 = cxd_mul(dP, dP);
                Cxd fd2 = cxd_mul(P, d2P);
                Cxd denom = cxd_sub(cxd_scale(dp2, 2.0), fd2);
                double denomAbs2 = cxd_abs2(denom);
                if (denomAbs2 < 1e-30) break;
                Cxd numer = cxd_scale(cxd_mul(P, dP), 2.0);
                Cxd step = cxd_div(numer, denom);
                zd = cxd_sub(zd, step);
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = coord;
            for (; it < maxIter; ++it) {
                Cx P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2(coeffs, z, &P, &dP, &d2P);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                Cx dp2 = cx_mul(dP, dP);
                Cx fd2 = cx_mul(P, d2P);
                Cx denom = cx_sub(cx_scale(dp2, 2.0f), fd2);
                float denomAbs2 = cx_abs2(denom);
                if (denomAbs2 < 1e-20f) break;
                Cx numer = cx_scale(cx_mul(P, dP), 2.0f);
                Cx step = cx_div(numer, denom);
                z = cx_sub(z, step);
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (UsesSpecializedEscapeTimeFormula(ft)) {
        z = coord;
        const McMullenPresetConfig mcmullenConfig = ResolveMcMullenPresetConfig(params.mcmullen_preset);

        for (; it < maxIter; ++it) {
            if (ft == FractalType::mcmullen) {
                if (StepMcMullenEscapeState(mcmullenConfig, &z) == SpecializedEscapeStepResult::pole) {
                    escaped = true;
                    break;
                }
            } else {
                StepCollatzEscapeState(&z);
            }

            if (cx_abs2(z) > SpecializedEscapeRadiusSquared()) {
                escaped = true;
                break;
            }
            if (!isfinite(z.x) || !isfinite(z.y)) {
                escaped = true;
                break;
            }
        }
    } else if (ft == FractalType::explaino_collatz) {
        // Explaino-Collatz: Newton's method on fixed points of the Collatz map.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);

        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            for (; it < maxIter; ++it) {
                const ExplainoCollatzStepResult stepResult = StepExplainoCollatzNewton(dampD, epsD, &zd, &pAbsD);
                if (stepResult == ExplainoCollatzStepResult::converged) break;
                if (stepResult == ExplainoCollatzStepResult::degenerate) break;
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                const ExplainoCollatzStepResult stepResult = StepExplainoCollatzNewton(userDamp, eps, &z, &pAbs);
                if (stepResult == ExplainoCollatzStepResult::converged) break;
                if (stepResult == ExplainoCollatzStepResult::degenerate) break;
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else {
        // Escape-time family.
        bool canPerturb = (refOrbit != nullptr) && (refLen >= (maxIter + 1));
        bool doPerturb = canPerturb && (ft == FractalType::mandelbrot || ft == FractalType::julia);

        if (doPerturb) {
            // Perturbation deep zoom.
            // Mandelbrot: z0=0, c = refC0 + dc, delta0=0
            // Julia: z0 = refZ0 + dz0, c fixed, delta0=dz0
            Cxd dc{0.0, 0.0};
            Cxd delta{0.0, 0.0};

            if (ft == FractalType::mandelbrot) {
                dc = cxd_sub(coordD, cxd_from_double2(refC0));
                delta = {0.0, 0.0};
            } else {
                // Julia: ref orbit uses z0 reference encoded in refC0
                dc = {0.0, 0.0};
                delta = cxd_sub(coordD, cxd_from_double2(refC0));
            }

            for (; it < maxIter; ++it) {
                Cxd zref = cxd_from_double2(refOrbit[it]);
                Cxd zcur = cxd_add(zref, delta);
                if (cxd_abs2(zcur) > 4.0) {
                    escaped = true;
                    z = {(float)zcur.x, (float)zcur.y};
                    break;
                }

                // delta_{n+1} = 2*zref*delta + delta^2 + dc
                Cxd term1 = cxd_scale(cxd_mul(zref, delta), 2.0);
                Cxd term2 = cxd_mul(delta, delta);
                delta = cxd_add(cxd_add(term1, term2), dc);

                if (!isfinite(delta.x) || !isfinite(delta.y)) {
                    escaped = true;
                    break;
                }

                z = {(float)zcur.x, (float)zcur.y};
            }
        } else {
            // Standard direct escape-time iteration shared with the probe sampler.
            if (useFP64) {
                EscapeTimeDirectState<Cxd> state = InitEscapeTimeDirectState(ft, coordD);
                const double powerFloat = (double)params.multibrot_power_float;
                const Cxd lambdaConstD{(double)params.lambda_real, (double)params.lambda_imag};
                const Cxd phoenixPD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};

                for (; it < maxIter; ++it) {
                    StepEscapeTimeDirectState(ft, powerFloat, params.multibrot_power, lambdaConstD, phoenixPD, &state);
                    if (cxd_abs2(state.z) > DirectEscapeTimeRadiusSquared<double>()) {
                        escaped = true;
                        break;
                    }
                    if (!isfinite(state.z.x) || !isfinite(state.z.y) || !isfinite(state.z_prev.x) || !isfinite(state.z_prev.y)) {
                        escaped = true;
                        break;
                    }
                }
                z = {(float)state.z.x, (float)state.z.y};
            } else {
                EscapeTimeDirectState<Cx> state = InitEscapeTimeDirectState(ft, coord);
                const float powerFloat = params.multibrot_power_float;
                const Cx lambdaConst{params.lambda_real, params.lambda_imag};
                const Cx phoenixP{params.phoenix_p_real, params.phoenix_p_imag};

                for (; it < maxIter; ++it) {
                    StepEscapeTimeDirectState(ft, powerFloat, params.multibrot_power, lambdaConst, phoenixP, &state);
                    if (cx_abs2(state.z) > DirectEscapeTimeRadiusSquared<float>()) {
                        escaped = true;
                        break;
                    }
                    if (!isfinite(state.z.x) || !isfinite(state.z.y) || !isfinite(state.z_prev.x) || !isfinite(state.z_prev.y)) {
                        escaped = true;
                        break;
                    }
                }
                z = state.z;
            }
        }
    }
    FractalSampleResult result;
    result.iterations = it;
    result.final_z_x = z.x;
    result.final_z_y = z.y;
    result.residual = pAbs;
    result.converged = converged;
    result.escaped = escaped;
    return result;
