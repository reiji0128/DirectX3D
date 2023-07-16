#include "App.h"
#include <algorithm>
#include "FileUtil.h"
#include "Logger.h"
#include "CommonStates.h"
#include "DirectXHelpers.h"
#include <stdio.h>
#include <SimpleMath.h>

#include <io.h>
#include <fcntl.h>

using namespace DirectX::SimpleMath;

namespace
{
	// �E�B���h�E�N���X��
	const auto className = TEXT("DirectX3D");

	/// <summary>
	/// �̈�̌������v�Z
	/// </summary>
	inline int ComputeIntersectionArea(int ax1, int ay1,
		                               int ax2, int ay2,
		                               int bx1, int by1,
		                               int bx2, int by2)
	{
		return (std::max)(0, (std::min)(ax2, bx2) - (std::max)(ax1, bx1))
			 * (std::max)(0, (std::min)(ay2, by2) - (std::max)(ay1, by1));
	}

	enum COLOR_SPACE_TYPE
	{
		COLOR_SPACE_BT709,     // ITU-R BT.709
		COLOR_SPACE_BT2100_PO, // ITU-R BT.2100 PQ System
	};

	enum TONEMAP_TYPE
	{
		TONEMAP_NONE = 0, // �g�[���}�b�v����
		TONEMAP_REINHARD, // Reinhard�g�[���}�b�v
		TONEMAP_GT,       // GT�g�[���}�b�v
	};

	struct alignas(256) CbTonemap
	{
		int   Type;          // �g�[���}�b�v�^�C�v
		int   ColorSpace;    // �o�͐F���
		float BaseLuminance; // ��P�x�l[nit]
		float MaxLuminance;  // �ő�P�x�l[nit]
	};

	// ���b�V���p�̃R���X�^���g�o�b�t�@�[�̍\����
	struct alignas(256) CbMesh
	{
		Matrix World; // ���[���h�s��
	};

	// �ϊ��s��p�̃R���X�^���g�o�b�t�@�[�̍\����
	struct alignas(256) CbTransform
	{
		Matrix View; // �r���[�s��
		Matrix Proj; // �ˉe�s��
	};

	// ���C�g�p�̃R���X�^���g�o�b�t�@�[�̍\����
	struct alignas(256) CbLight
	{
		Vector3 LightPosition;     // ���C�g�ʒu
		float   LightInvSqrRadius; // ���C�g�̋t2�攼�a

		Vector3 LightColor;        // ���C�g�J���[
		float   LightIntensity;    // ���C�g���x

		Vector3 LightForward;      // ���C�g�̏Ǝ˕���
		float   LightAngleScale;   // 1.0f / (cosInner - cosOuter)

		float   LightAngleOffset;  // -cosOuter * LightAngleScale
		int     LightType;         // ���C�g�^�C�v
		float   Padding[2];        // �p�f�B���O
	};

	// �J�����p�̃R���X�^���g�o�b�t�@�[�̍\����
	struct alignas(256) CbCamera
	{
		Vector3 CameraPosition;   // �J�����ʒu
	};

	struct alignas(256) CbMaterial
	{
		Vector3 BaseColor; // ��{�F
		float   Alpha;     // ���ߓx
		float   Roughness; // �ʂ̑e��
		float   Metallic;  // �����x
	};

	/// <summary>
	/// �F�x���擾
	/// </summary>
	/// <param name="value"></param>
	/// <returns></returns>
	inline UINT16 GetChromaticityCoord(double value)
	{
		return UINT16(value * 50000);
	}

	/// <summary>
	/// �|�C���g���C�g�p�����[�^�[�̌v�Z
	/// </summary>
	/// <param name="pos">���C�g���W</param>
	/// <param name="radius">���C�g�̔��a</param>
	/// <param name="color">���C�g�J���[</param>
	/// <param name="intensity">���C�g���x</param>
	/// <returns>�|�C���g�p�����[�^�[</returns>
	CbLight ComputePointLight(const Vector3& pos, float radius, const Vector3& color, float intensity)
	{
		CbLight result;
		result.LightPosition     = pos;
		result.LightInvSqrRadius = 1.0f / (radius * radius);
		result.LightColor        = color;
		result.LightIntensity    = intensity;

		return result;
	}

	/// <summary>
	/// �X�|�b�g���C�g�p�����[�^�̌v�Z
	/// </summary>
	/// <param name="lightType"></param>
	/// <param name="dir"></param>
	/// <param name="pos"></param>
	/// <param name="radius"></param>
	/// <param name="color"></param>
	/// <param name="intensity"></param>
	/// <param name="innerAngle"></param>
	/// <param name="outerAngle"></param>
	/// <returns></returns>
	CbLight ComputeSpotLight(int            lightType,
		                     const Vector3& dir,
		                     const Vector3& pos,
		                     float          radius,
		                     const Vector3& color,
		                     float          intensity,
		                     float          innerAngle,
		                     float          outerAngle)
	{
		auto conInnerAngle = cos(innerAngle);
		auto cosOuterAngle = cos(outerAngle);

		CbLight result;
		result.LightPosition     = pos;
		result.LightInvSqrRadius = 1.0f / (radius * radius);
		result.LightColor        = color;
		result.LightIntensity    = intensity;
		result.LightForward      = dir;
		result.LightAngleScale   = 1.0f / DirectX::XMMax(0.001f, (conInnerAngle - cosOuterAngle));
		result.LightAngleOffset  = -cosOuterAngle * result.LightAngleScale;
		result.LightType         = lightType;

		return result;
	}

	Vector3 CalcLightColor(float time)
	{
		auto c = fmodf(time, 3.0f);
		auto result = Vector3(0.25f, 0.25f, 0.25f);

		if (c < 1.0f)
		{
			result.x += 1.0f - c;
			result.y += c;
		}
		else if (c < 2.0f)
		{
			c -= 1.0f;
			result.y += 1.0f - c;
			result.z += c;
		}
		else
		{
			c -= 2.0f;
			result.z += 1.0f - c;
			result.x += c;
		}

		return result;
	}
}

App::App::App(uint32_t windowWidth, uint32_t windowHeight)
	:m_hInst(nullptr)
	,m_hWnd(nullptr)
	,m_Width(windowWidth)
	,m_Height(windowHeight)
	,m_FrameIndex(0)
	,m_RotateAngle(0.0f)
	,m_TonemapType(TONEMAP_NONE)
	,m_ColorSpace(COLOR_SPACE_BT709)
	,m_BaseLuminance(100.0f)
	,m_MaxLuminance(100.0f)
	,m_Exposure(1.0f)
{
}

