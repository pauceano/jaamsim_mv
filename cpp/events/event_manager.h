/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2002-2014 Ausenco Engineering Canada Inc.
 * Copyright (C) 2017-2022 JaamSim Software Inc.
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
 * Translated from Java: com.jaamsim.events.EventManager
 *
 * Simplified single-threaded variant. The original Java implementation uses a
 * custom EventTree, native threads, and Process/WaitTarget for multi-threaded
 * process-oriented simulation. This C++ version replaces that with a
 * std::priority_queue and std::function callbacks.
 */

#ifndef JAAMSIM_EVENTS_EVENT_MANAGER_H
#define JAAMSIM_EVENTS_EVENT_MANAGER_H

#include <cstdint>
#include <functional>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

namespace jaamsim::events {

// ===========================================================================
// Event — lightweight schedule entry
// ===========================================================================

struct Event {
    double time = 0.0;
    int priority = 5;       // lower = higher precedence (0 = highest)
    uint64_t sequence = 0;  // tiebreaker for FIFO/LIFO ordering
    std::function<void()> action;

    bool operator<(const Event& rhs) const noexcept {
        if (time != rhs.time) return time > rhs.time;               // earlier first
        if (priority != rhs.priority) return priority > rhs.priority; // lower prio first
        return sequence > rhs.sequence;                              // FIFO: lower seq first
    }
};

// ===========================================================================
// Conditional — evaluated each tick before time advance
// ===========================================================================

class Conditional {
public:
    virtual ~Conditional() = default;
    virtual bool evaluate() = 0;
};

// ===========================================================================
// EventManager — discrete-event engine
// ===========================================================================

class EventManager {
public:
    EventManager();
    explicit EventManager(std::string name);

    ~EventManager() = default;
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;
    EventManager(EventManager&&) noexcept = default;
    EventManager& operator=(EventManager&&) noexcept = default;

    // ===================================================================
    // Scheduling
    // ===================================================================

    /// Schedule an event at a future time.
    /// @param time    absolute simulation time (seconds)
    /// @param priority lower = executed first (default 5 = PRI_NORMAL)
    /// @param fifo    true = FIFO tiebreaking, false = LIFO
    /// @param action  callback to execute
    void schedule(double time, int priority, bool fifo,
                  std::function<void()> action);

    /// Schedule after a relative delay.
    void schedule_after(double delay_seconds, int priority, bool fifo,
                        std::function<void()> action);

    /// Register a conditional that's evaluated each tick.
    /// Fires when evaluate() returns true.
    void wait_until(Conditional* cond, std::function<void()> action);

    /// Schedule an external callback for conditional evaluation.
    void schedule_until(Conditional* cond, std::function<void()> action);

    /// Cancel a specific event. Not implemented in simplified variant.
    void kill_event(void* handle) { /* stub */ }

    /// Interrupt and immediately run an event. Not implemented.
    void interrupt_event(void* handle) { /* stub */ }

    // ===================================================================
    // Execution control
    // ===================================================================

    /// Run the simulation until the specified duration is reached.
    /// Returns the total simulated time (may be less than requested if
    /// the event queue is exhausted).
    double run(double duration_seconds);

    /// Pause execution at the next event boundary.
    void pause() noexcept { execute_events_ = false; }

    /// Resume execution.
    void resume() noexcept { execute_events_ = true; }

    /// Resume and run for a specified duration.
    void resume_for(double duration_seconds);

    /// Step exactly one event.
    void step_one();

    /// Step all events at the next simulation time.
    void step_one_time();

    // ===================================================================
    // Time queries
    // ===================================================================

    double current_time() const noexcept { return current_time_; }
    bool is_running() const noexcept { return execute_events_; }
    bool has_events() const noexcept { return !event_queue_.empty(); }
    double next_event_time() const;

    // ===================================================================
    // Configuration
    // ===================================================================

    void set_tick_length(double seconds) noexcept;
    double tick_length() const noexcept { return tick_length_; }

    /// Convert seconds to the nearest discrete tick.
    int64_t seconds_to_ticks(double seconds) const noexcept;

    /// Convert ticks to seconds.
    double ticks_to_seconds(int64_t ticks) const noexcept;

    // ===================================================================
    // Real-time mode
    // ===================================================================

    void set_real_time(bool on, double factor = 1.0) noexcept;
    bool real_time() const noexcept { return real_time_; }
    double real_time_factor() const noexcept { return real_time_factor_; }

