#pragma once

#include "core/math.h"
#include "renderer_3d/model.h"
#include "animation/animator.h"

#include <vector>
#include <array>
#include <string>

namespace buz{

    class world_renderer_3d;
}

enum class animations{
	idle,
	walk,
	count
};

struct textured_model;
class collision_world;

class character{
public:

    character(const textured_model& textured_model, buz::vec3 pos, float turn_speed_deg_per_s);
    ~character() = default;

    void turn_toward(buz::vec3 intent, float dt);

    void set_yaw(float yaw);
    void set_pos(buz::vec3 new_pos);
    void set_size(buz::vec3 new_size);

    void set_animation(const animations& animation);

    void move(buz::vec3 displacement, const collision_world& world);

    buz::vec3 get_forward() const;
    float get_yaw() const;
    buz::vec3 get_pos() const;
    buz::vec3 get_centre() const;
    buz::vec3 get_size() const;
    const buz::model& get_model() const;
    animator& get_animator();

    void draw_3d(buz::world_renderer_3d& world_3d) const;

private:

    float m_yaw_degrees{0};
    float m_yaw_turning_speed_degrees{1.f};
    buz::vec3 m_pos;
    buz::vec3 m_size;

    const textured_model& m_textured_model;
    
	std::array<std::string, static_cast<std::size_t>(animations::count)> m_animation_names{"idle", "walk"};

    animator m_animator;

};