App::App::~App()
{
}

void App::App::Run()
{
	// ������������������
	if (InitApp())
	{
		MainLoop();
	}

	TermApp();
}

bool App::App::InitApp()
{
	// �E�B���h�E�̏������Ɏ��s������
	if (!InitWindow())
	{
		return false;
	}

	// Direct3D12�̏�����.
	if (!InitD3D())
	{
		return false;
	}

	// �A�v���P�[�V�����ŗL�̏�����
	if (!OnInit())
	{
		return false;
	}

	// �E�B���h�E��\��.
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	// �E�B���h�E���X�V.
	UpdateWindow(m_hWnd);

	// �E�B���h�E�Ƀt�H�[�J�X��ݒ�.
	SetFocus(m_hWnd);

	return true;
}

void App::App::TermApp()
{
	// �A�v���P�[�V�����ŗL�̏I������
	OnTerm();

	// Direct3D 12�̏I������.
	TermD3D();

	// �E�B���h�E�̏I������
	TermWindow();
}

bool App::App::InitWindow()
{
	// �C���X�^���X�n���h���̎擾
	auto hInst = GetModuleHandle(nullptr);

	// �C���X�^���X���Ȃ����
	if (hInst == nullptr)
	{
		return false;
	}


	// �E�B���h�E�̎w��
	WNDCLASSEX wc = {};
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.hIcon         = LoadIcon(hInst, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(hInst, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName  = nullptr;
	wc.lpszClassName = className;
	wc.hIconSm       = LoadIcon(hInst, IDI_APPLICATION);

	// �E�B���h�E�̓o�^
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	// �C���X�^���X�n���h���̐ݒ�
	m_hInst = hInst;

	// �E�B���h�E�T�C�Y�̎w��
	RECT rc = {};
	rc.right  = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	// �E�B���h�E�T�C�Y�̒���
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	// �E�B���h�E�̐���
	m_hWnd = CreateWindowEx(0, 
		                    className,
		                    TEXT("Sample"),
		                    style,
		                    CW_USEDEFAULT,
		                    CW_USEDEFAULT,
		                    rc.right - rc.left,
		                    rc.bottom - rc.top,
		                    nullptr,
		                    nullptr,
		                    m_hInst,
		                    this);

	// �E�B���h�E�n���h�����Ȃ����
	if (m_hWnd == nullptr)
	{
		return false;
	}

	return true;
}

void App::App::TermWindow()
{
	// �E�B���h�E�̓o�^����
	if (m_hInst != nullptr)
	{
		UnregisterClass(className, m_hInst);
	}

	m_hInst = nullptr;
	m_hWnd  = nullptr;
}

void App::App::MainLoop()
{
	MSG msg = {};

	// WM_QUIT���b�Z�[�W�łȂ����
	while (WM_QUIT != msg.message)
	{
		// �A�v���P�[�V�������Ƀ��b�Z�[�W�������Ă��邩
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}
}

bool App::App::InitD3D()
{
#if defined(DEBUG) || defined(_DEBUG)

	{
		ComPtr<ID3D12Debug> pDebug;
		auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(pDebug.GetAddressOf()));

		// �f�o�b�O���C���[��L����
		if (SUCCEEDED(hr))
		{
			pDebug->EnableDebugLayer();
		}
	}

//#endif

	// �f�o�C�X�̐���
	auto hr = D3D12CreateDevice(nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(m_pDevice.GetAddressOf()));

	if (FAILED(hr))
	{
		return false;
	}
//#if defined(DEBUG) || defined(_DEBUG)
//
//	ComPtr<ID3D12InfoQueue> pInfoQueue;
//	hr = m_pDevice->QueryInterface(IID_PPV_ARGS(pInfoQueue.GetAddressOf()));
//
//	if (SUCCEEDED(hr))
//	{
//		// Error���Ƀu���[�N�𔭐�������
//		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
//	}

#endif

	// �R�}���h�L���[�̐���
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pQueue.GetAddressOf()));

		if (FAILED(hr))
		{
			return false;
		}
	}

	// �X���b�v�`�F�C���̐���
	{
		// DXGI�t�@�N�g���[�̐���
		ComPtr<IDXGIFactory4> pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(pFactory.GetAddressOf()));

		if (FAILED(hr))
		{
			return false;
		}

		// �X���b�v�`�F�C���̐ݒ�
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_Width;
		desc.BufferDesc.Height = m_Height;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.BufferDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.OutputWindow = m_hWnd;
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// �X���b�v�`�F�C���̐���
		ComPtr<IDXGISwapChain> pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, pSwapChain.GetAddressOf());

		// �X���b�v�`�F�C���̐����Ɏ��s������
		if (FAILED(hr))
		{
			return false;
		}

		// IDXGISwapChain3���擾
		hr = pSwapChain.As(&m_pSwapChain);

		if (FAILED(hr))
		{
			return false;
		}

		// �o�b�N�o�b�t�@�ԍ����擾
		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		// �s�v�ɂȂ����̂ŉ��
		pFactory.Reset();
		pSwapChain.Reset();
	}

	// �f�B�X�N���v�^�v�[���̐���
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NodeMask = 1;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 512;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		if (!D3D::DescriptorPool::Create(m_pDevice.Get(), &desc, &m_pPool[POOL_TYPE_RES]))
		{
			return false;
		}

		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		desc.NumDescriptors = 256;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		if (!D3D::DescriptorPool::Create(m_pDevice.Get(), &desc, &m_pPool[POOL_TYPE_SMP]))
		{
			return false;
		}

		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NumDescriptors = 512;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		if (!D3D::DescriptorPool::Create(m_pDevice.Get(), &desc, &m_pPool[POOL_TYPE_RTV]))
		{
			return false;
		}

		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.NumDescriptors = 512;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		if (!D3D::DescriptorPool::Create(m_pDevice.Get(), &desc, &m_pPool[POOL_TYPE_DSV]))
		{
			return false;
		}
	}

	// �R�}���h���X�g�̐���.
	{
		if (!m_CommandList.Init(m_pDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, FrameCount))
		{
			return false;
		}
	}

	// �����_�[�^�[�Q�b�g�r���[�̐���.
	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			if (!m_ColorTarget[i].InitFromBackBuffer(
				m_pDevice.Get(),
				m_pPool[POOL_TYPE_RTV],
				true,
				i,
				m_pSwapChain.Get()))
			{
				return false;
			}
		}
	}

	// �[�x�X�e���V���o�b�t�@�̐���
	{
		if (!m_DepthTarget.Init(m_pDevice.Get(),
			m_pPool[POOL_TYPE_DSV],
			nullptr,
			m_Width,
			m_Height,
			DXGI_FORMAT_D32_FLOAT,
			1.0f,
			0.0f))
		{
			return false;
		}
	}

	// �t�F���X�̐���
	if (!m_Fence.Init(m_pDevice.Get()))
	{
		return false;
	}

	// �r���[�|�[�g�̐ݒ�
	{
		m_Viewport.TopLeftX = 0.0f;
		m_Viewport.TopLeftY = 0.0f;
		m_Viewport.Width = float(m_Width);
		m_Viewport.Height = float(m_Height);
		m_Viewport.MinDepth = 0.0f;
		m_Viewport.MaxDepth = 1.0f;
	}

	// �V�U�[��`�̐ݒ�
	{
		m_Scissor.left = 0;
		m_Scissor.right = m_Width;
		m_Scissor.top = 0;
		m_Scissor.bottom = m_Height;
	}

	return true;
}

