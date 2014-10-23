#pragma once

#ifdef _DEBUG
#define LOG_D(msg) std::cout \
  << "(" << std::this_thread::get_id() << ")  " \
/*<< " (" << static_cast<void*>(this) << ")  "*/ \
  << msg \
  << std::endl << std::flush
#else
#define LOG_D(msg) (void)0
#endif