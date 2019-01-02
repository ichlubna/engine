#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera
{
    public:
        glm::vec3 position{0.0f,0.0f,1.0f};
        glm::quat orientation;
        glm::mat4 getViewProjectionMatrix() const;
        float fov{glm::radians(45.0f)};
        float aspect{16.0f/9.0f};
        float near{0.1f};
        float far{100.0f};
        Camera() { update(); };
    private:
        glm::vec3 up;
        glm::vec3 right;
        glm::vec3 front;
        glm::vec3 worldUp{0.0f,1.0f,0.0};
        void update(); 
};
