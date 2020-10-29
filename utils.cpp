#include "utils.h"

inline std::wstring Utils::to_wstring(const std::string& str, const std::locale& loc)
{
	std::vector<wchar_t> buf(str.size());
	std::use_facet<std::ctype<wchar_t>>(loc).widen(str.data(), str.data() + str.size(), buf.data());

	return std::wstring(buf.data(), buf.size());
}

inline std::string Utils::to_string(const std::wstring& str, const std::locale& loc)
{
	std::vector<char> buf(str.size());
	std::use_facet<std::ctype<wchar_t>>(loc).narrow(str.data(), str.data() + str.size(), '?', buf.data());

	return std::string(buf.data(), buf.size());
}

CString Utils::ConvertSidToString(PSID pSID)
{
	ATLASSERT(pSID != NULL);

	if (pSID == NULL)
		AtlThrow(E_POINTER);

	LPTSTR pszSID = NULL;
	if (!ConvertSidToStringSid(pSID, &pszSID))
		AtlThrowLastWin32();

	CString strSID(pszSID);

	LocalFree(pszSID);
	pszSID = NULL;

	return strSID;
}

std::string Utils::GetCurrentUserSid()
{
	HANDLE hToken = NULL;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		//_tprintf(_T("OpenProcessToken failed. GetLastError returned: %d\n"),
		GetLastError();
		return false;
	}

	DWORD dwBufferSize = 0;
	if (!GetTokenInformation(hToken, TokenUser, NULL, 0, &dwBufferSize) && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
	{
		//_tprintf(_T("GetTokenInformation failed. GetLastError returned: %d\n"),
		GetLastError();

		// Cleanup
		CloseHandle(hToken);
		hToken = NULL;

		return false;
	}



	std::vector<BYTE> buffer;
	buffer.resize(dwBufferSize);
	PTOKEN_USER pTokenUser = reinterpret_cast<PTOKEN_USER>(&buffer[0]);

	if (!GetTokenInformation(
		hToken,
		TokenUser,
		pTokenUser,
		dwBufferSize,
		&dwBufferSize))
	{
		//_tprintf(_T("2 GetTokenInformation failed. GetLastError returned: %d\n"),
		GetLastError();

		// Cleanup
		CloseHandle(hToken);
		hToken = NULL;

		return false;
	}


	if (!IsValidSid(pTokenUser->User.Sid))
	{
		//_tprintf(_T("The owner SID is invalid.\n"));

		// Cleanup
		CloseHandle(hToken);
		hToken = NULL;

		return false;
	}

	//return to_string(ConvertSidToString(pTokenUser->User.Sid).GetString());
	return ConvertSidToString(pTokenUser->User.Sid).GetString();
}