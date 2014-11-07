#pragma once

#include <cstdint>
#include <string>


namespace sma
{
// clang-format off
static std::uint8_t* uint8_p(char* s) { return reinterpret_cast<std::uint8_t*>(s); }
static std::uint8_t* uint8_p(unsigned char* s) { return reinterpret_cast<std::uint8_t*>(s); }

static const std::uint8_t* uint8_cp(std::string s) { return reinterpret_cast<const std::uint8_t*>(s.c_str()); }
static const std::uint8_t* uint8_cp(const char* s) { return reinterpret_cast<const std::uint8_t*>(s); }
static const std::uint8_t* uint8_cp(const unsigned char* s) { return reinterpret_cast<const std::uint8_t*>(s); }

static char* char_p(std::uint8_t* b) { return reinterpret_cast<char*>(b); }
static unsigned char* uchar_p(std::uint8_t* b) { return reinterpret_cast<unsigned char*>(b); }

static const char* char_cp(const std::uint8_t* b) { return reinterpret_cast<const char*>(b); }
static const unsigned char* uchar_cp(const std::uint8_t* b) { return reinterpret_cast<const unsigned char*>(b); }

static std::string copy_string(const std::uint8_t* s) { return std::string(char_cp(s)); }
static std::string copy_string(const std::uint8_t* s, std::size_t len) { return std::string(char_cp(s), char_cp(s)+len); }
// clang-format on
}
