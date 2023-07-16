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
	// ウィンドウクラス名
	const auto className = TEXT("DirectX3D");

	/// <summary>
	/// 領域の交差を計算
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
		TONEMAP_NONE = 0, // トーンマップ無し
		TONEMAP_REINHARD, // Reinhardトーンマップ
		TONEMAP_GT,       // GTトーンマップ
	};

	struct alignas(256) CbTonemap
	{
		int   Type;          // トーンマップタイプ
		int   ColorSpace;    // 出力色空間
		float BaseLuminance; // 基準輝度値[nit]
		float MaxLuminance;  // 最大輝度値[nit]
	};

	// メッシュ用のコンスタントバッファーの構造体
	struct alignas(256) CbMesh
	{
		Matrix World; // ワールド行列
	};

	// 変換行列用のコンスタントバッファーの構造体
	struct alignas(256) CbTransform
	{
		Matrix View; // ビュー行列
		Matrix Proj; // 射影行列
	};

	// ライト用のコンスタントバッファーの構造体
	struct alignas(256) CbLight
	{
		Vector3 LightPosition;     // ライト位置
		float   LightInvSqrRadius; // ライトの逆2乗半径

		Vector3 LightColor;        // ライトカラー
		float   LightIntensity;    // ライト強度

		Vector3 LightForward;      // ライトの照射方向
		float   LightAngleScale;   // 1.0f / (cosInner - cosOuter)

		float   LightAngleOffset;  // -cosOuter * LightAngleScale
		int     LightType;         // ライトタイプ
		float   Padding[2];        // パディング
	};

	// カメラ用のコンスタントバッファーの構造体
	struct alignas(256) CbCamera
	{
		Vector3 CameraPosition;   // カメラ位置
	};

	struct alignas(256) CbMaterial
	{
		Vector3 BaseColor; // 基本色
		float   Alpha;     // 透過度
		float   Roughness; // 面の粗さ
		float   Metallic;  // 金属度
	};

	/// <summary>
	/// 色度を取得
	/// </summary>
	/// <param name="value"></param>
	/// <returns></returns>
	inline UINT16 GetChromaticityCoord(double value)
	{
		return UINT16(value * 50000);
	}

	/// <summary>
	/// ポイントライトパラメーターの計算
	/// </summary>
	/// <param name="pos">ライト座標</param>
	/// <param name="radius">ライトの半径</param>
	/// <param name="color">ライトカラー</param>
	/// <param name="intensity">ライト強度</param>
	/// <returns>ポイントパラメーター</returns>
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
	/// スポットライトパラメータの計算
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
	// 初期化が完了したか
	if (InitApp())
	{
		MainLoop();
	}

	TermApp();
}

bool App::App::InitApp()
{
	// ウィンドウの初期化に失敗したか
	if (!InitWindow())
	{
		return false;
	}

	// Direct3D12の初期化.
	if (!InitD3D())
	{
		return false;
	}

	// アプリケーション固有の初期化
	if (!OnInit())
	{
		return false;
	}

	// ウィンドウを表示.
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	// ウィンドウを更新.
	UpdateWindow(m_hWnd);

	// ウィンドウにフォーカスを設定.
	SetFocus(m_hWnd);

	return true;
}

void App::App::TermApp()
{
	// アプリケーション固有の終了処理
	OnTerm();

	// Direct3D 12の終了処理.
	TermD3D();

	// ウィンドウの終了処理
	TermWindow();
}

bool App::App::InitWindow()
{
	// インスタンスハンドルの取得
	auto hInst = GetModuleHandle(nullptr);

	// インスタンスがなければ
	if (hInst == nullptr)
	{
		return false;
	}


	// ウィンドウの指定
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

	// ウィンドウの登録
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	// インスタンスハンドルの設定
	m_hInst = hInst;

	// ウィンドウサイズの指定
	RECT rc = {};
	rc.right  = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	// ウィンドウサイズの調整
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	// ウィンドウの生成
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

	// ウィンドウハンドルがなければ
	if (m_hWnd == nullptr)
	{
		return false;
	}

	return true;
}

