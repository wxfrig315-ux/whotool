#include "utils.h"
#include <iostream>

std::wstring checkError(std::wstring msg)
{
    std::wstringstream wss;
    wss << "[-] " << msg << L" failed, Err=" << GetLastError() << L"\n";
    return wss.str();
}

void print_line(const std::wstring& msg)
{
    std::wcout << msg << std::endl;
}
