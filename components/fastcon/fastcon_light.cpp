#include <algorithm>
#include "esphome/core/log.h"
#include "fastcon_light.h"
#include "fastcon_controller.h"
#include "utils.h"

namespace esphome
{
    namespace fastcon
    {
        static const char *const TAG = "fastcon.light";

        void FastconLight::setup()
        {
            if (this->controller_ == nullptr)
            {
                ESP_LOGE(TAG, "Controller not set for light %d!", this->light_id_);
                this->mark_failed();
                return;
            }
            ESP_LOGCONFIG(TAG, "Setting up Fastcon BLE light (ID: %d)...", this->light_id_);
        }

        void FastconLight::set_controller(FastconController *controller)
        {
            this->controller_ = controller;
        }

        light::LightTraits FastconLight::get_traits()
        {
            auto traits = light::LightTraits();
            traits.set_supported_color_modes({light::ColorMode::RGB, light::ColorMode::WHITE, light::ColorMode::BRIGHTNESS, light::ColorMode::COLD_WARM_WHITE});
            traits.set_min_mireds(153);
            traits.set_max_mireds(500);
            // Effects will be enabled once protocol is discovered
            // Uncomment and add effects once effect codes are known:
            // traits.add_supported_effect("Rainbow");
            // traits.add_supported_effect("Flash");
            // traits.add_supported_effect("Fade");
            return traits;
        }

        void FastconLight::write_state(light::LightState *state)
        {
            // Check if an effect is active
            fastcon::LightEffect effect;
            std::string effect_name = state->get_effect_name();
            if (!effect_name.empty())
            {
                // Map effect names to BRMesh effect configurations
                ESP_LOGD(TAG, "Effect active: %s", effect_name.c_str());
                
                // Helper to parse speed from effect name (e.g., "Christmas Slow" -> 25)
                uint8_t speed = 50; // Default medium speed
                if (effect_name.find(" Slow") != std::string::npos) speed = 25;
                else if (effect_name.find(" Fast") != std::string::npos) speed = 75;
                
                // Type 0x48: Simple effects (1 parameter)
                if (effect_name.find("Romantic") == 0) {
                    effect.type = 0x48; effect.effect_id = 0x42; effect.param_count = 1;
                    effect.params[0] = 0x00; effect.speed = speed;
                }
                else if (effect_name.find("Cozy") == 0) {
                    effect.type = 0x48; effect.effect_id = 0x42; effect.param_count = 1;
                    effect.params[0] = 0x01; effect.speed = speed;
                }
                else if (effect_name.find("Christmas") == 0) {
                    effect.type = 0x48; effect.effect_id = 0x42; effect.param_count = 1;
                    effect.params[0] = 0x04; effect.speed = speed;
                }
                else if (effect_name.find("Winter") == 0) {
                    effect.type = 0x48; effect.effect_id = 0x42; effect.param_count = 1;
                    effect.params[0] = 0x0b; effect.speed = speed;
                }
                else if (effect_name.find("Halloween") == 0) {
                    effect.type = 0x48; effect.effect_id = 0x42; effect.param_count = 1;
                    effect.params[0] = 0x0c; effect.speed = speed;
                }
                else if (effect_name.find("Valentines") == 0) {
                    effect.type = 0x48; effect.effect_id = 0x42; effect.param_count = 1;
                    effect.params[0] = 0x0d; effect.speed = speed;
                }
                // Type 0x58: Medium effects (2 parameters)
                else if (effect_name.find("Fresh") == 0) {
                    effect.type = 0x58; effect.effect_id = 0x42; effect.param_count = 2;
                    effect.params[0] = 0x02; effect.params[1] = 0x0a; effect.speed = speed;
                }
                else if (effect_name.find("Three Color Flash") == 0) {
                    effect.type = 0x58; effect.effect_id = 0xc2; effect.param_count = 2;
                    effect.params[0] = 0x04; effect.params[1] = 0x05; 
                    effect.speed = (speed == 25) ? 75 : (speed == 75) ? 25 : 50; // Invert for flash
                }
                // Type 0x88: Complex effects (5 parameters)
                else if (effect_name.find("Full Color Fade") == 0) {
                    effect.type = 0x88; effect.effect_id = 0x42; effect.param_count = 5;
                    effect.params[0] = 0x03; effect.params[1] = 0x01;
                    effect.params[2] = 0x05; effect.params[3] = 0x04;
                    effect.params[4] = 0x06; effect.speed = speed;
                }
                else if (effect_name.find("Full Color Flash") == 0) {
                    effect.type = 0x88; effect.effect_id = 0xc2; effect.param_count = 5;
                    effect.params[0] = 0x03; effect.params[1] = 0x01;
                    effect.params[2] = 0x05; effect.params[3] = 0x04;
                    effect.params[4] = 0x06; 
                    effect.speed = (speed == 25) ? 75 : (speed == 75) ? 25 : 50; // Invert for flash
                }
                else {
                    ESP_LOGW(TAG, "Unknown effect: %s", effect_name.c_str());
                }
            }

            // Get the light data bits from the state
            auto light_data = this->controller_->get_light_data(this->light_id_, state, effect);

            // Debug output - print the light state values
            bool is_on = (light_data[0] & 0x80) != 0;
            float brightness = ((light_data[0] & 0x7F) / 127.0f) * 100.0f;
            if (light_data.size() == 1)
            {
                ESP_LOGD(TAG, "Writing state: light_id=%d, on=%d, brightness=%.1f%%", light_id_, is_on, brightness);
            }
            else
            {
                auto r = light_data[2];
                auto g = light_data[3];
                auto b = light_data[1];
                auto warm = light_data[4];
                auto cold = light_data[5];
                ESP_LOGD(TAG, "Writing state: light_id=%d, on=%d, brightness=%.1f%%, rgb=(%d,%d,%d), warm=%d, cold=%d", light_id_, is_on, brightness, r, g, b, warm, cold);
            }

            // Generate the advertisement payload
            auto adv_data = this->controller_->single_control(this->light_id_, light_data);

            // Debug output - print payload as hex
            auto hex_str = vector_to_hex_string(adv_data).data();
            ESP_LOGD(TAG, "Advertisement Payload (%d bytes): %s", adv_data.size(), hex_str);

            // Send the advertisement
            this->controller_->queueCommand(this->light_id_, adv_data);
        }
    } // namespace fastcon
} // namespace esphome
