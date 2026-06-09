/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2014 Ausenco Engineering Canada Inc.
 * Copyright (C) 2018 JaamSim Software Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <cstdint>
#include <string>

namespace jaamsim::states {

class StateRecord {
public:
    StateRecord(std::string state, bool is_working)
        : name_(std::move(state)), working_(is_working) {}

    const std::string& name() const { return name_; }
    bool is_working() const { return working_; }

    void add_ticks(int64_t ticks) {
        total_ticks_ += ticks;
        current_cycle_ticks_ += ticks;
    }

    void finish_warmup() {
        init_ticks_ = total_ticks_;
        total_ticks_ = 0;
        completed_cycle_ticks_ = 0;
    }

    void finish_cycle() {
        completed_cycle_ticks_ += current_cycle_ticks_;
        current_cycle_ticks_ = 0;
    }

    void set_start_tick(int64_t tick) { start_tick_ = tick; }
    int64_t start_tick() const { return start_tick_; }
    int64_t init_ticks() const { return init_ticks_; }
    int64_t total_ticks() const { return total_ticks_; }
    int64_t current_cycle_ticks() const { return current_cycle_ticks_; }
    int64_t completed_cycle_ticks() const { return completed_cycle_ticks_; }

private:
    std::string name_;
    bool working_;
    int64_t init_ticks_{0};
    int64_t total_ticks_{0};
    int64_t completed_cycle_ticks_{0};
    int64_t current_cycle_ticks_{0};
    int64_t start_tick_{0};
};

}  // namespace jaamsim::states
