#pragma once

#include "color_pipeline_core.h"

#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace color_pipeline_graph_receipt {

inline void WriteJsonString(std::ostream& out, const std::string& value) {
    out << '"';
    for (char c : value) {
        switch (c) {
        case '\\': out << "\\\\"; break;
        case '"': out << "\\\""; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                constexpr char kHex[] = "0123456789abcdef";
                const unsigned char byte = static_cast<unsigned char>(c);
                out << "\\u00" << kHex[(byte >> 4) & 0x0f] << kHex[byte & 0x0f];
            } else {
                out << c;
            }
            break;
        }
    }
    out << '"';
}

inline const ColorPipelineParamState* FindParam(const ColorPipelineRowState& row, const char* path) {
    if (!path) {
        return nullptr;
    }
    for (const ColorPipelineParamState& param : row.parameter_values) {
        if (param.path == path) {
            return &param;
        }
    }
    return nullptr;
}

inline void WriteOptionalNumber(std::ostream& out, const ColorPipelineParamState* param) {
    if (!param || (param->type != "float" && param->type != "double" && param->type != "int")) {
        out << "null";
        return;
    }
    out << param->number_value;
}

inline void WriteOptionalEnum(std::ostream& out, const ColorPipelineParamState* param) {
    if (!param || param->type != "enum") {
        out << "null";
        return;
    }
    WriteJsonString(out, param->enum_value);
}

inline void WriteParamValue(std::ostream& out, const ColorPipelineParamState& param) {
    out << "{\"path\": ";
    WriteJsonString(out, param.path);
    out << ", \"type\": ";
    WriteJsonString(out, param.type);
    if (param.type == "bool") {
        out << ", \"bool_value\": " << (param.bool_value ? "true" : "false");
    } else if (param.type == "enum") {
        out << ", \"enum_value\": ";
        WriteJsonString(out, param.enum_value);
    } else {
        out << ", \"number_value\": " << param.number_value;
    }
    out << "}";
}

inline std::string NodeId(const std::string& laneId, std::size_t rowIndex) {
    return laneId + "." + std::to_string(rowIndex);
}

inline void WriteEdge(std::ostream& out, const std::string& from, const std::string& to, const char* kind, bool& firstEdge) {
    if (!firstEdge) {
        out << ",\n";
    }
    firstEdge = false;
    const std::string id = from + "->" + to;
    out << "    {\"id\": ";
    WriteJsonString(out, id);
    out << ", \"from\": ";
    WriteJsonString(out, from);
    out << ", \"to\": ";
    WriteJsonString(out, to);
    out << ", \"kind\": ";
    WriteJsonString(out, kind ? kind : "sequential_flow");
    out << "}";
}

