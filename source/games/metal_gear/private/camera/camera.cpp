#include "camera.h"

camera::camera(float fov, buz::vec3 base_offset) :
    m_fov{fov},
    m_offset{base_offset},
    m_target{0.f, 0.f, 0.f},
    m_near_limit{0.1f},
    m_far_limit{200.f}
{

}

void camera::set_target(const buz::vec3& target){
    m_target = target;
}


buz::mat4 camera::get_view_matrix() const{
    buz::vec3 eye = m_target + m_offset;
    buz::mat4 view_matrix = glm::lookAtRH(eye, m_target, buz::vec3{0.f, 1.f, 0.f});
    
    return view_matrix;
}
buz::mat4 camera::get_projx_matrix(buz::vec2 window_size) const{
    buz::mat4 projx_matrix = glm::perspectiveFovRH_ZO(buz::radians(m_fov), window_size.x, window_size.y, m_near_limit, m_far_limit);
    return projx_matrix;
}

void camera::set_offset(const buz::vec3& offset){
    m_offset = offset;
}