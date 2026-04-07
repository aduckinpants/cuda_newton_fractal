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
            {"newton", "Newton"},
            {"nova", "Nova"},
            {"mandelbrot", "Mandelbrot"},
            {"julia", "Julia"},
            {"burning_ship", "Burning Ship"},
            {"multibrot", "Multibrot"},
            {"phoenix", "Phoenix"},
            {"explaino", "Explaino"},
            {"explaino_y", "Explaino Y"},
            {"explaino_fp", "Explaino FP"},
            {"explaino_nova", "Explaino Nova"},
            {"explaino_halley", "Explaino Halley"},
            {"explaino_dual", "Explaino DualSeed"},
            {"explaino_mult", "Explaino Multiplicity"},
            {"explaino_phoenix", "Explaino Phoenix"},
            {"explaino_transcendental", "Explaino Transcendental"},
            {"explaino_inertial", "Explaino Inertial"},
            {"explaino_julia", "Explaino Julia"},
            {"explaino_rational", "Explaino Rational"},
            {"multicorn", "Multicorn"},
            {"halley", "Halley"},
            {"collatz", "Collatz"},
            {"explaino_collatz", "Explaino Collatz"},
            {"mcmullen", "McMullen"},
            {"lambda", "Lambda"},
            {"explaino_lambda", "Explaino Lambda"},
            {"explaino_rational_escape", "Explaino Rational Escape"},
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
        c.def.v = 2048.0;
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
        c.def.v = 1536.0;
        render.controls.push_back(std::move(c));
    }
    s.panels.push_back(std::move(view));
    s.panels.push_back(std::move(fractal));
    s.panels.push_back(std::move(render));
    return s;
}
