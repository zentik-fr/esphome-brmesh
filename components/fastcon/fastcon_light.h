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
            uint8_t effect_id{0};     // 0 = no effect, 1+ = effect codes (to be discovered)
            uint8_t speed{128};       // Effect speed (0-255)
            uint8_t param1{0};        // Additional parameter 1
            uint8_t param2{0};        // Additional parameter 2
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
