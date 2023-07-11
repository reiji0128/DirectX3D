#pragma once
#include <string>
#include <Shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

/// <summary>
/// ファイルパスの検索
/// </summary>
/// <param name="filename">検索するファイルパス</param>
/// <param name="result">検索結果の格納先</param>
/// <returns>
/// true  : ファイルを発見
/// false : ファイルが見つかりません
/// </returns>
/// <memo>
/// 検索ルールは以下の通り.
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
/// ファイルパスの検索
/// </summary>
/// <param name="filename">検索するファイルパス</param>
/// <param name="result">検索結果の格納先</param>
/// <returns>
/// true  : ファイルを発見
/// false : ファイルが見つかりません
/// </returns>
/// <memo>
/// 検索ルールは以下の通り.
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
/// ディレクトリパスを削除し、ファイル名を返却
/// </summary>
/// <param name="path">ディレクトリパスを取り除くファイルパス</param>
/// <returns>ファイル名</returns>
std::string RemoveDirectoryPathA(const std::string& path);

/// <summary>
/// ディレクトリパスを削除し、ファイル名を返却
/// </summary>
/// <param name="path">ディレクトリパスを取り除くファイルパス</param>
/// <returns>ファイル名</returns>
std::wstring RemoveDirectoryPathW(const std::wstring& path);

/// <summary>
/// ディレクトリ名の取得
/// </summary>
/// <param name="path">ファイルパス</param>
/// <returns>ディレクトリ名</returns>
std::string GetDirectoryPathA(const char* filePath);

/// <summary>
/// ディレクトリ名の取得
/// </summary>
/// <param name="path">ファイルパス</param>
/// <returns>ディレクトリ名</returns>
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