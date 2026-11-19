#pragma once

#include "core/math.h"
#include "renderer_3d/model.h"
#include "animation/animator.h"

#include <vector>

namespace buz{

    class world_renderer_3d;
}

class character{
public:

    character(const buz::model& model, buz::vec3 pos);
    ~character() = default;

    void turn_toward(buz::vec3 intent, float dt);

    void set_yaw(float yaw);
    void set_pos(buz::vec3 new_pos);
    void set_size(buz::vec3 new_size);

    buz::vec3 get_forward() const;
    float get_yaw() const;
    buz::vec3 get_pos() const;
    buz::vec3 get_size() const;
    const buz::model& get_model() const;
    animator& get_animator();

    void draw_3d(buz::world_renderer_3d& world_3d) const;

private:

    float m_yaw_degrees{0};
    float m_yaw_turning_speed_degrees{1.f};
    buz::vec3 m_pos;
    buz::vec3 m_size;

    const buz::model& m_model;
    
    animator m_animator;

};