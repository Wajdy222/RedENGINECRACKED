#pragma once
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <GUI/imgui/imgui.h>
#include "functions.h"
#include <GUI\imgui\TextEditor.h>
#include <GUI\imgui\ImGuiFileDialog.h>

#include <boost/filesystem.hpp>

namespace Menu
{
	namespace Options
	{
		static bool anticheatBypass = true;
		static char searchName[48] = "Search for Resource";
	}
	static bool isOpen = true;
	static TextEditor editor;
	static TextEditor dump;

	void Init();
	void Render();
}