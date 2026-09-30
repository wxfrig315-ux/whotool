#include "who.h"
#include <iostream>
#include <shellapi.h>

void displayLocalLogons(std::wstring ServerName, std::wstring UserName)
{
	bool first = TRUE;
	std::wstringstream	logger;
	HKEY	usersKey = nullptr;
	DWORD	index = 0;
	DWORD	subKeyNameSize = 0;
	FILETIME	lastWriteTime{};
	std::vector<TCHAR>	subKeyNameBuf(MAX_PATH);
	std::wstring subKeyName;
	PSID	sid = nullptr;
	std::vector<TCHAR> userNameBuf(1024);
	std::vector<TCHAR> domainNameBuf(1024);
	DWORD userNameSize{};
	DWORD domainNameSize{};
	SID_NAME_USE sidType{};
	SYSTEMTIME loggonTime{};
	std::wstring loggonTimeString;
	if (ServerName.empty() == FALSE)
	{
		logger << L"Connecting to Registry of " << ServerName << L"...\n";
		print_line(logger.str());
		logger.str(L"");

		if (RegConnectRegistryW(ServerName.c_str(), HKEY_USERS, &usersKey) != ERROR_SUCCESS) 
		{

			logger << L"Error opening HKEY_USERS for " << ServerName << L"\n";
			logger << checkError(logger.str());
			print_line(logger.str());
			logger.str(L"");
		}

	}
	else
	{
		if (RegOpenKey(HKEY_USERS, NULL, &usersKey) != ERROR_SUCCESS) 
		{

			logger << checkError(L"Error opening HKEY_USERS");
			print_line(logger.str());
			logger.str(L"");
		}
	}

	//
	// Enumerate keys under HKEY_USERS
	//
	subKeyNameSize = subKeyNameBuf.size();
	userNameSize = (DWORD)userNameBuf.size();
	domainNameSize = (DWORD)domainNameBuf.size();
	while (!RegEnumKeyExW(usersKey,
		index, subKeyNameBuf.data(), &subKeyNameSize,
		NULL, NULL, NULL, &lastWriteTime))
	{
		subKeyNameSize = subKeyNameBuf.size();
		userNameSize = (DWORD)userNameBuf.size();
		domainNameSize = (DWORD)domainNameBuf.size();

		subKeyName.assign(subKeyNameBuf.data(), subKeyNameBuf.data() + subKeyNameSize);
		if (_wcsicmp(subKeyName.c_str(), L".default") && !wcsstr(subKeyName.c_str(), L"Classes"))
		{
			if (ConvertStringSidToSidW(subKeyName.c_str(), &sid) != 0)
			{
				if (LookupAccountSidW(
					ServerName.c_str(),
					sid,
					userNameBuf.data(),
					&userNameSize,
					domainNameBuf.data(),
					&domainNameSize,
					&sidType
				))
				{
					std::wstring name(userNameBuf.data(), userNameSize);
					if (first && name.empty() == FALSE)
					{
						first = false;
						print_line(L"[*] Users logged on locally:\n\n");
					}
					loggonTime = getLocalLogonTime(usersKey, subKeyName);
					loggonTimeString = convertLogonTime(loggonTime);
					
					logger << L"\t" << domainNameBuf.data() << L"\\" << name << L"\t" << loggonTimeString << std::endl;

					print_line(logger.str());
					logger.str(L"");
				}
				
			}

		}
		index++;
	}
	RegCloseKey(usersKey);
	if (first && !UserName.empty())
	{
		print_line(L"No one is logged on locally.\n");
	}
	if (sid)
	{
		FreeSid(sid);
	}
}

std::wstring convertLogonTime(SYSTEMTIME logonTimes)
{
	std::wstring	logonDate;
	std::wstring	logonTime;
	std::vector<WCHAR> logonDateBuf(MAX_PATH);
	std::vector<WCHAR> logonTimeBuf(MAX_PATH);
	std::wstring logonString;
	DWORD dwTimeLength{}, dwDateLength{};


	GetDateFormatW(LOCALE_USER_DEFAULT, 0,
		&logonTimes, NULL,
		logonDateBuf.data(), logonDateBuf.size()/ sizeof(WCHAR));

	GetTimeFormatW(LOCALE_USER_DEFAULT, 0,
		&logonTimes, NULL,
		logonTimeBuf.data(), logonTimeBuf.size() / sizeof(WCHAR));


	logonDate.assign(logonDateBuf.data());
	logonTime.assign(logonTimeBuf.data());

	logonString = L"\t" + logonDate + L"\t" + logonTime;

	return logonString;
}

