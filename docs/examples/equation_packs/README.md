# Generic CUDA Equation Packs

These v1 packs are AST JSON inputs for the Generic CUDA equation-pack workbench.
They lower to `GenericFunctionDesc` and execute through `generic.sample`.

They are not new live viewport fractal types, dynamic CUDA kernels, or Salticid
`sample_fn` adapters.

Run an example after publishing the runtime:

```powershell
py -3.14 tools/reality_toolkit/scripts/run_generic_equation_pack_workbench.py `
  --pack-json docs/examples/equation_packs/newton_z3_minus_1_pack.json `
  --out-dir artifacts/equation_pack_workbench/newton_z3_minus_1 `
  --backend cuda
```
