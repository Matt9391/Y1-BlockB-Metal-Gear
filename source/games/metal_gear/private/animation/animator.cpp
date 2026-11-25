#include "animator.h"
#include <cmath>

animator::animator(const std::vector<buz::joint_pose>& rest_pose)
	: m_clip{ nullptr }
	, m_pose(rest_pose)
	, m_current_frame{ 0 }
	, m_clip_duration{ 0.0f }
	, m_elapsed_time{ 0.0f }
{
}

void animator::set_pose(const std::vector<buz::joint_pose>& new_pose)
{
	m_pose = new_pose;
}

void animator::set_animation(const buz::keyframe_clip* clip)
{	
	if(clip == nullptr) return;
	if(m_clip == clip) return;

	m_clip = clip;
	m_current_frame = 0;
	m_elapsed_time = 0.f;
	m_clip_duration = (m_clip->frames.size() - 1) / m_clip->frames_per_second;
	set_pose(m_clip->frames[m_current_frame]);
}

const std::vector<buz::joint_pose>& animator::get_pose() const
{
	return m_pose;
}

//TODO: implement catmull spline
//start will be the one that gets edited and returned so it has to be copy
std::vector<buz::joint_pose> animator::blend_pose(std::vector<buz::joint_pose> start_pose, const std::vector<buz::joint_pose>& end_pose, float t)
{
	assert(start_pose.size() == end_pose.size() && "poses sizes mismatch");

    
	for (size_t i = 0; i < start_pose.size(); i++)
	{

        buz::joint_pose& start_joint = start_pose[i];
        const buz::joint_pose& end_joint = end_pose[i];

        start_joint.translation = glm::mix(start_joint.translation, end_joint.translation, t);
        start_joint.rotation = glm::slerp(start_joint.rotation, end_joint.rotation, t);
        start_joint.scale = glm::mix(start_joint.scale, end_joint.scale, t);

	}

    return start_pose;
}

void animator::play_animation(float dt_s)
{
	if (!m_clip)
	{
		return;
	}

	m_elapsed_time += dt_s;

	m_elapsed_time = std::fmod(m_elapsed_time, m_clip_duration);
	m_current_frame = int(m_elapsed_time * m_clip->frames_per_second);
	int next_frame = m_current_frame + 1;

    assert(next_frame < m_clip->frames.size() && "next frame is out of bounds"); //this should never happen
    next_frame = buz::clamp(next_frame, 0, int(m_clip->frames.size() - 1));

    float t = m_elapsed_time * m_clip->frames_per_second - m_current_frame; //t = i.t - i
	set_pose(blend_pose(m_clip->frames[m_current_frame], m_clip->frames[next_frame], t)); //smooth animationnnn
	// set_pose(m_clip->frames[m_current_frame]);
}
