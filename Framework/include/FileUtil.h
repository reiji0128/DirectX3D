#pragma once
#include <string>
#include <Shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

/// <summary>
/// �t�@�C���p�X�̌���
/// </summary>
/// <param name="filename">��������t�@�C���p�X</param>
/// <param name="result">�������ʂ̊i�[��</param>
/// <returns>
/// true  : �t�@�C���𔭌�
/// false : �t�@�C����������܂���
/// </returns>
/// <memo>
/// �������[���͈ȉ��̒ʂ�.
//      .\
//      ..\
//      ..\..\
//      .\res\
//      %EXE_DIR%\
//      %EXE_DIR%\..\
//      %EXE_DIR%\..\..\
//      %EXE_DIR%\res\
/// </memo>
bool SearchFilePathA(const char* filename, std::string& result);

/// <summary>
/// �t�@�C���p�X�̌���
/// </summary>
/// <param name="filename">��������t�@�C���p�X</param>
/// <param name="result">�������ʂ̊i�[��</param>
/// <returns>
/// true  : �t�@�C���𔭌�
/// false : �t�@�C����������܂���
/// </returns>
/// <memo>
/// �������[���͈ȉ��̒ʂ�.
//      .\
//      ..\
//      ..\..\
//      .\res\
//      %EXE_DIR%\
//      %EXE_DIR%\..\
//      %EXE_DIR%\..\..\
//      %EXE_DIR%\res\
/// </memo>
bool SearchFilePathW(const wchar_t* filename, std::wstring& result);

/// <summary>
/// �f�B���N�g���p�X���폜���A�t�@�C������ԋp
/// </summary>
/// <param name="path">�f�B���N�g���p�X����菜���t�@�C���p�X</param>
/// <returns>�t�@�C����</returns>
std::string RemoveDirectoryPathA(const std::string& path);

/// <summary>
/// �f�B���N�g���p�X���폜���A�t�@�C������ԋp
/// </summary>
/// <param name="path">�f�B���N�g���p�X����菜���t�@�C���p�X</param>
/// <returns>�t�@�C����</returns>
std::wstring RemoveDirectoryPathW(const std::wstring& path);

/// <summary>
/// �f�B���N�g�����̎擾
/// </summary>
/// <param name="path">�t�@�C���p�X</param>
/// <returns>�f�B���N�g����</returns>
std::string GetDirectoryPathA(const char* filePath);

/// <summary>
/// �f�B���N�g�����̎擾
/// </summary>
/// <param name="path">�t�@�C���p�X</param>
/// <returns>�f�B���N�g����</returns>
std::wstring GetDirectoryPathW(const wchar_t* filePath);


#if defined(UNICODE) || defined(_UNICODE)

    inline bool SearchFilePath(const wchar_t* filename, std::wstring& result)
    {
        return SearchFilePathW(filename, result);
    }
    
    inline std::wstring RemoveDirectoryPath(const std::wstring& path)
    {
        return RemoveDirectoryPathW(path);
    }
    
    inline std::wstring GetDirectoryPath(const wchar_t* path)
    {
        return GetDirectoryPathW(path);
    }

#else

    inline bool SearchFilePath(const char* filename, std::string& result)
    {
        return SearchFilePathA(filename, result);
    }
    
    inline std::string RemoveDirectoryPath(const std::string& path)
    {
        return RemoveDirectoryPathA(path);
    }
    
    inline std::string GetDirectoryPath(const char* path)
    {
        return GetDirectoryPathA(path);
    }

#endif//defined(UNICODE) || defined(_UNICODE)