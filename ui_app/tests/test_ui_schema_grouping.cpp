#include "../src/ui_schema_grouping.h"

#include <cstdio>

static int g_passed = 0;
static int g_failed = 0;

static void Check(bool cond, const char* name) {
    if (cond) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

static UISchemaControl MakeGroupedControl() {
    UISchemaControl control{};
    control.id = "fractal_type";
    control.type = "enum";
    control.label = "Fractal Type";
    control.options = {
        {"explaino", "Explaino", "Common"},
        {"newton", "Newton", "Root-Finding"},
        {"spider", "Spider", "Escape-Time"},
        {"mystery", "Mystery", ""},
        {"hidden", "Hidden", ""},
    };
    return control;
}

static UISchemaControl MakeUngroupedControl() {
    UISchemaControl control{};
    control.id = "palette";
    control.type = "enum";
    control.label = "Palette";
    control.options = {
        {"classic", "Classic", ""},
        {"ember", "Ember", ""},
    };
    return control;
}

static void TestGroupedOptionsExposeOtherFallbackAndOrder() {
    const UISchemaControl control = MakeGroupedControl();
    const std::vector<std::string> groups = CollectOptionGroups(control);

    Check(HasGroupedOptions(control),
        "TestGroupedOptionsExposeOtherFallbackAndOrder_HasExplicitGroups");
    Check(groups.size() == 4u,
        "TestGroupedOptionsExposeOtherFallbackAndOrder_GroupCount");
    Check(groups.size() >= 4u && groups[0] == "Common" && groups[1] == "Root-Finding" && groups[2] == "Escape-Time" && groups[3] == "Other",
        "TestGroupedOptionsExposeOtherFallbackAndOrder_GroupOrder");
    Check(OptionGroupForId(control, "mystery") == "Other",
        "TestGroupedOptionsExposeOtherFallbackAndOrder_EmptyGroupNormalizesToOther");
    Check(OptionGroupForId(control, "missing") == "Common",
        "TestGroupedOptionsExposeOtherFallbackAndOrder_UnknownOptionFallsBackToFirstGroup");

    const std::vector<const UISchemaOption*> otherOptions = OptionsForGroup(control, "Other");
    Check(otherOptions.size() == 2u,
        "TestGroupedOptionsExposeOtherFallbackAndOrder_OtherOptionCount");
    Check(otherOptions.size() >= 2u && otherOptions[0]->id == "mystery" && otherOptions[1]->id == "hidden",
        "TestGroupedOptionsExposeOtherFallbackAndOrder_OtherOptionOrder");
}

static void TestUngroupedOptionsStayUngrouped() {
    const UISchemaControl control = MakeUngroupedControl();
    const std::vector<std::string> groups = CollectOptionGroups(control);
    const std::vector<const UISchemaOption*> visibleOptions = OptionsForGroup(control, "ignored");

    Check(!HasGroupedOptions(control),
        "TestUngroupedOptionsStayUngrouped_NoExplicitGroups");
    Check(groups.empty(),
        "TestUngroupedOptionsStayUngrouped_NoCollectedGroups");
    Check(OptionGroupForId(control, "classic").empty(),
        "TestUngroupedOptionsStayUngrouped_KnownOptionHasNoGroup");
    Check(OptionGroupForId(control, "missing").empty(),
        "TestUngroupedOptionsStayUngrouped_MissingOptionHasNoGroup");
    Check(visibleOptions.size() == control.options.size(),
        "TestUngroupedOptionsStayUngrouped_OptionsForGroupReturnsAllOptions");
}

int main() {
    TestGroupedOptionsExposeOtherFallbackAndOrder();
    TestUngroupedOptionsStayUngrouped();

    std::printf("test_ui_schema_grouping: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}