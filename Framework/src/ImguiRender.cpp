#include "ImguiRender.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
void Render::ImguiRender::ImGuiWinProcHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp);
}

Render::ImguiRender::ImguiRender()
{
}

Render::ImguiRender::~ImguiRender()
{
}

bool Render::ImguiRender::Init(ID3D12Device* pDevice, HWND hwnd)
{
	m_Heap = CreateDescriptorHeap(pDevice);

	if (m_Heap == nullptr)
	{
		return false;
	}

	if (ImGui::CreateContext() == nullptr)
	{
		assert(0);

		return false;
	}

	if (!InitWin(hwnd))
	{
		return false;
	}

	if (!InitDX12(pDevice))
	{
		return false;
	}

	return true;
}

ComPtr<ID3D12DescriptorHeap> Render::ImguiRender::GetHeap()
{
	return m_Heap;
}

void Render::ImguiRender::Begin()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Rendering Test Menu");
	ImGui::SetWindowSize(ImVec2(400, 500), ImGuiCond_::ImGuiCond_FirstUseEver);
	ImGui::End();
}

void Render::ImguiRender::Draw(ID3D12GraphicsCommandList* pCmdList)
{
	static bool blnChk = false;
	ImGui::Checkbox("CheckboxTest", &blnChk);

	static int radio = 0;
	ImGui::RadioButton("Radio 1", &radio, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Radio 2", &radio, 1);
	ImGui::SameLine();
	ImGui::RadioButton("Radio 3", &radio, 2);

	static int nSlider = 0;
	ImGui::SliderInt("Int Slider", &nSlider, 0, 100);

	static float fSlider = 0.0f;
	ImGui::SliderFloat("Float Slider", &fSlider, 0.0f, 100.0f);

	static float col3[3] = {};
	ImGui::ColorPicker3("ColorPicker3", col3, ImGuiColorEditFlags_::ImGuiColorEditFlags_InputRGB);

	static float col4[4] = {};
	ImGui::ColorPicker4("ColorPicker4", col4, ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);

	ImGui::Render();
	pCmdList->SetDescriptorHeaps(1, m_Heap.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCmdList);
}

bool Render::ImguiRender::InitWin(HWND hwnd)
{
	bool blnResult = ImGui_ImplWin32_Init(hwnd);

	if (!blnResult)
	{
		assert(0);

		return false;
	}

	return true;
}

bool Render::ImguiRender::InitDX12(ID3D12Device* pDevice)
{
	bool blnResult = ImGui_ImplDX12_Init(pDevice,                    // DirectX12デバイス
		             3,
		             DXGI_FORMAT_R8G8B8A8_UNORM,                     // 書き込み先RTVのフォーマット
		             m_Heap.Get(),                                   // imgui用ディスクリプタヒープ
		             m_Heap->GetCPUDescriptorHandleForHeapStart(),   // CPUハンドル
		             m_Heap->GetGPUDescriptorHandleForHeapStart());  // GPUハンドル

	if (!blnResult)
	{
		assert(0);

		return false;
	}

	return true;
}

ComPtr<ID3D12DescriptorHeap> Render::ImguiRender::CreateDescriptorHeap(ID3D12Device* pDevice)
{
	ComPtr<ID3D12DescriptorHeap> ret;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(ret.ReleaseAndGetAddressOf()));

	return ret;
}
