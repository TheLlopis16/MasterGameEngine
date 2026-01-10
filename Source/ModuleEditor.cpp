#include "Globals.h"
#include "ImGuiPass.h"

#include "ModuleEditor.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include <thread>

ModuleEditor::ModuleEditor() {}

ModuleEditor::~ModuleEditor()
{
    delete imGuiPass;
}

bool ModuleEditor::init()
{
    ModuleD3D12* d3d12 = app->getD3D12();
    imGuiPass = new ImGuiPass(d3d12->getDevice(), d3d12->getHwnd());
    lastFrameTime = std::chrono::high_resolution_clock::now();

    return true;
}

void ModuleEditor::update()
{

    using namespace std::chrono;

    auto now = high_resolution_clock::now();
    double ms = duration<double, std::milli>(now - lastFrameTime).count();
    lastFrameTime = now;

    currentMS = ms;
    currentFPS = (ms > 0.0) ? (1000.0 / ms) : 0.0;

    if (maxFps > 0)
    {
        double targetMs = 1000.0 / maxFps;

        if (ms < targetMs)
        {
            double sleepMs = targetMs - ms;
            std::this_thread::sleep_for(duration<double, std::milli>(sleepMs));

            currentMS += sleepMs;
        }
    }

    sampleAccumulator += currentMS;

    if (sampleAccumulator >= sampleIntervalMs)
    {
        sampleAccumulator = 0.0;

        fpsHistory[historyIndex] = (float)currentFPS;
        msHistory[historyIndex] = (float)currentMS;

        historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    }

}

//void ModuleEditor::preRender()
//{
//    imGuiPass->startFrame();
//    mainMenu();
//    consoleWindow();
//    configurationWindow();
//}

//void ModuleEditor::render()
//{
//    D3D12Module* d3d12 = app->getD3D12();
//    ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();
//
//    commandList->Reset(d3d12->getCurrentCommandAllocator(), nullptr);
//
//    ID3D12Resource* backBuffer = d3d12->getCurrentBackBuffer();
//    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
//    commandList->ResourceBarrier(1, &barrier);
//
//    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12->getRenderTargetDescriptor();
//    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
//
//    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
//
//    imGuiPass->record(commandList);
//
//    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
//    commandList->ResourceBarrier(1, &barrier1);
//
//    commandList->Close();
//    ID3D12CommandList* commandLists[] = { commandList };
//    d3d12->getCommandQueue()->ExecuteCommandLists(1, commandLists);
//}

//void ModuleEditor::consoleWindow()
//{
//    ImGui::Begin("Console");
//    ImGui::End();
//}

void ModuleEditor::mainMenu()
{

    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Exit"))
            exit(1);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Assets")) {
        if (ImGui::MenuItem("Show in Explorer"))
        {
            ShellExecuteA(
                NULL,
                "open",
                "explorer.exe",
                "Assets",
                NULL,
                SW_SHOW
            );
        }
        if (ImGui::MenuItem("Show/Hide Assets Window"))
        {
            setShowAssets(!showAssets);
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

}

void ModuleEditor::configurationWindow()
{
    ImGui::Begin("Configuration");
    ImGui::SetNextWindowSize(ImVec2(320, 640), ImGuiCond_Always);

    ImGui::Text("Options");
    if (ImGui::CollapsingHeader("Application"))
    {

        ImGui::Text("App Name:");
        ImGui::SameLine();
        ImGui::Text("Engine");

        int lastIndex = (historyIndex - 1 + HISTORY_SIZE) % HISTORY_SIZE;

        ImGui::SliderInt("Max FPS", &maxFps, 0, 240);

        char title[25];
        sprintf_s(title, "Framerate: %.1f", fpsHistory[lastIndex]);
        ImGui::PlotHistogram("##framerate", fpsHistory, HISTORY_SIZE, historyIndex, title, 0.0f, 3000.0f, ImVec2(310, 100));

        sprintf_s(title, "Milliseconds: %.1f", msHistory[lastIndex]);
        ImGui::PlotHistogram("##milliseconds", msHistory, HISTORY_SIZE, historyIndex, title, 0.0f, 40.0f, ImVec2(310, 100));
    }


    ImGui::End();
}

void ModuleEditor::assetsWindow()
{
    ImGui::Begin("Assets");

    ImGui::Text("Path: %s", currentAssetsDir.string().c_str());

    if (currentAssetsDir != assetsRoot)
    {
        if (ImGui::Button("Back"))
        {
            currentAssetsDir = currentAssetsDir.parent_path();
        }
    }

    ImGui::Separator();

    if (ImGui::BeginTable("AssetsTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableHeadersRow();

        try
        {
            for (const auto& entry : std::filesystem::directory_iterator(currentAssetsDir))
            {
                const auto& path = entry.path();
                std::string name = path.filename().string();
                bool isDir = entry.is_directory();

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (isDir)
                {
                    std::string label = name;
                    if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        if (ImGui::IsMouseDoubleClicked(0))
                        {
                            currentAssetsDir = path;
                        }
                    }
                }
                else
                {
                    ImGui::Selectable(name.c_str(), false);
                }

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(isDir ? "Folder" : "File");
            }
        }
        catch (const std::exception& e)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
