#include "game_settings.h"
#include "core/json.h"
#include "core/log.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(snake_settings, spawn_position,  walk_speed_m_per_s, turn_speed_deg_per_s)

game_settings::game_settings(std::string_view path)
{   
    
    buz::json file = buz::load_json(path);

    if(file.is_discarded()){
        buz::log::error("[game_setttings] Json file \"{}\" contains invalid json", path);
        return;
    }
    m_snake_settings = file.at("snake").get<snake_settings>();  
}

const snake_settings& game_settings::get_snake() const { return m_snake_settings; }