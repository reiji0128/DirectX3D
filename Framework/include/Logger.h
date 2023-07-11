#pragma once

/// <summary>
/// ログの出力
/// </summary>
/// <param name="format">フォーマット</param>
/// <param name=""></param>
void OutputLog(const char* format, ...);

#ifndef DLOG

	#if defined(DEBUG) || defined(_DEBUG)
		#define DLOG( x, ... ) OutputLog( x "\n", ##__VA_ARGS__ );
	#else
		#define DLOG( x, ... ) 
	#endif

#endif

#ifndef ELOG

	#define ELOG( x, ... ) OutputLog( "[File : %s, Line : %d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__ )

#endif