void App::App::TermWindow()
{
	// ウィンドウの登録解除
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

	// WM_QUITメッセージでなければ
	while (WM_QUIT != msg.message)
	{
		// アプリケーション宛にメッセージが送られているか
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

		// デバッグレイヤーを有効化
		if (SUCCEEDED(hr))
		{
			pDebug->EnableDebugLayer();
		}
	}

//#endif

	// デバイスの生成
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
//		// Error時にブレークを発生させる
//		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
//	}

#endif

	// コマンドキューの生成
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

	// スワップチェインの生成
	{
		// DXGIファクトリーの生成
		ComPtr<IDXGIFactory4> pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(pFactory.GetAddressOf()));

		if (FAILED(hr))
		{
			return false;
		}

		// スワップチェインの設定
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

		// スワップチェインの生成
		ComPtr<IDXGISwapChain> pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, pSwapChain.GetAddressOf());

		// スワップチェインの生成に失敗したか
		if (FAILED(hr))
		{
			return false;
		}

		// IDXGISwapChain3を取得
		hr = pSwapChain.As(&m_pSwapChain);

		if (FAILED(hr))
		{
			return false;
		}

		// バックバッファ番号を取得
		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		// 不要になったので解放
		pFactory.Reset();
		pSwapChain.Reset();
	}

	// ディスクリプタプールの生成
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

	// コマンドリストの生成.
	{
		if (!m_CommandList.Init(m_pDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, FrameCount))
		{
			return false;
		}
	}

	// レンダーターゲットビューの生成.
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

	// 深度ステンシルバッファの生成
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

	// フェンスの生成
	if (!m_Fence.Init(m_pDevice.Get()))
	{
		return false;
	}

	// ビューポートの設定
	{
		m_Viewport.TopLeftX = 0.0f;
		m_Viewport.TopLeftY = 0.0f;
		m_Viewport.Width = float(m_Width);
		m_Viewport.Height = float(m_Height);
		m_Viewport.MinDepth = 0.0f;
		m_Viewport.MaxDepth = 1.0f;
	}

	// シザー矩形の設定
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
	// GPU処理の完了を待機
	m_Fence.Sync(m_pQueue.Get());

	// フェンス破棄
	m_Fence.Term();

	// レンダーターゲットビューの破棄
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_ColorTarget[i].Term();
	}

	// 深度ステンシルビューの破棄
	m_DepthTarget.Term();

	// コマンドリストの破棄
	m_CommandList.Term();

	for (auto i = 0; i < POOL_COUNT; ++i)
	{
		if (m_pPool[i] != nullptr)
		{
			m_pPool[i]->Release();
			m_pPool[i] = nullptr;
		}
	}

	// スワップチェインの破棄
	m_pSwapChain.Reset();

	// コマンドキューの破棄
	m_pQueue.Reset();

	// デバイスの破棄
	m_pDevice.Reset();
}

