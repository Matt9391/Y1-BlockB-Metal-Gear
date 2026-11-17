#pragma once

#include "core/math.h"
#include "renderer_3d/model.h"

#include <vector>

namespace buz{

    class world_renderer_3d;
}

class character{
public:

    character(const buz::model& model, buz::vec3 pos);
    ~character() = default;


    void set_yaw(float yaw);
    void set_pose(const std::vector<buz::joint_pose>& new_pose);
    void set_pos(buz::vec3 new_pos);

    buz::vec3 get_forward() const;
    float get_yaw() const;
    buz::vec3 get_pos() const;
    const buz::model& get_model() const;
    const std::vector<buz::joint_pose>& get_current_pose() const;

    void draw_3d(buz::world_renderer_3d& world_3d) const;

private:

    float m_yaw{0};
    buz::vec3 m_pos;

    const buz::model& m_model;
    std::vector<buz::joint_pose> m_current_pose;


};