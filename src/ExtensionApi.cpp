// wxl-water: WXL extension entry points for the modern water pipeline alone. The shared CoAVolFog
// engine/D3D9 layer is carried verbatim so the module is self-contained; this seam loads only the
// water data and installs only the water hooks.
// Copyright (C) 2026 WarcraftXL. GPLv3.

#include "config.h"
#include "d3d9_wrap.h"
#include "engine.h"
#include "hooks.h"
#include "log.h"
#include "water_data.h"

#include "WxlOverlay.hpp"

#include "wxl/PluginApi.h"

#include <windows.h>

#include <cstdarg>
#include <cstdio>
#include <string>

namespace
{
    const WXL_Api* g_api = nullptr;

    std::string ModuleDirectory(HMODULE module)
    {
        char path[MAX_PATH] = {};
        DWORD n = GetModuleFileNameA(module, path, MAX_PATH);
        std::string dir(path, n);
        size_t slash = dir.find_last_of("\\/");
        return slash == std::string::npos ? std::string() : dir.substr(0, slash + 1);
    }

    HMODULE ThisModule()
    {
        HMODULE module = nullptr;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCSTR>(&ThisModule), &module);
        return module;
    }

    void LogF(const char* fmt, ...)
    {
        if (!g_api || !g_api->Log) return;
        va_list ap;
        va_start(ap, fmt);
        char line[1024];
        std::vsnprintf(line, sizeof(line), fmt, ap);
        va_end(ap);
        g_api->Log(WXL_LOG_INFO, "wxl-water", "%s", line);
    }

    void Attach()
    {
        const std::string dir = ModuleDirectory(ThisModule());
        LogOpen((dir + "wxl-water.log").c_str());

        GlobalConfig().Load(dir + "wxl-water.ini");
        const Config& cfg = GlobalConfig().Get();
        LogF("wxl-water loaded from %s", dir.c_str());

        if (g_api)
        {
            wxl_volfog::SetPanelStore(&GlobalConfig(), g_api);
            RegisterVolFogPanel(*g_api);
        }

        if (!engine::IsSupportedClient())
        {
            LogF("host is not the 3.3.5a (12340) client; engine hooks skipped");
            return;
        }
        if (!cfg.enable)
        {
            LogF("Enable=0; the client runs unmodified");
            return;
        }
        if (!cfg.hooks)
        {
            LogF("EngineHooks=0; the client runs unmodified");
            return;
        }
        const bool engineHooks = InstallEngineHooks();
        AllowFogOnNewDevices(engineHooks);
        if (engineHooks && InstallWaterHooks())
        {
            if (!GlobalWaterData().Load(dir + "waterdata.bin"))
                GlobalWaterData().Load(dir + "..\\..\\waterdata.bin");
        }
    }
}

extern "C" __declspec(dllexport) const WXL_PluginInfo* __cdecl WXL_Query()
{
    static const WXL_PluginInfo info = {
        sizeof(WXL_PluginInfo),
        WXL_API_VERSION,
        "wxl-water",
        1,
        WXL_CLIENT_BUILD,
    };
    return &info;
}

extern "C" __declspec(dllexport) int __cdecl WXL_Load(const WXL_Api* api)
{
    if (!api || api->apiVersion != WXL_API_VERSION) return 0;
    g_api = api;
    Attach();
    if (api->Log) api->Log(WXL_LOG_INFO, "wxl-water", "modern water ready");
    return 1;
}
