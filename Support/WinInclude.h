#pragma once

#define NOMINMAX

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdlib>
#include <wincodec.h>
#include <iostream>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;


#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif