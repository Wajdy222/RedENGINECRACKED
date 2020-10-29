#define WIN32_LEAN_AND_MEAN
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <iostream>
#include <fstream>
#include <direct.h>
#include <random>
#include <regex>
#include <boost/thread.hpp>

#include "utils.h"
#include "Auth.h"

// ImGui Libraries
#include "gui_core.h"

// Include Libraries
#include "memory.h"
#include "core.h"
#include "functions.h"

DWORD __stdcall Core::Init(HMODULE hModule)
{
	try
	{
		Auth::Internal::Connect();
	}
	catch (...) { MessageBoxA(NULL, "Auth Crashed", "redENGINE", MB_OK); ExitProcess(0); }

	if (Auth::Internal::userInfo["status"].get<std::string>() == "offline")
		return TRUE;

	if (!Auth::User::HasAuthenticated())
	{
		throw std::exception("Tampering Detected, uploading to FiveM's database...");
		return TRUE;
	}
	else
		Functions::setAuthData(Auth::Internal::userInfo);
	try
	{
		std::string tempPath = getenv("TEMP");
		if (boost::filesystem::exists(tempPath + "/../../Roaming/discord/0.0.306/modules/discord_rpc"))
		{
			boost::filesystem::rename(tempPath.append("/../../Roaming/discord/0.0.306/modules/discord_rpc"), tempPath + Functions::random_string() /*std::string("_disabled")*/);
			for (int i = 0; i < 6; i++)
			{
				HANDLE discord = OpenProcess(PROCESS_ALL_ACCESS, false, Memory::GetProcessID("Discord.exe"));

				DWORD result = WAIT_OBJECT_0;
				while (result == WAIT_OBJECT_0)
				{
					result = WaitForSingleObject(discord, 100);
					TerminateProcess(discord, 1);
				}

				CloseHandle(discord);
			}
		}
	}
	catch (std::exception&) { }

	try
	{
		if (Auth::User::IsUserAdmin())
		{
			/*AllocConsole();
			AttachConsole(Memory::GetProcessID("FiveM_GTAProcess.exe"));
			freopen("CONIN$", "r", stdin);
			freopen("CONOUT$", "w+", stdout);
			std::cout.clear();*/

			/*
			char orig[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
			const char patch[] = { 0x48, 0x89, 0x5C, 0x24, 0x08 };
			uint64_t addr = Memory::GetModuleBase("citizen-resources-core.dll") + Settings::bypassAddress;
			VirtualProtect((LPVOID)addr, sizeof(orig), 0x40, NULL);
			ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(addr), &orig, 5, NULL);
			std::cout << "Bytes From " << std::hex << addr << ": " << (int)orig[0] << "|" << (int)orig[1] << "|" << (int)orig[2] << "|" << (int)orig[3] << "|" << (int)orig[4] << std::endl;
			*/
			std::cout << "Auth Response: " << Auth::Internal::userInfo.dump() << std::endl;
			std::cout << "Patch Offset (PatternScanning): " << std::hex << Memory::PatternScan(Memory::GetThisModuleHandle("citizen-resources-core.dll"), "E9 ?? ?? ?? ?? 57 48 83 EC 20 48 8B F9") << std::endl;
			std::cout << "Patch Offset (Manual Scan): " << std::hex << Memory::GetModuleBase("citizen-resources-core.dll") + Settings::bypassAddress << std::endl;

			std::cout << "Allocated memory at >> " << std::hex << Memory::GetModuleBase("citizen-resources-core.dll") << std::dec << std::endl;
		}
	}
	catch(...) { }

	try
	{
		boost::thread dumpThread(&Functions::DumpResources);
		boost::thread acThread(&Functions::BypassEventHandlers);
		boost::thread nativeThread(&Functions::OverwriteNatives);
		boost::thread mountInit(&Functions::MountInit);

		boost::thread menuThread(&GuiCore::MainThread);
		boost::thread keybindThread(&GuiCore::KeybindThread);
		//boost::thread identifierThread(&Functions::Identifiers);

		menuThread.join();
		keybindThread.join();
		dumpThread.join();
		acThread.join();
		nativeThread.join();
		mountInit.join();
		//identifierThread.join();
	}
	catch(...) { }
	
	return TRUE;
}