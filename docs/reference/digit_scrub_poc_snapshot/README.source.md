# Digit Scrub POC

Standalone Web/TypeScript proof-of-concept for editing one numeric parameter as place-value digits with a focused residual scrubber.

## Commands

```powershell
npm install
npm run dev
npm run dev:iis
npm run test
npm run build
```

`npm run dev:iis` opens the browser-facing app through IIS Express at `http://localhost:8095`. The script also starts the local Vite API/WebSocket proxy on `http://127.0.0.1:5173`, because the broker bridge endpoints are Node middleware and IIS Express only serves the built static UI.

## Salticid Metadata Target

```json
{
  "widget": "digit_scrub",
  "base": 10,
  "precision": 12,
  "scrub_profile": "cubic",
  "safe_to_scrub": true,
  "patch_mode": "runtime_preview_then_apply"
}
```

The POC supports integer bases `2` through `36`. It keeps an exact rational value behind the finite digit view so changing display base is not a destructive edit, then projects to JavaScript `number` only for runtime-style preview.
