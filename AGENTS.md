# AGENTS.md

This workspace is the **wxl-modern-water** module (listed as **Modern Water**): a single WarcraftXL
extension that adds modern water shading to the World of Warcraft 3.3.5a **(build 12340)** client.
It is the water half of the former combined `wxl-vol-fog` module — GPU FFT waves, foam, sun
highlights, player/creature ripples, scene reflections and depth-aware absorption, tuned from the
WarcraftXL overlay. Fog lives in the separate `wxl-vol-fog` module.

The framework that loads this module is **WarcraftXL**, whose source now lives at:

```
C:\Users\Strix\Documents\Github Projects\WXL-BUIDLER\wxl-build\wxl-core
```

Read that core's `README.md` for the full framework description; the notes below are how to work in
*this* tree.

## Layout

```
src/                 the CoAVolFog water sources, the shared engine/D3D9 layer, and the WXL seam
shaders/             the HLSL passes the water renderer compiles and includes
data/                waterdata.bin (zone colours / wave spectra)
tools/               the Forever water data converter and helper scripts
cmake/               shader list cmake (WaterShaders.cmake, WaterFftShaders.cmake)
module.cmake         fxc shader build + config/data deployment (picked up by the core's extension loop)
wxl.json             extension manifest (id wxl-modern-water, entry wxl-modern-water.dll, conflicts wxl-vol-fog)
wxl-modern-water.ini default settings, also the players' documentation
store/               store listing description
```

`module.cmake` was inherited from `wxl-vol-fog`, so its comments and `VOLFOG_*` variable names still
refer to the old combined module; functionally it compiles the HLSL passes with `fxc` into generated
headers and deploys `wxl-modern-water.ini` + `data/waterdata.bin` beside the DLL.

Everything targets **32-bit (Win32)** — the client is a 32-bit process. Sources are **C++20** with a
static CRT.

## Workflow rules

- **Always build the module DLL into this folder** (`wxl-modern-water.dll` next to this file). Do not
  leave the build output only under the core's `build/`.
- **Before replacing an existing DLL, back it up** by renaming/creating a `.bak` copy first
  (`wxl-modern-water.dll` → `wxl-modern-water.dll.bak`).
- The game must be **closed** while deploying — the client locks the loaded DLL.
- This module is **mutually exclusive** with `wxl-vol-fog` (`wxl.json` declares the conflict):
  each installs its own D3D9 device wrapper and engine call-site hooks, so only one may be loaded.
- Vendored code under `wxl-build/wxl-core/deps/` and `vendor/` is third-party: don't edit it.
- Go through the SDK (`include/wxl/`, `wxl::game`, `wxl::events`) and do not `#include "offsets/..."`
  directly — that boundary is enforced by the core's SDK check.

## Building the core

The core lives at `C:\Users\Strix\Documents\Github Projects\WXL-BUIDLER\wxl-build\wxl-core`.
Requirements: CMake ≥ 3.25 (its `CMakeLists.txt` needs ≥ 3.20), a Win32 C++ toolchain (Visual Studio
2022 recommended), the Windows SDK (for `fxc.exe`), and a legally-obtained 3.3.5a (12340) client.

The supported path is the core's `build.ps1`, which configures `-A Win32`, builds **Release**, and
deploys into the client:

```powershell
$core = "C:\Users\Strix\Documents\Github Projects\WXL-BUIDLER\wxl-build\wxl-core"
cd $core
.\build.ps1 -ClientPath "D:\Path\To\Client"   # first run: configures + builds + deploys
.\build.ps1                                    # later runs reuse the cached client path
.\build.ps1 -Clean                             # wipe build\ and rebuild from scratch
.\build.ps1 -AutoPatch                         # also run wxl-patcher.exe on the client's Wow.exe
```

`build.ps1` builds every extension target at once; use the focused per-target path below when working
on this module.

## Building this module

The core **auto-discovers** every folder under `wxl-build/wxl-core/extensions/<name>/`: one shared
library per folder, built from its `*.cpp`, with no per-module `CMakeLists.txt` required. An
extension defines `WXL_EXTENSION`, exports the two ABI entry points (`WXL_Query` / `WXL_Load`, see
`include/wxl/PluginApi.h`), and subclasses `wxl::ext::EventScript` for events. Optional sibling
`module.cmake` (extra include dirs / link libs / shader builds) is picked up automatically.

Build this module by staging it into the core's `extensions/` folder and building the named target.
Use the module name **`wxl-modern-water`**: the core derives both the CMake target and the output
DLL from the staged folder name, so this produces `wxl-modern-water.dll` (the former `wxl-water`
name is gone):

