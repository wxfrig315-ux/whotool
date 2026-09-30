#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#pragma comment(lib, "Netapi32.lib")

std::wstring checkError(std::wstring msg);

// Write a wide string to the console (UTF-16) followed by a newline.
void print_line(const std::wstring& msg);
