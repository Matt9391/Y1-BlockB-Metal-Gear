#include "game_math.h"

namespace game_math
{
    buz::vec3 flat_vector_to(const buz::vec3& from, const buz::vec3& to) {
        buz::vec3 from_to = to - from;
        from_to.y = 0;
        return from_to;
    }
} 