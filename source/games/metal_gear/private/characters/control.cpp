#include "control.h"

#include "engine/input.h"

namespace player_control{
    buz::vec3 compute_direction(const buz::input& input, const buz::vec3 forward, const buz::vec3 right){
        int w = input.is_key_down(buz::key::w);
        int s = input.is_key_down(buz::key::s);
        int a = input.is_key_down(buz::key::a);
        int d = input.is_key_down(buz::key::d);

        float z_direction = static_cast<float>(w - s);
        float x_direction = static_cast<float>(d - a);
        buz::vec3 dir = forward * z_direction + right * x_direction;

        if(buz::length(dir) != 0){
            dir = buz::normalize(dir);
        }

        return dir;
    }
}