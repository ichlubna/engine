#ifndef WINDOW_ABSTRACT_HDR
#define WINDOW_ABSTRACT_HDR

//abstract class defining the needed GPU actions
//can be inherited by various GPU APIs such as Vulkan, OpenGL, DirectX...
#include "window.h"

class Gpu
{
	public:
        //TODO multiple shaders/pipelines
		virtual void render() = 0;
		Gpu(Window *w) : windowPtr{w} {};
	protected:
		Window *windowPtr;
};

#endif
