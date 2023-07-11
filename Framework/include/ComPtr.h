#pragma once
#include <wrl/client.h>

// スマートポインタ
template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;