void displaySessionLogons(std::wstring ServerName, std::wstring UserName)
{
	LPSESSION_INFO_10 pBuf = NULL;
	LPSESSION_INFO_10 pTmpBuf;
	DWORD		dwLevel = 10;
	DWORD		dwPrefMaxLen = 0xFFFFFFFF;
	DWORD		dwEntriesRead = 0;
	DWORD		dwTotalEntries = 0;
	DWORD		dwResumeHandle = 0;
	DWORD		i;
	DWORD		dwTotalCount = 0;
	LPWSTR		pszClientName = NULL;
	LPWSTR		pszUserName = NULL;
	NET_API_STATUS nStatus;
	PSID			sid;
	DWORD		sidSize, domainNameSize;
	std::vector<BYTE>			sidBuffer(MAX_SID_SIZE);
	std::vector<TCHAR>		domainNameBuf(1024);
	std::wstring domainName;
	SID_NAME_USE sidType;
	std::wstring logonTimeString;
	BOOLEAN		first = TRUE;
	SYSTEMTIME	logonTime;
	std::wstringstream logger;
	do
	{
		nStatus = NetSessionEnum((LPWSTR)ServerName.c_str(),
			pszClientName,
			pszUserName,
			dwLevel,
			(LPBYTE*)&pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);
		

		if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA)
		{
			pTmpBuf = pBuf;
			
			if (pTmpBuf == nullptr)
			{
				break;
			}
			for (size_t i = 0; i < dwEntriesRead; i++)
			{
				sid = (PSID)sidBuffer.data();
				sidSize = sidBuffer.size();
				domainNameSize = domainNameBuf.size();

				if (pTmpBuf->sesi10_username[0])
				{
					if (first)
					{
						first = false;
						print_line(L"\n[*] Users logged on via resource shares:\n\n");
					}

					logonTime =  getSessionLogonTime(pTmpBuf->sesi10_time);
					logonTimeString = convertLogonTime(logonTime);

					if (LookupAccountNameW(pTmpBuf->sesi10_cname,
						pTmpBuf->sesi10_username,
						sid,
						&sidSize,
						domainNameBuf.data(),
						&domainNameSize,
						&sidType))
					{
						domainName.assign(domainNameBuf.data(), domainNameSize);
						logger << L"\t" << domainName << L"\\" << pTmpBuf->sesi10_username << L"\t" << logonTimeString;
					}
					else
					{
						domainName = L"(null)";
						logger << L"\t" << domainName << L"\\" << pTmpBuf->sesi10_username << L"\t" << logonTimeString;

					}
					print_line(logger.str());
					logger.str(L"");
				}
				pTmpBuf++;
			}
		}
		else 
		{

			logger << L"Unable to query resource logons\n";
			print_line(logger.str());
			logger.str(L"");
			first = FALSE;
		}

		if (pBuf != NULL) 
		{

			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}

	}
	while (nStatus != ERROR_MORE_DATA);
}

SYSTEMTIME getLocalLogonTime(HKEY hCurrentUsers, std::wstring UserSid)
{
	HKEY	hKey;
	std::wstring	keyName;
	std::vector<TCHAR>	classBuf(1024);
	DWORD	classSize;
	DWORD	subKeys;
	DWORD	maxSubKeyLen;
	DWORD	maxClassLen;
	DWORD	values;
	DWORD	maxValueNameLen;
	DWORD	maxValueLen;
	DWORD	secDescLen;
	FILETIME lastWriteTime;
	SYSTEMTIME	LogonTime{};

	keyName = UserSid + L"\\Volatile Environment";
	if (RegOpenKey(hCurrentUsers, keyName.data(), &hKey) != ERROR_SUCCESS) 
	{

		return SYSTEMTIME();
	}
	classSize = classBuf.size();
	if (RegQueryInfoKey(hKey,
		classBuf.data(), &classSize, NULL,
		&subKeys, &maxSubKeyLen,
		&maxClassLen,
		&values, &maxValueNameLen, &maxValueLen,
		&secDescLen,
		&lastWriteTime) != ERROR_SUCCESS) 
	{

		RegCloseKey(hKey);
		return SYSTEMTIME();
	}
	FileTimeToLocalFileTime(&lastWriteTime, &lastWriteTime);
	FileTimeToSystemTime(&lastWriteTime, &LogonTime);
	RegCloseKey(hKey);
	return LogonTime;
}

