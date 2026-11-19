#include "animator.h"

animator::animator(const std::vector<buz::joint_pose>& rest_pose)
	: m_pose(rest_pose)
    , m_clip{nullptr}
{
    m_timer.reset();
}

void animator::set_pose(const std::vector<buz::joint_pose>& new_pose)
{
    m_pose = new_pose;

}

void animator::set_animation(const buz::keyframe_clip* clip){
    m_clip = clip;
}


const std::vector<buz::joint_pose>& animator::get_pose() const
{
    return m_pose;
}

void animator::play_animation() 
{
    if(!m_clip) return;

    if(m_timer.elapsed_seconds() > k_frame_duration){
        m_current_frame = (m_current_frame + 1) % m_clip->frames.size();
        
        set_pose(m_clip->frames[m_current_frame]);

        m_timer.reset();

    }
}
    