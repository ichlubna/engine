#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>

class Assets
{
	public:
        struct Vertex
        {
            glm::vec3 position;
            //2bits free and xyz - 10bits per coord
            int32_t normal;
            uint32_t uv; 
        }; 
        struct Model
        {
            std::vector<Vertex> vertices;
            std::vector<uint16_t> indices;
        };

    std::shared_ptr<Model> loadModel(char *path);
	private:
        int32_t packNormals(glm::vec3 normal) const;
        
};
