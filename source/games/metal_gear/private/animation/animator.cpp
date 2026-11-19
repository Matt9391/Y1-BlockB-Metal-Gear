#include "animator.h"
#include <cmath>

animator::animator(const std::vector<buz::joint_pose>& rest_pose)
	: m_pose(rest_pose)
    , m_clip{nullptr}
    , m_current_frame{0}
    , m_elapsed_time{0.0f}
{
}

void animator::set_pose(const std::vector<buz::joint_pose>& new_pose)
{
    m_pose = new_pose;

}

void animator::set_animation(const buz::keyframe_clip* clip){
    m_clip = clip;
    m_current_frame = 0;
    m_clip_duration = (m_clip->frames.size() - 1) / m_clip->frames_per_second;
    set_pose(m_clip->frames[m_current_frame]);
}


const std::vector<buz::joint_pose>& animator::get_pose() const
{
    return m_pose;
}

void animator::play_animation(float dt_s) 
{
    if(!m_clip) return;

    m_elapsed_time += dt_s;

    m_elapsed_time = std::fmod(m_elapsed_time, m_clip_duration);
    m_current_frame = int(m_elapsed_time * m_clip->frames_per_second);

    set_pose(m_clip->frames[m_current_frame]);

}
    