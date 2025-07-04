#pragma once
#include "plugin.hpp"

namespace Toolbox
{
    using toolbox_button_t = VCVLightBezel<GrayModuleLightWidget>;

    struct ToolboxModule : Module
    {
    };

    struct ToolboxWidget : ModuleWidget
    {
    };
}
