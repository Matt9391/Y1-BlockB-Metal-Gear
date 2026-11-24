#pragma once

#include "characters/character.h"
#include "collision/collider.h"
#include <vector>

namespace buz{
    class input;
};

struct snake_settings;

class snake
{
public:
	snake(const buz::model& model, const snake_settings& settings);
	~snake() = default;

	void on_update(float dt, const buz::vec3& dir,  const std::vector<collider::box>& boxes);

	const character& get_character() const;

private:
	character m_character;
	float m_movement_speed;
};
