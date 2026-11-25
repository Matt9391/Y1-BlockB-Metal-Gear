# Math, logging, asserts, json, random numbers and file paths: the basics
# every other module builds on. glm and nlohmann are public because math.h
# and json.h expose their types. The 3D template adds triangles on the cpu
# (mesh_data, the built-in shapes) and texture_id, which names an image, such
# as one inside a model file, without loading it onto the graphics card.
BUZ_MODULE(core
           PUBLIC_DEPS glm nlohmann
           PRIVATE_DEPS rnd stb)

# Stop windows.h from defining min/max macros, which would break glm::min,
# std::min and friends. PUBLIC so game code gets the same treatment.
TARGET_COMPILE_DEFINITIONS(buz_core PUBLIC NOMINMAX)

# The public headers use C++20. PUBLIC, so everything that links the engine
# compiles as C++20 too, also a game project built against the binary
# package, which does not get the root CMakeLists.txt's CMAKE_CXX_STANDARD.
TARGET_COMPILE_FEATURES(buz_core PUBLIC cxx_std_20)
