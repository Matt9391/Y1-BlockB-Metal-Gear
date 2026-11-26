#pragma once

#include "guard_brain.h"
#include "character.h"


struct guard_unit
{
    guard_brain brain;
    character body;
};