```powershell
$core = "C:\Users\Strix\Documents\Github Projects\WXL-BUIDLER\wxl-build\wxl-core"
$repo = "C:\Users\Strix\Documents\Github Projects\WXL\wxl-modern-water"
$name = "wxl-modern-water"
$dst  = "$core\extensions\$name"
New-Item -ItemType Directory -Force -Path $dst | Out-Null
Copy-Item "$repo\*" -Destination $dst -Recurse -Force

cmake -S $core -B "$core\build" -A Win32
cmake --build "$core\build" --config Release --target $name --parallel

# copy wxl-modern-water.dll back into the module folder, backing up any existing DLL first
if (Test-Path "$repo\$name.dll") { Copy-Item "$repo\$name.dll" "$repo\$name.dll.bak" -Force }
Copy-Item "$core\build\Release\$name.dll" "$repo\$name.dll" -Force
```

`module.cmake` needs `fxc.exe` from the Windows SDK; set `-DVOLFOG_FXC=<path>` on the configure line
if it is not found. When `CLIENT_PATH` is set it also deploys `wxl-modern-water.ini` and `data/waterdata.bin`
to `<client>\Extensions\wxl-modern-water\` (the deploy path follows the staged folder name).

## Configuring

The core reads `WarcraftXL.cfg` next to `Wow.exe` (template:
`<core>\docs\WarcraftXL.cfg.example`) for logging, rendering and storage knobs shared by
every extension. This module's own settings live in `wxl-modern-water.ini` (deployed to
`<client>\Extensions\wxl-modern-water\`) and can also be tuned live from the WarcraftXL overlay under
**Modern Water**.

## Packaging and release

The release asset is a **single zip** named `wxl-modern-water.zip`. The WarcraftXL Hub installer
fetches exactly one asset and only understands a bare `.dll` or a zip it can extract, and this module
ships an `.ini` and an 8 MB `waterdata.bin` beside the DLL, so a bare DLL would leave the data behind.
`wxl.json`'s `deploy.match` is `"*.zip"` and must stay that way.

**Zip contents.** Everything sits at the zip root (no sub-folder), because the Hub requires
`<id>.dll` at the root and the core reads the ini and data from the extension's own folder:

```
wxl-modern-water.zip
  wxl-modern-water.dll
  wxl-modern-water.ini
  waterdata.bin
```

Do **not** ship sources, shaders, `wxl.json`, `store/`, the `.bak` DLL or logs in the zip.

**Creating the zip.** After building the DLL into this folder (see *Building this module* above), run:

```powershell
.\tools\package.ps1            # packages the DLL + ini + data next to this file
```

`tools/package.ps1` builds `wxl-modern-water.zip` in the repo root from the current
`wxl-modern-water.dll`, `wxl-modern-water.ini` and `data\waterdata.bin`. It refuses to run if any of
them is missing, so build first. The GitHub workflow (`.github/workflows/release.yml`) performs the
same staging on CI and publishes the zip as a Release tagged `v<version>`.

## The manifest and the store listing

`wxl.json` is the machine + store manifest. When you change the module:

- Bump `extension.version` (semver). A bugfix that ships a new DLL is a **patch** bump (e.g.
  `1.1.0` → `1.1.1`); a new feature is a **minor** bump. The release tag tracks this field, so
  forgetting to bump it updates the existing release in place instead of creating a new one.
- Keep `extension.id`, `entry`, `assets`, `conflicts` and `deploy` accurate. `assets` lists the files
  the Hub deploys beside the DLL (`wxl-modern-water.ini`, `waterdata.bin`); `conflicts` must keep
  `wxl-vol-fog` while the two modules replace the same graphics device.
- The `listing` block is **for players, not developers**: `title`, `tagline`, `description`,
  `categories` and `accent` are what the Hub shows. `description` points at `store/description.md`.

`store/description.md` is the store page players read. Write it for someone who just wants to install
and use the module:

- Lead with what the player sees and feels (waves, ripples, reflections, per-zone colours), not with
  the implementation (no D3D9, HLSL, `fxc`, CMake, offsets or "CoAVolFog pipeline").
- Include a short **Requirements**, **Installing** and **If the water does not appear** section, and
  always keep the `wxl-vol-fog` conflict warning and the CoAVolFog/GPLv3 credit (required by the
  license).
- Never put build instructions, repo layout or developer notes in the store description; those belong
  in this file, `README.md` and `tools/`.
