#pragma once

#include <array>
#include <vector>
#include "esphome/core/component.h"
#include "esphome/components/light/light_output.h"
#include "fastcon_controller.h"

namespace esphome
{
    namespace fastcon
    {
        enum class LightState
        {
            OFF,
            WARM_WHITE,
            RGB
        };

        // Structure to hold effect information
        struct LightEffect
        {
            uint8_t type{0};          // Command type: 0x48 (1 param), 0x58 (2 params), 0x88 (5 params), 0x9e (timed)
            uint8_t effect_id{0};     // Effect ID (e.g., 0x42 for fade effects, 0xc2 for flash)
            uint8_t speed{50};        // Effect speed (1-100 for normal, 200-2 for inverted flash effects)
            uint8_t params[5]{0};     // Up to 5 parameters for complex effects (Type 0x88)
            uint8_t param_count{0};   // Number of parameters used (1-5)
        };

        class FastconLight : public Component, public light::LightOutput
        {
        public:
            FastconLight(uint8_t light_id) : light_id_(light_id) {}

            void setup() override;
            light::LightTraits get_traits() override;
            void write_state(light::LightState *state) override;
            void set_controller(FastconController *controller);

        protected:
            FastconController *controller_{nullptr};
            uint8_t light_id_;
        };
    } // namespace fastcon
} // namespace esphome
