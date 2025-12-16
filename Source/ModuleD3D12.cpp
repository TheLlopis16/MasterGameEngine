#include "Globals.h"
#include "ModuleD3D12.h"

ModuleD3D12::ModuleD3D12(HWND hwnd) : hWnd(hwnd) {}

ModuleD3D12::~ModuleD3D12()
{
	flush();
}

bool ModuleD3D12::init()
{
	getWindowSize(windowWidth, windowHeight);

#if defined(_DEBUG)
	enableDebugLayer();
#endif
	createDevice();

#if defined(_DEBUG)
	createInfoQueue();
#endif
	createCommandQueue();
	createCommandList();
	createSwapChain();
	createRTV();
	createFence();

	return true;
}

void ModuleD3D12::preRender()
{
	currentIndex = swapChain->GetCurrentBackBufferIndex();
	if (fenceValues[currentIndex] != 0)
	{
		fence->SetEventOnCompletion(fenceValues[currentIndex], event);
		WaitForSingleObject(event, INFINITE);
		fenceValues[currentIndex];
	}
	commandAllocators[currentIndex]->Reset();
}

void ModuleD3D12::postRender()
{
	swapChain->Present(0, 0);
	fenceValues[currentIndex] = ++fenceCounter;
	commandQueue->Signal(fence.Get(), fenceValues[currentIndex]);
}

void ModuleD3D12::enableDebugLayer()
{
	ComPtr<ID3D12Debug> debugInterface;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
	debugInterface->EnableDebugLayer();
}

void ModuleD3D12::createDevice()
{
#if defined(_DEBUG)
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
#else
	CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
#endif
	factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
	D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
}

void ModuleD3D12::createInfoQueue()
{
	device.As(&infoQueue);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
}

void ModuleD3D12::createCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc = {};

	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue));
}

void ModuleD3D12::createCommandList()
{
	for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
	{
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]));
	}

	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList));
	commandList->Close();
}

void ModuleD3D12::createSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};

	desc.Width = windowWidth;
	desc.Height = windowHeight;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	desc.Stereo = FALSE;
	desc.SampleDesc = { 1, 0 };
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = FRAMES_IN_FLIGHT;

	desc.Scaling = DXGI_SCALING_NONE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &desc, nullptr, nullptr, &swapChain1);
	swapChain1.As(&swapChain);
}

void ModuleD3D12::createRTV()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = FRAMES_IN_FLIGHT;

	device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvDescriptorHeap));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	unsigned descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
	{
		swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, rtvCPUHandle);
		rtvCPUHandle.ptr += descriptorSize;
	}
}

void ModuleD3D12::createFence()
{
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	event = CreateEvent(NULL, FALSE, FALSE, NULL);
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleD3D12::getRenderTargetDescriptor()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), currentIndex, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
}

unsigned ModuleD3D12::getWindowSize(unsigned& width, unsigned& height)
{
	RECT client = {};
	GetClientRect(hWnd, &client);

	width = unsigned(client.right - client.left);
	height = unsigned(client.bottom - client.top);
	return width, height;
}

void ModuleD3D12::resize()
{
	unsigned w, h;
	getWindowSize(w, h);

	if (w != windowWidth || h != windowHeight)
	{
		windowWidth = w;
		windowHeight = h;
		flush();

		for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
		{
			backBuffers[i].Reset();
			fenceValues[i] = 0;
		}

		DXGI_SWAP_CHAIN_DESC desc = {};

		swapChain->GetDesc(&desc);
		swapChain->ResizeBuffers(FRAMES_IN_FLIGHT, windowWidth, windowHeight, desc.BufferDesc.Format, desc.Flags);

		if (windowWidth > 0 && windowHeight > 0)
		{
			createRTV();
		}
	}

}

void ModuleD3D12::flush()
{
	commandQueue->Signal(fence.Get(), ++fenceCounter);
	fence->SetEventOnCompletion(fenceCounter, event);
	WaitForSingleObject(event, INFINITE);
}