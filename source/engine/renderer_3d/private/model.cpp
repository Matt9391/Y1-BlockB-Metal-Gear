#include "renderer_3d/model.h"

namespace buz
{
	//--------------------------------------------------------------
	int model::find_node(const std::string& name) const
	{
		for (size_t index = 0; index < nodes.size(); ++index)
		{
			if (nodes[index].name == name) { return static_cast<int>(index); }
		}
		return -1;
	}

	//--------------------------------------------------------------
	const keyframe_clip* model::find_clip(const std::string& name) const
	{
		for (const keyframe_clip& clip : clips)
		{
			if (clip.name == name) { return &clip; }
		}
		return nullptr;
	}

	//--------------------------------------------------------------
	std::vector<node_transform> model::get_rest_pose() const
	{
		std::vector<node_transform> poses;
		poses.reserve(nodes.size());
		for (const model_node& node : nodes)
		{
			poses.push_back(node.local_transform);
		}
		return poses;
	}
}
