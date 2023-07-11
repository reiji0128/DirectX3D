#pragma once

/// <summary>
/// ���O�̏o��
/// </summary>
/// <param name="format">�t�H�[�}�b�g</param>
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