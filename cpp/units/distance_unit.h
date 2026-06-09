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

class DistanceUnit : public Unit {
public:
    inline static const DistanceUnit m  {"m",    1.0};

    // Conversion factors preserved from JaamSim units.inc
    inline static const DistanceUnit km {"km",   1000.0};
    inline static const DistanceUnit cm {"cm",   0.01};
    inline static const DistanceUnit mm {"mm",   0.001};

    // Imperial units — factors preserved from JaamSim units-imperial.inc
    inline static const DistanceUnit mi {"mi",   1609.344};
    inline static const DistanceUnit ft {"ft",   0.3048};
    inline static const DistanceUnit in {"in",   0.0254};

    using Unit::Unit;
};

}  // namespace jaamsim::units
