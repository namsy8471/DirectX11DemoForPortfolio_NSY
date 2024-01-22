#include "IMGUIManager.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"



IMGUIManager* IMGUIManager::_instance = nullptr;

IMGUIManager::IMGUIManager()
{

}

IMGUIManager::~IMGUIManager()
{

}

IMGUIManager* IMGUIManager::Instance()
{
	if (_instance == nullptr)
		_instance = new IMGUIManager;
	return _instance;
}

bool IMGUIManager::Initialize(HWND hwnd, ID3D11Device* m_device, ID3D11DeviceContext* m_deviceContext)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	if (!ImGui_ImplWin32_Init(hwnd))
		return false;
	
	if (!ImGui_ImplDX11_Init(m_device, m_deviceContext))
		return false;

	return true;
}

void IMGUIManager::Frame()
{
	// (Your code process and dispatch Win32 messages)
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	bool* isOpen = new bool(true);
	ImGui::ShowDemoWindow(isOpen); // Show demo window! :)
}

void IMGUIManager::Render()
{
	// Rendering
	// (Your code clears your framebuffer, renders your other stuff etc.)
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	// (Your code calls swapchain's Present() function)
}

void IMGUIManager::ShutDown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
