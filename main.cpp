#include <iostream>
#include <string>
#include <io.h>
#include <fcntl.h>

#include "who.h"

static void print_banner()
{
    std::wcout << L"==============================================\n";
    std::wcout << L"        whotool - Who is logged on?\n";
    std::wcout << L"==============================================\n";
    std::wcout << L"Select mode:\n";
    std::wcout << L"  1) All logons (local + network sessions) on this computer\n";
    std::wcout << L"  2) Local logons only on this computer\n";
    std::wcout << L"  3) Local logons on a specific computer\n";
    std::wcout << L"  4) Find which computer a user is logged on to\n";
    std::wcout << L"  0) exit\n";
    std::wcout << L"> ";
    std::wcout.flush();
}

int main()
{
    // Enable wide (UTF-16) console mode so Unicode / Vietnamese output works.
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    for (;;)
    {
        print_banner();
        std::wstring line;
        if (!std::getline(std::wcin, line)) break;

        if (line == L"1")
        {
            who(L"-x");
        }
        else if (line == L"2")
        {
            who(L"-l");
        }
        else if (line == L"3")
        {
            std::wstring computer;
            std::wcout << L"Computer name (e.g. \\\\SERVER): ";
            std::wcout.flush();
            if (!std::getline(std::wcin, computer)) break;
            if (!computer.empty())
            {
                who(L"-x " + computer);
            }
        }
        else if (line == L"4")
        {
            std::wstring user;
            std::wcout << L"Username: ";
            std::wcout.flush();
            if (!std::getline(std::wcin, user)) break;
            if (!user.empty())
            {
                who(user);
            }
        }
        else if (line == L"0" || line == L"q" || line == L"quit" || line == L"exit")
        {
            break;
        }
        else if (!line.empty())
        {
            std::wcout << L"[-] Unknown option: " << line << L"\n";
        }
    }
    return 0;
}
