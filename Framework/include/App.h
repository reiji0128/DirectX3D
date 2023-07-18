#pragma once
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "ComPtr.h"
#include "DescriptorPool.h"
#include "ColorTarget.h"
#include "DepthTarget.h"
#include "CommandList.h"
#include "RootSignature.h"
#include "Fence.h"
#include "Mesh.h"
#include "Texture.h"
#include "InlineUtil.h"
#include "ConstantBuffer.h"
#include "Material.h"
#include "ImguiRender.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace App
{
	class App
	{
	public:

		/// <summary>
		/// コンストラクタ
		/// </summary>
		/// <param name="windowWidth">ウィンドウの幅</param>
		/// <param name="windowHeight">ウィンドウの高さ</param>
		App(uint32_t windowWidth, uint32_t windowHeight);

		/// <summary>
		/// デストラクタ
		/// </summary>
		~App();

		/// <summary>
		/// 実行処理
		/// </summary>
		void Run();

	private:

		bool OnInit();
		void OnTerm();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <returns>
		/// true  :初期化成功
		/// false :初期化失敗
		/// </returns>
		bool InitApp();

		/// <summary>
		/// 終了処理
		/// </summary>
		void TermApp();

		/// <summary>
		/// ウィンドウの初期化
		/// </summary>
		/// <returns>
		/// true  :初期化成功
		/// false :初期化失敗
		/// </returns>
		bool InitWindow();

		/// <summary>
		/// ウィンドウの終了処理
		/// </summary>
		void TermWindow();

		bool InitD3D();
		void TermD3D();

		/// <summary>
		/// メインループ
		/// </summary>
		void MainLoop();

		void Render();

		void MsgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

		void ChangeDisplayMode(bool hdr);

		/// <summary>
		/// シーンの描画
		/// </summary>
		/// <param name="pCmdList"></param>
		void DrawScene(ID3D12GraphicsCommandList* pCmdList);

		/// <summary>
		/// メッシュの描画
		/// </summary>
		/// <param name="pCmdList"></param>
		void DrawMesh(ID3D12GraphicsCommandList* pCmdList);

		/// <summary>
		/// トーンマップの適用
		/// </summary>
		/// <param name="pCmdList"></param>
		void DrawTonemap(ID3D12GraphicsCommandList* pCmdList);

		/// <summary>
		/// 画面に描画し、次のフレームの準備を行う
		/// </summary>
		/// <param name="interval"></param>
		void Present(uint32_t interval);

		bool IsSupportHDR() const { return m_SupportHDR; }

		float GetMaxLuminance() const { return m_MaxLuminance; }

		float GetMinLuminance() const { return m_MinLuminance; }

		/// <summary>
		/// ディスプレイがHDR出力をサポートしているかどうかをチェック
		/// </summary>
		void CheckSupportHDR();

		/// <summary>
		/// ウィンドウプロシージャ
		/// </summary>
		/// <param name="hWnd">ウィンドウハンドル</param>
		/// <param name="msg"></param>
		/// <param name="wp"></param>
		/// <param name="lp"></param>
		/// <returns></returns>
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

		enum POOL_TYPE
		{
			POOL_TYPE_RES = 0, // CBV・SRV・UAV
			POOL_TYPE_SMP = 1, // Sampler
			POOL_TYPE_RTV = 2, // RTV
			POOL_TYPE_DSV = 3, // DSV
			POOL_COUNT    = 4,
		};

		static const uint32_t FrameCount = 2; // フレームバッファ数

		HINSTANCE m_hInst;  // インスタンスハンドル
		HWND      m_hWnd;   // ウィンドウハンドル
		uint32_t  m_Width;  // ウィンドウの幅
		uint32_t  m_Height; // ウィンドウの高さ 

		ComPtr<IDXGIFactory4>      m_pFactory;                            // DXGIファクトリー
		ComPtr<ID3D12Device>       m_pDevice;                             // デバイス
		ComPtr<ID3D12CommandQueue> m_pQueue;                              // コマンドキュー
		ComPtr<IDXGISwapChain4>    m_pSwapChain;                          // スワップチェイン
		D3D::ColorTarget           m_ColorTarget[FrameCount];             // カラーターゲット
		D3D::DepthTarget           m_DepthTarget;                         // 深度ターゲット
		D3D::DescriptorPool*       m_pPool[POOL_COUNT];                   // ディスクリプタプール
		D3D::CommandList           m_CommandList;                         // コマンドリスト
		D3D::Fence                 m_Fence;                               // フェンス
		uint32_t                   m_FrameIndex;                          // フレーム番号
		D3D12_VIEWPORT             m_Viewport;                            // ビューポート
		D3D12_RECT                 m_Scissor;                             // シザー矩形
													
		ComPtr<ID3D12PipelineState>            m_pScenePSO;               // シーン用のパプラインステート
		D3D::RootSignature                     m_SceneRootSig;            // シーン用のルートシグネチャ
		ComPtr<ID3D12PipelineState>            m_pTonemapPSO;             // トーンマップ用パイプラインステート
		D3D::RootSignature                     m_TonemapRootSig;          // トーンマップ用ルートシグネチャ
		D3D::ColorTarget                       m_SceneColorTarget;        // シーン用レンダーターゲット
		D3D::DepthTarget                       m_SceneDepthTarget;        // シーン用深度ターゲット
		Resource::VertexBuffer                 m_QuadVB;                  // 頂点バッファ
		Resource::VertexBuffer                 m_WallVB;                  // 壁用頂点バッファ
		Resource::VertexBuffer                 m_FloorVB;                 // 床用頂点バッファ
		std::vector<Render::Mesh*>             m_pMesh;                   // メッシュ
		Resource::ConstantBuffer               m_TonemapCB[FrameCount];   // トーンマップ用の定数バッファ
		Resource::ConstantBuffer               m_LightCB[FrameCount];     // ライト用の定数バッファ
		Resource::ConstantBuffer               m_CameraCB[FrameCount];    // カメラ用の定数バッファ
		Resource::ConstantBuffer               m_TransformCB[FrameCount]; // 変換用の定数バッファ
		Resource::ConstantBuffer               m_MeshCB[FrameCount];      // メッシュ用の定数バッファ
		Resource::Material                     m_Material;    // マテリアル
		ComPtr<ID3D12PipelineState>            m_pPSO;        // パイプラインステートです.
		ComPtr<ID3D12RootSignature>            m_pRootSig;    // ルートシグニチャです.
		float                                  m_RotateAngle; // 回転角です.

		bool m_SupportHDR;     // HDRディスプレイをサポートしているかどうか
		float m_MaxLuminance;  // ディスプレイの最大輝度値
		float m_MinLuminance;  // ディスプレイの最小輝度値
		float m_BaseLuminance; // 基準輝度値
		float m_Exposure;      // 露光値
		int   m_TonemapType;   // トーンマップタイプ
		int   m_ColorSpace;    // 出力色空間
		int   m_LightType;     // ライトタイプ

		Render::ImguiRender m_ImguiRender;

		std::chrono::system_clock::time_point m_StartTime;    // 開始時刻
	};
}