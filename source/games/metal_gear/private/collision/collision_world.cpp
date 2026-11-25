#include "collision_world.h"
#include "collision_math.h"

collision_world::collision_world(const std::vector<collider::box> boxes)
	: m_boxes(boxes)
{
}

const std::vector<collider::box>& collision_world::get_boxes() const
{
	return m_boxes;
}

buz::vec3 collision_world::resolve_move(const collider::box& box, buz::vec3 displacement) const{
    return collision_math::resolve_boxes(box.centre, box.size, displacement, m_boxes);
}