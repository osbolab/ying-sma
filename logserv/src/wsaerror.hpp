#pragma once

#include <winsock.h>
#include <string>


extern std::string WSAGetLastErrorMessage(char const* prefix, int error = 0);