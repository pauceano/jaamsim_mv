/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2014 Ausenco Engineering Canada Inc.
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

#include <functional>

namespace jaamsim::states {

struct StateRecord;

/**
 * Callback type for state change notifications.
 *
 * Invoked after a StateEntity transitions between states.
 * @param prev  Previous state record (valid reference)
 * @param next  New state record (valid reference, same as entity's current state)
 */
using StateChangeCallback = std::function<void(const StateRecord& prev, const StateRecord& next)>;

/**
 * Interface for objects that monitor state entity transitions.
 *
 * The is_watching() guard allows a listener to selectively observe
 * specific entities without global registration.
 */
class StateEntityListener {
public:
    virtual ~StateEntityListener() = default;

    virtual bool is_watching(class StateEntity* ent) = 0;

    virtual void update_for_state_change(class StateEntity* ent,
                                         const StateRecord& prev,
                                         const StateRecord& next) = 0;
};

}
