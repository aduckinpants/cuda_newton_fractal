#include "ui_schema.h"

#include "enum_id_utils.h"
#include "fractal_family_rules.h"

#include <algorithm>
#include <sstream>

static std::string type_error(const char* what) {
    std::ostringstream oss;
    oss << "Schema error: " << what;
    return oss.str();
}

static bool get_string(const json_min::Value& o, const char* key, std::string& out) {
    const auto* v = o.get(key);
    if (!v || !v->is_string()) return false;
    out = v->as_string();
    return true;
}

static bool get_number(const json_min::Value& o, const char* key, double& out) {
    const auto* v = o.get(key);
    if (!v || !v->is_number()) return false;
    out = v->as_number();
    return true;
}

static bool get_bool(const json_min::Value& o, const char* key, bool& out) {
    const auto* v = o.get(key);
    if (!v || !v->is_bool()) return false;
    out = v->as_bool();
    return true;
}

static void parse_numeric_control_metadata(const json_min::Value& o, UISchemaControl* ctrl) {
    double n = 0.0;
    if (get_number(o, "ui_min", n)) { ctrl->ui_min = n; ctrl->has_ui_min = true; }
    if (get_number(o, "ui_max", n)) { ctrl->ui_max = n; ctrl->has_ui_max = true; }
    if (get_number(o, "min", n)) { ctrl->min = n; ctrl->has_min = true; }
    if (get_number(o, "max", n)) { ctrl->max = n; ctrl->has_max = true; }
    if (get_number(o, "step", n)) { ctrl->step = n; ctrl->has_step = true; }
}

static void apply_explaino_axis_registry(UISchema* schema) {
    if (!schema) return;
    for (UISchemaPanel& panel : schema->panels) {
        for (UISchemaControl& control : panel.controls) {
            const ExplainoAxisDescriptor* axis = FindExplainoAxisDescriptor(control.id);
            if (!axis) continue;

            control.has_visible_if = true;
            control.visible_if.op = "in";
            control.visible_if.path = "fractal.view.fractal_type";
            control.visible_if.value = FractalTypeId(ExplainoCanonicalFractalType());

            const char* carrierId = FractalTypeId(axis->carrier_fractal_type);
            if (carrierId && control.visible_if.value != carrierId) {
                control.visible_if.value += ",";
                control.visible_if.value += carrierId;
            }
        }
    }
}

