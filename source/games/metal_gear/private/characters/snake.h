#pragma once

#include "characters/character.h"

namespace buz{
    class input;
};

class snake
{
public:
	snake(const buz::model& model, buz::vec3 pos);
	~snake() = default;

	void on_update(const buz::input& input, const buz::vec3 forward, const buz::vec3 right);

	const character& get_character() const;

private:
	character m_character;
};
