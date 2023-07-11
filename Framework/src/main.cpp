#include "App.h"

#if defined(DEBUG) || defined(_DEBUG)

	#define _CRTDBG_MAP_ALLOC
	#include<crtdbg.h>

#endif

int wmain(int argc, wchar_t** argv, wchar_t** evnp)
{
#if defined(DEBUG) || defined(_DEBUG)

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#endif

	_CrtSetBreakAlloc(156494);

	// アプリケーションを実行
	App::App app(960, 540);
	app.Run();

	return 0;
}