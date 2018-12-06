#include "simulation.h"
#include "gpuVulkan.h"
#include "windowGlfw.h"

Simulation::Simulation(GpuAPI gpuApi, WindowAPI windowApi)
{
	//probably not needed
	//const Window::Extensions windowExt = window->getRequiredExtensions();
	switch(windowApi)
	{
		case WINDOW_GLFW:
			window = std::make_unique<WindowGlfw>(1600,900);
		break;
		
		default:
			throw std::runtime_error("Selected window API not implemented.");
		break;
	}

	switch(gpuApi)
	{
		case GPU_VULKAN:
			gpu = std::make_unique<GpuVulkan>(window.get());
		break;
		
		default:
			throw std::runtime_error("Selected GPU API not implemented.");
		break;
	}	
}

//TODO make it a class member of Inputs
//Standalone class
template <typename... Args>
bool keysPressed(long keys, Args... args) {
  return keys & composeKeys(args...);
}
template <typename T, typename... Args>
bool composeKeys(T keyFlag, Args... args) {
  return keyFlag | composeKeys(args...);
}
template <typename T>
bool composeKeys(T t) {
    return t;
}

void Simulation::processinputs(Window::Inputs inputs)
{
    if(keysPressed(inputs.keys, inputs.ALT, inputs.ENTER))
        window->switchFullscreen();
}

void Simulation::run() 
{
	bool close = false;
	while(!close)
	{
        Window::Inputs inputs = window->checkInputs();
		close = inputs.close;
        
        gpu->render();
	}
}