void App::App::Render()
{
	// コマンドの記録を開始
	auto pCmd = m_CommandList.Reset();

	ID3D12DescriptorHeap* const pHeaps[] =
	{
		m_pPool[POOL_TYPE_RES]->GetHeap()
	};

	pCmd->SetDescriptorHeaps(1, pHeaps);

	{
		// ディスクリプタの取得
		auto handleRTV = m_SceneColorTarget.GetHandleRTV();
		auto handleDSV = m_SceneDepthTarget.GetHandleDSV();

		// 書き込み用リソースバリア設定
		DirectX::TransitionResource(pCmd,
			                        m_SceneColorTarget.GetResource(),
									D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
									D3D12_RESOURCE_STATE_RENDER_TARGET);

		// レンダーターゲットを設定
		pCmd->OMSetRenderTargets(1, &handleRTV->HandleCPU, FALSE, &handleDSV->HandleCPU);

		// レンダーターゲットをクリア
		m_SceneColorTarget.ClearView(pCmd);
		m_SceneDepthTarget.ClearView(pCmd);

		// シーンの描画
		DrawScene(pCmd);

		// 読み込み用リソースバリア設定
		DirectX::TransitionResource(pCmd,
			                        m_SceneColorTarget.GetResource(),
		                            D3D12_RESOURCE_STATE_RENDER_TARGET,
		                            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	// フレームバッファに描画
	{
		// 書き込み用リソースバリア設定
		DirectX::TransitionResource(pCmd,
			                        m_ColorTarget[m_FrameIndex].GetResource(),
			                        D3D12_RESOURCE_STATE_PRESENT,
			                        D3D12_RESOURCE_STATE_RENDER_TARGET);

		// ディスクリプタ取得
		auto handleRTV = m_ColorTarget[m_FrameIndex].GetHandleRTV();
		auto handleDSV = m_DepthTarget.GetHandleDSV();

		// レンダーターゲットを設定
		pCmd->OMSetRenderTargets(1, &handleRTV->HandleCPU, FALSE, &handleDSV->HandleCPU);

		// レンダーターゲットをクリア
		m_ColorTarget[m_FrameIndex].ClearView(pCmd);
		m_DepthTarget.ClearView(pCmd);

		// トーンマップを適用
		DrawTonemap(pCmd);

		// 表示用リソースバリアの設定
		DirectX::TransitionResource(pCmd,
			                        m_ColorTarget[m_FrameIndex].GetResource(),
			                        D3D12_RESOURCE_STATE_RENDER_TARGET,
			                        D3D12_RESOURCE_STATE_PRESENT);
	}

	// コマンドの記録を終了.
	pCmd->Close();

	// コマンドを実行
	ID3D12CommandList* pLists[] = { pCmd };
	m_pQueue->ExecuteCommandLists(1, pLists);

	// 画面に表示
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

			// HDRモード.
			case 'H':
			{
				ChangeDisplayMode(true);
			}
			break;

			// SDRモード.
			case 'S':
			{
				ChangeDisplayMode(false);
			}
			break;

			// トーンマップなし.
			case 'N':
			{
				m_TonemapType = TONEMAP_NONE;
			}
			break;

			// Reinhardトーンマップ.
			case 'R':
			{
				m_TonemapType = TONEMAP_REINHARD;
			}
			break;

			// GTトーンマップ
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
				TEXT("HDRがサポートされていないディスプレイです."),
				TEXT("HDR非サポート"),
				MB_OK | MB_ICONINFORMATION);

			ELOG("Error : Display not support HDR.");
			return;
		}

		auto hr = m_pSwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);

		if (FAILED(hr))
		{
			MessageBox(nullptr,
				TEXT("ITU-R BT.2100 PQ Systemの色域設定に失敗しました"),
				TEXT("色域設定失敗"),
				MB_OK | MB_ICONERROR);

			ELOG("Error : IDXGISwapChain::SetColorSpace1() Failed.");
			return;
		}

		DXGI_HDR_METADATA_HDR10 metaData = {};

		// ITU-R BT.2100の原刺激と白色点を設定
		metaData.RedPrimary[0] = GetChromaticityCoord(0.708);
		metaData.RedPrimary[1] = GetChromaticityCoord(0.292);
		metaData.BluePrimary[0] = GetChromaticityCoord(0.170);
		metaData.BluePrimary[1] = GetChromaticityCoord(0.797);
		metaData.GreenPrimary[0] = GetChromaticityCoord(0.131);
		metaData.GreenPrimary[1] = GetChromaticityCoord(0.046);
		metaData.WhitePoint[0] = GetChromaticityCoord(0.3127);
		metaData.WhitePoint[1] = GetChromaticityCoord(0.3290);

		// ディスプレイがサポートすると最大輝度値と最小輝度値を設定
		metaData.MaxMasteringLuminance = UINT(GetMaxLuminance() * 10000);
		metaData.MinMasteringLuminance = UINT(GetMinLuminance() * 0.001);

		// 最大値を 2000 [nit]に設定
		metaData.MaxContentLightLevel = 2000;

		hr = m_pSwapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(DXGI_HDR_METADATA_HDR10), &metaData);

		if (FAILED(hr))
		{
			ELOG("Error : IDXGISwapChain::SetHDRMetaData() Failed.");
		}

		m_BaseLuminance = 100.0f;
		m_MaxLuminance = GetMaxLuminance();

		// 成功したことを知らせるダイアログを出力
		std::string message;
		message += "HDRディスプレイ用に設定を変更しました\n\n";
		message += "色空間 : ITU-R BT.2100 PQ\n";
		message += "最大輝度値 : ";
		message += std::to_string(GetMaxLuminance());
		message += "[nit]\n";
		message += "最小輝度値 : ";
		message += std::to_string(GetMinLuminance());
		message += " [nit]\n";

		MessageBoxA(nullptr,
			        message.c_str(),
			        "HDR設定成功",
			        MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		auto hr = m_pSwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);

		if (FAILED(hr))
		{
			MessageBox(nullptr,
				TEXT("ITU-R BT.709の色域設定に失敗しました"),
				TEXT("色域設定失敗"),
				MB_OK | MB_ICONERROR);

			ELOG("Error : IDXGISwapChain::SetColorSpace1() Failed.");

			return;
		}

		DXGI_HDR_METADATA_HDR10 metaData = {};

		// ITU -R BT.709の原刺激と白色点を設定
		metaData.RedPrimary  [0] = GetChromaticityCoord(0.640);
		metaData.RedPrimary  [1] = GetChromaticityCoord(0.330);
		metaData.BluePrimary [0] = GetChromaticityCoord(0.300);
		metaData.BluePrimary [1] = GetChromaticityCoord(0.600);
		metaData.GreenPrimary[0] = GetChromaticityCoord(0.150);
		metaData.GreenPrimary[1] = GetChromaticityCoord(0.060);
		metaData.WhitePoint  [0] = GetChromaticityCoord(0.3127);
		metaData.WhitePoint  [1] = GetChromaticityCoord(0.3290);

		// ディスプレイがサポートすると最大輝度値と最小輝度値を設定
		metaData.MaxMasteringLuminance = UINT(GetMaxLuminance() * 10000);
		metaData.MinMasteringLuminance = UINT(GetMinLuminance() * 0.001);

		// 最大値を 100[nit]に設定
		metaData.MaxContentLightLevel = 100;

		hr = m_pSwapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(DXGI_HDR_METADATA_HDR10), &metaData);

		if (FAILED(hr))
		{
			ELOG("Error : IDXGISwapChain::SetHDRMetaData() Failed.");
		}

		m_BaseLuminance = 100.0f;
		m_MaxLuminance  = 100.0f;

		// 成功したことを知らせるダイアログを出力
		std::string message;
		message += "SDRディスプレイ用に設定を変更しました\n\n";
		message += "色空間：ITU-R BT.709\n";
		message += "最大輝度値：";
		message += std::to_string(GetMaxLuminance());
		message += " [nit]\n";
		message += "最小輝度値：";
		message += std::to_string(GetMinLuminance());
		message += " [nit]\n";

		MessageBoxA(nullptr,
			        message.c_str(),
			        "SDR設定成功",
			        MB_OK | MB_ICONINFORMATION);
	}
}

