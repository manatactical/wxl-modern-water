// wxl-water: the WXL overlay bridge for the modern water pipeline. Only the water settings are
// exposed; the CoAVolFog fog knobs are not shown.
// Copyright (C) 2026 WarcraftXL. GPLv3.

#include "WxlOverlay.hpp"

#include "config.h"
#include "d3d9_wrap.h"
#include "hooks.h"
#include "msaa_depth.h"

#include "wxl/PluginApi.h"

#include <cstdio>

namespace
{
    const ConfigStore* g_store = nullptr;
    const WXL_Api*     g_api = nullptr;

    const char* const kLogLevelNames[] = {"Errors", "Info", "Debug"};
    const char* const kWaterQualityNames[] = {"Low: 128 waves, sky only", "Medium: 128 waves, scene refl.",
                                              "High: 256 waves, finer refl."};
    const char* const kWaterDebugViewNames[] = {"Off",        "Normals",      "Foam",   "Transmittance",
                                                "Reflection", "Liquid class", "Ripples"};

    struct PanelEdit
    {
        Config& c;
        int changed = 0;

        PanelEdit(const WXL_Api& api, Config& config) : c(config), api_(&api) {}

        void Check(const char* label, bool& value)
        {
            int on = value ? 1 : 0;
            if (api_->UiCheckbox(label, &on)) { value = on != 0; changed = 1; }
        }
        void Float(const char* label, float& value, float lo, float hi)
        { changed |= api_->UiSliderFloat(label, &value, lo, hi); }
        void Combo(const char* label, int& value, const char* const* items, int count, int first = 0)
        {
            int index = value - first;
            if (api_->UiCombo(label, &index, items, count)) { value = index + first; changed = 1; }
        }
        bool Section(const char* title) { return api_->UiCollapsingHeader(title) != 0; }

        const WXL_Api* api_;
    };
}

namespace wxl_volfog
{
    void SetPanelStore(const ConfigStore* store, const WXL_Api* api)
    {
        g_store = store;
        g_api = api;
    }
}

static void __cdecl DrawPanel(void*)
{
    if (!g_store || !g_api) return;
    const WXL_Api& api = *g_api;
    ConfigStore& store = *const_cast<ConfigStore*>(g_store);
    Config edited = store.Get();
    PanelEdit p(api, edited);

    const WaterFrameStatus water = LastWaterFrameStatus();
    char line[256];
    std::snprintf(line, sizeof(line), "Water: %s (%s)", water.drawn ? "drawing" : "not drawing",
                  water.reason ? water.reason : "no reason");
    api.UiText(line);

    const MultisamplingStatus ms = CurrentMultisamplingStatus();
    std::snprintf(line, sizeof(line), "Antialiasing: %s", ms.method && *ms.method ? ms.method : "off");
    api.UiText(line);
    api.UiSeparator();

    if (p.Section("Water"))
    {
        p.Check("Modern water", edited.water);
        p.Combo("Water quality", edited.waterQuality, kWaterQualityNames, 3, 1);
        p.Float("Waves", edited.waterWaves, 0.0f, 2.0f);
        p.Float("Wind", edited.waterWind, 0.5f, 10.0f);
        p.Float("Foam", edited.waterFoam, 0.0f, 2.0f);
        p.Float("Reflections", edited.waterReflections, 0.0f, 2.0f);
        p.Float("Sun highlight", edited.waterSpecular, 0.0f, 4.0f);
        p.Float("Clarity", edited.waterClarity, 0.25f, 4.0f);
        p.Float("Zone colours", edited.waterZoneColors, 0.0f, 1.0f);
        p.Float("Ripples", edited.waterRipples, 0.0f, 2.0f);
        p.Check("Client splashes", edited.waterClientSplashes);
        p.Combo("Water view", edited.waterDebugView, kWaterDebugViewNames, 7);
    }
    if (p.Section("Antialiasing"))
        p.Check("Keep the game's multisampling", edited.multisampling);
    if (p.Section("Debug"))
        p.Combo("Log level", edited.logLevel, kLogLevelNames, 3);

    if (p.changed)
        store.Apply(edited);

    api.UiSeparator();
    if (api.UiButton("Save"))
        store.Save();
    api.UiSameLine();
    if (api.UiButton("Revert"))
        store.Revert();
    api.UiText(store.HasUnsavedChanges() ? "Unsaved changes" : "Matches wxl-water.ini");
}

void AttachOverlay(IDirect3DDevice9*, HWND) {}
void DetachOverlay(IDirect3DDevice9*) {}
void ReleaseOverlayDeviceObjects(IDirect3DDevice9*) {}
void DrawOverlay(IDirect3DDevice9*) {}
bool OverlayVisible() { return false; }
PanelPlacement OverlayPanelPlacement() { return PanelPlacement(); }

void RegisterVolFogPanel(const WXL_Api& api) { api.UiAddPanel("Modern Water", &DrawPanel, nullptr); }
