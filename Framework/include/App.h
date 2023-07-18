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
		/// �R���X�g���N�^
		/// </summary>
		/// <param name="windowWidth">�E�B���h�E�̕�</param>
		/// <param name="windowHeight">�E�B���h�E�̍���</param>
		App(uint32_t windowWidth, uint32_t windowHeight);

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~App();

		/// <summary>
		/// ���s����
		/// </summary>
		void Run();

	private:

		bool OnInit();
		void OnTerm();

		/// <summary>
		/// ����������
		/// </summary>
		/// <returns>
		/// true  :����������
		/// false :���������s
		/// </returns>
		bool InitApp();

		/// <summary>
		/// �I������
		/// </summary>
		void TermApp();

		/// <summary>
		/// �E�B���h�E�̏�����
		/// </summary>
		/// <returns>
		/// true  :����������
		/// false :���������s
		/// </returns>
		bool InitWindow();

		/// <summary>
		/// �E�B���h�E�̏I������
		/// </summary>
		void TermWindow();

		bool InitD3D();
		void TermD3D();

		/// <summary>
		/// ���C�����[�v
		/// </summary>
		void MainLoop();

		void Render();

		void MsgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

		void ChangeDisplayMode(bool hdr);

		/// <summary>
		/// �V�[���̕`��
		/// </summary>
		/// <param name="pCmdList"></param>
		void DrawScene(ID3D12GraphicsCommandList* pCmdList);

		/// <summary>
		/// ���b�V���̕`��
		/// </summary>
		/// <param name="pCmdList"></param>
		void DrawMesh(ID3D12GraphicsCommandList* pCmdList);

		/// <summary>
		/// �g�[���}�b�v�̓K�p
		/// </summary>
		/// <param name="pCmdList"></param>
		void DrawTonemap(ID3D12GraphicsCommandList* pCmdList);

		/// <summary>
		/// ��ʂɕ`�悵�A���̃t���[���̏������s��
		/// </summary>
		/// <param name="interval"></param>
		void Present(uint32_t interval);

		bool IsSupportHDR() const { return m_SupportHDR; }

		float GetMaxLuminance() const { return m_MaxLuminance; }

		float GetMinLuminance() const { return m_MinLuminance; }

		/// <summary>
		/// �f�B�X�v���C��HDR�o�͂��T�|�[�g���Ă��邩�ǂ������`�F�b�N
		/// </summary>
		void CheckSupportHDR();

		/// <summary>
		/// �E�B���h�E�v���V�[�W��
		/// </summary>
		/// <param name="hWnd">�E�B���h�E�n���h��</param>
		/// <param name="msg"></param>
		/// <param name="wp"></param>
		/// <param name="lp"></param>
		/// <returns></returns>
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

		enum POOL_TYPE
		{
			POOL_TYPE_RES = 0, // CBV�ESRV�EUAV
			POOL_TYPE_SMP = 1, // Sampler
			POOL_TYPE_RTV = 2, // RTV
			POOL_TYPE_DSV = 3, // DSV
			POOL_COUNT    = 4,
		};

		static const uint32_t FrameCount = 2; // �t���[���o�b�t�@��

		HINSTANCE m_hInst;  // �C���X�^���X�n���h��
		HWND      m_hWnd;   // �E�B���h�E�n���h��
		uint32_t  m_Width;  // �E�B���h�E�̕�
		uint32_t  m_Height; // �E�B���h�E�̍��� 

		ComPtr<IDXGIFactory4>      m_pFactory;                            // DXGI�t�@�N�g���[
		ComPtr<ID3D12Device>       m_pDevice;                             // �f�o�C�X
		ComPtr<ID3D12CommandQueue> m_pQueue;                              // �R�}���h�L���[
		ComPtr<IDXGISwapChain4>    m_pSwapChain;                          // �X���b�v�`�F�C��
		D3D::ColorTarget           m_ColorTarget[FrameCount];             // �J���[�^�[�Q�b�g
		D3D::DepthTarget           m_DepthTarget;                         // �[�x�^�[�Q�b�g
		D3D::DescriptorPool*       m_pPool[POOL_COUNT];                   // �f�B�X�N���v�^�v�[��
		D3D::CommandList           m_CommandList;                         // �R�}���h���X�g
		D3D::Fence                 m_Fence;                               // �t�F���X
		uint32_t                   m_FrameIndex;                          // �t���[���ԍ�
		D3D12_VIEWPORT             m_Viewport;                            // �r���[�|�[�g
		D3D12_RECT                 m_Scissor;                             // �V�U�[��`
													
		ComPtr<ID3D12PipelineState>            m_pScenePSO;               // �V�[���p�̃p�v���C���X�e�[�g
		D3D::RootSignature                     m_SceneRootSig;            // �V�[���p�̃��[�g�V�O�l�`��
		ComPtr<ID3D12PipelineState>            m_pTonemapPSO;             // �g�[���}�b�v�p�p�C�v���C���X�e�[�g
		D3D::RootSignature                     m_TonemapRootSig;          // �g�[���}�b�v�p���[�g�V�O�l�`��
		D3D::ColorTarget                       m_SceneColorTarget;        // �V�[���p�����_�[�^�[�Q�b�g
		D3D::DepthTarget                       m_SceneDepthTarget;        // �V�[���p�[�x�^�[�Q�b�g
		Resource::VertexBuffer                 m_QuadVB;                  // ���_�o�b�t�@
		Resource::VertexBuffer                 m_WallVB;                  // �Ǘp���_�o�b�t�@
		Resource::VertexBuffer                 m_FloorVB;                 // ���p���_�o�b�t�@
		std::vector<Render::Mesh*>             m_pMesh;                   // ���b�V��
		Resource::ConstantBuffer               m_TonemapCB[FrameCount];   // �g�[���}�b�v�p�̒萔�o�b�t�@
		Resource::ConstantBuffer               m_LightCB[FrameCount];     // ���C�g�p�̒萔�o�b�t�@
		Resource::ConstantBuffer               m_CameraCB[FrameCount];    // �J�����p�̒萔�o�b�t�@
		Resource::ConstantBuffer               m_TransformCB[FrameCount]; // �ϊ��p�̒萔�o�b�t�@
		Resource::ConstantBuffer               m_MeshCB[FrameCount];      // ���b�V���p�̒萔�o�b�t�@
		Resource::Material                     m_Material;    // �}�e���A��
		ComPtr<ID3D12PipelineState>            m_pPSO;        // �p�C�v���C���X�e�[�g�ł�.
		ComPtr<ID3D12RootSignature>            m_pRootSig;    // ���[�g�V�O�j�`���ł�.
		float                                  m_RotateAngle; // ��]�p�ł�.

		bool m_SupportHDR;     // HDR�f�B�X�v���C���T�|�[�g���Ă��邩�ǂ���
		float m_MaxLuminance;  // �f�B�X�v���C�̍ő�P�x�l
		float m_MinLuminance;  // �f�B�X�v���C�̍ŏ��P�x�l
		float m_BaseLuminance; // ��P�x�l
		float m_Exposure;      // �I���l
		int   m_TonemapType;   // �g�[���}�b�v�^�C�v
		int   m_ColorSpace;    // �o�͐F���
		int   m_LightType;     // ���C�g�^�C�v

		Render::ImguiRender m_ImguiRender;

		std::chrono::system_clock::time_point m_StartTime;    // �J�n����
	};
}