void App::App::TermD3D()
{
	// GPU�����̊�����ҋ@
	m_Fence.Sync(m_pQueue.Get());

	// �t�F���X�j��
	m_Fence.Term();

	// �����_�[�^�[�Q�b�g�r���[�̔j��
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_ColorTarget[i].Term();
	}

	// �[�x�X�e���V���r���[�̔j��
	m_DepthTarget.Term();

	// �R�}���h���X�g�̔j��
	m_CommandList.Term();

	for (auto i = 0; i < POOL_COUNT; ++i)
	{
		if (m_pPool[i] != nullptr)
		{
			m_pPool[i]->Release();
			m_pPool[i] = nullptr;
		}
	}

	// �X���b�v�`�F�C���̔j��
	m_pSwapChain.Reset();

	// �R�}���h�L���[�̔j��
	m_pQueue.Reset();

	// �f�o�C�X�̔j��
	m_pDevice.Reset();
}

void App::App::Render()
{
	// �R�}���h�̋L�^���J�n
	auto pCmd = m_CommandList.Reset();

	ID3D12DescriptorHeap* const pHeaps[] =
	{
		m_pPool[POOL_TYPE_RES]->GetHeap()
	};

	pCmd->SetDescriptorHeaps(1, pHeaps);

	{
		// �f�B�X�N���v�^�̎擾
		auto handleRTV = m_SceneColorTarget.GetHandleRTV();
		auto handleDSV = m_SceneDepthTarget.GetHandleDSV();

		// �������ݗp���\�[�X�o���A�ݒ�
		DirectX::TransitionResource(pCmd,
			                        m_SceneColorTarget.GetResource(),
									D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
									D3D12_RESOURCE_STATE_RENDER_TARGET);

		// �����_�[�^�[�Q�b�g��ݒ�
		pCmd->OMSetRenderTargets(1, &handleRTV->HandleCPU, FALSE, &handleDSV->HandleCPU);

		// �����_�[�^�[�Q�b�g���N���A
		m_SceneColorTarget.ClearView(pCmd);
		m_SceneDepthTarget.ClearView(pCmd);

		// �V�[���̕`��
		DrawScene(pCmd);

		// �ǂݍ��ݗp���\�[�X�o���A�ݒ�
		DirectX::TransitionResource(pCmd,
			                        m_SceneColorTarget.GetResource(),
		                            D3D12_RESOURCE_STATE_RENDER_TARGET,
		                            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	// �t���[���o�b�t�@�ɕ`��
	{
		// �������ݗp���\�[�X�o���A�ݒ�
		DirectX::TransitionResource(pCmd,
			                        m_ColorTarget[m_FrameIndex].GetResource(),
			                        D3D12_RESOURCE_STATE_PRESENT,
			                        D3D12_RESOURCE_STATE_RENDER_TARGET);

		// �f�B�X�N���v�^�擾
		auto handleRTV = m_ColorTarget[m_FrameIndex].GetHandleRTV();
		auto handleDSV = m_DepthTarget.GetHandleDSV();

		// �����_�[�^�[�Q�b�g��ݒ�
		pCmd->OMSetRenderTargets(1, &handleRTV->HandleCPU, FALSE, &handleDSV->HandleCPU);

		// �����_�[�^�[�Q�b�g���N���A
		m_ColorTarget[m_FrameIndex].ClearView(pCmd);
		m_DepthTarget.ClearView(pCmd);

		// �g�[���}�b�v��K�p
		DrawTonemap(pCmd);

		// �\���p���\�[�X�o���A�̐ݒ�
		DirectX::TransitionResource(pCmd,
			                        m_ColorTarget[m_FrameIndex].GetResource(),
			                        D3D12_RESOURCE_STATE_RENDER_TARGET,
			                        D3D12_RESOURCE_STATE_PRESENT);
	}

	// �R�}���h�̋L�^���I��.
	pCmd->Close();

	// �R�}���h�����s
	ID3D12CommandList* pLists[] = { pCmd };
	m_pQueue->ExecuteCommandLists(1, pLists);

	// ��ʂɕ\��
	Present(1);
}

void App::App::MsgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if ((msg == WM_KEYDOWN)    ||
		(msg == WM_SYSKEYDOWN) ||
		(msg == WM_KEYUP)      ||
		(msg == WM_SYSKEYUP))
	{
		DWORD mask = (1 << 29);

		auto isKeyDown = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
		auto isAltDown = ((lp & mask) != 0);
		auto keyCode   = uint32_t(wp);

		if (isKeyDown)
		{
			switch (keyCode)
			{
			case VK_ESCAPE:
			{
				PostQuitMessage(0);
			}
			break;

			// HDR���[�h.
			case 'H':
			{
				ChangeDisplayMode(true);
			}
			break;

			// SDR���[�h.
			case 'S':
			{
				ChangeDisplayMode(false);
			}
			break;

			// �g�[���}�b�v�Ȃ�.
			case 'N':
			{
				m_TonemapType = TONEMAP_NONE;
			}
			break;

			// Reinhard�g�[���}�b�v.
			case 'R':
			{
				m_TonemapType = TONEMAP_REINHARD;
			}
			break;

			// GT�g�[���}�b�v
			case 'G':
			{
				m_TonemapType = TONEMAP_GT;
			}
			break;
			}
		}
	}
}

void App::App::ChangeDisplayMode(bool hdr)
{
	if (hdr)
	{
		if (!IsSupportHDR())
		{
			MessageBox(nullptr,
				TEXT("HDR���T�|�[�g����Ă��Ȃ��f�B�X�v���C�ł�."),
				TEXT("HDR��T�|�[�g"),
				MB_OK | MB_ICONINFORMATION);

			ELOG("Error : Display not support HDR.");
			return;
		}

		auto hr = m_pSwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);

		if (FAILED(hr))
		{
			MessageBox(nullptr,
				TEXT("ITU-R BT.2100 PQ System�̐F��ݒ�Ɏ��s���܂���"),
				TEXT("�F��ݒ莸�s"),
				MB_OK | MB_ICONERROR);

			ELOG("Error : IDXGISwapChain::SetColorSpace1() Failed.");
			return;
		}

		DXGI_HDR_METADATA_HDR10 metaData = {};

		// ITU-R BT.2100�̌��h���Ɣ��F�_��ݒ�
		metaData.RedPrimary[0] = GetChromaticityCoord(0.708);
		metaData.RedPrimary[1] = GetChromaticityCoord(0.292);
		metaData.BluePrimary[0] = GetChromaticityCoord(0.170);
		metaData.BluePrimary[1] = GetChromaticityCoord(0.797);
		metaData.GreenPrimary[0] = GetChromaticityCoord(0.131);
		metaData.GreenPrimary[1] = GetChromaticityCoord(0.046);
		metaData.WhitePoint[0] = GetChromaticityCoord(0.3127);
		metaData.WhitePoint[1] = GetChromaticityCoord(0.3290);

		// �f�B�X�v���C���T�|�[�g����ƍő�P�x�l�ƍŏ��P�x�l��ݒ�
		metaData.MaxMasteringLuminance = UINT(GetMaxLuminance() * 10000);
		metaData.MinMasteringLuminance = UINT(GetMinLuminance() * 0.001);

		// �ő�l�� 2000 [nit]�ɐݒ�
		metaData.MaxContentLightLevel = 2000;

		hr = m_pSwapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(DXGI_HDR_METADATA_HDR10), &metaData);

		if (FAILED(hr))
		{
			ELOG("Error : IDXGISwapChain::SetHDRMetaData() Failed.");
		}

		m_BaseLuminance = 100.0f;
		m_MaxLuminance = GetMaxLuminance();

		// �����������Ƃ�m�点��_�C�A���O���o��
		std::string message;
		message += "HDR�f�B�X�v���C�p�ɐݒ��ύX���܂���\n\n";
		message += "�F��� : ITU-R BT.2100 PQ\n";
		message += "�ő�P�x�l : ";
		message += std::to_string(GetMaxLuminance());
		message += "[nit]\n";
		message += "�ŏ��P�x�l : ";
		message += std::to_string(GetMinLuminance());
		message += " [nit]\n";

		MessageBoxA(nullptr,
			        message.c_str(),
			        "HDR�ݒ萬��",
			        MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		auto hr = m_pSwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);

		if (FAILED(hr))
		{
			MessageBox(nullptr,
				TEXT("ITU-R BT.709�̐F��ݒ�Ɏ��s���܂���"),
				TEXT("�F��ݒ莸�s"),
				MB_OK | MB_ICONERROR);

			ELOG("Error : IDXGISwapChain::SetColorSpace1() Failed.");

			return;
		}

		DXGI_HDR_METADATA_HDR10 metaData = {};

		// ITU -R BT.709�̌��h���Ɣ��F�_��ݒ�
		metaData.RedPrimary  [0] = GetChromaticityCoord(0.640);
		metaData.RedPrimary  [1] = GetChromaticityCoord(0.330);
		metaData.BluePrimary [0] = GetChromaticityCoord(0.300);
		metaData.BluePrimary [1] = GetChromaticityCoord(0.600);
		metaData.GreenPrimary[0] = GetChromaticityCoord(0.150);
		metaData.GreenPrimary[1] = GetChromaticityCoord(0.060);
		metaData.WhitePoint  [0] = GetChromaticityCoord(0.3127);
		metaData.WhitePoint  [1] = GetChromaticityCoord(0.3290);

		// �f�B�X�v���C���T�|�[�g����ƍő�P�x�l�ƍŏ��P�x�l��ݒ�
		metaData.MaxMasteringLuminance = UINT(GetMaxLuminance() * 10000);
		metaData.MinMasteringLuminance = UINT(GetMinLuminance() * 0.001);

		// �ő�l�� 100[nit]�ɐݒ�
		metaData.MaxContentLightLevel = 100;

		hr = m_pSwapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(DXGI_HDR_METADATA_HDR10), &metaData);

		if (FAILED(hr))
		{
			ELOG("Error : IDXGISwapChain::SetHDRMetaData() Failed.");
		}

		m_BaseLuminance = 100.0f;
		m_MaxLuminance  = 100.0f;

		// �����������Ƃ�m�点��_�C�A���O���o��
		std::string message;
		message += "SDR�f�B�X�v���C�p�ɐݒ��ύX���܂���\n\n";
		message += "�F��ԁFITU-R BT.709\n";
		message += "�ő�P�x�l�F";
		message += std::to_string(GetMaxLuminance());
		message += " [nit]\n";
		message += "�ŏ��P�x�l�F";
		message += std::to_string(GetMinLuminance());
		message += " [nit]\n";

		MessageBoxA(nullptr,
			        message.c_str(),
			        "SDR�ݒ萬��",
			        MB_OK | MB_ICONINFORMATION);
	}
}

