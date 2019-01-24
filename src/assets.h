#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>

class Assets
{
	public:
        //TODO measure difference in GPU performance when Vertex size is power of 2
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
        
        struct Texture
        {
            unsigned int width;
            unsigned int height;
            static const unsigned int BYTES_PER_PIXEL = 4;
            //RGBA 8 bit values
            std::vector<char> pixels;
        };

    [[nodiscard]] std::shared_ptr<Model> loadModel(const char *path) const;
    [[nodiscard]] std::shared_ptr<Texture> loadTexture(const char *path) const;
	private:
        int32_t packNormals(glm::vec3 normal) const;
        
};
