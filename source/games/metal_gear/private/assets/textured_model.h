#pragma once

#include "renderer_3d/model.h"
#include "render_core/texture.h"

#include <vector>
#include <memory>

struct textured_model{
	buz::model model;
	std::vector<std::shared_ptr<buz::texture>> textures;
};
