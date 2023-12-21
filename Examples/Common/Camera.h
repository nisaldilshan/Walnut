#pragma once
#include <Walnut/GLM/GLM.h>

namespace Camera
{
    

class PerspectiveCamera
{
public:
    PerspectiveCamera(float fov, float nearClip, float farClip);
    ~PerspectiveCamera() = default;

    void SetViewportSize(float width, float height);
    void OnUpdate();
    glm::mat4x4 GetViewMatrix();
    glm::mat4x4 GetProjectionMatrix() const;
private:
    void UpdateProjection();

    glm::mat4x4 m_view = glm::mat4(1.0f);
    glm::mat4x4 m_projection = glm::mat4(1.0f);

    float m_FOV = 0.0f;
    float m_nearClip = 0.0f;
    float m_farClip = 0.0f;

    float m_viewportWidth = 0;
    float m_viewportHeight = 0;
};


} // namespace Camera