void App::App::DrawScene(ID3D12GraphicsCommandList* pCmdList)
{
	auto cameraPos = Vector3(-0.5f, 0.0f, 2.0f);

	// ���C�g�o�b�t�@�̍X�V
	{
		auto matrix = Matrix::CreateRotationY(m_RotateAngle);

		auto ptr = m_LightCB[m_FrameIndex].GetPtr<CbLight>();
		ptr->LightColor = Vector3(1.0f, 1.0f, 1.0f);
		ptr->LightForward = Vector3::TransformNormal(Vector3(0.0f, 1.0f, 1.0f), matrix);
		ptr->LightIntensity = 5.0f;
		m_RotateAngle += 0.01f;
	}

	// �J�����o�b�t�@�̍X�V
	{
		auto ptr = m_CameraCB[m_FrameIndex].GetPtr<CbCamera>();
		ptr->CameraPosition = cameraPos;
	}

	// ���b�V���̃��[���h�s��̍X�V
	{
		auto ptr = m_MeshCB[m_FrameIndex].GetPtr<CbMesh>();
		ptr->World = Matrix::Identity;
	}

	// �ϊ��p�����[�^�̍X�V
	{
		auto fovY = DirectX::XMConvertToRadians(37.5f);
		auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);

		auto ptr = m_TransformCB[m_FrameIndex].GetPtr<CbTransform>();
		ptr->View = Matrix::CreateLookAt(cameraPos, Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
		ptr->Proj = Matrix::CreatePerspectiveFieldOfView(fovY, aspect, 0.1f, 1000.0f);
	}
	
	pCmdList->SetGraphicsRootSignature(m_SceneRootSig.GetPtr());
	pCmdList->SetGraphicsRootDescriptorTable(0, m_TransformCB[m_FrameIndex].GetHandleGPU());
	pCmdList->SetGraphicsRootDescriptorTable(2, m_LightCB    [m_FrameIndex].GetHandleGPU());
	pCmdList->SetGraphicsRootDescriptorTable(3, m_CameraCB   [m_FrameIndex].GetHandleGPU());
	pCmdList->SetPipelineState(m_pScenePSO.Get());
	pCmdList->RSSetViewports(1, &m_Viewport);
	pCmdList->RSSetScissorRects(1, &m_Scissor);

	// �I�u�W�F�N�g�̕`��
	{
		pCmdList->SetGraphicsRootDescriptorTable(1, m_MeshCB[m_FrameIndex].GetHandleGPU());
		DrawMesh(pCmdList);
	}
}

