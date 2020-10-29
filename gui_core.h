#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <GUI/kiero/kiero.h>
#include <GUI/imgui/imgui.h>
#include <GUI/imgui/imgui_impl_win32.h>
#include <GUI/imgui/imgui_impl_dx11.h>

#include "menu.h"

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void InitImGui();

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

namespace GuiCore
{
	DWORD WINAPI MainThread();
	DWORD WINAPI KeybindThread();
}