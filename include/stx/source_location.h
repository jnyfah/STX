/**
 * @file source_location.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-04
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#pragma once

#include <cstdint>

#include "stx/config.h"

STX_BEGIN_NAMESPACE

///
/// The `SourceLocation`  class represents certain information about the source
/// code, such as file names, line numbers, and function names. Previously,
/// functions that desire to obtain this information about the call site (for
/// logging, testing, or debugging purposes) must use macros so that predefined
/// macros like `__LINE__` and `__FILE__` are expanded in the context of the
/// caller. The `SourceLocation` class provides a better alternative.
///
///
/// based on: https://en.cppreference.com/w/cpp/utility/source_location
///
struct [[nodiscard]] SourceLocation
{
  static constexpr SourceLocation current(
#if STX_HAS_BUILTIN(FILE) || (defined(__cpp_lib_source_location) && \
                              __cpp_lib_source_location >= 201907L)
      char const *file = __builtin_FILE(),
#elif defined(__FILE__)
      char const    *file     = __FILE__,
#else
      char const    *file = "unknown",
#endif

#if STX_HAS_BUILTIN(FUNCTION) || (defined(__cpp_lib_source_location) && \
                                  __cpp_lib_source_location >= 201907L)
      char const *function = __builtin_FUNCTION(),
#else
      char const    *function = "unknown",
#endif

#if STX_HAS_BUILTIN(LINE) || (defined(__cpp_lib_source_location) && \
                              __cpp_lib_source_location >= 201907L)
      uint_least32_t line = __builtin_LINE(),
#elif defined(__LINE__)
      uint_least32_t line     = __LINE__,
#else
      uint_least32_t line = 0,
#endif

#if STX_HAS_BUILTIN(COLUMN) || (defined(__cpp_lib_source_location) && \
                                __cpp_lib_source_location >= 201907L)
      uint_least32_t column = __builtin_COLUMN()
#else
      uint_least32_t column   = 0
#endif
  )
  {
    SourceLocation loc{};
    loc.line_     = line;
    loc.column_   = column;
    loc.file_     = file;
    loc.function_ = function;
    return loc;
  }

  constexpr SourceLocation() :
      line_{0}, column_{0}, file_{""}, function_{""}
  {}

  /// return the column number represented by this object
  constexpr uint_least32_t column() const
  {
    return column_;
  }

  /// return the line number represented by this object
  constexpr uint_least32_t line() const
  {
    return line_;
  }

  /// return the file name represented by this object
  constexpr char const *file_name() const
  {
    return file_;
  }

  /// return the name of the function represented by this object, if any
  constexpr char const *function_name() const
  {
    return function_;
  }

private:
  uint_least32_t line_;
  uint_least32_t column_;
  char const    *file_;
  char const    *function_;
};

STX_END_NAMESPACE
