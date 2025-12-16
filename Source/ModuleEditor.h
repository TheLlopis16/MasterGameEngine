#pragma once

#include "Module.h"
#include <chrono>

class ImGuiPass;
class ModuleD3D12;

class ModuleEditor : public Module
{
	ImGuiPass* imguiPass = nullptr;
	ImGuiIO* io = nullptr;

	int maxFps = 0;
	double currentFPS = 0.0;
	double currentMS = 0.0;
	static const int HISTORY_SIZE = 100;
	float fpsHistory[HISTORY_SIZE] = {};
	float msHistory[HISTORY_SIZE] = {};
	int historyIndex = 0;

	double sampleIntervalMs = 16.0;   // sample every ~16 ms (about 60 Hz)
	double sampleAccumulator = 0.0;

	std::chrono::high_resolution_clock::time_point lastFrameTime;

public:
	ModuleEditor();
	~ModuleEditor();

	bool init() override;
	void update() override;
	void render() override;

private:
	void consoleWindow();
	void mainMenu();
	void configurationWindow();
};