UISchemaLoadResult LoadUISchemaFromJson(const json_min::Value& root) {
    UISchemaLoadResult r;
    if (!root.is_object()) {
        r.error = type_error("top-level must be object");
        return r;
    }

    UISchema s;
    if (!get_string(root, "schema_version", s.schema_version)) {
        r.error = type_error("missing schema_version string");
        return r;
    }
    if (!get_string(root, "namespace", s.name_space)) {
        r.error = type_error("missing namespace string");
        return r;
    }

    bool incomplete = false;
    if (get_bool(root, "incomplete", incomplete)) {
        s.incomplete = incomplete;
    }

    if (auto* notes = root.get("notes")) {
        if (!notes->is_array()) {
            r.error = type_error("notes must be array");
            return r;
        }
        for (const auto& n : notes->as_array()) {
            if (!n.is_string()) continue;
            s.notes.push_back(n.as_string());
        }
    }

    const auto* panels = root.get("panels");
    if (!panels || !panels->is_array()) {
        r.error = type_error("missing panels[]");
        return r;
    }

    for (const auto& p : panels->as_array()) {
        if (!p.is_object()) {
            r.error = type_error("panel must be object");
            return r;
        }

        UISchemaPanel panel;
        if (!get_string(p, "id", panel.id) || !get_string(p, "label", panel.label)) {
            r.error = type_error("panel missing id/label");
            return r;
        }
        double pOrder = 0.0;
        if (get_number(p, "order", pOrder)) {
            panel.order = (int)pOrder;
            panel.has_order = true;
        }

        const auto* ctrls = p.get("controls");
        if (!ctrls || !ctrls->is_array()) {
            r.error = type_error("panel missing controls[]");
            return r;
        }

        for (const auto& c : ctrls->as_array()) {
            if (!c.is_object()) {
                r.error = type_error("control must be object");
                return r;
            }

            UISchemaControl ctrl;
            if (!get_string(c, "id", ctrl.id) || !get_string(c, "type", ctrl.type) || !get_string(c, "label", ctrl.label)) {
                r.error = type_error("control missing id/type/label");
                return r;
            }

            get_string(c, "value_type", ctrl.value_type);

            parse_numeric_control_metadata(c, &ctrl);

            if (auto* logv = c.get("logarithmic")) {
                if (logv->is_bool()) ctrl.logarithmic = logv->as_bool();
            }

            if (auto* defv = c.get("default")) {
                ctrl.def = *defv;
                ctrl.has_default = true;
            }

            if (auto* hv = c.get("help")) {
                if (hv->is_string()) {
                    ctrl.help = hv->as_string();
                    ctrl.has_help = true;
                }
            }

            if (auto* ov = c.get("options")) {
                if (!ov->is_array()) {
                    r.error = type_error("options must be array");
                    return r;
                }
                for (const auto& opt : ov->as_array()) {
                    if (!opt.is_object()) continue;
                    UISchemaOption o;
                    if (!get_string(opt, "id", o.id) || !get_string(opt, "label", o.label)) continue;
                    get_string(opt, "group", o.group);
                    ctrl.options.push_back(std::move(o));
                }
            }

            if (auto* bv = c.get("binding")) {
                if (bv->is_null()) {
                    // allowed only if schema is incomplete; UI layer can ignore
                } else if (!bv->is_object()) {
                    r.error = type_error("binding must be object or null");
                    return r;
                } else {
                    UISchemaBinding b;
                    if (!get_string(*bv, "kind", b.kind) || !get_string(*bv, "path", b.path)) {
                        r.error = type_error("binding missing kind/path");
                        return r;
                    }
                    ctrl.binding = std::move(b);
                    ctrl.has_binding = true;
                }
            }

            if (auto* vv = c.get("visible_if")) {
                if (!vv->is_object()) {
                    r.error = type_error("visible_if must be object");
                    return r;
                }
                UISchemaPredicate pred;
                if (!get_string(*vv, "op", pred.op) || !get_string(*vv, "path", pred.path)) {
                    r.error = type_error("visible_if missing op/path");
                    return r;
                }
                // value is stored as string for this demo (enums are string ids)
                if (auto* pv = vv->get("value")) {
                    if (pv->is_string()) pred.value = pv->as_string();
                    else if (pv->is_number()) pred.value = std::to_string(pv->as_number());
                    else if (pv->is_bool()) pred.value = pv->as_bool() ? "true" : "false";
                }
                ctrl.visible_if = std::move(pred);
                ctrl.has_visible_if = true;
            }

            double cOrder = 0.0;
            if (get_number(c, "order", cOrder)) {
                ctrl.order = (int)cOrder;
                ctrl.has_order = true;
            }

            panel.controls.push_back(std::move(ctrl));
        }

        std::stable_sort(panel.controls.begin(), panel.controls.end(), [](const UISchemaControl& a, const UISchemaControl& b) {
            if (a.has_order != b.has_order) return a.has_order && !b.has_order;
            if (a.has_order && b.has_order && a.order != b.order) return a.order < b.order;
            return false; // preserve input order
        });

        s.panels.push_back(std::move(panel));
    }

    std::stable_sort(s.panels.begin(), s.panels.end(), [](const UISchemaPanel& a, const UISchemaPanel& b) {
        if (a.has_order != b.has_order) return a.has_order && !b.has_order;
        if (a.has_order && b.has_order && a.order != b.order) return a.order < b.order;
        return false; // preserve input order
    });

    apply_explaino_axis_registry(&s);
    r.schema = std::move(s);
    return r;
}
