#pragma once

/// <summary>
/// nullptr���l������delete����
/// </summary>
/// <param name="ptr">�|�C���^</param>
template<typename T>
inline void SafeDelete(T* ptr)
{
	if (ptr != nullptr)
	{
		delete ptr;
		ptr = nullptr;
	}
}

/// <summary>
/// nullptr ���l������delete[] ����
/// </summary>
/// <param name="ptr">�|�C���^�z��</param>
template<typename T>
inline void SafeDeleteArray(T*& ptr)
{
    if (ptr != nullptr)
    {
        delete[] ptr;
        ptr = nullptr;
    }
}

/// <summary>
///  nullptr ���l������Release()����
/// </summary>
/// <param name="ptr">�|�C���^</param>
template<typename T>
inline void SafeRelease(T*& ptr)
{
    if (ptr != nullptr)
    {
        ptr->Release();
        ptr = nullptr;
    }
}

/// <summary>
/// nullptr ���l������Term() ���\�b�h���Ăяo��, delete ����
/// </summary>
/// <param name="ptr">�|�C���^</param>
template<typename T>
inline void SafeTerm(T*& ptr)
{
    if (ptr != nullptr)
    {
        ptr->Term();
        delete ptr;
        ptr = nullptr;
    }
}