void App::App::DrawMesh(ID3D12GraphicsCommandList* pCmdList)
{
	for (size_t i = 0; i < m_pMesh.size(); ++i)
	{
		// �}�e���A��ID���擾
		auto id = m_pMesh[i]->GetMaterialID();

		// �e�N�X�`����ݒ�
		pCmdList->SetGraphicsRootDescriptorTable(4, m_Material.GetTextureHandle(id, Resource::TU_BASE_COLOR));
		pCmdList->SetGraphicsRootDescriptorTable(5, m_Material.GetTextureHandle(id, Resource::TU_METALLIC));
		pCmdList->SetGraphicsRootDescriptorTable(6, m_Material.GetTextureHandle(id, Resource::TU_ROUGHNESS));
		pCmdList->SetGraphicsRootDescriptorTable(7, m_Material.GetTextureHandle(id, Resource::TU_NORMAL));

		// ���b�V���̕`��
		m_pMesh[i]->Draw(pCmdList);
	}
}

void App::App::DrawTonemap(ID3D12GraphicsCommandList* pCmdList)
{
	// �萔�o�b�t�@�X�V
	{
		auto ptr = m_TonemapCB[m_FrameIndex].GetPtr<CbTonemap>();
		ptr->Type          = m_TonemapType;
		ptr->ColorSpace    = m_ColorSpace;
		ptr->BaseLuminance = m_BaseLuminance;
		ptr->MaxLuminance  = m_MaxLuminance;
	}

	pCmdList->SetGraphicsRootSignature(m_TonemapRootSig.GetPtr());
	pCmdList->SetGraphicsRootDescriptorTable(0, m_TonemapCB[m_FrameIndex].GetHandleGPU());
	pCmdList->SetGraphicsRootDescriptorTable(1, m_SceneColorTarget.GetHandleSRV()->HandleGPU);

	pCmdList->SetPipelineState(m_pTonemapPSO.Get());
	pCmdList->RSSetViewports(1, &m_Viewport);
	pCmdList->RSSetScissorRects(1, &m_Scissor);
	
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCmdList->IASetVertexBuffers(0, 1, &m_QuadVB.GetView());
	pCmdList->DrawInstanced(3, 1, 0, 0);
}

