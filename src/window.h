#ifndef GPU_ABSTRACT_HDR
#define GPU_ABSTRACT_HDR
#include <vector>
#include "inputs.h"

class Window
{
	public:
		struct WinSize
		{
			int width;
			int height;
		};

		Inputs inputs;

		virtual checkInputs() = 0;
		virtual void getVulkanSurface(void *instance, void *surface) const = 0;
		virtual void addRequiredWindowExt(std::vector<const char*> &extensions) const = 0;
        virtual WinSize getFramebufferSize() const = 0;	
        virtual void switchFullscreen() = 0;

		//WinSize getSize() const {return {width, height};}
		Window(unsigned int w, unsigned int h) : width{w}, height{h} {};
	protected:
		unsigned int width;
		unsigned int height;
};

#endif