SYSTEMTIME getSessionLogonTime(DWORD dwSecond)
{
	if (dwSecond ==0 )
	{
		return SYSTEMTIME();
	}
	SYSTEMTIME LogonTime{};
	ULARGE_INTEGER fileTimeInteger;
	FILETIME     fileTime;

	GetLocalTime(&LogonTime);
	SystemTimeToFileTime(&LogonTime, &fileTime);
	fileTimeInteger = *(PULARGE_INTEGER)&fileTime;
	fileTimeInteger.QuadPart -= dwSecond * 10000000;
	fileTime = *(PFILETIME)&fileTimeInteger;
	FileTimeToSystemTime(&fileTime, &LogonTime);

	return LogonTime;
}

void who(const std::wstring& args)
{
	//parse parameter
	std::wstringstream logger;
	PSERVER_INFO_100 serverInfo = NULL;
	DWORD  entries{}, totalEntries{}, resumeHandle{}, i{};
	DWORD  status{};
	std::vector<std::wstring> argsV;
	BOOL showLocalOnly = FALSE;
	BOOL showAll = FALSE;
	LPWSTR* szArglist = NULL;
	std::wstring wszUserName;
	std::wstring wszServerName;
	int nArgs{};
	
	// Parse command line arguments

	if (!args.empty())
	{
		szArglist = CommandLineToArgvW(args.c_str(), &nArgs);

		if (NULL == szArglist)
		{
			logger << L"CommandLineToArgvW failed\n";
			print_line(logger.str());
			logger.str(L"");

		}
		argsV.assign(szArglist, szArglist + nArgs);
	}

	if (argsV.size() > 3)
	{
		print_line(L"More arguments than expected\n");
		logger.str(L"");
	}

	for (int arg = 0; arg < argsV.size(); arg++) 
	{
		switch (argsV[arg][0]) 
		{

		case L'-':
			switch (argsV[arg][1]) 
			{

			case L'l':
			case L'L':
			{
				showAll = TRUE;
				showLocalOnly = TRUE;
				break;
			}
			case L'x':
			{
				showAll = TRUE;
				break;
			}
			default:
				break;
			}
			break;
		default:
			if (argsV[arg][0] != L'\\') 
			{
				wszUserName = argsV[arg];

			}
			else 
			{
				wszServerName = argsV[arg];
				showLocalOnly = TRUE;
			}
		}


	}
	if (!wszUserName.empty())
	{
		do 
		{
			status = NetServerEnum(NULL, 100, (PBYTE*)&serverInfo, -1,
				&entries, &totalEntries, SV_TYPE_NT,
				NULL, &resumeHandle);

			if (status == NERR_Success || status == ERROR_MORE_DATA)
			{

				for (i = 0; i < entries; i++) 
				{

					logger << L"Checking  " << serverInfo[i].sv100_name  << L"...";
					print_line(logger.str());
					logger.str(L"");
					displayLocalLogons(serverInfo[i].sv100_name, wszUserName);
					if (!showLocalOnly)
					{
						displaySessionLogons(serverInfo[i].sv100_name, wszUserName);
					}
				}
			}
			else if (status != ERROR_MORE_DATA) 
			{

				logger << checkError(L"Error browing network");
				print_line(logger.str());
				logger.str(L"");
			}
			if (serverInfo) 
			{

				NetApiBufferFree(serverInfo);
				serverInfo = NULL;
			}
			if (status == NERR_Success)
			{
				logger << wszUserName << L" is not logged on to any computer.\n\n";
				print_line(logger.str());
				logger.str(L"");
			}
		} 
		while (status == ERROR_MORE_DATA);
	}
	else if(showAll)
	{
		// Display local logons

		displayLocalLogons(wszServerName, L"");

		// Display session logons

		if (!showLocalOnly)
		{
			displaySessionLogons(wszServerName, L"");
		}
	}
	if (szArglist)
	{
		LocalFree(szArglist);
	}
	print_line(L"\n\n[*] Final who\n");
}
