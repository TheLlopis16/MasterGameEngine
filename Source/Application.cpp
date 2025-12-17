#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleD3D12.h"
#include "Exercise1.h"
#include "Exercise2.h"
#include "Exercise3.h"
#include "Exercise4.h"
#include "Assignment1.h"
#include "ModuleEditor.h"
#include "ModuleResources.h"
#include "ModuleShaderDescriptors.h"
#include "ModuleSampler.h"
#include "ModuleCamera.h"


Application::Application(int argc, wchar_t** argv, void* hWnd)
{
    modules.push_back(new ModuleInput((HWND)hWnd));
    modules.push_back(d3d12 = new ModuleD3D12((HWND)hWnd));
    modules.push_back(camera = new ModuleCamera());
    //modules.push_back(new Exercise1());
    //modules.push_back(new ModuleEditor());
    modules.push_back(resources = new ModuleResources());
    modules.push_back(shaderDescriptors = new ModuleShaderDescriptors());
    modules.push_back(samplers = new ModuleSampler());
    //modules.push_back(new Exercise2());
    //modules.push_back(new Exercise3());
    //modules.push_back(new Exercise4());
    modules.push_back(new Assignment1());
}

Application::~Application()
{
    cleanUp();

	for(auto it = modules.rbegin(); it != modules.rend(); ++it)
    {
        delete *it;
    }
}
 
bool Application::init()
{
	bool ret = true;

	for(auto it = modules.begin(); it != modules.end() && ret; ++it)
		ret = (*it)->init();

    lastMilis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	return ret;
}

void Application::update()
{
    using namespace std::chrono_literals;

    // Update milis
    uint64_t currentMilis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    elapsedMilis = currentMilis - lastMilis;
    lastMilis = currentMilis;
    tickSum -= tickList[tickIndex];
    tickSum += elapsedMilis;
    tickList[tickIndex] = elapsedMilis;
    tickIndex = (tickIndex + 1) % MAX_FPS_TICKS;

    if (!app->paused)
    {
        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->update();

        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->preRender();

        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->render();

        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->postRender();
    }
}

bool Application::cleanUp()
{
	bool ret = true;

	for(auto it = modules.rbegin(); it != modules.rend() && ret; ++it)
		ret = (*it)->cleanUp();

	return ret;
}