void App::App::Present(uint32_t interval)
{
	// ��ʂɕ\��
	m_pSwapChain->Present(interval, 0);

	// �����҂�
	m_Fence.Wait(m_pQueue.Get(), INFINITE);

	// �t���[���ԍ����X�V
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

void App::App::CheckSupportHDR()
{
	// ��������Ă��Ȃ��ꍇ�͏������Ȃ�.
	if (m_pSwapChain == nullptr || m_pFactory == nullptr || m_pDevice == nullptr)
	{
		return;
	}

	HRESULT hr = S_OK;

	// �E�B���h�E�̈���擾.
	RECT rect;
	GetWindowRect(m_hWnd, &rect);

	if (m_pFactory->IsCurrent() == false)
	{
		m_pFactory.Reset();
		hr = CreateDXGIFactory2(0, IID_PPV_ARGS(m_pFactory.GetAddressOf()));
		if (FAILED(hr))
		{
			return;
		}
	}

	ComPtr<IDXGIAdapter1> pAdapter;
	hr = m_pFactory->EnumAdapters1(0, pAdapter.GetAddressOf());
	if (FAILED(hr))
	{
		return;
	}

	UINT i = 0;
	ComPtr<IDXGIOutput> currentOutput;
	ComPtr<IDXGIOutput> bestOutput;
	int bestIntersectArea = -1;

	// �e�f�B�X�v���C�𒲂ׂ�.
	while (pAdapter->EnumOutputs(i, &currentOutput) != DXGI_ERROR_NOT_FOUND)
	{
		auto ax1 = rect.left;
		auto ay1 = rect.top;
		auto ax2 = rect.right;
		auto ay2 = rect.bottom;

		// �f�B�X�v���C�̐ݒ���擾.
		DXGI_OUTPUT_DESC desc;
		hr = currentOutput->GetDesc(&desc);
		if (FAILED(hr))
		{
			return;
		}

		auto bx1 = desc.DesktopCoordinates.left;
		auto by1 = desc.DesktopCoordinates.top;
		auto bx2 = desc.DesktopCoordinates.right;
		auto by2 = desc.DesktopCoordinates.bottom;

		// �̈悪��v���邩�ǂ������ׂ�.
		int intersectArea = ComputeIntersectionArea(ax1, ay1, ax2, ay2, bx1, by1, bx2, by2);
		if (intersectArea > bestIntersectArea)
		{
			bestOutput = currentOutput;
			bestIntersectArea = intersectArea;
		}

		i++;
	}

	// ��ԓK���Ă���f�B�X�v���C.
	ComPtr<IDXGIOutput6> pOutput6;
	hr = bestOutput.As(&pOutput6);
	if (FAILED(hr))
	{
		return;
	}

	// �o�͐ݒ���擾.
	DXGI_OUTPUT_DESC1 desc1;
	hr = pOutput6->GetDesc1(&desc1);
	if (FAILED(hr))
	{
		return;
	}

	// �F��Ԃ� ITU-R BT.2100 PQ���T�|�[�g���Ă��邩�ǂ����`�F�b�N.
	m_SupportHDR = (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
	m_MaxLuminance = desc1.MaxLuminance;
	m_MinLuminance = desc1.MinLuminance;
}

bool App::App::OnInit()
{
	// ���b�V���̃��[�h
	{
		std::wstring path;

		// �t�@�C���p�X������
		if (!SearchFilePath(L"Resource/material_test/material_test.obj", path))
		{
			ELOG("Error : File Not Found.");
			return false;
		}

		std::wstring dir = GetDirectoryPath(path.c_str());

		std::vector<Resource::ResMesh>     resMesh;
		std::vector<Resource::ResMaterial> resMaterial;

		// ���b�V�����\�[�X�̃��[�h
		if (!Resource::LoadMesh(path.c_str(), resMesh, resMaterial))
		{
			ELOG("Error : Load Mesh Failed. filepath = %ls", path.c_str());
			return false;
		}

		// ��������\��
		m_pMesh.reserve(resMesh.size());

		// ���b�V����������
		for (size_t i = 0; i < resMesh.size(); ++i)
		{
			// ���b�V������
			auto mesh = new(std::nothrow)Render::Mesh();

			// �`�F�b�N
			if (mesh == nullptr)
			{
				ELOG("Error : Out of memory.");
				return false;
			}

			// ����������
			if (!mesh->Init(m_pDevice.Get(), resMesh[i]))
			{
				ELOG("Error : Mesh Initialize Failed.");
				delete mesh;
				return false;
			}

			// ����������o�^
			m_pMesh.push_back(mesh);
		}

		// �������œK��
		m_pMesh.shrink_to_fit();

		// �}�e���A��������
		if (!m_Material.Init(m_pDevice.Get(), m_pPool[POOL_TYPE_RES], sizeof(CbMaterial), resMaterial.size()))
		{
			ELOG("Error : Material::Init() Failed.");
			return false;
		}

		// ���\�[�X�o�b�`��p��
		DirectX::ResourceUploadBatch batch(m_pDevice.Get());

		// �o�b�`�J�n
		batch.Begin();

		// �e�N�X�`���ƃ}�e���A����ݒ�
		{
			/* �����ł̓}�e���A�������ߑł��ł��邱�Ƃ�O��Ƀn�[�h�R�[�f�B���O���Ă��܂�. */
			m_Material.SetTexture(0, Resource::TU_BASE_COLOR, dir + L"wall_bc.dds", batch);
			m_Material.SetTexture(0, Resource::TU_METALLIC  , dir + L"wall_m.dds", batch);
			m_Material.SetTexture(0, Resource::TU_ROUGHNESS , dir + L"wall_r.dds", batch);
			m_Material.SetTexture(0, Resource::TU_NORMAL    , dir + L"wall_n.dds", batch);

			m_Material.SetTexture(1, Resource::TU_BASE_COLOR, dir + L"matball_bc.dds", batch);
			m_Material.SetTexture(1, Resource::TU_METALLIC  , dir + L"matball_m.dds", batch);
			m_Material.SetTexture(1, Resource::TU_ROUGHNESS , dir + L"matball_r.dds", batch);
			m_Material.SetTexture(1, Resource::TU_NORMAL    , dir + L"matball_n.dds", batch);
		}

		// �o�b�`�I��
		auto future = batch.End(m_pQueue.Get());

		// �o�b�`������ҋ@
		future.wait();
	}

	// ���C�g�o�b�t�@�̐ݒ�
	{
		for (auto i = 0; i < FrameCount; ++i)
		{
			if (!m_LightCB[i].Init(m_pDevice.Get(), m_pPool[POOL_TYPE_RES], sizeof(CbLight)))
			{
				ELOG("Error : ConstantBuffer::Init() Failed.");
				return false;
			}

			auto ptr = m_LightCB[i].GetPtr<CbLight>();
			*ptr = ComputePointLight(Vector3(0.0f, 1.0f, 1.5f), 1.0f, Vector3(1.0f, 0.5f, 0.0f), 10.0f);
		}
	}

	// �J�����o�b�t�@�̐ݒ�
	{
		for (auto i = 0; i < FrameCount; ++i)
		{
			if (!m_CameraCB[i].Init(m_pDevice.Get(), m_pPool[POOL_TYPE_RES], sizeof(CbCamera)))
			{
				ELOG("Error : ConstantBuffer::Init() Failed.");
				return false;
			}
		}
	}

	// �V�[���p�J���[�^�[�Q�b�g�̐���
	{
		float clearColor[4] = { 1.2f,0.2f,0.2f,1.0f };

		if (!m_SceneColorTarget.Init(m_pDevice.Get(),
			                         m_pPool[POOL_TYPE_RTV],
			                         m_pPool[POOL_TYPE_RES],
			                         m_Width,
			                         m_Height,
			                         DXGI_FORMAT_R10G10B10A2_UNORM,
			                         clearColor))
		{
			ELOG("Error : ColorTarget::Init() Failed.");
			return false;
		}
	}

	// �V�[���p�[�x�^�[�Q�b�g�̐���
	{
		if (!m_SceneDepthTarget.Init(m_pDevice.Get(),
			                         m_pPool[POOL_TYPE_DSV],
			                         nullptr,
			                         m_Width,
			                         m_Height,
			                         DXGI_FORMAT_D32_FLOAT,
			                         1.0f,
			                         0))
		{
			ELOG("Error : DepthTarget::Init() Failed.");
			return false;
		}
	}

	// ���[�g�V�O�j�`���̐���
	{
		D3D::RootSignature::Desc desc;
		desc.Begin(8)
			.SetCBV(D3D::ShaderStage::VS, 0, 0)
			.SetCBV(D3D::ShaderStage::VS, 1, 1)
			.SetCBV(D3D::ShaderStage::PS, 2, 1)
			.SetCBV(D3D::ShaderStage::PS, 3, 2)
			.SetSRV(D3D::ShaderStage::PS, 4, 0)
			.SetSRV(D3D::ShaderStage::PS, 5, 1)
			.SetSRV(D3D::ShaderStage::PS, 6, 2)
			.SetSRV(D3D::ShaderStage::PS, 7, 3)
			.AddStaticSmp(D3D::ShaderStage::PS, 0, D3D::SamplerState::LinearWrap)
			.AddStaticSmp(D3D::ShaderStage::PS, 1, D3D::SamplerState::LinearWrap)
			.AddStaticSmp(D3D::ShaderStage::PS, 2, D3D::SamplerState::LinearWrap)
			.AddStaticSmp(D3D::ShaderStage::PS, 3, D3D::SamplerState::LinearWrap)
			.AllowIL()
			.End();

		if (!m_SceneRootSig.Init(m_pDevice.Get(), desc.GetDesc()))
		{
			ELOG("Error : RootSignature::Init() Failed.");
			return false;
		}
	}

	// �p�C�v���C���X�e�[�g�̐���
	{
		std::wstring vsPath;
		std::wstring psPath;

		// ���_�V�F�[�_�[�̌���
		if (!SearchFilePath(L"BasicVS.cso", vsPath))
		{
			ELOG("Error : Vertex Shader Not Found.");
			return false;
		}

		// �s�N�Z���V�F�[�_�[�̌���
		if (!SearchFilePath(L"DirectionalLightPS.cso", psPath))
		{
			ELOG("Error : Pixel Shader Node Found.");
			return false;
		}

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		// ���_�V�F�[�_�[�ǂݍ���
		auto hr = D3DReadFileToBlob(vsPath.c_str(), pVSBlob.GetAddressOf());

		if (FAILED(hr))
		{
			ELOG("Error : D3DReadFiledToBlob() Failed. path = %ls", vsPath.c_str());
			return false;
		}

		// �s�N�Z���V�F�[�_�[�ǂݍ���
		hr = D3DReadFileToBlob(psPath.c_str(), pPSBlob.GetAddressOf());

		if (FAILED(hr))
		{
			ELOG("Error : D3DReadFileToBlob() Failed. path = %ls", psPath.c_str());
			return false;
		}

		D3D12_INPUT_ELEMENT_DESC elements[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// �O���t�B�b�N�X�p�C�v���C���X�e�[�g�̐ݒ�
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.InputLayout                     = { elements, 4};
		desc.pRootSignature                  = m_SceneRootSig.GetPtr();
		desc.VS                              = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() };
		desc.PS                              = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() };
		desc.RasterizerState                 = DirectX::CommonStates::CullNone;
		desc.BlendState                      = DirectX::CommonStates::Opaque;
		desc.DepthStencilState               = DirectX::CommonStates::DepthDefault;
		desc.SampleMask                      = UINT_MAX;
		desc.PrimitiveTopologyType           = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets                = 1;
		desc.RTVFormats[0]                   = m_SceneColorTarget.GetRTVDesc().Format;
		desc.DSVFormat                       = m_SceneDepthTarget.GetDSVDesc().Format;
		desc.SampleDesc.Count                = 1;
		desc.SampleDesc.Quality              = 0;

		// �p�C�v���C���X�e�[�g�𐶐�
		hr = m_pDevice->CreateGraphicsPipelineState(&desc,
			                                        IID_PPV_ARGS(m_pScenePSO.GetAddressOf()));

		if (FAILED(hr))
		{
			ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. retcode = 0x%x", hr);
			return false;
		}
	}

	// �g�[���}�b�v�p���[�g�V�O�l�`���̐���
	{
		D3D::RootSignature::Desc desc;
		desc.Begin(2)
			.SetCBV(D3D::ShaderStage::PS, 0, 0)
			.SetSRV(D3D::ShaderStage::PS, 1, 0)
			.AddStaticSmp(D3D::ShaderStage::PS, 0, D3D::SamplerState::LinearWrap)
			.AllowIL()
			.End();

		if (!m_TonemapRootSig.Init(m_pDevice.Get(), desc.GetDesc()))
		{
			ELOG("Error : RootSignature::Init() Failed.");
			return false;
		}
	}

	// �g�[���}�b�v�p�p�C�v���C���X�e�[�g�̐���
	{
		std::wstring vsPath;
		std::wstring psPath;

		// ���_�V�F�[�_������
		if (!SearchFilePath(L"TonemapVS.cso", vsPath))
		{
			ELOG("Error : Vertex Shader Not Found.");
			return false;
		}

		// �s�N�Z���V�F�[�_������
		if (!SearchFilePath(L"TonemapPS.cso", psPath))
		{
			ELOG("Error : Pixel Shader Node Found.");
			return false;
		}

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		// ���_�V�F�[�_��ǂݍ���
		auto hr = D3DReadFileToBlob(vsPath.c_str(), pVSBlob.GetAddressOf());
		if (FAILED(hr))
		{
			ELOG("Error : D3DReadFiledToBlob() Failed. path = %ls", vsPath.c_str());
			return false;
		}

		// �s�N�Z���V�F�[�_��ǂݍ���
		hr = D3DReadFileToBlob(psPath.c_str(), pPSBlob.GetAddressOf());
		if (FAILED(hr))
		{
			ELOG("Error : D3DReadFileToBlob() Failed. path = %ls", psPath.c_str());
			return false;
		}

		D3D12_INPUT_ELEMENT_DESC elements[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// �O���t�B�b�N�p�C�v���C���X�e�[�g�̐ݒ�
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.InputLayout           = { elements, 2 };
		desc.pRootSignature        = m_TonemapRootSig.GetPtr();
		desc.VS                    = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() };
		desc.PS                    = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() };
		desc.RasterizerState       = DirectX::CommonStates::CullNone;
		desc.BlendState            = DirectX::CommonStates::Opaque;
		desc.DepthStencilState     = DirectX::CommonStates::DepthDefault;
		desc.SampleMask            = UINT_MAX;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets      = 1;
		desc.RTVFormats[0]         = m_ColorTarget[0].GetRTVDesc().Format;
		desc.DSVFormat             = m_DepthTarget.GetDSVDesc().Format;
		desc.SampleDesc.Count      = 1;
		desc.SampleDesc.Quality    = 0;

		// �p�C�v���C���X�e�[�g�𐶐�
		hr = m_pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pTonemapPSO.GetAddressOf()));
		if (FAILED(hr))
		{
			ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. retcode = 0x%x", hr);
			return false;
		}
	}

	// ���_�o�b�t�@�̐���
	{
		struct Vertex
		{
			float px;
			float py;

			float tx;
			float ty;
		};

		if (!m_QuadVB.Init<Vertex>(m_pDevice.Get(), 3))
		{
			ELOG("Error : VertexBuffer::Init() Failed.");
			return false;
		}

		auto ptr = m_QuadVB.Map<Vertex>();
		assert(ptr != nullptr);
		ptr[0].px = -1.0f;  ptr[0].py =  1.0f;  ptr[0].tx = 0.0f;   ptr[0].ty = -1.0f;
		ptr[1].px =  3.0f;  ptr[1].py =  1.0f;  ptr[1].tx = 2.0f;   ptr[1].ty = -1.0f;
		ptr[2].px = -1.0f;  ptr[2].py = -3.0f;  ptr[2].tx = 0.0f;   ptr[2].ty =  1.0f;
		m_QuadVB.Unmap();
	}

	// �Ǘp���_�o�b�t�@�̐���
	{
		struct BasicVertex
		{
			Vector3 Position;
			Vector3 Normal;
			Vector2 TexCoord;
			Vector3 Tangent;
		};

		if (!m_WallVB.Init<BasicVertex>(m_pDevice.Get(), 6))
		{
			ELOG("Error : VertexBuffer::Init() Failed.");
			return false;
		}

		auto size = 10.0f;
		auto ptr = m_WallVB.Map<BasicVertex>();
		assert(ptr != nullptr);

		ptr[0].Position = Vector3(-size, size, 0.0f);
		ptr[0].Normal   = Vector3(0.0f, 0.0f, 1.0f);
		ptr[0].TexCoord = Vector2(0.0f, 1.0f);
		ptr[0].Tangent  = Vector3(1.0f, 0.0f, 0.0f);

		ptr[1].Position = Vector3(size, size, 0.0f);
		ptr[1].Normal   = Vector3(0.0f, 0.0f, 1.0f);
		ptr[1].TexCoord = Vector2(1.0f, 1.0f);
		ptr[1].Tangent  = Vector3(1.0f, 0.0f, 0.0f);

		ptr[2].Position = Vector3(size, -size, 0.0f);
		ptr[2].Normal   = Vector3(0.0f, 0.0f, 1.0f);
		ptr[2].TexCoord = Vector2(1.0f, 0.0f);
		ptr[2].Tangent  = Vector3(1.0f, 0.0f, 0.0f);

		ptr[3].Position = Vector3(-size, size, 0.0f);
		ptr[3].Normal   = Vector3(0.0f, 0.0f, 1.0f);
		ptr[3].TexCoord = Vector2(0.0f, 1.0f);
		ptr[3].Tangent  = Vector3(1.0f, 0.0f, 0.0f);

		ptr[4].Position = Vector3(size, -size, 0.0f);
		ptr[4].Normal   = Vector3(0.0f, 0.0f, 1.0f);
		ptr[4].TexCoord = Vector2(1.0f, 0.0f);
		ptr[4].Tangent  = Vector3(1.0f, 0.0f, 0.0f);

		ptr[5].Position = Vector3(-size, -size, 0.0f);
		ptr[5].Normal   = Vector3(0.0f, 0.0f, 1.0f);
		ptr[5].TexCoord = Vector2(0.0f, 0.0f);
		ptr[5].Tangent  = Vector3(1.0f, 0.0f, 0.0f);

		m_WallVB.Unmap();
	}

	for (auto i = 0; i < FrameCount; ++i)
	{
		if (!m_TonemapCB[i].Init(m_pDevice.Get(), m_pPool[POOL_TYPE_RES], sizeof(CbTonemap)))
		{
			ELOG("Error : ConstantBuffer::Init() Failed.");
			return false;
		}
	}

	// �ϊ��s��p�̒萔�o�b�t�@�̐���
	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			// �萔�o�b�t�@������
			if (!m_TransformCB[i].Init(m_pDevice.Get(), m_pPool[POOL_TYPE_RES], sizeof(CbTransform)))
			{
				ELOG("Error : ConstantBuffer::Init() Failed.");
				return false;
			}

			// �J�����ݒ�.
			auto eyePos    = Vector3(0.0f, 1.0f, 2.0f);
			auto targetPos = Vector3::Zero;
			auto upward    = Vector3::UnitY;

			// ������p�ƃA�X�y�N�g��̐ݒ�.
			auto fovY = DirectX::XMConvertToRadians(37.5f);
			auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);

			// �ϊ��s���ݒ�.
			auto ptr = m_TransformCB[i].GetPtr<CbTransform>();
			ptr->View  = Matrix::CreateLookAt(eyePos, targetPos, upward);
			ptr->Proj  = Matrix::CreatePerspectiveFieldOfView(fovY, aspect, 0.1f, 1000.0f);
		}

		m_RotateAngle = DirectX::XMConvertToRadians(-60.0f);
	}

	// ���b�V���p�o�b�t�@�̐���
	{
		for (auto i = 0; i < FrameCount; ++i)
		{
			if (!m_MeshCB[i].Init(m_pDevice.Get(), m_pPool[POOL_TYPE_RES], sizeof(CbMesh)))
			{
				ELOG("Error : ConstantBuffer::Init() Failed.");
				return false;
			}

			auto ptr = m_MeshCB[i].GetPtr<CbMesh>();
			ptr->World = Matrix::Identity;
		}
	}

