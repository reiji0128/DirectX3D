#pragma once

/// <summary>
/// nullptrを考慮したdelete処理
/// </summary>
/// <param name="ptr">ポインタ</param>
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
/// nullptr を考慮したdelete[] 処理
/// </summary>
/// <param name="ptr">ポインタ配列</param>
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
///  nullptr を考慮したRelease()処理
/// </summary>
/// <param name="ptr">ポインタ</param>
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
/// nullptr を考慮してTerm() メソッドを呼び出し, delete 処理
/// </summary>
/// <param name="ptr">ポインタ</param>
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