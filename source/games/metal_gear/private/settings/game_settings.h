#pragma once

#include <string_view>

#include "setting_types.h"


class game_settings
{
public:
    game_settings(std::string_view path);

    const snake_settings& get_snake() const;

private:
    snake_settings m_snake_settings;
};