#if 0

	// �e�N�X�`�����[�h.
	{
		DirectX::ResourceUploadBatch batch(m_pDevice.Get());

		// �o�b�`�J�n.
		batch.Begin();

		//-------------------------------------------
		//�e�N�X�`���ǂݍ��݂��K�v�ȏ������ȉ��ɋL�q.
		//-------------------------------------------


		// �o�b�`�I��.
		auto future = batch.End(m_pQueue.Get());

		// ������ҋ@.
		future.wait();
	}

#endif

	// �J�n���Ԃ��L�^.
	m_StartTime = std::chrono::system_clock::now();

	return true;
}

void App::App::OnTerm()
{
	m_QuadVB.Term();

	for (auto i = 0; i < FrameCount; ++i)
	{
		m_TonemapCB  [i].Term();
		m_LightCB    [i].Term();
		m_CameraCB   [i].Term();
		m_TransformCB[i].Term();
	}

	// ���b�V���j��
	for (size_t i = 0; i < m_pMesh.size(); ++i)
	{
		SafeTerm(m_pMesh[i]);
	}
	m_pMesh.clear();
	m_pMesh.shrink_to_fit();

	// �}�e���A���j��
	m_Material.Term();

	// �ϊ��o�b�t�@�j��
	for (auto i = 0; i < FrameCount; ++i)
	{
		m_MeshCB[i].Term();
	}

	m_SceneColorTarget.Term();
	m_SceneDepthTarget.Term();

	m_pScenePSO   .Reset();
	m_SceneRootSig.Term();

	m_pTonemapPSO   .Reset();
	m_TonemapRootSig.Term();
}

LRESULT App::App::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	auto instance = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (msg)
	{
	case WM_CREATE:
		{
		auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lp);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		break;

	case WM_DESTROY:
		{ 
			PostQuitMessage(0);
		}
		break;

	case WM_MOVE:
		{
			instance->CheckSupportHDR();
		}
		break;

	case WM_DISPLAYCHANGE:
		{
			instance->CheckSupportHDR();
		}
		break;

	default:
	
		break;
	}

	if (instance != nullptr)
	{
		instance->MsgProc(hWnd, msg, wp, lp);
	}

	return DefWindowProc(hWnd, msg, wp, lp);
}
