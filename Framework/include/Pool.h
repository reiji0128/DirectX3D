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
		/// �R���X�g���N�^
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
		/// �f�X�g���N�^
		/// </summary>
		~Pool()
		{
			Term();
		}

		/// <summary>
		/// ������
		/// </summary>
		/// <param name="count">�m�ۂ���A�C�e����</param>
		/// <returns>
		/// true  : ����������
		/// false : ���������s
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

			// �C���f�b�N�X��U��
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
		/// �I������
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
		/// �A�C�e���̊m��
		/// </summary>
		/// <param name="func">���[�U�[�ɂ�鏉��������</param>
		/// <returns>�m�ۂ����A�C�e���̃|�C���^</returns>
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

			// ���������蓖��
			auto val = new((void*)item) T();

			// �������̕K�v������ΌĂяo��
			if (func != nullptr)
			{
				func(item->m_Index, val);
			}

			return val;
		}

		/// <summary>
		/// �A�C�e���̊J��
		/// </summary>
		/// <param name="pValue">�������A�C�e���̃|�C���^</param>
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

	// �Q�b�^�[ //

		/// <summary>
		/// ���A�C�e�����̎擾
		/// </summary>
		/// <returns>���A�C�e����</returns>
		uint32_t GetSize() const { return m_Capacity; }

		/// <summary>
		/// �g�p���̃A�C�e�����̎擾
		/// </summary>
		/// <returns>�g�p���̃A�C�e����</returns>
		uint32_t GetUsedCount() const { return m_Count; }

		/// <summary>
		/// ���p�\�ȃA�C�e�����̎擾
		/// </summary>
		/// <returns>���p�\�ȃA�C�e����</returns>
		uint32_t GetAvailableCount() const { return m_Capacity - m_Count; }

	private:

		struct Item
		{
			T        m_Value; // �l
			uint32_t m_Index; // �C���f�b�N�X
			Item*    m_pNext; // ���̃A�C�e���ւ̃|�C���^
			Item*    m_pPrev; // �O�̃A�C�e���ւ̃|�C���^

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
		/// �A�C�e���̎擾
		/// </summary>
		/// <param name="index">�擾����A�C�e���̃C���f�b�N�X</param>
		/// <returns>�A�C�e���̃|�C���^</returns>
		Item* GetItem(uint32_t index)
		{
			assert(0 <= index && index <= m_Capacity + 2);

			return reinterpret_cast<Item*>(m_pBuffer + sizeof(Item) * index);
		}

		/// <summary>
		/// �A�C�e���Ƀ����������蓖�Ă�
		/// </summary>
		/// <param name="index">�擾����A�C�e���̃C���f�b�N�X</param>
		/// <returns>�A�C�e���̃|�C���^</returns>
		Item* AssignItem(uint32_t index)
		{
			assert(0 <= index && index <= m_Capacity + 2);

			auto buf = (m_pBuffer + sizeof(Item) * index);

			return new(buf) Item;
		}

		uint8_t*   m_pBuffer;  // �o�b�t�@
		Item*      m_pActive;  // �A�N�e�B�u�A�C�e���̃|�C���^
		Item*      m_pFree;    // �t���[�A�C�e���̃|�C���^
		uint32_t   m_Capacity; // ���A�C�e����
		uint32_t   m_Count;    // �m�ۂ����A�C�e����
		std::mutex m_Mutex;    // �~���[�e�b�N�X

		Pool            (const Pool&) = delete;
		void operator = (const Pool&) = delete;
	};
}