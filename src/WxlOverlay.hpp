// wxl-vol-fog: overlay/panel bridge declarations.
// Copyright (C) 2026 WarcraftXL. GPLv3.

#pragma once

#include "overlay.h"
#include "config.h"

#include "wxl/PluginApi.h"

namespace wxl_volfog
{
    /// Points the panel at the module's config store and service table.
    void SetPanelStore(const ConfigStore* store, const WXL_Api* api);
}

/// Registers the volumetric-fog settings panel on the core overlay.
void RegisterVolFogPanel(const WXL_Api& api);
