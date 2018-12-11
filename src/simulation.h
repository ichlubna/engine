#include <memory>
#include "window.h"
#include "gpu.h"

class Simulation
{
	public:
		enum GpuAPI {GPU_VULKAN, GPU_OPENGL};
		enum WindowAPI {WINDOW_GLFW, WINDOW_QT};

		void run();
		Simulation(GpuAPI api = GPU_VULKAN, WindowAPI = WINDOW_GLFW);
	private:
        bool end{false};
		std::unique_ptr<Window> window;
		std::unique_ptr<Gpu> gpu;	
        void processInputs();
};
