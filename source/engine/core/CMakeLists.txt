# Math, logging, asserts, json, random numbers and file paths: the basics
# every other module builds on. glm and nlohmann are public because math.h
# and json.h expose their types.
BUZ_MODULE(core TIER tmpl_2d
           PUBLIC_DEPS glm nlohmann
           PRIVATE_DEPS rnd stb)

# Stop windows.h from defining min/max macros, which would break glm::min,
# std::min and friends. PUBLIC so game code gets the same treatment.
TARGET_COMPILE_DEFINITIONS(buz_core PUBLIC NOMINMAX)
