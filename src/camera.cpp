#include "camera.h"

glm::mat4 Camera::getViewProjectionMatrix() const
{
    return glm::perspective(fov,aspect,near,far)*glm::lookAt(position, position+front, up);
}

void Camera::update()
{
    front = glm::vec3() * orientation;
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
