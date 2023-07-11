#pragma once
#include <d3d12.h>
#include "ComPtr.h"

namespace D3D
{
	class Fence
	{
	public:

		/// <summary>
		/// コンストラクタ
		/// </summary>
		Fence();

		/// <summary>
		/// デストラクタ
		/// </summary>
		~Fence();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="pDevice">デバイス</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(ID3D12Device* pDevice);

		/// <summary>
		/// 終了処理
		/// </summary>
		void Term();

		/// <summary>
		/// シグナル状態になるまで指定された時間待機
		/// </summary>
		/// <param name="pQueue">コマンドキュー</param>
		/// <param name="timeout">タイムアウト時間(ミリ秒)</param>
		void Wait(ID3D12CommandQueue* pQueue, UINT timeout);

		/// <summary>
		/// シグナル状態になるまで待機
		/// </summary>
		/// <param name="pQueue">コマンドキュー</param>
		void Sync(ID3D12CommandQueue* pQueue);

	private:

		ComPtr<ID3D12Fence> m_pFence;  // フェンス
		HANDLE              m_Event;   // イベント
		UINT                m_Counter; // カウンター

		Fence           (const Fence&) = delete; // アクセス禁止
		void operator = (const Fence&) = delete; // アクセス禁止
	};
}