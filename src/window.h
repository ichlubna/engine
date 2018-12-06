#ifndef GPU_ABSTRACT_HDR
#define GPU_ABSTRACT_HDR
#include <vector>

class Window
{
	public:
		struct WinSize
		{
			int width;
			int height;
		};
		struct Inputs
		{
			enum Keys {	ESC = 1,
				    LMB = 2, RMB = 4, MMB = 8,
					W = 16, A = 32, S = 64, D = 128,
                    ALT = 256, ENTER = 512};
			//expecting max 32 keys, plus possible modifiers
			long keys {0};
			double mouseX {0}, mouseY {0}, mouseScroll {0};
			bool close {false};
		};

		virtual const Inputs& checkInputs() = 0;
		virtual void getVulkanSurface(void *instance, void *surface) const = 0;
		virtual void addRequiredWindowExt(std::vector<const char*> &extensions) const = 0;
        virtual WinSize getFramebufferSize() const = 0;	
        virtual void switchFullscreen() = 0;

		//WinSize getSize() const {return {width, height};}
		Window(unsigned int w, unsigned int h) : width{w}, height{h} {};
	protected:
		Inputs inputs;
		unsigned int width;
		unsigned int height;
};

#endif
