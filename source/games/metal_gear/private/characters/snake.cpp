#include "snake.h"

snake::snake(const buz::model& model, buz::vec3 pos)
    : m_character(model, pos)
{
}

void snake::on_update()
{
}

const character& snake::get_character() const
{
    return m_character;
}