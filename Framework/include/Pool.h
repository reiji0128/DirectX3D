#pragma once
#include <cstdint>
#include <mutex>
#include <cassert>
#include <functional>

namespace System
{
	template<typename T>

	class Pool
	{
	public:

		/// <summary>
		/// コンストラクタ
		/// </summary>
		Pool()
		:m_pBuffer (nullptr)
		,m_pActive (nullptr)
		,m_pFree   (nullptr)
		,m_Capacity(0)
		,m_Count   (0)
		{
		}

		/// <summary>
		/// デストラクタ
		/// </summary>
		~Pool()
		{
			Term();
		}

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="count">確保するアイテム数</param>
		/// <returns>
		/// true  : 初期化成功
		/// false : 初期化失敗
		/// </returns>
		bool Init(uint32_t count)
		{
			std::lock_guard<std::mutex> guard(m_Mutex);

			m_pBuffer = static_cast<uint8_t*>(malloc(sizeof(Item) * (count + 2)));

			if (m_pBuffer == nullptr)
			{
				return false;
			}

			m_Capacity = count;

			// インデックスを振る
			for (auto i = 2u, j = 0u; i < m_Capacity + 2; ++i, ++j)
			{
				auto item = GetItem(i);
				item->m_Index = j;
			}

			m_pActive = GetItem(0);
			m_pActive->m_pPrev  = m_pActive->m_pNext = m_pActive;
			m_pActive->m_Index = uint32_t(-1);

			m_pFree = GetItem(1);
			m_pFree->m_Index = uint32_t(-2);

			for (auto i = 1u; i < m_Capacity + 2; ++i)
			{
				GetItem(i)->m_pPrev = nullptr;
				GetItem(i)->m_pNext = GetItem(i + 1);
			}

			GetItem(m_Capacity + 1)->m_pPrev = m_pFree;

			m_Count = 0;

			return true;
		}

		/// <summary>
		/// 終了処理
		/// </summary>
		void Term()
		{
			std::lock_guard<std::mutex> guard(m_Mutex);

			if (m_pBuffer)
			{
				free(m_pBuffer);
				m_pBuffer = nullptr;
			}

			m_pActive  = nullptr;
			m_pFree    = nullptr;
			m_Capacity = 0;
			m_Count    = 0;
		}

		/// <summary>
		/// アイテムの確保
		/// </summary>
		/// <param name="func">ユーザーによる初期化処理</param>
		/// <returns>確保したアイテムのポインタ</returns>
		T* Alloc(std::function<void(uint32_t, T*)> func = nullptr)
		{
			std::lock_guard<std::mutex>guard(m_Mutex);

			if (m_pFree->m_pNext == m_pFree || m_Count + 1 > m_Capacity)
			{
				return nullptr;
			}

			auto item = m_pFree->m_pNext;
			m_pFree->m_pNext = item->m_pNext;

			item->m_pPrev = m_pActive->m_pPrev;
			item->m_pNext = m_pActive;
			item->m_pPrev->m_pNext = item->m_pNext->m_pPrev = item;

			m_Count++;

			// メモリ割り当て
			auto val = new((void*)item) T();

			// 初期化の必要があれば呼び出す
			if (func != nullptr)
			{
				func(item->m_Index, val);
			}

			return val;
		}

		/// <summary>
		/// アイテムの開放
		/// </summary>
		/// <param name="pValue">解放するアイテムのポインタ</param>
		void Free(T* pValue)
		{
			if (pValue == nullptr)
			{
				return;
			}

			std::lock_guard<std::mutex> guard(m_Mutex);

			auto item = reinterpret_cast<Item*>(pValue);

			item->m_pPrev->m_pNext = item->m_pNext;
			item->m_pNext->m_pPrev = item->m_pPrev;

			item->m_pPrev = nullptr;
			item->m_pNext = m_pFree->m_pNext;

			m_pFree->m_pNext = item;
			m_Count--;
		}

	// ゲッター //

		/// <summary>
		/// 総アイテム数の取得
		/// </summary>
		/// <returns>総アイテム数</returns>
		uint32_t GetSize() const { return m_Capacity; }

		/// <summary>
		/// 使用中のアイテム数の取得
		/// </summary>
		/// <returns>使用中のアイテム数</returns>
		uint32_t GetUsedCount() const { return m_Count; }

		/// <summary>
		/// 利用可能なアイテム数の取得
		/// </summary>
		/// <returns>利用可能なアイテム数</returns>
		uint32_t GetAvailableCount() const { return m_Capacity - m_Count; }

	private:

		struct Item
		{
			T        m_Value; // 値
			uint32_t m_Index; // インデックス
			Item*    m_pNext; // 次のアイテムへのポインタ
			Item*    m_pPrev; // 前のアイテムへのポインタ

			Item()
				:m_Value()
				,m_Index(0)
				,m_pNext(nullptr)
				,m_pPrev(nullptr)
			{
			}

			~Item()
			{
			}
		};

		/// <summary>
		/// アイテムの取得
		/// </summary>
		/// <param name="index">取得するアイテムのインデックス</param>
		/// <returns>アイテムのポインタ</returns>
		Item* GetItem(uint32_t index)
		{
			assert(0 <= index && index <= m_Capacity + 2);

			return reinterpret_cast<Item*>(m_pBuffer + sizeof(Item) * index);
		}

		/// <summary>
		/// アイテムにメモリを割り当てる
		/// </summary>
		/// <param name="index">取得するアイテムのインデックス</param>
		/// <returns>アイテムのポインタ</returns>
		Item* AssignItem(uint32_t index)
		{
			assert(0 <= index && index <= m_Capacity + 2);

			auto buf = (m_pBuffer + sizeof(Item) * index);

			return new(buf) Item;
		}

		uint8_t*   m_pBuffer;  // バッファ
		Item*      m_pActive;  // アクティブアイテムのポインタ
		Item*      m_pFree;    // フリーアイテムのポインタ
		uint32_t   m_Capacity; // 総アイテム数
		uint32_t   m_Count;    // 確保したアイテム数
		std::mutex m_Mutex;    // ミューテックス

		Pool            (const Pool&) = delete;
		void operator = (const Pool&) = delete;
	};
}