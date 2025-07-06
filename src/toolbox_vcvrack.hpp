#pragma once
#include "plugin.hpp"

namespace Toolbox
{
    class GateTriggerReceiver
    {
    private:
        float prevVoltage{};
        bool gate{};
        bool trigger{};

    public:
        bool isGateActive() const { return gate; }
        bool isTriggerActive() const { return trigger; }

        void initialize()
        {
            prevVoltage = 0;
            gate = false;
            trigger = false;
        }

        void update(float voltage)
        {
            trigger = false;
            if (prevVoltage < 1 && voltage >= 1)
            {
                trigger = !gate;
                gate = true;
            }
            else if (prevVoltage >= 0.1 && voltage < 0.1)
            {
                gate = false;
            }
            prevVoltage = voltage;
        }

        bool updateGate(float voltage)
        {
            update(voltage);
            return gate;
        }

        bool updateTrigger(float voltage)
        {
            update(voltage);
            return trigger;
        }
    };

    template <typename integer_t>
    inline bool OneShotCountdown(integer_t& counter)
    {
        return (counter > 0) && (0 == --counter);
    }

    // My custom overlay class is very similar to SvgPanel,
    // only without a border drawn around it.
    // It provides a transparent overlay on top of my main SvgPanel,
    // and enables hiding/showing layers at will.
    struct SvgOverlay : Widget
    {
        FramebufferWidget* fb;
        SvgWidget *sw;
        std::shared_ptr<window::Svg> svg;

        explicit SvgOverlay(std::shared_ptr<window::Svg> _svg)
        {
            fb = new FramebufferWidget;
            addChild(fb);

            sw = new SvgWidget;
            fb->addChild(sw);

            svg = _svg;
            sw->setSvg(_svg);

            box.size = fb->box.size = sw->box.size;
        }

        static SvgOverlay* Load(std::string relativeFileName)
        {
            std::string filename = asset::plugin(pluginInstance, relativeFileName);
            std::shared_ptr<Svg> svg = Svg::load(filename);
            return new SvgOverlay(svg);
        }

        void step() override
        {
            fb->oversample = (APP->window->pixelRatio < 2.0f) ? 2.0f : 1.0f;
            Widget::step();
        }
    };

    using button_base_t = VCVLightBezel<GrayModuleLightWidget>;

    struct ToolboxButton : button_base_t
    {
    };

    struct ToolboxModule : Module
    {
    };

    struct ToolboxWidget : ModuleWidget
    {
    };
}
