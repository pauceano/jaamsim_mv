/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2011 Ausenco Engineering Canada Inc.
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

#include "unit.h"

namespace jaamsim::units {

class TimeUnit : public Unit {
public:
    inline static const TimeUnit seconds  {"s",    1.0};

    // Conversion factors preserved from JaamSim units.inc
    inline static const TimeUnit minutes  {"min",  60.0};
    inline static const TimeUnit hours    {"h",    3600.0};
    inline static const TimeUnit days     {"d",    86400.0};
    inline static const TimeUnit weeks    {"w",    604800.0};
    inline static const TimeUnit years    {"y",    31536000.0};
    inline static const TimeUnit milliseconds {"ms", 1.0e-3};
    inline static const TimeUnit microseconds  {"us", 1.0e-6};
    inline static const TimeUnit nanoseconds   {"ns", 1.0e-9};

    using Unit::Unit;
};

}  // namespace jaamsim::units
