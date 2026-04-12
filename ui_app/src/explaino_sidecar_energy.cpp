#include "explaino_sidecar_energy.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace {

constexpr double kEnergyCostGamma = 1.0;
constexpr double kRecommendationEpsilon = 1.0e-9;
constexpr double kUnavailableNumeric = std::numeric_limits<double>::quiet_NaN();

double ClampUnit(double value) {
    return std::max(0.0, std::min(1.0, value));
}

bool IsEnergyNumericType(const std::string& type) {
    return type == "float" || type == "double" || type == "int";
}

int StatusRank(SidecarEnergyLandscapeRowStatus status) {
    switch (status) {
    case SidecarEnergyLandscapeRowStatus::available:
        return 0;
    case SidecarEnergyLandscapeRowStatus::inactive:
        return 1;
    case SidecarEnergyLandscapeRowStatus::missing_cost_hint:
        return 2;
    case SidecarEnergyLandscapeRowStatus::unsupported_type:
        return 3;
    }
    return 4;
}

bool EnergyRowOrder(const SidecarEnergyLandscapeRow& left,
    const SidecarEnergyLandscapeRow& right) {
    if (StatusRank(left.status) != StatusRank(right.status)) {
        return StatusRank(left.status) < StatusRank(right.status);
    }
    if (left.status == SidecarEnergyLandscapeRowStatus::available) {
        if (left.energy != right.energy) {
            return left.energy > right.energy;
        }
        if (left.effective_information_gain != right.effective_information_gain) {
            return left.effective_information_gain > right.effective_information_gain;
        }
    }
    if (left.label != right.label) {
        return left.label < right.label;
    }
    return left.path < right.path;
}

bool PeakEnergyOrder(const SidecarEnergyLandscapeRow& left,
    const SidecarEnergyLandscapeRow& right) {
    if (left.energy != right.energy) {
        return left.energy > right.energy;
    }
    if (left.effective_information_gain != right.effective_information_gain) {
        return left.effective_information_gain > right.effective_information_gain;
    }
    if (left.label != right.label) {
        return left.label < right.label;
    }
    return left.path < right.path;
}

void PopulateRowFields(
    const SidecarBudgetRow& budgetRow,
    const SidecarLensProjectionRow& lensRow,
    SidecarEnergyLandscapeRow* outRow) {
    outRow->label = budgetRow.label;
    outRow->path = budgetRow.path;
    outRow->type = budgetRow.type;
    outRow->guidance = lensRow.guidance;
    outRow->current_value = lensRow.current_value;
    outRow->estimated_information_gain = budgetRow.estimated_information_gain;
    outRow->effective_information_gain = kUnavailableNumeric;
    outRow->cumulative_information_gain = budgetRow.cumulative_information_gain;
    outRow->information_gradient = lensRow.information_gradient;
    outRow->information_curvature = lensRow.information_curvature;
    outRow->posterior_uncertainty = ClampUnit(budgetRow.posterior_uncertainty);
    outRow->decode_stability = ClampUnit(budgetRow.decode_stability);
    outRow->cost_hint = kUnavailableNumeric;
    outRow->gamma = kUnavailableNumeric;
    outRow->energy = kUnavailableNumeric;
    outRow->active_min = lensRow.active_min;
    outRow->active_max = lensRow.active_max;
    outRow->active_fraction = lensRow.active_fraction;
    outRow->observation_count = budgetRow.observation_count;
}

} // namespace

