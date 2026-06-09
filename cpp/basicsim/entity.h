/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2002-2011 Ausenco Engineering Canada Inc.
 * Copyright (C) 2016-2026 JaamSim Software Inc.
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
 * Translated from Java: com.jaamsim.basicsim.Entity
 */

#ifndef JAAMSIM_BASICSIM_ENTITY_H
#define JAAMSIM_BASICSIM_ENTITY_H

#include <cstdint>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace jaamsim::basicsim {

// Forward declarations
class JaamSimModel;
class Simulation;

class Entity {
public:
    // =======================================================================
    // Event priority constants
    // =======================================================================

    static constexpr int PRI_HIGHEST = 0;
    static constexpr int PRI_HIGHER  = 1;
    static constexpr int PRI_HIGH    = 2;
    static constexpr int PRI_NORMAL  = 5;
    static constexpr int PRI_MED_LOW = 7;
    static constexpr int PRI_LOW     = 10;
    static constexpr int PRI_LOWER   = 11;
    static constexpr int PRI_LOWEST  = 99;

    // =======================================================================
    // Future event insertion rules
    // =======================================================================

    static constexpr bool EVT_FIFO = true;
    static constexpr bool EVT_LIFO = false;

    // =======================================================================
    // Input category labels
    // =======================================================================

    static constexpr const char* KEY_INPUTS   = "Key Inputs";
    static constexpr const char* OPTIONS      = "Options";
    static constexpr const char* GRAPHICS     = "Graphics";
    static constexpr const char* THRESHOLDS   = "Thresholds";
    static constexpr const char* MAINTENANCE  = "Maintenance";
    static constexpr const char* FONT         = "Font";
    static constexpr const char* FORMAT       = "Format";
    static constexpr const char* GUI          = "GUI";
    static constexpr const char* MULTIPLE_RUNS = "Multiple Runs";

    // =======================================================================
    // Entity flags (bitmask)
    // =======================================================================

    static constexpr int FLAG_TRACE      = 0x01;
    static constexpr int FLAG_ADDED      = 0x20;
    static constexpr int FLAG_EDITED     = 0x40;
    static constexpr int FLAG_GENERATED  = 0x80;
    static constexpr int FLAG_DEAD       = 0x0100;
    static constexpr int FLAG_REGISTERED = 0x0200;
    static constexpr int FLAG_RETAINED   = 0x0400;
    static constexpr int FLAG_POOLED     = 0x0800;

    // =======================================================================
    // Construction / destruction
    // =======================================================================

    Entity();
    virtual ~Entity() = default;

    // Non-copyable (entities have unique identity)
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    // Movable
    Entity(Entity&&) noexcept = default;
    Entity& operator=(Entity&&) noexcept = default;

    // =======================================================================
    // Identity
    // =======================================================================

    const std::string& name() const noexcept { return entity_name_; }
    uint64_t entity_number() const noexcept { return entity_number_; }

    void set_local_name(const std::string& name) { entity_name_ = name; }
    const std::string& local_name() const noexcept { return entity_name_; }

    // =======================================================================
    // Model access
    // =======================================================================

    JaamSimModel* sim_model() noexcept { return sim_model_; }
    const JaamSimModel* sim_model() const noexcept { return sim_model_; }
    Simulation* simulation();
    const Simulation* simulation() const;

    // =======================================================================
    // Parent / child
    // =======================================================================

    Entity* parent() noexcept { return parent_; }
    const Entity* parent() const noexcept { return parent_; }
    void set_parent(Entity* new_parent);

    void add_child(std::unique_ptr<Entity> child);
    const std::vector<std::unique_ptr<Entity>>& children() const noexcept {
        return children_;
    }

    // =======================================================================
    // Prototype / clone
    // =======================================================================

    Entity* prototype() noexcept { return prototype_; }
    const Entity* prototype() const noexcept { return prototype_; }
    void set_prototype(Entity* proto) { prototype_ = proto; }

    const std::vector<Entity*>& clone_list() const noexcept { return clone_list_; }
    void add_clone(Entity* clone);
    void remove_clone(Entity* clone);

    // =======================================================================
    // Flags
    // =======================================================================

    bool test_flag(int flag) const noexcept { return (flags_ & flag) != 0; }
    void set_flag(int flag) noexcept { flags_ |= flag; }
    void clear_flag(int flag) noexcept { flags_ &= ~flag; }
    int flags() const noexcept { return flags_; }