void App::App::DrawScene(ID3D12GraphicsCommandList* pCmdList)
{
	auto cameraPos = Vector3(-0.5f, 0.0f, 2.0f);

	// ライトバッファの更新
	{
		auto matrix = Matrix::CreateRotationY(m_RotateAngle);

		auto ptr = m_LightCB[m_FrameIndex].GetPtr<CbLight>();
		ptr->LightColor = Vector3(1.0f, 1.0f, 1.0f);
		ptr->LightForward = Vector3::TransformNormal(Vector3(0.0f, 1.0f, 1.0f), matrix);
		ptr->LightIntensity = 5.0f;
		m_RotateAngle += 0.01f;
	}

	// カメラバッファの更新
	{
		auto ptr = m_CameraCB[m_FrameIndex].GetPtr<CbCamera>();
		ptr->CameraPosition = cameraPos;
	}

	// メッシュのワールド行列の更新
	{
		auto ptr = m_MeshCB[m_FrameIndex].GetPtr<CbMesh>();
		ptr->World = Matrix::Identity;
	}

	// 変換パラメータの更新
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

	// オブジェクトの描画
	{
		pCmdList->SetGraphicsRootDescriptorTable(1, m_MeshCB[m_FrameIndex].GetHandleGPU());
		DrawMesh(pCmdList);
	}
}

