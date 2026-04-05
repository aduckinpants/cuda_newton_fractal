from tools.reality_toolkit.fractal_explorer.seed_sweep import build_seed_values, format_seed_label


def test_build_seed_values_inclusive_step_range() -> None:
    assert build_seed_values(seed_start=0.70, seed_stop=0.80, seed_step=0.02) == [0.70, 0.72, 0.74, 0.76, 0.78, 0.80]


def test_build_seed_values_uses_explicit_seeds() -> None:
    assert build_seed_values(explicit_seeds=[0.745, 0.755, 0.775]) == [0.745, 0.755, 0.775]


def test_build_seed_values_rejects_zero_step() -> None:
    try:
        build_seed_values(seed_start=0.70, seed_stop=0.80, seed_step=0.0)
    except ValueError as exc:
        assert "non-zero" in str(exc)
    else:
        raise AssertionError("Expected zero seed_step to raise ValueError")


def test_format_seed_label_uses_fixed_precision() -> None:
    assert format_seed_label(0.755) == "0.755000"