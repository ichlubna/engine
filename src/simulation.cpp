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

void Simulation::processinputs(Window::Inputs inputs)
{
    if(window->inputs.pressed(Inputs::ALT, Inputs::ENTER)))
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

