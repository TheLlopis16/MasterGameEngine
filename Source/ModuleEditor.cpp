#include "Globals.h"
#include "ImGuiPass.h"

#include "ModuleEditor.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include <thread>

ModuleEditor::ModuleEditor() {}

ModuleEditor::~ModuleEditor()
{
    delete imguiPass;
}

bool ModuleEditor::init()
{
    ModuleD3D12* d3d12 = app->getD3D12();
    imguiPass = new ImGuiPass(d3d12->getDevice(), d3d12->getHwnd());
    io = &ImGui::GetIO();
    lastFrameTime = std::chrono::high_resolution_clock::now();

    return imguiPass;
}

void ModuleEditor::update()
{

    using namespace std::chrono;

    // Measure frame time since last update
    auto now = high_resolution_clock::now();
    double ms = duration<double, std::milli>(now - lastFrameTime).count();
    lastFrameTime = now;

    currentMS = ms;
    currentFPS = (ms > 0.0) ? (1000.0 / ms) : 0.0;

    // --- FPS LIMITER ---
    if (maxFps > 0)
    {
        double targetMs = 1000.0 / maxFps;

        if (ms < targetMs)
        {
            double sleepMs = targetMs - ms;
            std::this_thread::sleep_for(duration<double, std::milli>(sleepMs));

            // Add sleep to the final measured ms
            currentMS += sleepMs;
        }
    }

    // --- HISTORY UPDATE ---
    sampleAccumulator += currentMS;

    if (sampleAccumulator >= sampleIntervalMs)
    {
        sampleAccumulator = 0.0;

        fpsHistory[historyIndex] = (float)currentFPS;
        msHistory[historyIndex] = (float)currentMS;

        historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    }

    imguiPass->startFrame();
    mainMenu();
    consoleWindow();
    configurationWindow();
}

void ModuleEditor::render()
{
    ModuleD3D12* d3d12 = app->getD3D12();
    ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();

    commandList->Reset(d3d12->getCurrentCommandAllocator(), nullptr);

    ID3D12Resource* backBuffer = d3d12->getCurrentBackBuffer();
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12->getRenderTargetDescriptor();
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    imguiPass->record(commandList);

    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier1);

    commandList->Close();
    ID3D12CommandList* commandLists[] = { commandList };
    d3d12->getCommandQueue()->ExecuteCommandLists(1, commandLists);
}

void ModuleEditor::consoleWindow()
{
    ImGui::Begin("Console");
    ImGui::End();
}

void ModuleEditor::mainMenu()
{

    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File")) {
        ImGui::MenuItem("Save");
        if (ImGui::MenuItem("Exit"))
            exit(1);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Settings")) {
        ImGui::MenuItem("Editor Settings");
        ImGui::MenuItem("Preferences");
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

}

void ModuleEditor::configurationWindow()
{

    ImGui::Begin("Configuration");

    ImGui::Text("Options");
    if (ImGui::CollapsingHeader("Application"))
    {

        ImGui::Text("App Name:");
        ImGui::SameLine();
        ImGui::Text("MasterEngine");

        ImGui::Text("Organization:");
        ImGui::SameLine();
        ImGui::Text("UPC CITM");

        int lastIndex = (historyIndex - 1 + HISTORY_SIZE) % HISTORY_SIZE;
        // Sliders, inputs, checkboxes, etc.
        ImGui::SliderInt("Max FPS", &maxFps, 0, 60);

        // FPS graph
        char title[25];
        sprintf_s(title, "Framerate: %.1f", fpsHistory[lastIndex]);
        ImGui::PlotHistogram("##framerate", fpsHistory, HISTORY_SIZE, historyIndex, title, 0.0f, 3000.0f, ImVec2(310, 100));

        // MS graph
        sprintf_s(title, "Milliseconds: %.1f", msHistory[lastIndex]);
        ImGui::PlotHistogram("##milliseconds", msHistory, HISTORY_SIZE, historyIndex, title, 0.0f, 40.0f, ImVec2(310, 100));
    }

    if (ImGui::CollapsingHeader("Window Options"))
    {
        ImGui::Text("This is the window options");
    }

    ImGui::End();
}