    bool is_added() const noexcept      { return test_flag(FLAG_ADDED); }
    bool is_generated() const noexcept  { return test_flag(FLAG_GENERATED); }
    bool is_registered() const noexcept { return test_flag(FLAG_REGISTERED); }
    bool is_retained() const noexcept   { return test_flag(FLAG_RETAINED); }
    bool is_dead() const noexcept       { return test_flag(FLAG_DEAD); }
    bool is_edited() const noexcept     { return test_flag(FLAG_EDITED); }
    bool is_pooled() const noexcept;

    bool trace() const noexcept { return test_flag(FLAG_TRACE); }
    void set_trace(bool on) noexcept {
        on ? set_flag(FLAG_TRACE) : clear_flag(FLAG_TRACE);
    }

    // =======================================================================
    // Active / lifecycle
    // =======================================================================

    bool is_active() const noexcept { return active_; }
    void set_active(bool on) noexcept { active_ = on; }

    virtual void early_init() {}
    virtual void late_init() {}
    virtual void start_up() {}
    virtual void do_end() {}
    virtual void close() {}
    virtual void clear_statistics() {}

    /// Kill this entity and all its children/clones.
    virtual void kill();

    /// Reverse a previous kill().
    virtual void restore();

    // =======================================================================
    // Validation / post-processing
    // =======================================================================

    virtual void validate();
    virtual void post_define();
    virtual void post_load() {}
    virtual void set_inputs_for_drag_and_drop() {}

    // =======================================================================
    // Attributes / custom outputs
    // =======================================================================

    const std::vector<std::string>& attribute_names() const noexcept {
        return attribute_names_;
    }
    void add_attribute(const std::string& name);

    // =======================================================================
    // Description
    // =======================================================================

    const std::string& description() const noexcept { return description_; }
    void set_description(const std::string& desc) { description_ = desc; }

protected:
    // -------------------------------------------------------------------
    // Fields
    // -------------------------------------------------------------------

    JaamSimModel* sim_model_ = nullptr;
    std::string entity_name_;
    uint64_t entity_number_ = 0;
    int flags_ = 0;

    Entity* parent_ = nullptr;
    std::vector<std::unique_ptr<Entity>> children_;

    Entity* prototype_ = nullptr;
    std::vector<Entity*> clone_list_;
    std::vector<Entity*> clone_pool_;

    static constexpr int MAX_POOL = 100;

    bool active_ = true;
    std::string description_;

    std::vector<std::string> attribute_names_;

private:
    // Implementation details to be provided in the .cpp translation unit.
    // Kept header-only for this simplified translation.
};

// ===========================================================================
// Inline implementations
// ===========================================================================

inline Entity::Entity()
    : entity_number_(0)  // would be assigned by JaamSimModel in full impl
    , flags_(0) {
}

inline void Entity::add_child(std::unique_ptr<Entity> child) {
    if (child) {
        child->parent_ = this;
        children_.push_back(std::move(child));
    }
}

inline void Entity::add_clone(Entity* clone) {
    clone_list_.push_back(clone);
}

inline void Entity::remove_clone(Entity* clone) {
    for (auto it = clone_list_.begin(); it != clone_list_.end(); ++it) {
        if (*it == clone) {
            clone_list_.erase(it);
            return;
        }
    }
}

inline bool Entity::is_pooled() const noexcept {
    return test_flag(FLAG_POOLED)
        || (parent_ != nullptr && parent_->is_pooled());
}

inline void Entity::set_parent(Entity* new_parent) {
    parent_ = new_parent;
}

inline void Entity::add_attribute(const std::string& name) {
    attribute_names_.push_back(name);
}

inline void Entity::validate() {
    // Base validation: override in subclasses
}

inline void Entity::post_define() {
    // Base post-definition: override in subclasses
}

inline void Entity::kill() {
    if (is_dead()) return;

    set_flag(FLAG_DEAD);

    for (auto& clone : clone_list_) {
        clone->kill();
    }
    clone_pool_.clear();

    if (prototype_ != nullptr)
        prototype_->remove_clone(this);

    for (auto& child : children_) {
        child->kill();
    }
}

inline void Entity::restore() {
    for (auto& child : children_) {
        child->restore();
    }
    for (auto* clone : clone_list_) {
        clone->restore();
    }

    clear_flag(FLAG_DEAD);

    if (prototype_ != nullptr)
        prototype_->add_clone(this);
}

}  // namespace jaamsim::basicsim

#endif  // JAAMSIM_BASICSIM_ENTITY_H
