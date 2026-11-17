#include "snake.h"
#include "engine/input.h"
#include "core/log.h"

snake::snake(const buz::model& model, buz::vec3 pos)
    : m_character(model, pos)
{
}

void snake::on_update(const buz::input& input, const buz::vec3 forward, const buz::vec3 right)
{
    int w = input.is_key_down(buz::key::w);
    int s = input.is_key_down(buz::key::s);
    int a = input.is_key_down(buz::key::a);
    int d = input.is_key_down(buz::key::d);

    float vertical = static_cast<float>(w - s);
    float horizontal = static_cast<float>(d - a);
    buz::vec3 intent = forward * vertical + right * horizontal;

    buz::log::info("x: {}, y: {}, z: {}", intent.x, intent.y, intent.z);

    m_character.set_pos(m_character.get_pos() + intent / 10.f);

}

const character& snake::get_character() const
{
    return m_character;
}