/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2011 Ausenco Engineering Canada Inc.
 * Copyright (C) 2018-2025 JaamSim Software Inc.
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

#include <string>

namespace jaamsim::units {

/**
 * Abstract base for all unit types.
 *
 * Each concrete Unit holds a conversion factor that transforms a value in
 * this unit to the corresponding SI unit:
 *
 *   si_value = unit_value * conversion_factor
 *
 * Default SI factor is 1.0 (this unit IS the SI unit).
 */
class Unit {
public:
    Unit(std::string name, double conversion_factor = 1.0)
        : name_(std::move(name)), factor_(conversion_factor) {}

    virtual ~Unit() = default;

    /** Convert a value expressed in this unit to the SI equivalent. */
    virtual double to_si(double value) const { return value * factor_; }

    /** Convert an SI value to a value expressed in this unit. */
    virtual double from_si(double si_value) const { return si_value / factor_; }

    /** Human-readable name for this unit (e.g. "s", "m", "km/h"). */
    virtual const std::string& name() const { return name_; }

    double conversion_factor() const { return factor_; }

    /**
     * Return the conversion factor from this unit to another unit of the same
     * type so that: value_in_other = value_in_this * getConversionFactorToUnit(other)
     */
    double conversion_factor_to(const Unit& other) const {
        return factor_ / other.factor_;
    }

private:
    std::string name_;
    double factor_;
};

}  // namespace jaamsim::units
