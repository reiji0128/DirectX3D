#include "Fence.h"

D3D::Fence::Fence()
    :m_pFence (nullptr)
    ,m_Event  (nullptr)
    ,m_Counter(0)
{
}

D3D::Fence::~Fence()
{
    Term();
}

bool D3D::Fence::Init(ID3D12Device* pDevice)
{
    if (pDevice == nullptr)
    {
        return false;
    }

    // イベントの生成
    m_Event = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

    if (m_Event == nullptr)
    {
        return false;
    }

    // フェンスの生成
    auto hr = pDevice->CreateFence(0,
                                   D3D12_FENCE_FLAG_NONE,
                                   IID_PPV_ARGS(m_pFence.GetAddressOf()));

    if (FAILED(hr))
    {
        return false;
    }

    // カウンタの設定
    m_Counter = 1;

    return true;
}

void D3D::Fence::Term()
{
    // ハンドルを閉じる
    if (m_Event != nullptr)
    {
        CloseHandle(m_Event);
        m_Event = nullptr;
    }

    // フェンスオブジェクトを破棄
    m_pFence.Reset();

    // カウンターのリセット
    m_Counter = 0;
}

void D3D::Fence::Wait(ID3D12CommandQueue* pQueue, UINT timeout)
{
    if (pQueue == nullptr)
    {
        return;
    }

    const auto fenceValue = m_Counter;

    // シグナル処理
    auto hr = pQueue->Signal(m_pFence.Get(), fenceValue);

    if (FAILED(hr))
    {
        return;
    }

    // カウンターを増やす
    m_Counter++;

    // 次のフレームの描画準備ができてなければ待機
    if (m_pFence->GetCompletedValue() < fenceValue)
    {
        // 完了時にイベントを設定
        auto hr = m_pFence->SetEventOnCompletion(fenceValue, m_Event);

        if (FAILED(hr))
        {
            return;
        }

        // 待機処理
        if (WAIT_OBJECT_0 != WaitForSingleObjectEx(m_Event, timeout, FALSE))
        {
            return;
        }
    }
}

void D3D::Fence::Sync(ID3D12CommandQueue* pQueue)
{
    if (pQueue == nullptr)
    {
        return;
    }

    // シグナル処理
    auto hr = pQueue->Signal(m_pFence.Get(), m_Counter);

    if (FAILED(hr))
    {
        return;
    }

    // 完了時にイベントを設定
    hr = m_pFence->SetEventOnCompletion(m_Counter, m_Event);

    if (FAILED(hr))
    {
        return;
    }

    // 待機処理
    if (WAIT_OBJECT_0 != WaitForSingleObjectEx(m_Event, INFINITE, FALSE))
    {
        return;
    }

    // カウンターを増やす
    m_Counter++;
}