void App::App::DrawMesh(ID3D12GraphicsCommandList* pCmdList)
{
	for (size_t i = 0; i < m_pMesh.size(); ++i)
	{
		// マテリアルIDを取得
		auto id = m_pMesh[i]->GetMaterialID();

		// テクスチャを設定
		pCmdList->SetGraphicsRootDescriptorTable(4, m_Material.GetTextureHandle(id, Resource::TU_BASE_COLOR));
		pCmdList->SetGraphicsRootDescriptorTable(5, m_Material.GetTextureHandle(id, Resource::TU_METALLIC));
		pCmdList->SetGraphicsRootDescriptorTable(6, m_Material.GetTextureHandle(id, Resource::TU_ROUGHNESS));
		pCmdList->SetGraphicsRootDescriptorTable(7, m_Material.GetTextureHandle(id, Resource::TU_NORMAL));

		// メッシュの描画
		m_pMesh[i]->Draw(pCmdList);
	}
}

void App::App::DrawTonemap(ID3D12GraphicsCommandList* pCmdList)
{
	// 定数バッファ更新
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
	// 画面に表示
	m_pSwapChain->Present(interval, 0);

	// 完了待ち
	m_Fence.Wait(m_pQueue.Get(), INFINITE);

	// フレーム番号を更新
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

void App::App::CheckSupportHDR()
{
	// 何も作られていない場合は処理しない.
	if (m_pSwapChain == nullptr || m_pFactory == nullptr || m_pDevice == nullptr)
	{
		return;
	}

	HRESULT hr = S_OK;

	// ウィンドウ領域を取得.
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

	// 各ディスプレイを調べる.
	while (pAdapter->EnumOutputs(i, &currentOutput) != DXGI_ERROR_NOT_FOUND)
	{
		auto ax1 = rect.left;
		auto ay1 = rect.top;
		auto ax2 = rect.right;
		auto ay2 = rect.bottom;

		// ディスプレイの設定を取得.
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

		// 領域が一致するかどうか調べる.
		int intersectArea = ComputeIntersectionArea(ax1, ay1, ax2, ay2, bx1, by1, bx2, by2);
		if (intersectArea > bestIntersectArea)
		{
			bestOutput = currentOutput;
			bestIntersectArea = intersectArea;
		}

		i++;
	}

	// 一番適しているディスプレイ.
	ComPtr<IDXGIOutput6> pOutput6;
	hr = bestOutput.As(&pOutput6);
	if (FAILED(hr))
	{
		return;
	}

	// 出力設定を取得.
	DXGI_OUTPUT_DESC1 desc1;
	hr = pOutput6->GetDesc1(&desc1);
	if (FAILED(hr))
	{
		return;
	}

	// 色空間が ITU-R BT.2100 PQをサポートしているかどうかチェック.
	m_SupportHDR = (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
	m_MaxLuminance = desc1.MaxLuminance;
	m_MinLuminance = desc1.MinLuminance;
}

bool App::App::OnInit()
{
	// メッシュのロード
	{
		std::wstring path;

		// ファイルパスを検索
		if (!SearchFilePath(L"Resource/material_test/material_test.obj", path))
		{
			ELOG("Error : File Not Found.");
			return false;
		}

		std::wstring dir = GetDirectoryPath(path.c_str());

		std::vector<Resource::ResMesh>     resMesh;
		std::vector<Resource::ResMaterial> resMaterial;

		// メッシュリソースのロード
		if (!Resource::LoadMesh(path.c_str(), resMesh, resMaterial))
		{
			ELOG("Error : Load Mesh Failed. filepath = %ls", path.c_str());
			return false;
		}

		// メモリを予約
		m_pMesh.reserve(resMesh.size());

		// メッシュを初期化
		for (size_t i = 0; i < resMesh.size(); ++i)
		{
			// メッシュ生成
			auto mesh = new(std::nothrow)Render::Mesh();

			// チェック
			if (mesh == nullptr)
			{
				ELOG("Error : Out of memory.");
				return false;
			}

			// 初期化処理
			if (!mesh->Init(m_pDevice.Get(), resMesh[i]))
			{
				ELOG("Error : Mesh Initialize Failed.");
				delete mesh;
				return false;
			}

			// 成功したら登録
			m_pMesh.push_back(mesh);
		}

		// メモリ最適化
		m_pMesh.shrink_to_fit();

		// マテリアル初期化
		if (!m_Material.Init(m_pDevice.Get(), m_pPool[POOL_TYPE_RES], sizeof(CbMaterial), resMaterial.size()))
		{
			ELOG("Error : Material::Init() Failed.");
			return false;
		}

		// リソースバッチを用意
		DirectX::ResourceUploadBatch batch(m_pDevice.Get());

		// バッチ開始
		batch.Begin();

		// テクスチャとマテリアルを設定
		{
			/* ここではマテリアルが決め打ちであることを前提にハードコーディングしています. */
			m_Material.SetTexture(0, Resource::TU_BASE_COLOR, dir + L"wall_bc.dds", batch);
			m_Material.SetTexture(0, Resource::TU_METALLIC  , dir + L"wall_m.dds", batch);
			m_Material.SetTexture(0, Resource::TU_ROUGHNESS , dir + L"wall_r.dds", batch);
			m_Material.SetTexture(0, Resource::TU_NORMAL    , dir + L"wall_n.dds", batch);

			m_Material.SetTexture(1, Resource::TU_BASE_COLOR, dir + L"matball_bc.dds", batch);
			m_Material.SetTexture(1, Resource::TU_METALLIC  , dir + L"matball_m.dds", batch);
			m_Material.SetTexture(1, Resource::TU_ROUGHNESS , dir + L"matball_r.dds", batch);
			m_Material.SetTexture(1, Resource::TU_NORMAL    , dir + L"matball_n.dds", batch);
		}

		// バッチ終了
		auto future = batch.End(m_pQueue.Get());

		// バッチ完了を待機
		future.wait();
	}

	// ライトバッファの設定
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

	// カメラバッファの設定
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

	// シーン用カラーターゲットの生成
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

	// シーン用深度ターゲットの生成
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

	// ルートシグニチャの生成
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

	// パイプラインステートの生成
	{
		std::wstring vsPath;
		std::wstring psPath;

		// 頂点シェーダーの検索
		if (!SearchFilePath(L"BasicVS.cso", vsPath))
		{
			ELOG("Error : Vertex Shader Not Found.");
			return false;
		}

		// ピクセルシェーダーの検索
		if (!SearchFilePath(L"DirectionalLightPS.cso", psPath))
		{
			ELOG("Error : Pixel Shader Node Found.");
			return false;
		}

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		// 頂点シェーダー読み込み
		auto hr = D3DReadFileToBlob(vsPath.c_str(), pVSBlob.GetAddressOf());

		if (FAILED(hr))
		{
			ELOG("Error : D3DReadFiledToBlob() Failed. path = %ls", vsPath.c_str());
			return false;
		}

		// ピクセルシェーダー読み込み
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

		// グラフィックスパイプラインステートの設定
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

		// パイプラインステートを生成
		hr = m_pDevice->CreateGraphicsPipelineState(&desc,
			                                        IID_PPV_ARGS(m_pScenePSO.GetAddressOf()));

		if (FAILED(hr))
		{
			ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. retcode = 0x%x", hr);
			return false;
		}
	}

	// トーンマップ用ルートシグネチャの生成
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

	// トーンマップ用パイプラインステートの生成
	{
		std::wstring vsPath;
		std::wstring psPath;

		// 頂点シェーダを検索
		if (!SearchFilePath(L"TonemapVS.cso", vsPath))
		{
			ELOG("Error : Vertex Shader Not Found.");
			return false;
		}

		// ピクセルシェーダを検索
		if (!SearchFilePath(L"TonemapPS.cso", psPath))
		{
			ELOG("Error : Pixel Shader Node Found.");
			return false;
		}

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		// 頂点シェーダを読み込む
		auto hr = D3DReadFileToBlob(vsPath.c_str(), pVSBlob.GetAddressOf());
		if (FAILED(hr))
		{
			ELOG("Error : D3DReadFiledToBlob() Failed. path = %ls", vsPath.c_str());
			return false;
		}

		// ピクセルシェーダを読み込む
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

		// グラフィックパイプラインステートの設定
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

		// パイプラインステートを生成
		hr = m_pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pTonemapPSO.GetAddressOf()));
		if (FAILED(hr))
		{
			ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. retcode = 0x%x", hr);
			return false;
		}
	}

	// 頂点バッファの生成
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

	// 壁用頂点バッファの生成
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

	// 変換行列用の定数バッファの生成
	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			// 定数バッファ初期化
			if (!m_TransformCB[i].Init(m_pDevice.Get(), m_pPool[POOL_TYPE_RES], sizeof(CbTransform)))
			{
				ELOG("Error : ConstantBuffer::Init() Failed.");
				return false;
			}

			// カメラ設定.
			auto eyePos    = Vector3(0.0f, 1.0f, 2.0f);
			auto targetPos = Vector3::Zero;
			auto upward    = Vector3::UnitY;

			// 垂直画角とアスペクト比の設定.
			auto fovY = DirectX::XMConvertToRadians(37.5f);
			auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);

			// 変換行列を設定.
			auto ptr = m_TransformCB[i].GetPtr<CbTransform>();
			ptr->View  = Matrix::CreateLookAt(eyePos, targetPos, upward);
			ptr->Proj  = Matrix::CreatePerspectiveFieldOfView(fovY, aspect, 0.1f, 1000.0f);
		}

		m_RotateAngle = DirectX::XMConvertToRadians(-60.0f);
	}

	// メッシュ用バッファの生成
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

	// テクスチャロード.
	{
		DirectX::ResourceUploadBatch batch(m_pDevice.Get());

		// バッチ開始.
		batch.Begin();

		//-------------------------------------------
		//テクスチャ読み込みが必要な処理を以下に記述.
		//-------------------------------------------


		// バッチ終了.
		auto future = batch.End(m_pQueue.Get());

		// 完了を待機.
		future.wait();
	}

#endif

	// 開始時間を記録.
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

	// メッシュ破棄
	for (size_t i = 0; i < m_pMesh.size(); ++i)
	{
		SafeTerm(m_pMesh[i]);
	}
	m_pMesh.clear();
	m_pMesh.shrink_to_fit();

	// マテリアル破棄
	m_Material.Term();

	// 変換バッファ破棄
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
