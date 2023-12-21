#include "Camera.h"

namespace Camera
{

PerspectiveCamera::PerspectiveCamera(float fov, float nearClip, float farClip)
    : m_FOV(fov)
    , m_nearClip(nearClip)
    , m_farClip(farClip)
{}

void PerspectiveCamera::SetViewportSize(float width, float height) 
{ 
    m_viewportWidth = width; 
    m_viewportHeight = height; 
    UpdateProjection(); 
}

void PerspectiveCamera::OnUpdate()
{
}

glm::mat4x4 PerspectiveCamera::GetViewMatrix() 
{ 
    m_view = glm::lookAt(glm::vec3(-2.0f, -3.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0, 0, 1));
    return m_view; 
}

glm::mat4x4 PerspectiveCamera::GetProjectionMatrix() const 
{ 
    return m_projection; 
}

void PerspectiveCamera::UpdateProjection()
{
    m_projection = glm::perspective(glm::radians(m_FOV), m_viewportWidth / m_viewportHeight, m_nearClip, m_farClip);
}
    
} // namespace Camera


