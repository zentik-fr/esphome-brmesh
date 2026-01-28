#include "esphome/core/component_iterator.h"
#include "esphome/core/log.h"
#include "esphome/components/light/color_mode.h"
#include "fastcon_controller.h"
#include "protocol.h"

namespace esphome
{
    namespace fastcon
    {
        static const char *const TAG = "fastcon.controller";

        void FastconController::queueCommand(uint32_t light_id_, const std::vector<uint8_t> &data)
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (queue_.size() >= max_queue_size_)
            {
                ESP_LOGW(TAG, "Command queue full (size=%d), dropping command for light %d",
                         queue_.size(), light_id_);
                return;
            }

            Command cmd;
            cmd.data = data;
            cmd.timestamp = millis();
            cmd.retries = 0;

            queue_.push(cmd);
            ESP_LOGV(TAG, "Command queued, queue size: %d", queue_.size());
        }

        void FastconController::clear_queue()
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            std::queue<Command> empty;
            std::swap(queue_, empty);
        }

        void FastconController::setup()
        {
            ESP_LOGCONFIG(TAG, "Setting up Fastcon BLE Controller...");
            ESP_LOGCONFIG(TAG, "  Advertisement interval: %d-%d", this->adv_interval_min_, this->adv_interval_max_);
            ESP_LOGCONFIG(TAG, "  Advertisement duration: %dms", this->adv_duration_);
            ESP_LOGCONFIG(TAG, "  Advertisement gap: %dms", this->adv_gap_);
            ESP_LOGCONFIG(TAG, "  Command retries: %d", this->command_retries_);
            ESP_LOGCONFIG(TAG, "  Command interval: %dms", this->command_interval_);
        }

        // Encode effect data based on effect type (0x48, 0x58, 0x88)
        std::vector<uint8_t> encode_effect_data(uint8_t light_id, const fastcon::LightEffect &effect)
        {
            std::vector<uint8_t> data;

            // Type 0x48: Simple effects (1 parameter)
            if (effect.type == 0x48 && effect.param_count >= 1)
            {
                data = std::vector<uint8_t>{0x48, light_id, effect.speed, effect.effect_id, effect.params[0], 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                ESP_LOGD(TAG, "Type 0x48: speed=%d, effect_id=0x%02x, param=0x%02x",
                         effect.speed, effect.effect_id, effect.params[0]);
            }
            // Type 0x58: Medium effects (2 parameters)
            else if (effect.type == 0x58 && effect.param_count >= 2)
            {
                data = std::vector<uint8_t>{0x58, light_id, effect.speed, effect.effect_id, effect.params[0], effect.params[1], 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                ESP_LOGD(TAG, "Type 0x58: speed=%d, effect_id=0x%02x, params=0x%02x,0x%02x",
                         effect.speed, effect.effect_id, effect.params[0], effect.params[1]);
            }
            // Type 0x88: Complex effects (5 parameters)
            else if (effect.type == 0x88 && effect.param_count >= 5)
            {
                data = std::vector<uint8_t>{0x88, light_id, effect.speed, effect.effect_id,
                        effect.params[0], effect.params[1], effect.params[2], effect.params[3], effect.params[4],
                        0x00, 0x00, 0x00};
                ESP_LOGD(TAG, "Type 0x88: speed=%d, effect_id=0x%02x, params=0x%02x,0x%02x,0x%02x,0x%02x,0x%02x",
                         effect.speed, effect.effect_id, effect.params[0], effect.params[1],
                         effect.params[2], effect.params[3], effect.params[4]);
            }
            else
            {
                ESP_LOGW(TAG, "Invalid effect configuration: type=0x%02x, param_count=%d",
                         effect.type, effect.param_count);
            }

            return data;
        }

        void FastconController::loop()
        {
            const uint32_t now = millis();

            switch (adv_state_)
            {
            case AdvertiseState::IDLE:
            {
                // Vérifier s'il y a une nouvelle commande dans la queue
                {
                    std::lock_guard<std::mutex> lock(queue_mutex_);
                    if (!queue_.empty())
                    {
                        // Regarder la prochaine commande sans la retirer
                        const Command& next_cmd = queue_.front();
                        
                        // Si on a une commande en cours ET que la nouvelle est différente
                        if (has_current_command_ && next_cmd.data != current_command_.data)
                        {
                            // Nouvelle commande prioritaire : abandonner l'actuelle
                            ESP_LOGD(TAG, "New priority command detected, interrupting current command (%d/%d sent)", 
                                     current_command_.retries + 1, command_retries_);
                            has_current_command_ = false;
                        }
                        
                        // Si on n'a pas de commande en cours, prendre la suivante
                        if (!has_current_command_)
                        {
                            current_command_ = queue_.front();
                            queue_.pop();
                            
                            // Détecter si c'est une nouvelle commande différente
                            if (current_command_.data != last_command_data_)
                            {
                                // Nouvelle commande : réinitialiser le compteur
                                current_command_.retries = 0;
                                last_command_data_ = current_command_.data;
                                ESP_LOGD(TAG, "New command detected, resetting retry counter");
                            }
                            
                            has_current_command_ = true;
                        }
                    }
                    else if (!has_current_command_)
                    {
                        // Pas de commande en cours et queue vide
                        return;
                    }
                }

                Command& cmd = current_command_;

                esp_ble_adv_params_t adv_params = {
                    .adv_int_min = adv_interval_min_,
                    .adv_int_max = adv_interval_max_,
                    .adv_type = ADV_TYPE_NONCONN_IND,
                    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
                    .peer_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                    .peer_addr_type = BLE_ADDR_TYPE_PUBLIC,
                    .channel_map = ADV_CHNL_ALL,
                    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
                };

                uint8_t adv_data_raw[31] = {0};
                uint8_t adv_data_len = 0;

                // Add flags
                adv_data_raw[adv_data_len++] = 2;
                adv_data_raw[adv_data_len++] = ESP_BLE_AD_TYPE_FLAG;
                adv_data_raw[adv_data_len++] = ESP_BLE_ADV_FLAG_BREDR_NOT_SPT | ESP_BLE_ADV_FLAG_GEN_DISC;

                // Add manufacturer data
                adv_data_raw[adv_data_len++] = cmd.data.size() + 2;
                adv_data_raw[adv_data_len++] = ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE;
                adv_data_raw[adv_data_len++] = MANUFACTURER_DATA_ID & 0xFF;
                adv_data_raw[adv_data_len++] = (MANUFACTURER_DATA_ID >> 8) & 0xFF;

                memcpy(&adv_data_raw[adv_data_len], cmd.data.data(), cmd.data.size());
                adv_data_len += cmd.data.size();

                esp_err_t err = esp_ble_gap_config_adv_data_raw(adv_data_raw, adv_data_len);
                if (err != ESP_OK)
                {
                    ESP_LOGW(TAG, "Error setting raw advertisement data (err=%d): %s", err, esp_err_to_name(err));
                    return;
                }

                err = esp_ble_gap_start_advertising(&adv_params);
                if (err != ESP_OK)
                {
                    ESP_LOGW(TAG, "Error starting advertisement (err=%d): %s", err, esp_err_to_name(err));
                    return;
                }

                adv_state_ = AdvertiseState::ADVERTISING;
                state_start_time_ = now;
                ESP_LOGV(TAG, "Started advertising");
                break;
            }

            case AdvertiseState::ADVERTISING:
            {
                if (now - state_start_time_ >= adv_duration_)
                {
                    esp_ble_gap_stop_advertising();
                    adv_state_ = AdvertiseState::GAP;
                    state_start_time_ = now;
                    ESP_LOGV(TAG, "Stopped advertising, entering gap period");
                }
                break;
            }

            case AdvertiseState::GAP:
            {
                // Calculer le temps de gap pour respecter command_interval total
                uint32_t gap_duration = command_interval_ > adv_duration_ ? command_interval_ - adv_duration_ : 0;
                
                if (now - state_start_time_ >= gap_duration)
                {
                    // Incrémenter le compteur de retries
                    if (has_current_command_)
                    {
                        current_command_.retries++;
                        if (current_command_.retries >= command_retries_)
                        {
                            // Commande envoyée suffisamment de fois, passer à la suivante
                            has_current_command_ = false;
                            ESP_LOGD(TAG, "Command sent %d times, moving to next", command_retries_);
                        }
                        else
                        {
                            ESP_LOGV(TAG, "Retrying command (%d/%d)", current_command_.retries + 1, command_retries_);
                        }
                    }
                    adv_state_ = AdvertiseState::IDLE;
                    ESP_LOGV(TAG, "Gap period complete");
                }
                break;
            }
            }
        }

        std::vector<uint8_t> FastconController::get_light_data(uint8_t light_id, light::LightState *state, const LightEffect &effect)
        {
            std::vector<uint8_t> light_data = {
                0, // 0 - On/Off Bit + 7-bit Brightness
                0, // 1 - Blue byte
                0, // 2 - Red byte
                0, // 3 - Green byte
                0, // 4 - Warm byte
                0, // 5 - Cold byte
                // 6 - Effect ID (if effect.effect_id != 0)
                // 7 - Effect Speed (if effect.effect_id != 0)
                // 8 - Effect Param1 (if effect.effect_id != 0)
                // 9 - Effect Param2 (if effect.effect_id != 0)
            };

            // TODO: need to figure out when esphome is changing to white vs setting brightness

            auto values = state->current_values;

            bool is_on = values.is_on();
            if (!is_on)
            {
                return std::vector<uint8_t>({0x00});
            }

            auto color_mode = values.get_color_mode();
            bool has_white = (static_cast<uint8_t>(color_mode) & static_cast<uint8_t>(light::ColorCapability::WHITE)) != 0;
            float brightness = std::min(values.get_brightness() * 127.0f, 127.0f); // clamp the value to at most 127
            light_data[0] = 0x80 + static_cast<uint8_t>(brightness);

            if (has_white)
            {
                // White mode: RGB à 0, warm/cold à 50/50 pour blanc neutre
                light_data[1] = 0x00;  // Blue = 0
                light_data[2] = 0x00;  // Red = 0
                light_data[3] = 0x00;  // Green = 0
                light_data[4] = 0x7f;  // Warm = 50% (127)
                light_data[5] = 0x7f;  // Cold = 50% (127)
                return light_data;
            }

            bool has_rgb = (static_cast<uint8_t>(color_mode) & static_cast<uint8_t>(light::ColorCapability::RGB)) != 0;
            if (has_rgb)
            {
                light_data[1] = static_cast<uint8_t>(values.get_blue() * 255.0f);
                light_data[2] = static_cast<uint8_t>(values.get_red() * 255.0f);
                light_data[3] = static_cast<uint8_t>(values.get_green() * 255.0f);
            }

            bool has_cold_warm = (static_cast<uint8_t>(color_mode) & static_cast<uint8_t>(light::ColorCapability::COLD_WARM_WHITE)) != 0;
            if (has_cold_warm)
            {
                light_data[4] = static_cast<uint8_t>(values.get_warm_white() * 255.0f);
                light_data[5] = static_cast<uint8_t>(values.get_cold_white() * 255.0f);
            }

            // TODO figure out if we can use these, and how
            bool has_temp = (static_cast<uint8_t>(color_mode) & static_cast<uint8_t>(light::ColorCapability::COLOR_TEMPERATURE)) != 0;
            if (has_temp)
            {
                float temperature = values.get_color_temperature();
                if (temperature < 153)
                {
                    light_data[4] = 0xff;
                    light_data[5] = 0x00;
                }
                else if (temperature > 500)
                {
                    light_data[4] = 0x00;
                    light_data[5] = 0xff;
                }
                else
                {
                    // Linear interpolation between (153, 0xff) and (500, 0x00)
                    light_data[4] = (uint8_t)(((500 - temperature) * 255.0f + (temperature - 153) * 0x00) / (500 - 153));
                    light_data[5] = (uint8_t)(((temperature - 153) * 255.0f + (500 - temperature) * 0x00) / (500 - 153));
                }
            }

            // If an effect is active, use effect encoding instead of RGB/White data
            if (effect.type != 0 && effect.effect_id != 0)
            {
                ESP_LOGD(TAG, "Effect active: type=0x%02x, effect_id=0x%02x, speed=%d",
                         effect.type, effect.effect_id, effect.speed);
                return encode_effect_data(light_id, effect);
            }

            return light_data;
        }

        std::vector<uint8_t> FastconController::single_control(uint32_t light_id_, const std::vector<uint8_t> &light_data)
        {
            std::vector<uint8_t> result_data(12);

            result_data[0] = 2 | (((0xfffffff & (light_data.size() + 1)) << 4));
            result_data[1] = light_id_;
            std::copy(light_data.begin(), light_data.end(), result_data.begin() + 2);

            // Debug output - print payload as hex
            auto hex_str = vector_to_hex_string(result_data).data();
            ESP_LOGD(TAG, "Inner Payload (%d bytes): %s", result_data.size(), hex_str);

            return this->generate_command(5, light_id_, result_data, true);
        }

        std::vector<uint8_t> FastconController::generate_command(uint8_t n, uint32_t light_id_, const std::vector<uint8_t> &data, bool forward)
        {
            static uint8_t sequence = 0;

            // Create command body with header
            std::vector<uint8_t> body(data.size() + 4);
            uint8_t i2 = (light_id_ / 256);

            // Construct header
            body[0] = (i2 & 0b1111) | ((n & 0b111) << 4) | (forward ? 0x80 : 0);
            body[1] = sequence++; // Use and increment sequence number
            if (sequence >= 255)
                sequence = 1;

            body[2] = this->mesh_key_[3]; // Safe key

            // Copy data
            std::copy(data.begin(), data.end(), body.begin() + 4);

            // Calculate checksum
            uint8_t checksum = 0;
            for (size_t i = 0; i < body.size(); i++)
            {
                if (i != 3)
                {
                    checksum = checksum + body[i];
                }
            }
            body[3] = checksum;

            // Encrypt header and data
            for (size_t i = 0; i < 4; i++)
            {
                body[i] = DEFAULT_ENCRYPT_KEY[i & 3] ^ body[i];
            }

            for (size_t i = 0; i < data.size(); i++)
            {
                body[4 + i] = this->mesh_key_[i & 3] ^ body[4 + i];
            }

            // Prepare the final payload with RF protocol formatting
            std::vector<uint8_t> addr = {DEFAULT_BLE_FASTCON_ADDRESS.begin(), DEFAULT_BLE_FASTCON_ADDRESS.end()};
            return prepare_payload(addr, body);
        }
    } // namespace fastcon
} // namespace esphome
