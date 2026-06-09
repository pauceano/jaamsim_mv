/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2014 Ausenco Engineering Canada Inc.
 * Copyright (C) 2018-2024 JaamSim Software Inc.
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
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "state_entity_listener.h"
#include "state_record.h"

namespace jaamsim::states {

class StateEntity {
public:
    static constexpr const char* STATE_IDLE     = "Idle";
    static constexpr const char* STATE_WORKING  = "Working";
    static constexpr const char* STATE_INACTIVE = "Inactive";

    StateEntity() = default;
    virtual ~StateEntity() = default;

    virtual void early_init();
    virtual void late_init();
    virtual void do_end();
    virtual void clear_statistics();
    virtual void collect_cycle_stats();
    virtual void close();

    virtual std::string get_initial_state() const;
    virtual bool is_valid_state(const std::string& state) const;
    virtual bool is_valid_working_state(const std::string& state) const;

    const StateRecord* current_state() const { return present_state_; }
    bool is_working_state() const;

    const StateRecord* get_state(const std::string& name) const;

    std::vector<const StateRecord*> get_state_records() const;

    int64_t get_ticks_in_state(int64_t sim_ticks, const StateRecord* state) const;
    int64_t get_ticks_in_state(const StateRecord* state) const;

    int64_t get_current_cycle_ticks(int64_t sim_ticks, const StateRecord* state) const;
    int64_t get_current_cycle_ticks(const StateRecord* state) const;

    int64_t get_completed_cycle_ticks(const StateRecord* state) const;
    int64_t get_init_ticks(const StateRecord* state) const;

    void set_present_state(const std::string& state);

    void add_state(const std::string& state);

    const std::unordered_set<std::string>& working_state_list() const { return working_state_list_; }
    void set_working_state_list(std::unordered_set<std::string> list) { working_state_list_ = std::move(list); }

    void add_state_listener(StateChangeCallback cb) {
        state_callbacks_.push_back(std::move(cb));
    }

    const std::vector<StateChangeCallback>& state_listeners() const {
        return state_callbacks_;
    }

    void add_state_listener(StateEntityListener* listener) {
        state_entity_listeners_.push_back(listener);
    }

    virtual void state_changed(const StateRecord& prev, const StateRecord& next);

    int64_t last_state_collection_tick() const { return last_state_collection_tick_; }
    void set_last_state_collection_tick(int64_t tick) { last_state_collection_tick_ = tick; }

    int64_t working_ticks() const { return working_ticks_; }

    bool use_current_cycle() const { return use_current_cycle_; }
    void set_use_current_cycle(bool v) { use_current_cycle_ = v; }

private:
    void init_state_data();
    void update_state_stats();
    StateRecord create_record(const std::string& state);
    int64_t get_working_ticks(int64_t sim_ticks) const;

    StateRecord* present_state_{nullptr};
    std::unordered_map<std::string, StateRecord> states_;
    std::unordered_set<std::string> working_state_list_;
    std::vector<StateChangeCallback> state_callbacks_;
    std::vector<StateEntityListener*> state_entity_listeners_;
    int64_t last_state_collection_tick_{0};
    int64_t working_ticks_{0};
    bool use_current_cycle_{false};
};

}
