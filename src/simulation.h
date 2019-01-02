#include "window.h"
#include "camera.h"
#include "gpu.h"

class Simulation
{
	public:
		enum GpuAPI {GPU_VULKAN, GPU_OPENGL};
		enum WindowAPI {WINDOW_GLFW, WINDOW_QT};

		void run();
		Simulation(GpuAPI api = GPU_VULKAN, WindowAPI = WINDOW_GLFW);
	private:
        //the interval of one simulation step in seconds
        const double stepSize{0.01};
        //if enabled, one frame takes the stepSize time, else the fps is unbound
        //TODO test if physics works the same on both
        bool fixedFPS{false}; 
        bool end{false};
		std::unique_ptr<Window> window;
		std::unique_ptr<Gpu> gpu;	
		std::unique_ptr<Assets> assets;	
		std::unique_ptr<Camera> camera;	
        void processInputs();
        void step();
};
