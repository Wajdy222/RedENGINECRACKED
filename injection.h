#pragma once

#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <C:\Users\moloz\Desktop\redENGINE\packages\boost\filesystem.hpp>
#include <urlmon.h>
#pragma  comment(lib, "urlmon.lib")
#pragma  comment(lib,"wininet.lib")

#include "MemEx.h"

#define _CRT_SECURE_NO_WARNINGS

namespace Injector
{
	std::string dllPath;

	inline bool file_exists(const std::string& name)
	{
		std::ifstream f(name.c_str());
		return f.good();
	}

	std::string processName(std::string game)
	{
		if (game == "FiveM")
			return "FiveM_GTAProcess.exe";
		else if (game == "RedM Executor")
			return "RedM.exe";
	}

	bool download(std::string game)
	{
		std::string downloadPath;

		char* buf = nullptr;
		size_t sz = 0;
		if (_dupenv_s(&buf, &sz, "localappdata") == 0 && buf != nullptr)
		{
			downloadPath = buf;
			free(buf);
		}
		else
			return false;

		if (!boost::filesystem::exists(downloadPath.append("\\Google\\")))
			boost::filesystem::create_directory(downloadPath);

		if (!boost::filesystem::exists(downloadPath.append("cached_files\\")))
			boost::filesystem::create_directory(downloadPath);

		if (boost::filesystem::exists(downloadPath.append("http_client.dll")))
			boost::filesystem::remove(downloadPath);

		dllPath = downloadPath;

		if (game == "FiveM")
			return (URLDownloadToFileA(nullptr, "https://redengine.eu/download.php?dll=redengine_executor", downloadPath.c_str(), 0, nullptr) == S_OK) ? true : false;

		return false;
	}

	bool inject(std::string game)
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		if (Memory::GetProcessID(processName(game)) == 0)
		{
			std::cout << "Waiting for ";
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
			std::cout << processName(game);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			std::cout << " to open" << std::endl;
		}
		else
		{
			std::cout << "Close Your ";
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
			std::cout << game;
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			std::cout << ". Closing the Injector" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(5));
			ExitProcess(0);
		}

		if (!download(game))
		{
			std::cout << "Injecting error #3" << std::endl;
			return false;
		}

		while (Memory::GetProcessID(processName(game)) == 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(125));

		if (processName(game) == "FiveM_GTAProcess.exe")
			while (Memory::GetProcessID("FiveM_ChromeBrowser") == 0)
				std::this_thread::sleep_for(std::chrono::milliseconds(125));

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		DWORD pid = Memory::GetProcessID(processName(game));

		HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
		if (!hProcess)
		{
			std::cout << "Can't Open Process, something's not right" << std::endl;
			return false;
		}

		char DllName[MAX_PATH];

		if (!GetFullPathName(dllPath.c_str(), MAX_PATH, DllName, NULL))
		{
			std::cout << "Injecting error #4" << std::endl;
			return false;
		}

		if (!file_exists(DllName))
		{
			std::cout << "Injecting error #2" << std::endl;
			return false;
		}
		else
		{
			MemEx* mem = new MemEx;
			try
			{
				std::cout << std::endl << "Injecting memory..." << std::endl;
				return mem->Open(hProcess) && mem->Inject(dllPath.c_str(), INJECTION_METHOD::LOAD_LIBRARY);
			}
			catch (...) { std::cout << "Exception" << std::endl; }

			/*void* allocated_memory = VirtualAllocEx(hProcess, nullptr, 0x10000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (!allocated_memory)
			{
				std::cout << "Failed allocating memory on " << processName(game) << std::endl;
				return false;
			}

			if (!WriteProcessMemory(hProcess, allocated_memory, dllPath.c_str(), MAX_PATH, nullptr))
			{
				std::cout << "Failed writing memory on " << processName(game) << std::endl;
				return false;
			}

			HANDLE injectThread = CreateRemoteThread(hProcess, nullptr, NULL, LPTHREAD_START_ROUTINE(LoadLibraryA), allocated_memory, NULL, nullptr);
			if (!injectThread)
			{
				std::cout << "Failed injecting in " << processName(game) << std::endl;
				return false;
			}

			CloseHandle(hProcess);
			VirtualFreeEx(hProcess, allocated_memory, NULL, MEM_RELEASE);*/

			return true;
		}
	};
}