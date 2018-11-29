#include <stdexcept>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> 
#include <iterator>
#include "windowGlfw.h"

const WindowGlfw::Inputs& WindowGlfw::checkInputs()
{
	glfwPollEvents();
	inputs.close = (glfwWindowShouldClose(window) || inputs.keys & Inputs::KEY_ESC);

	return inputs;
}
/*#include <string.h>
const WindowGlfw::Extensions WindowGlfw::getRequiredExtensions() const
{
	Extensions ext;
	strcpy(ext.names, *glfwGetRequiredInstanceExtensions(&ext.count));
	std::cout << ext.names;
	return ext;	
}
*/

void WindowGlfw::getVulkanSurface(void *instance ,void *surface) const
{
	if(glfwCreateWindowSurface(*(VkInstance*)instance, window, nullptr, (VkSurfaceKHR*) surface) != VK_SUCCESS)
		throw std::runtime_error("Cannot create surface."); 
}

void WindowGlfw::addRequiredWindowExt(std::vector<const char*> &extensions) const
{
	unsigned int count;
	const char** requiredExt = glfwGetRequiredInstanceExtensions(&count);
	for (unsigned int i=0; i<count; i++)
		extensions.push_back(requiredExt[i]);
}

WindowGlfw::WindowGlfw(unsigned int w, unsigned int h) : Window{w,h}
{
	glfwSetErrorCallback([]([[maybe_unused]]int error, const char* description){throw std::runtime_error(description);});
	if(!glfwInit())
		throw std::runtime_error("Cannot init GLFW.");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(width, height, "Engine", nullptr, nullptr);
	if(!window)	
		throw std::runtime_error("Cannot create window (GLFW).");
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED | GLFW_CURSOR_HIDDEN);


	glfwSetWindowUserPointer(window, this);

	glfwSetKeyCallback(window, 
		[]([[maybe_unused]]GLFWwindow *window, int key, [[maybe_unused]]int scancode, int action, [[maybe_unused]]int mods)
		{
			WindowGlfw *thisWindowGlfw = reinterpret_cast<WindowGlfw*>(glfwGetWindowUserPointer(window));
			bool set = (action == GLFW_PRESS);
			long flag = 0;	

			switch(key)
			{
				case GLFW_KEY_ESCAPE:
					flag = Inputs::KEY_ESC;
				break;	
				
				case GLFW_KEY_W:
					flag = Inputs::KEY_W;
				break;

				case GLFW_KEY_A:
					flag = Inputs::KEY_A;
				break;

				case GLFW_KEY_S:
					flag = Inputs::KEY_S;
				break;

				case GLFW_KEY_D:
					flag = Inputs::KEY_D;
				break;	
				
				default:
				break;
			}
		
			if(set)	
				thisWindowGlfw->inputs.keys |= flag;
			else
				thisWindowGlfw->inputs.keys &= ~flag;
		});
	
	glfwSetMouseButtonCallback(window, 
		[]([[maybe_unused]]GLFWwindow *window, int button, int action, [[maybe_unused]]int mods)
		{
			WindowGlfw *thisWindowGlfw = reinterpret_cast<WindowGlfw*>(glfwGetWindowUserPointer(window));
			bool set = (action == GLFW_PRESS);
			long flag = 0;	

			switch(button)
			{
				case GLFW_MOUSE_BUTTON_LEFT:
					flag = Inputs::KEY_LMB;
				break;	
				
				case GLFW_MOUSE_BUTTON_RIGHT:
					flag = Inputs::KEY_RMB;
				break;
	
				case GLFW_MOUSE_BUTTON_MIDDLE:
					flag = Inputs::KEY_MMB;
				break;		

				default:
				break;
			}
		
			if(set)	
				thisWindowGlfw->inputs.keys |= flag;
			else
				thisWindowGlfw->inputs.keys &= ~flag;
		});

	glfwSetCursorPosCallback(window,
		[](GLFWwindow *window, double xPos, double yPos)
		{
			WindowGlfw *thisWindowGlfw = reinterpret_cast<WindowGlfw*>(glfwGetWindowUserPointer(window));
			thisWindowGlfw->inputs.mouseX = xPos;				
			thisWindowGlfw->inputs.mouseY = yPos;				
		});
	
	glfwSetScrollCallback(window,
		[](GLFWwindow *window, [[maybe_unused]]double xOffset, double yOffset)
		{
			WindowGlfw *thisWindowGlfw = reinterpret_cast<WindowGlfw*>(glfwGetWindowUserPointer(window));
			thisWindowGlfw->inputs.mouseScroll = yOffset;
		});	
}

WindowGlfw::~WindowGlfw()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}
