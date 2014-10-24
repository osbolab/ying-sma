#pragma once

#ifdef _DEBUG
#define LOG_D(msg) std::cout \
  << msg \
  << std::endl << std::flush
#define LOG_TD(msg) std::cout \
  << "(" << std::this_thread::get_id() << ")  " \
  << msg \
  << std::endl << std::flush
#else
#define LOG_D(msg) (void)0
#define LOG_TD(msg) (void)0
#endif