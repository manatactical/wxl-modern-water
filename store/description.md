# Modern Water

GPU FFT waves, ripples and scene reflections for the World of Warcraft 3.3.5a (build 12340) client,
packaged as a WarcraftXL extension. It is the water half of the former combined volumetric-fog-and-
water module: the rendering pipeline is the CoAVolFog water code, loaded through WarcraftXL instead
of a `version.dll` proxy and tuned from the WarcraftXL overlay.

## What it does

- **Waves.** A GPU FFT simulates the water surface, with foam on crests, sun and moon highlights and
  the zone's own water colours read from `waterdata.bin`.
- **Ripples.** Players, creatures, pets and mounts leave wakes, rings and splash impulses.
- **Reflections and refraction.** The water reflects the sky and the scene and refracts what is
  behind it, with depth-aware absorption and shore foam.
- **Settings panel.** All options are exposed on the WarcraftXL overlay under **Modern Water**.

Fog is a separate module (`wxl-vol-fog`). The two conflict: each installs its own D3D9 device wrapper
and engine hooks, so load only one at a time.

## Files deployed

| File | Purpose |
|---|---|
| `wxl-modern-water.dll` | the extension |
| `wxl-modern-water.ini` | settings, also the in-game documentation |
| `waterdata.bin` | Forever water data (from `tools/convert_forever_water.py`) |

## Building

```powershell
$name = "wxl-modern-water"
$dst  = "wxl-build\wxl-core\extensions\$name"
New-Item -ItemType Directory -Force -Path $dst | Out-Null
Copy-Item "modules\$name\*" -Destination $dst -Recurse -Force

cmake -S wxl-build\wxl-core -B wxl-build\wxl-core\build -A Win32
cmake --build wxl-build\wxl-core\build --config Release --target $name --parallel
```
