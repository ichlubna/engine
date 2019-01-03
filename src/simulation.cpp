#include <chrono>
#include <thread>
#include "simulation.h"
#include "gpuVulkan.h"
#include "windowGlfw.h"
#include<iostream>
Simulation::Simulation(GpuAPI gpuApi, WindowAPI windowApi) : assets{std::make_unique<Assets>()}, camera{std::make_unique<Camera>()}
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
double previousX = 0.0, previousY = 0.0;
void Simulation::processInputs()
{
    const Inputs& inputs = window->getInputs();

    //FPS Camera
    float mouseSensitivity = 0.001;
    Inputs::mousePosition mp = inputs.getMousePosition();
    double relativeX = mp.x - previousX, relativeY = mp.y - previousY;
    previousX = mp.x, previousY = mp.y;
    camera->turn(relativeY*mouseSensitivity, relativeX*mouseSensitivity);
    relativeX = relativeY = 0.0;
    float cameraSpeed = 0.1;
    if(inputs.pressed(Inputs::W))
        camera->move(Camera::Direction::FRONT, cameraSpeed); 
    if(inputs.pressed(Inputs::S))
        camera->move(Camera::Direction::BACK, cameraSpeed); 
    if(inputs.pressed(Inputs::A))
        camera->move(Camera::Direction::LEFT, cameraSpeed);
    if(inputs.pressed(Inputs::D))
        camera->move(Camera::Direction::RIGHT, cameraSpeed);
 
    if(inputs.pressed(Inputs::Key::ESC))
        end = true;
    if(inputs.pressed(Inputs::ALT, Inputs::ENTER))
        window->switchFullscreen();

    if(inputs.close)
        end = true;
}

void Simulation::step()
{
    processInputs();
    //physics(stepSize)
}

void Simulation::run() 
{
    auto model = assets->loadModel("../assets/geometry/boxREAL.obj");
    gpu->addModel(model); 

    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> remainingTime{0.0};
    std::chrono::duration<double> stepTime{stepSize};
   
    if(fixedFPS)
        while(!end)
        {        
            auto currentTime = std::chrono::high_resolution_clock::now();
            step(); 
            gpu->render();
            auto endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> frameTime = endTime - currentTime;
            if(frameTime < stepTime)
                std::this_thread::sleep_for(stepTime-frameTime); 
        }
    else 
        while(!end)
        {
            auto newTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> frameTime = newTime - currentTime;
            currentTime = newTime;
            remainingTime += frameTime;

            while (remainingTime >= stepTime)
            {   
                step(); 
                remainingTime -= stepTime;
            }
            
            gpu->updateViewProjectionMatrix(camera->getViewProjectionMatrix());
            gpu->render();
        }
}

