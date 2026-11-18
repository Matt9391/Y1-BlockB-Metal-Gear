#pragma once

#include "characters/character.h"
#include "collision/collider.h"
#include <vector>

namespace buz{
    class input;
};
class snake
{
public:
	snake(const buz::model& model, buz::vec3 pos);
	~snake() = default;

	void on_update(float dt, const buz::vec3& dir,  const std::vector<collider::box>& boxes);

	const character& get_character() const;

private:
	character m_character;
	float m_movement_speed;
};