inline void WriteColorPipelineGraphReceiptJson(
    std::ostream& out,
    const std::vector<ColorPipelineLaneState>& lanes,
    const std::vector<std::string>& validationMessages,
    const std::string& sourceStackKind) {
    out << "{\n";
    out << "    \"schema_id\": \"viewer.color_pipeline_graph_receipt.v1\",\n";
    out << "    \"execution_authority\": \"linear_row_stack\",\n";
    out << "    \"ui_projection\": \"linear_color_stack\",\n";
    out << "    \"source_stack_kind\": ";
    WriteJsonString(out, sourceStackKind.empty() ? "unknown" : sourceStackKind);
    out << ",\n";
    out << "    \"nodes\": [";
    bool firstNode = true;
    for (const ColorPipelineLaneState& lane : lanes) {
        for (std::size_t rowIndex = 0; rowIndex < lane.rows.size(); ++rowIndex) {
            const ColorPipelineRowState& row = lane.rows[rowIndex];
            if (!firstNode) {
                out << ',';
            }
            firstNode = false;
            const std::string nodeId = NodeId(lane.lane_id, rowIndex);
            out << "\n      {\n";
            out << "        \"id\": ";
            WriteJsonString(out, nodeId);
            out << ",\n";
            out << "        \"lane_id\": ";
            WriteJsonString(out, lane.lane_id);
            out << ",\n";
            out << "        \"row_index\": " << rowIndex << ",\n";
            out << "        \"ui_row_id\": " << row.ui_row_id << ",\n";
            out << "        \"function_id\": ";
            WriteJsonString(out, row.function_id);
            out << ",\n";
            out << "        \"enabled\": " << (row.enabled ? "true" : "false") << ",\n";
            out << "        \"active_execution\": " << (row.enabled ? "true" : "false") << ",\n";
            out << "        \"fail_closed_reason\": ";
            if (row.enabled) {
                out << "null";
            } else {
                WriteJsonString(out, "row disabled");
            }
            out << ",\n";
            out << "        \"blend_weight\": ";
            WriteOptionalNumber(out, FindParam(row, "signal.blend_weight"));
            out << ",\n";
            out << "        \"sdf_applicator\": ";
            WriteOptionalEnum(out, FindParam(row, "signal.sdf_gate"));
            out << ",\n";
            out << "        \"sdf_gate_width_px\": ";
            WriteOptionalNumber(out, FindParam(row, "signal.sdf_gate_width_px"));
            out << ",\n";
            out << "        \"sdf_field_downsample\": ";
            WriteOptionalEnum(out, FindParam(row, "signal.sdf_field_downsample"));
            out << ",\n";
            out << "        \"root_pattern_ref\": ";
            WriteOptionalEnum(out, FindParam(row, "signal.root_pattern_ref"));
            out << ",\n";
            out << "        \"params\": [";
            for (std::size_t paramIndex = 0; paramIndex < row.parameter_values.size(); ++paramIndex) {
                if (paramIndex > 0) {
                    out << ", ";
                }
                WriteParamValue(out, row.parameter_values[paramIndex]);
            }
            out << "]\n";
            out << "      }";
        }
    }
    if (!firstNode) {
        out << '\n';
    }
    out << "    ],\n";
    out << "    \"edges\": [";
    bool firstEdge = true;
    for (const ColorPipelineLaneState& lane : lanes) {
        std::string previousActive;
        for (std::size_t rowIndex = 0; rowIndex < lane.rows.size(); ++rowIndex) {
            if (!lane.rows[rowIndex].enabled) {
                continue;
            }
            const std::string current = NodeId(lane.lane_id, rowIndex);
            if (!previousActive.empty()) {
                WriteEdge(out, previousActive, current, "lane_sequential", firstEdge);
            }
            previousActive = current;
        }
    }
    std::vector<std::string> laneExitNodes;
    std::vector<std::string> laneEntryNodes;
    for (const ColorPipelineLaneState& lane : lanes) {
        std::string firstActive;
        std::string lastActive;
        for (std::size_t rowIndex = 0; rowIndex < lane.rows.size(); ++rowIndex) {
            if (!lane.rows[rowIndex].enabled) {
                continue;
            }
            const std::string current = NodeId(lane.lane_id, rowIndex);
            if (firstActive.empty()) {
                firstActive = current;
            }
            lastActive = current;
        }
        if (!firstActive.empty()) {
            laneEntryNodes.push_back(firstActive);
            laneExitNodes.push_back(lastActive);
        }
    }
    for (std::size_t index = 1; index < laneEntryNodes.size(); ++index) {
        WriteEdge(out, laneExitNodes[index - 1], laneEntryNodes[index], "lane_projection", firstEdge);
    }
    if (!firstEdge) {
        out << '\n';
    }
    out << "    ],\n";
    out << "    \"unsupported_routes\": [";
    for (std::size_t index = 0; index < validationMessages.size(); ++index) {
        if (index > 0) {
            out << ',';
        }
        out << "\n      {\"id\": ";
        WriteJsonString(out, std::string("validation.") + std::to_string(index));
        out << ", \"reason\": ";
        WriteJsonString(out, validationMessages[index]);
        out << "}";
    }
    if (!validationMessages.empty()) {
        out << '\n';
    }
    out << "    ]\n";
    out << "  }";
}

inline std::string BuildColorPipelineGraphReceiptJson(
    const std::vector<ColorPipelineLaneState>& lanes,
    const std::vector<std::string>& validationMessages,
    const std::string& sourceStackKind) {
    std::ostringstream out;
    WriteColorPipelineGraphReceiptJson(out, lanes, validationMessages, sourceStackKind);
    return out.str();
}

} // namespace color_pipeline_graph_receipt
