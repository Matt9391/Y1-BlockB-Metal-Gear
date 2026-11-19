#pragma once

#include "renderer_3d/model.h"
#include "core/timer.h"

#include <vector>

class animator
{
public:
	animator(const std::vector<buz::joint_pose>& rest_pose);
	~animator() = default;

    void set_animation(const buz::keyframe_clip* clip);
	const std::vector<buz::joint_pose>& get_pose() const;

	void play_animation();
private:

	void set_pose(const std::vector<buz::joint_pose>& new_pose);

	int m_current_frame;
	const buz::keyframe_clip* m_clip;
	std::vector<buz::joint_pose> m_pose;

    const float k_frame_duration{0.03f};
	buz::timer m_timer;
};