bool BuildSidecarEnergyLandscape(
    const SidecarHypothesisSpace& space,
    const SidecarBudgetState& budget,
    const SidecarLensProjection& lens,
    SidecarEnergyLandscape* outLandscape,
    std::string* outError) {
    if (!outLandscape) {
        if (outError) *outError = "BuildSidecarEnergyLandscape requires outLandscape";
        return false;
    }
    if (budget.function_id != space.function_id) {
        *outLandscape = {};
        if (outError) {
            *outError = "Sidecar energy landscape budget function_id mismatch: expected " +
                space.function_id + ", got " + budget.function_id;
        }
        return false;
    }
    if (lens.function_id != space.function_id) {
        *outLandscape = {};
        if (outError) {
            *outError = "Sidecar energy landscape lens function_id mismatch: expected " +
                space.function_id + ", got " + lens.function_id;
        }
        return false;
    }
    if (lens.fractal_type_id != budget.fractal_type_id) {
        *outLandscape = {};
        if (outError) {
            *outError = "Sidecar energy landscape fractal_type mismatch: budget=" +
                budget.fractal_type_id + ", lens=" + lens.fractal_type_id;
        }
        return false;
    }

    std::unordered_map<std::string, const SidecarParamSurfaceEntry*> surfaceByPath;
    surfaceByPath.reserve(space.applicable_parameters.size());
    for (const SidecarParamSurfaceEntry& entry : space.applicable_parameters) {
        const auto [_, inserted] = surfaceByPath.emplace(entry.path, &entry);
        if (!inserted) {
            *outLandscape = {};
            if (outError) *outError = "Sidecar energy landscape saw duplicate surface path: " + entry.path;
            return false;
        }
    }

    std::unordered_map<std::string, const SidecarLensProjectionRow*> lensByPath;
    lensByPath.reserve(lens.rows.size());
    for (const SidecarLensProjectionRow& row : lens.rows) {
        const auto [_, inserted] = lensByPath.emplace(row.path, &row);
        if (!inserted) {
            *outLandscape = {};
            if (outError) *outError = "Sidecar energy landscape saw duplicate lens row path: " + row.path;
            return false;
        }
    }

    std::unordered_map<std::string, const SidecarBudgetRow*> budgetByPath;
    budgetByPath.reserve(budget.rows.size());
    for (const SidecarBudgetRow& row : budget.rows) {
        const auto [_, inserted] = budgetByPath.emplace(row.path, &row);
        if (!inserted) {
            *outLandscape = {};
            if (outError) *outError = "Sidecar energy landscape saw duplicate budget row path: " + row.path;
            return false;
        }
    }

    SidecarEnergyLandscape next;
    next.function_id = space.function_id;
    next.fractal_type_id = budget.fractal_type_id;
    double totalEnergy = 0.0;
    bool hasPeak = false;
    SidecarEnergyLandscapeRow peakRow;

    for (const SidecarBudgetRow& budgetRow : budget.rows) {
        const auto surfaceIt = surfaceByPath.find(budgetRow.path);
        if (surfaceIt == surfaceByPath.end()) {
            *outLandscape = {};
            if (outError) *outError = "Sidecar energy landscape missing surface entry for path: " + budgetRow.path;
            return false;
        }

        const auto lensIt = lensByPath.find(budgetRow.path);
        if (lensIt == lensByPath.end()) {
            *outLandscape = {};
            if (outError) *outError = "Sidecar energy landscape missing lens row for path: " + budgetRow.path;
            return false;
        }

        const SidecarParamSurfaceEntry& surface = *surfaceIt->second;
        const SidecarLensProjectionRow& lensRow = *lensIt->second;
        if (budgetRow.type != surface.type) {
            *outLandscape = {};
            if (outError) {
                *outError = "Sidecar energy landscape budget type mismatch for path: " + budgetRow.path +
                    " (surface=" + surface.type + ", budget=" + budgetRow.type + ")";
            }
            return false;
        }
        if (lensRow.type != surface.type) {
            *outLandscape = {};
            if (outError) {
                *outError = "Sidecar energy landscape lens type mismatch for path: " + budgetRow.path +
                    " (surface=" + surface.type + ", lens=" + lensRow.type + ")";
            }
            return false;
        }

        SidecarEnergyLandscapeRow row;
        PopulateRowFields(budgetRow, lensRow, &row);
        if (!IsEnergyNumericType(surface.type)) {
            row.status = SidecarEnergyLandscapeRowStatus::unsupported_type;
            row.summary = "unsupported";
            row.reason = "energy requires numeric param surface";
        } else if (lensRow.inactive) {
            row.status = SidecarEnergyLandscapeRowStatus::inactive;
            row.summary = "inactive";
            row.reason = "lens marks this param inactive at the current view";
        } else if (!surface.has_cost_hint) {
            row.status = SidecarEnergyLandscapeRowStatus::missing_cost_hint;
            row.summary = "unavailable";
            row.reason = "energy requires cost_hint metadata";
        } else {
            if (!std::isfinite(surface.cost_hint) || surface.cost_hint < 0.0) {
                *outLandscape = {};
                if (outError) *outError = "Invalid sidecar energy landscape cost_hint for path: " + budgetRow.path;
                return false;
            }
            row.status = SidecarEnergyLandscapeRowStatus::available;
            row.summary = "available";
            row.reason = "cost-annotated numeric active param";
            row.cost_hint = surface.cost_hint;
            row.gamma = kEnergyCostGamma;
            row.effective_information_gain =
                row.estimated_information_gain * row.posterior_uncertainty * row.decode_stability;
            row.energy = row.effective_information_gain - row.gamma * row.cost_hint;
            row.recommendation_eligible = row.estimated_information_gain > kRecommendationEpsilon;
            ++next.available_row_count;
            if (row.recommendation_eligible) {
                ++next.recommendation_eligible_count;
            }
            totalEnergy += row.energy;
            if (!hasPeak || PeakEnergyOrder(row, peakRow)) {
                peakRow = row;
                hasPeak = true;
            }
        }
        next.rows.push_back(std::move(row));
    }

    if (next.available_row_count > 0) {
        next.mean_energy = totalEnergy / static_cast<double>(next.available_row_count);
        next.peak_energy = peakRow.energy;
        next.peak_path = peakRow.path;
    }

    std::stable_sort(next.rows.begin(), next.rows.end(), EnergyRowOrder);
    *outLandscape = std::move(next);
    return true;
}