    // ===================================================================
    // Introspection
    // ===================================================================

    const std::string& name() const noexcept { return name_; }
    size_t pending_event_count() const noexcept { return event_queue_.size(); }

private:
    std::string name_;
    double current_time_ = 0.0;
    double tick_length_ = 1e-6;
    uint64_t event_sequence_ = 0;
    bool execute_events_ = false;

    std::priority_queue<Event> event_queue_;

    struct ConditionalEntry {
        Conditional* condition;
        std::function<void()> action;
    };
    std::vector<ConditionalEntry> conditional_events_;

    bool real_time_ = false;
    double real_time_factor_ = 1.0;

    void evaluate_conditionals();
    void advance_time(double target_time);
};

// ===========================================================================
// Inline implementations
// ===========================================================================

inline EventManager::EventManager()
    : name_("default") {
}

inline EventManager::EventManager(std::string name)
    : name_(std::move(name)) {
}

inline void EventManager::set_tick_length(double seconds) noexcept {
    tick_length_ = seconds;
}

inline int64_t EventManager::seconds_to_ticks(double seconds) const noexcept {
    return static_cast<int64_t>(seconds / tick_length_ + 0.5);
}

inline double EventManager::ticks_to_seconds(int64_t ticks) const noexcept {
    return static_cast<double>(ticks) * tick_length_;
}

inline void EventManager::set_real_time(bool on, double factor) noexcept {
    real_time_ = on;
    real_time_factor_ = factor;
}

inline double EventManager::next_event_time() const {
    if (event_queue_.empty())
        throw std::runtime_error("EventManager: no pending events");
    return event_queue_.top().time;
}

inline void EventManager::schedule(double time, int priority, bool fifo,
                                    std::function<void()> action) {
    Event evt;
    evt.time     = time;
    evt.priority = priority;
    evt.sequence = event_sequence_++;
    evt.action   = std::move(action);
    event_queue_.push(std::move(evt));
}

inline void EventManager::schedule_after(double delay_seconds, int priority,
                                           bool fifo,
                                           std::function<void()> action) {
    schedule(current_time_ + delay_seconds, priority, fifo, std::move(action));
}

inline void EventManager::wait_until(Conditional* cond,
                                      std::function<void()> action) {
    conditional_events_.push_back({cond, std::move(action)});
}

inline void EventManager::schedule_until(Conditional* cond,
                                           std::function<void()> action) {
    conditional_events_.push_back({cond, std::move(action)});
}

inline void EventManager::resume_for(double duration_seconds) {
    execute_events_ = true;
    run(duration_seconds);
}

inline void EventManager::step_one() {
    if (event_queue_.empty()) return;

    bool was_running = execute_events_;
    execute_events_ = true;

    Event evt = std::move(const_cast<Event&>(event_queue_.top()));
    event_queue_.pop();

    current_time_ = evt.time;
    evt.action();

    execute_events_ = false;
    if (was_running) execute_events_ = true;
}

inline void EventManager::step_one_time() {
    if (event_queue_.empty()) return;

    execute_events_ = true;
    double target = event_queue_.top().time;
    advance_time(target);
    execute_events_ = false;
}

inline void EventManager::advance_time(double target_time) {
    current_time_ = target_time;
}

inline void EventManager::evaluate_conditionals() {
    auto it = conditional_events_.begin();
    while (it != conditional_events_.end()) {
        if (it->condition->evaluate()) {
            schedule(current_time_, 0, true, std::move(it->action));
            it = conditional_events_.erase(it);
        }
        else {
            ++it;
        }
    }
}

inline double EventManager::run(double duration_seconds) {
    execute_events_ = true;
    double stop_time = current_time_ + duration_seconds;

    while (execute_events_ && !event_queue_.empty()) {
        Event evt = std::move(const_cast<Event&>(event_queue_.top()));
        event_queue_.pop();

        if (evt.time > stop_time) {
            // Event is beyond the run window — put it back
            event_queue_.push(std::move(evt));
            current_time_ = stop_time;
            break;
        }

        current_time_ = evt.time;

        // Evaluate conditionals when we reach a new time
        evaluate_conditionals();

        // Execute the event
        evt.action();
    }

    execute_events_ = false;
    return current_time_;
}

}  // namespace jaamsim::events

#endif  // JAAMSIM_EVENTS_EVENT_MANAGER_H
