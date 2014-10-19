#pragma once

#ifdef _DEBUG
#define LOG_D(msg) std::cout << static_cast<void*>(this) << "  " << msg << std::endl
#else
#define LOG_D(msg) (void)0
#endif