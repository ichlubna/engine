#ifndef WINDOW_ABSTRACT_HDR
#define WINDOW_ABSTRACT_HDR

//abstract class defining the needed GPU actions
//can be inherited by various GPU APIs such as Vulkan, OpenGL, DirectX...
#include "window.h"
#include "assets.h"

class Gpu
{
	public:
		virtual void render() = 0;
		virtual void updateViewProjectionMatrix(glm::mat4 vpMatrix) = 0;
        virtual void addModel(std::shared_ptr<Assets::Model> model) = 0;
		Gpu(Window *w) : windowPtr{w} {};
	protected:
		Window *windowPtr;
};

#endif
