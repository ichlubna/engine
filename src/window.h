#ifndef GPU_ABSTRACT_HDR
#define GPU_ABSTRACT_HDR
#include <vector>

class Window
{
	public:
		struct WinSize
		{
			unsigned int width;
			unsigned int height;
		};
		struct Inputs
		{
			enum KeyFlags {	KEY_ESC = 1,
					KEY_LMB = 2, KEY_RMB = 4, KEY_MMB = 8,
					KEY_W = 16, KEY_A = 32, KEY_S = 64, KEY_D = 128};
			//expecting max 32 keys, plus possible modifiers
			long keys {0};
			double mouseX {0}, mouseY {0}, mouseScroll {0};
			bool close {false};
		};

		virtual const Inputs& checkInputs() = 0;
		virtual void getVulkanSurface(void *instance, void *surface) const = 0;
		virtual void addRequiredWindowExt(std::vector<const char*> &extensions) const = 0;

		WinSize getSize() const {return {width, height};}		
		Window(unsigned int w, unsigned int h) : width{w}, height{h} {};
	protected:
		Inputs inputs;
		unsigned int width;
		unsigned int height;
};

#endif
