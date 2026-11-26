#pragma once

#include "characters/character.h"
#include "collision/collider.h"
#include <vector>

namespace buz{
    class input;
};

struct snake_settings;
struct textured_model;
class collision_world;

class snake
{
public:
	snake(const textured_model& textured_model, const snake_settings& settings);
	~snake() = default;

	void on_update(float dt, const buz::vec3& dir, const collision_world& world);

	const character& get_character() const;

private:
	character m_character;
};
