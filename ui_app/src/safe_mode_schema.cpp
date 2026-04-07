#include "safe_mode_schema.h"

UISchema BuildSafeModeSchema() {
    UISchema s;
    s.schema_version = "1";
    s.name_space = "fractal";
    UISchemaPanel view;
    view.id = "view";
    view.label = "View (Safe Mode)";
    view.order = 10;
    view.has_order = true;
    {
        UISchemaControl c;
        c.id = "fractal_type";
        c.type = "combo";
        c.label = "Fractal Type";
        c.value_type = "enum";
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.fractal_type";
        c.has_default = true;
        c.def.v = std::string("newton");
        c.options = {
            {"explaino", "Explaino", "Common"},
            {"nova", "Nova", "Common"},
            {"mandelbrot", "Mandelbrot", "Common"},
            {"julia", "Julia", "Common"},
            {"burning_ship", "Burning Ship", "Common"},
            {"phoenix", "Phoenix", "Common"},
            {"newton", "Newton", "Root-Finding"},
            {"halley", "Halley", "Root-Finding"},
            {"multibrot", "Multibrot", "Escape-Time"},
            {"spider", "Spider", "Escape-Time"},
            {"celtic_mandelbrot", "Celtic Mandelbrot", "Escape-Time"},
            {"perpendicular_burning_ship", "Perpendicular Burning Ship", "Escape-Time"},
            {"multicorn", "Multicorn", "Escape-Time"},
            {"collatz", "Collatz", "Escape-Time"},
            {"mcmullen", "McMullen", "Escape-Time"},
            {"lambda", "Lambda", "Escape-Time"},
            {"explaino_y", "Explaino Y", "Explaino"},
            {"explaino_fp", "Explaino FP", "Explaino"},
            {"explaino_nova", "Explaino Nova", "Explaino"},
            {"explaino_halley", "Explaino Halley", "Explaino"},
            {"explaino_dual", "Explaino DualSeed", "Explaino"},
            {"explaino_mult", "Explaino Multiplicity", "Explaino"},
            {"explaino_phoenix", "Explaino Phoenix", "Explaino"},
            {"explaino_transcendental", "Explaino Transcendental", "Explaino"},
            {"explaino_inertial", "Explaino Inertial", "Explaino"},
            {"explaino_julia", "Explaino Julia", "Explaino"},
            {"explaino_rational", "Explaino Rational", "Explaino"},
            {"explaino_collatz", "Explaino Collatz", "Explaino"},
            {"explaino_lambda", "Explaino Lambda", "Explaino"},
            {"explaino_rational_escape", "Explaino Rational Escape", "Explaino"},
        };
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "center_x";
        c.type = "drag_float";
        c.label = "Center X";
        c.value_type = "float";
        c.min = -2.0; c.max = 2.0; c.step = 0.001;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.center.x";
        c.has_default = true;
        c.def.v = 0.0;
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "center_y";
        c.type = "drag_float";
        c.label = "Center Y";
        c.value_type = "float";
        c.min = -2.0; c.max = 2.0; c.step = 0.001;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.center.y";
        c.has_default = true;
        c.def.v = 0.0;
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "zoom";
        c.type = "drag_float";
        c.label = "Zoom";
        c.value_type = "float";
        c.min = 1.0e-12; c.max = 1.0e12; c.step = 0.01;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.zoom";
        c.has_default = true;
        c.def.v = 1.0;
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "rotation_deg";
        c.type = "drag_float";
        c.label = "Rotation (deg)";
        c.value_type = "float";
        c.min = -180.0; c.max = 180.0; c.step = 0.1;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.rotation";
        c.has_default = true;
        c.def.v = 0.0;
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "auto_refresh";
        c.type = "checkbox";
        c.label = "Continuous Render";
        c.value_type = "bool";
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.auto_refresh";
        c.has_default = true;
        c.def.v = false;
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "render_once";
        c.type = "button";
        c.label = "Render Once";
        c.has_binding = true;
        c.binding.kind = "action";
        c.binding.path = "fractal.actions.render_once";
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "reset_view";
        c.type = "button";
        c.label = "Reset View";
        c.has_binding = true;
        c.binding.kind = "action";
        c.binding.path = "fractal.actions.reset_view";
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "reset_all";
        c.type = "button";
        c.label = "Reset All";
        c.has_binding = true;
        c.binding.kind = "action";
        c.binding.path = "fractal.actions.reset_all";
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "load_state";
        c.type = "button";
        c.label = "Load Finding State";
        c.has_binding = true;
        c.binding.kind = "action";
        c.binding.path = "fractal.actions.load_state";
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "capture_finding";
        c.type = "button";
        c.label = "Capture Finding";
        c.has_binding = true;
        c.binding.kind = "action";
        c.binding.path = "fractal.actions.capture_finding";
        view.controls.push_back(std::move(c));
    }
    UISchemaPanel fractal;
    fractal.id = "fractal";
    fractal.label = "Fractal (Safe Mode)";
    fractal.order = 20;
    fractal.has_order = true;
    {
        UISchemaControl c;
        c.id = "max_iter";
        c.type = "slider_int";
        c.label = "Max Iterations";
        c.value_type = "int";
        c.min = 1; c.max = 5000; c.step = 1;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.params.max_iter";
        c.has_default = true;
        c.def.v = 500.0;
        fractal.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "exposure";
        c.type = "slider_float";
        c.label = "Exposure";
        c.value_type = "float";
        c.min = 0.1; c.max = 5.0; c.step = 0.01;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.params.exposure";
        c.has_default = true;
        c.def.v = 1.0;
        fractal.controls.push_back(std::move(c));
    }
    UISchemaPanel render;
    render.id = "render";
    render.label = "Render (Safe Mode)";
    render.order = 30;
    render.has_order = true;
    {
        UISchemaControl c;
        c.id = "width";
        c.type = "slider_int";
        c.label = "Width";
        c.value_type = "int";
        c.min = 64; c.max = 4096; c.step = 1;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.render.resolution.x";
        c.has_default = true;
        c.def.v = 1024.0;
        render.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "height";
        c.type = "slider_int";
        c.label = "Height";
        c.value_type = "int";
        c.min = 64; c.max = 4096; c.step = 1;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.render.resolution.y";
        c.has_default = true;
        c.def.v = 768.0;
        render.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "interaction_debounce_ms";
        c.type = "slider_int";
        c.label = "Interaction Debounce (ms)";
        c.value_type = "int";
        c.min = 0; c.max = 1000; c.step = 10;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.render.interaction_debounce_ms";
        c.has_default = true;
        c.def.v = 200.0;
        render.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "preview_target_fps";
        c.type = "slider_float";
        c.label = "Preview Target FPS";
        c.value_type = "float";
        c.min = 5.0; c.max = 120.0; c.step = 1.0;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.render.preview_target_fps";
        c.has_default = true;
        c.def.v = 30.0;
        render.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "preview_min_scale";
        c.type = "slider_float";
        c.label = "Preview Min Scale";
        c.value_type = "float";
        c.min = 0.25; c.max = 1.0; c.step = 0.05;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.render.preview_min_scale";
        c.has_default = true;
        c.def.v = 0.5;
        render.controls.push_back(std::move(c));
    }
    s.panels.push_back(std::move(view));
    s.panels.push_back(std::move(fractal));
    s.panels.push_back(std::move(render));
    return s;
}
