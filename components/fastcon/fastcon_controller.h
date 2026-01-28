#pragma once

#include <queue>
#include <mutex>
#include <vector>
#include "esphome/core/component.h"
#include "esphome/components/esp32_ble_server/ble_server.h"

namespace esphome
{
    namespace fastcon
    {
        // Structure to hold effect information
        struct LightEffect
        {
            uint8_t type{0};          // Command type: 0x48 (1 param), 0x58 (2 params), 0x88 (5 params), 0x9e (timed)
            uint8_t effect_id{0};     // Effect ID (e.g., 0x42 for fade effects, 0xc2 for flash)
            uint8_t speed{50};        // Effect speed (1-100 for normal, 200-2 for inverted flash effects)
            uint8_t params[5]{0};     // Up to 5 parameters for complex effects (Type 0x88)
            uint8_t param_count{0};   // Number of parameters used (1-5)
        };

        class FastconController : public Component
        {
        public:
            FastconController() = default;

            void setup() override;
            void loop() override;

            std::vector<uint8_t> get_light_data(uint8_t light_id, light::LightState *state, const LightEffect &effect = {});
            std::vector<uint8_t> single_control(uint32_t addr, const std::vector<uint8_t> &light_data);

            void queueCommand(uint32_t light_id_, const std::vector<uint8_t> &data);

            void clear_queue();
            bool is_queue_empty() const
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                return queue_.empty();
            }
            size_t get_queue_size() const
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                return queue_.size();
            }
            void set_max_queue_size(size_t size) { max_queue_size_ = size; }

            void set_mesh_key(std::array<uint8_t, 4> key) { mesh_key_ = key; }
            void set_adv_interval_min(uint16_t val) { adv_interval_min_ = val; }
            void set_adv_interval_max(uint16_t val)
            {
                adv_interval_max_ = val;
                if (adv_interval_max_ < adv_interval_min_)
                {
                    adv_interval_max_ = adv_interval_min_;
                }
            }
            void set_adv_duration(uint16_t val) { adv_duration_ = val; }
            void set_adv_gap(uint16_t val) { adv_gap_ = val; }
            void set_command_retries(uint8_t val) { command_retries_ = val; }
            void set_command_interval(uint16_t val) { command_interval_ = val; }

        protected:
            struct Command
            {
                std::vector<uint8_t> data;
                uint32_t timestamp;
                uint8_t retries{0};
            };

            std::queue<Command> queue_;
            mutable std::mutex queue_mutex_;
            size_t max_queue_size_{100};

            enum class AdvertiseState
            {
                IDLE,
                ADVERTISING,
                GAP
            };

            AdvertiseState adv_state_{AdvertiseState::IDLE};
            uint32_t state_start_time_{0};

            // Protocol implementation
            std::vector<uint8_t> generate_command(uint8_t n, uint32_t light_id_, const std::vector<uint8_t> &data, bool forward = true);

            std::array<uint8_t, 4> mesh_key_{};

            uint16_t adv_interval_min_{0x20};
            uint16_t adv_interval_max_{0x40};
            uint16_t adv_duration_{15};
            uint16_t adv_gap_{10};
            uint8_t command_retries_{20};
            uint16_t command_interval_{25};
            Command current_command_;
            bool has_current_command_{false};
            std::vector<uint8_t> last_command_data_;

            static const uint16_t MANUFACTURER_DATA_ID = 0xfff0;
        };

    } // namespace fastcon
} // namespace esphome
