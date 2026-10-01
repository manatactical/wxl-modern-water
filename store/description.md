# Modern Water

Bring the water of modern World of Warcraft to the **3.3.5a (build 12340)** client. Lakes, rivers,
seas and indoor pools come alive with waves, foam, ripples and reflections, while magma and slime
keep their original look. Everything is tuned from the in-game WarcraftXL overlay under
**Modern Water**.

## What you get

- **Moving water.** A GPU wave simulation gives every water surface real motion, with foam on the
  crests, sun and moon highlights, and shore foam where the water meets the land.
- **Your zone's colours.** The water picks up each zone's own day and night water colours, so it
  still looks like Azeroth and not a generic ocean.
- **Ripples and wakes.** Players, creatures, pets and mounts leave wakes, rings and splashes as they
  move through or jump into water.
- **Reflections and depth.** The surface reflects the sky and the world around it and shows what is
  below the surface, with clear, depth-aware absorption and refraction.
- **Full control.** Every effect has a slider in the WarcraftXL overlay, so you can make the water
  calm or rough, clear or murky, plain or full of reflections.

## Requirements

- World of Warcraft **3.3.5a (build 12340)**.
- The **WarcraftXL** framework installed and working.
- A graphics driver that supports the water shaders; NVIDIA, AMD and Intel cards are all fine.

## Installing

1. Install the module through the **WarcraftXL Hub**, or unzip this package into
   `<your client>\Extensions\wxl-modern-water\`.
2. Start the game. The water is on by default.
3. Press **Ctrl+F7** in game to open the WarcraftXL overlay and change any setting live. Changes are
   saved back to `wxl-modern-water.ini`, which also lists every option in plain language.

The package contains:

| File | What it is |
|---|---|
| `wxl-modern-water.dll` | the module itself |
| `wxl-modern-water.ini` | your saved settings and the full option reference |
| `waterdata.bin` | the wave and colour data for the water |

## Important: pick one water and fog module

**Modern Water conflicts with the Volumetric Fog module (`wxl-vol-fog`).** Both replace the game's
graphics device, so they cannot run at the same time — load only one of them. If both are installed,
the game will refuse to load them together and tell you in the log.

If you want fog as well, use the separate **Volumetric Fog** module on its own, or wait for the
combined experience; do not enable both.

## If the water does not appear

- Make sure `waterdata.bin` sits next to the DLL in the extension folder.
- Check `wxl-modern-water.log` in that same folder for a reason. The overlay also shows the current
  status of the water under **Modern Water**.
- If another extension replaces the graphics device (such as Volumetric Fog), remove it first.

## Credits and license

Modern Water is built on the water rendering work of **CoAVolFog** by **jealous-sound**
(<https://github.com/jealous-sound/coa-vfog>) and is distributed under the **GNU GPL v3**. See
`LICENSE` in the package for the full text.
