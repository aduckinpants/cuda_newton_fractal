#pragma once

#include "imgui.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct ImGuiStackEditorHeaderSpec {
	const char* add_button_label = "Add";
	bool can_add = true;
	const char* disabled_reason = nullptr;
};

struct ImGuiStackEditorHeaderResult {
	bool add_requested = false;
};

inline ImGuiStackEditorHeaderResult RenderImGuiStackEditorHeader(const ImGuiStackEditorHeaderSpec& spec) {
	ImGuiStackEditorHeaderResult result;
	const char* addButtonLabel =
		(spec.add_button_label && spec.add_button_label[0] != '\0') ? spec.add_button_label : "Add";
	ImGui::BeginDisabled(!spec.can_add);
	result.add_requested = ImGui::Button(addButtonLabel);
	ImGui::EndDisabled();
	if (!spec.can_add && spec.disabled_reason && spec.disabled_reason[0] != '\0') {
		ImGui::SameLine();
		ImGui::TextUnformatted(spec.disabled_reason);
	}
	return result;
}

inline bool EnsureImGuiStackEditorRowId(std::uint64_t* ioRowId, std::uint64_t* ioNextRowId) {
	if (!ioRowId || !ioNextRowId) {
		return false;
	}
	if (*ioNextRowId == 0) {
		*ioNextRowId = 1;
	}
	if (*ioRowId == 0) {
		*ioRowId = *ioNextRowId;
		++(*ioNextRowId);
		return true;
	}
	if (*ioNextRowId <= *ioRowId) {
		*ioNextRowId = *ioRowId + 1;
	}
	return true;
}

struct ImGuiStackEditorRowChromeSpec {
	const char* tree_node_id = "row";
	const char* header_label = "(row)";
	std::uint64_t stable_row_id = 0;
	bool* enabled = nullptr;
	bool allow_remove = true;
	bool allow_move_up = false;
	bool allow_move_down = false;
	const char* move_up_button_label = "Up";
	const char* move_down_button_label = "Down";
	const char* remove_button_label = "Remove";
	ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_DefaultOpen;
};

struct ImGuiStackEditorRowChromeResult {
	bool changed = false;
	bool open = false;
	bool remove_requested = false;
	bool move_up_requested = false;
	bool move_down_requested = false;
};

template <typename RenderBodyFn>
inline ImGuiStackEditorRowChromeResult RenderImGuiStackEditorRowChrome(
	const ImGuiStackEditorRowChromeSpec& spec,
	RenderBodyFn&& renderBody) {
	ImGuiStackEditorRowChromeResult result;
	const char* treeNodeId =
		(spec.tree_node_id && spec.tree_node_id[0] != '\0') ? spec.tree_node_id : "row";
	const char* headerLabel =
		(spec.header_label && spec.header_label[0] != '\0') ? spec.header_label : "(row)";
	const char* moveUpButtonLabel =
		(spec.move_up_button_label && spec.move_up_button_label[0] != '\0') ? spec.move_up_button_label : "Up";
	const char* moveDownButtonLabel =
		(spec.move_down_button_label && spec.move_down_button_label[0] != '\0') ? spec.move_down_button_label : "Down";
	const char* removeButtonLabel =
		(spec.remove_button_label && spec.remove_button_label[0] != '\0') ? spec.remove_button_label : "Remove";

	std::string pushId = std::string(treeNodeId) + ":" + std::to_string(spec.stable_row_id);
	ImGui::PushID(pushId.c_str());
	result.open = ImGui::TreeNodeEx("row", spec.tree_flags, "%s", headerLabel);
	if (spec.allow_move_up) {
		ImGui::SameLine();
		if (ImGui::SmallButton(moveUpButtonLabel)) {
			result.move_up_requested = true;
			result.changed = true;
		}
	}
	if (spec.allow_move_down) {
		ImGui::SameLine();
		if (ImGui::SmallButton(moveDownButtonLabel)) {
			result.move_down_requested = true;
			result.changed = true;
		}
	}
	if (spec.allow_remove) {
		ImGui::SameLine();
		if (ImGui::SmallButton(removeButtonLabel)) {
			result.remove_requested = true;
			result.changed = true;
		}
	}
	if (spec.enabled) {
		result.changed = ImGui::Checkbox("Enabled", spec.enabled) || result.changed;
	}
	if (result.open) {
		std::forward<RenderBodyFn>(renderBody)();
		ImGui::TreePop();
	}
	ImGui::PopID();
	return result;
}

inline ImGuiStackEditorRowChromeResult RenderImGuiStackEditorRowChrome(
	const ImGuiStackEditorRowChromeSpec& spec) {
	return RenderImGuiStackEditorRowChrome(spec, []() {});
}

inline bool RenderImGuiStackEditorValidationBox(
	const char* label,
	const std::vector<std::string>& messages,
	bool errorStyle = true) {
	if (messages.empty()) {
		return false;
	}
	const ImVec4 textColor = errorStyle
		? ImVec4(1.0f, 0.62f, 0.48f, 1.0f)
		: ImVec4(0.95f, 0.83f, 0.40f, 1.0f);
	const char* heading = (label && label[0] != '\0') ? label : "Validation";
	ImGui::PushStyleColor(ImGuiCol_Text, textColor);
	ImGui::TextUnformatted(heading);
	ImGui::PopStyleColor();
	for (const std::string& message : messages) {
		ImGui::BulletText("%s", message.c_str());
	}
	return true;
}