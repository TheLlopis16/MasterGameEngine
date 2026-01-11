#pragma once

#include "Module.h"
#include <chrono>
#include <filesystem>

class ImGuiPass;
class ModuleD3D12;
class Model;

class ModuleEditor : public Module
{
	ImGuiPass* imGuiPass = nullptr;

	int maxFps = 0;
	double currentFPS = 0.0;
	double currentMS = 0.0;
	static const int HISTORY_SIZE = 100;
	float fpsHistory[HISTORY_SIZE] = {};
	float msHistory[HISTORY_SIZE] = {};
	int historyIndex = 0;

	double sampleIntervalMs = 16.0;
	double sampleAccumulator = 0.0;

	std::chrono::high_resolution_clock::time_point lastFrameTime;

	std::filesystem::path assetsRoot = "Assets";
	std::filesystem::path currentAssetsDir = assetsRoot;

	bool showAssets = false;
	bool showConfig = true;
	bool showMenu = true;

public:
	ModuleEditor();
	~ModuleEditor();

	bool init() override;
	void update() override;
	int samplerIndex = 0;
	//void preRender() override;
	//void render() override;

	ImGuiPass* getImGui() { return imGuiPass; }
	bool getShowAssets() { return showAssets; }
	bool getShowConfig() { return showConfig; }
	bool getShowMenu() { return showMenu; }
	const int& getSamplerIndex() const { return samplerIndex; }

	void mainMenu();
	void configurationWindow();
	void assetsWindow();
	void modelOptions(Model& model);

	void setShowAssets(bool v) { showAssets = v; }
	void setShowConfig(bool v) { showConfig = v; }
	void setShowMenu(bool v) { showMenu = v; }
};