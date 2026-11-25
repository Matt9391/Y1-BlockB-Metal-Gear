#pragma once

#include "renderer_3d/model.h"

#include <vector>

class animator
{
public:
	animator(const std::vector<buz::node_transform>& rest_pose);
	~animator() = default;

    void set_animation(const buz::keyframe_clip* clip);
	const std::vector<buz::node_transform>& get_pose() const;

	void play_animation(float dt_s);
private:
	std::vector<buz::node_transform> blend_pose(std::vector<buz::node_transform> start_pose, const std::vector<buz::node_transform>& end_pose, float t);
	void set_pose(const std::vector<buz::node_transform>& new_pose);

	const buz::keyframe_clip* m_clip;
	std::vector<buz::node_transform> m_pose;
	
	int m_current_frame;

    float m_clip_duration;
	float m_elapsed_time;
};
