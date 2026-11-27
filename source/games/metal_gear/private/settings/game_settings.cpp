#include "game_settings.h"
#include "core/json.h"
#include "core/log.h"
#include "core/assert.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(snake_settings, spawn_position,  walk_speed_m_per_s, turn_speed_deg_per_s)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(guard_settings,  walk_speed_m_per_s, turn_speed_deg_per_s, arrival_radius, idle_time_s)

game_settings::game_settings(std::string_view path)
{   
    
    buz::json file = buz::load_json(path);

    if(file.is_discarded()){
        buz::log::error("[game_setttings] Json file \"{}\" contains invalid json or its not found", path);
        buz::fatal_error("[game_setttings] Json file \"{}\" contains invalid json or its not found", path);
        return;
    }
    m_snake_settings = file.at("snake").get<snake_settings>();  
    m_guard_settings = file.at("guard").get<guard_settings>();  
}

const snake_settings& game_settings::get_snake() const { return m_snake_settings; }
const guard_settings& game_settings::get_guard() const { return m_guard_settings; }