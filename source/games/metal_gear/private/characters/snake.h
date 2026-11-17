#pragma once

#include "characters/character.h"

class snake
{
public:
	snake(const buz::model& model, buz::vec3 pos);
	~snake() = default;

	void on_update();

	const character& get_character() const;

private:
	character m_character;
};
