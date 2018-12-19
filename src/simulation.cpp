#include "simulation.h"
#include "gpuVulkan.h"
#include "windowGlfw.h"

Simulation::Simulation(GpuAPI gpuApi, WindowAPI windowApi) : assets{std::make_unique<Assets>()}
{
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

void Simulation::processInputs()
{
    const Inputs& inputs = window->getInputs();

    if(inputs.pressed(Inputs::Key::ESC))
        end = true;
    else if(inputs.pressed(Inputs::ALT, Inputs::ENTER))
        window->switchFullscreen();

    if(inputs.close)
        end = true;
}

void Simulation::run() 
{
    auto model = assets->loadModel("../assets/geometry/box.obj");
    gpu->addModel(model); 
    
	while(!end)
	{
        processInputs(); 
        gpu->render();
	}
}

