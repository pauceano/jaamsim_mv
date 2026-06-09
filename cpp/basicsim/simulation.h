/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2002-2011 Ausenco Engineering Canada Inc.
 * Copyright (C) 2016-2025 JaamSim Software Inc.
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
 *
 * Translated from Java: com.jaamsim.basicsim.Simulation
 */

#ifndef JAAMSIM_BASICSIM_SIMULATION_H
#define JAAMSIM_BASICSIM_SIMULATION_H

#include "entity.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace jaamsim::basicsim {

class Simulation final : public Entity {
public:
    Simulation();
    ~Simulation() override = default;

    Simulation(const Simulation&) = delete;
    Simulation& operator=(const Simulation&) = delete;
    Simulation(Simulation&&) noexcept = default;
    Simulation& operator=(Simulation&&) noexcept = default;

    // ===================================================================
    // Key Inputs
    // ===================================================================

    double run_duration() const noexcept { return run_duration_; }
    void set_run_duration(double seconds) { run_duration_ = seconds; }

    double initialization_time() const noexcept { return initialization_time_; }
    void set_initialization_time(double seconds) { initialization_time_ = seconds; }

    bool gregorian_calendar() const noexcept { return gregorian_calendar_; }
    void set_gregorian_calendar(bool on) noexcept { gregorian_calendar_ = on; }

    const std::string& start_date() const noexcept { return start_date_; }
    void set_start_date(const std::string& date) { start_date_ = date; }

    bool exit_at_stop() const noexcept { return exit_at_stop_; }
    void set_exit_at_stop(bool on) noexcept { exit_at_stop_ = on; }

    bool exit_at_pause_condition() const noexcept { return exit_at_pause_condition_; }
    void set_exit_at_pause_condition(bool on) noexcept { exit_at_pause_condition_ = on; }

    // ===================================================================
    // Random seed control
    // ===================================================================

    int32_t global_substream_seed() const noexcept { return global_substream_seed_; }
    void set_global_substream_seed(int32_t seed) { global_substream_seed_ = seed; }

    // ===================================================================
    // Output report
    // ===================================================================

    bool print_report() const noexcept { return print_report_; }
    void set_print_report(bool on) noexcept { print_report_ = on; }

    const std::string& report_directory() const noexcept { return report_directory_; }
    void set_report_directory(const std::string& dir) { report_directory_ = dir; }

    // ===================================================================
    // Run configuration
    // ===================================================================

    int32_t number_of_replications() const noexcept { return number_of_replications_; }
    void set_number_of_replications(int32_t n) { number_of_replications_ = n; }

    int32_t number_of_threads() const noexcept { return number_of_threads_; }
    void set_number_of_threads(int32_t n) { number_of_threads_ = n; }

    bool print_replications() const noexcept { return print_replications_; }
    void set_print_replications(bool on) noexcept { print_replications_ = on; }

    bool print_confidence_intervals() const noexcept { return print_confidence_intervals_; }
    void set_print_confidence_intervals(bool on) noexcept { print_confidence_intervals_ = on; }

    bool print_run_labels() const noexcept { return print_run_labels_; }
    void set_print_run_labels(bool on) noexcept { print_run_labels_ = on; }

    // ===================================================================
    // Tracing / verification
    // ===================================================================

    bool enable_tracing() const noexcept { return enable_tracing_; }
    void set_enable_tracing(bool on) noexcept { enable_tracing_ = on; }

    bool trace_events() const noexcept { return trace_events_; }
    void set_trace_events(bool on) noexcept { trace_events_ = on; }

    bool verify_events() const noexcept { return verify_events_; }
    void set_verify_events(bool on) noexcept { verify_events_ = on; }

    // ===================================================================
    // Tick configuration
    // ===================================================================

    double tick_length() const noexcept { return tick_length_; }
    void set_tick_length(double seconds) { tick_length_ = seconds; }

    // ===================================================================
    // GUI
    // ===================================================================

    bool snap_to_grid() const noexcept { return snap_to_grid_; }
    void set_snap_to_grid(bool on) noexcept { snap_to_grid_ = on; }

    int32_t max_entities_to_display() const noexcept { return max_entities_to_display_; }
    void set_max_entities_to_display(int32_t n) { max_entities_to_display_ = n; }

    // ===================================================================
    // Lifecycle hooks
    // ===================================================================

    void early_init() override;
    void start_up() override;
    void do_end() override;

private:
    // Key Inputs
    double run_duration_              = 8760.0 * 3600.0;  // 8760 hours in seconds
    double initialization_time_       = 0.0;
    bool gregorian_calendar_          = true;
    std::string start_date_           = "2000-01-01";
    std::string pause_condition_;
    bool exit_at_pause_condition_     = false;
    bool exit_at_stop_                = false;

    // Random seed
    int32_t global_substream_seed_    = 0;

    // Output report
    bool print_report_                = false;
    std::string report_directory_;

    // Run configuration
    int32_t number_of_replications_   = 1;
    int32_t number_of_threads_        = 1;
    bool print_replications_          = false;
    bool print_confidence_intervals_  = false;
    bool print_run_labels_            = true;

    // Tracing
    bool enable_tracing_              = false;
    bool trace_events_                = false;
    bool verify_events_               = false;

    // Tick
    double tick_length_               = 1e-6;

    // GUI
    bool snap_to_grid_                = false;
    int32_t max_entities_to_display_  = 100000;
};

// ===========================================================================
// Inline implementations
// ===========================================================================

inline Simulation::Simulation() {
    set_local_name("Simulation");
}

inline void Simulation::early_init() {
    Entity::early_init();
    // Additional initialisation specific to Simulation
}

inline void Simulation::start_up() {
    Entity::start_up();
    // Schedule initial process targets
}

inline void Simulation::do_end() {
    Entity::do_end();
    // Generate output report
}

}  // namespace jaamsim::basicsim

#endif  // JAAMSIM_BASICSIM_SIMULATION_H
