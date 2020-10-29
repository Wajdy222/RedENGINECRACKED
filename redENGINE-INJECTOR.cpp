#include <iostream>
#include <random>
#include <string>
#include <fstream>
#include <sstream>
#include <strsafe.h>
#

#include "Auth.h"
#include "memory.h"
#include "injection.h"

std::string MenuHandler()
{
	system("CLS");

	std::cout << "Please select a game:" << std::endl;

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::vector<std::string> games = Auth::Internal::userInfo["data"]["products"].get<std::vector<std::string>>();

	int count = 1;

	std::vector<std::string> cgames;

	for (auto& game : games)
		if(game.find("menu_") == std::string::npos)
		{
			std::cout << "    [";
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
			std::cout << count;
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			std::cout << "] | " << game.c_str() << std::endl;
			count++;
			cgames.push_back(game);
		}

	int choice;

	std::cout << "Choice: ";

	do
		std::cin >> choice;
	while (choice <= 0 && choice > games.size());

	std::cout << std::endl;

	return cgames[choice - 1];
}

std::string randomstring(std::string::size_type length)
{
	static auto& chrs = "0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	thread_local static std::mt19937 rg{ std::random_device{}() };
	thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

	std::string s;

	s.reserve(length);

	while (length--)
		s += chrs[pick(rg)];

	return s;
}

void DeleteMe()
{
	TCHAR szModuleName[MAX_PATH];
	TCHAR szCmd[2 * MAX_PATH];
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };

	GetModuleFileName(NULL, szModuleName, MAX_PATH);

	StringCbPrintf(szCmd, 2 * MAX_PATH, TEXT("cmd.exe /C ping 1.1.1.1 -n 1 -w 3000 > Nul & Del /f /q \"%s\""), szModuleName);

	CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

int main(int argc, char* argv[])
{
#ifndef _WIN32
	std::cout << "This app only runs on windows." << std::endl;
	ExitProcess(0);
#endif

	if (!(argc == 2 && std::string(argv[1]) == "-start"))
	{
		std::string randomName = randomstring(16).append(".exe");

		HRESULT dl = URLDownloadToFile(nullptr, "https://redengine.eu/download.php?file=launcher", randomName.c_str(), 0, nullptr);

		if (dl == S_OK)
		{
			SHELLEXECUTEINFO info = { 0 };
			info.cbSize = sizeof(SHELLEXECUTEINFO);
			info.fMask = SEE_MASK_NOCLOSEPROCESS;
			info.hwnd = NULL;
			info.lpVerb = NULL;
			info.lpFile = randomName.c_str();
			info.lpParameters = "-start";
			info.lpDirectory = NULL;
			info.nShow = SW_SHOW;
			info.hInstApp = NULL;

			ShellExecuteEx(&info);
			DeleteMe();
		}
		else
		{
			std::cout << "URLDownloadToFile > 0x" << std::hex << dl << std::endl;
			std::cout << "Tried to download to " << randomName << std::endl;
			system("PAUSE");
		}

		return 0;
	}

	std::string gameToInject;

	try
	{
		Auth::Internal::Connect();

		if (!Auth::User::HasAuthenticated())
		{
			std::cout << "You haven't been successfully authenticated." << std::endl;
			system("PAUSE");
			ExitProcess(0);
		}
	}
	//catch (websocketpp::exception & ec) { std::cout << "Auth error: " << ec.what() << std::endl; }
	//catch (std::exception& ec) { std::cout << "Auth error: " << ec.what() << std::endl; }
	catch (...) { std::cout << "Crashed while authing" << std::endl; system("PAUSE"); ExitProcess(0); }

	try
	{
		gameToInject = MenuHandler();
	}
	//catch (std::exception& ec) { std::cout << "Menu error: " << ec.what() << std::endl; }
	catch (...) { std::cout << "Crashed while in menu" << std::endl; system("PAUSE"); ExitProcess(0); }

	try
	{
		if (Injector::inject(gameToInject))
		{
			std::cout << "Injected! Quitting in 5 seconds...";
			std::this_thread::sleep_for(std::chrono::seconds(5));
			ExitProcess(0);
		}
		else
		{
			system("PAUSE");
			ExitProcess(0);
		}
	}
	//catch (std::exception & ec) { std::cout << "Injecting error: " << ec.what() << std::endl; }
	catch (...) { std::cout << "Crashed while injecting" << std::endl; system("PAUSE"); ExitProcess(0); }
}