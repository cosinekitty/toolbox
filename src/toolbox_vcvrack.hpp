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
