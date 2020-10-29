#pragma once

#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <sstream>
#include <string>
#include <lmcons.h>
#include <Sddl.h>
#include <stdio.h>
#include <Windows.h>
#include <shlwapi.h>
#include <accctrl.h>
#include <aclapi.h>
#include <shlobj_core.h>
#include <tlhelp32.h>
#include <atldef.h>
#include <atlstr.h>
#include <vector>

namespace Utils
{
	inline std::wstring to_wstring(const std::string& str, const std::locale& loc = std::locale{});
	inline std::string to_string(const std::wstring& str, const std::locale& loc = std::locale{});
	CString ConvertSidToString(PSID pSID);
	std::string GetCurrentUserSid();
}