#pragma once
#include "utils.h"
#include <sddl.h>
#include <LM.h>

void displayLocalLogons(std::wstring ServerName, std::wstring UserName);
std::wstring convertLogonTime(SYSTEMTIME logonTime);
void displaySessionLogons(std::wstring ServerName, std::wstring UserName);
SYSTEMTIME getLocalLogonTime(HKEY hCurrentUsers, std::wstring UserSid);
SYSTEMTIME getSessionLogonTime(DWORD dwSecond);
void who(const std::wstring& args);
