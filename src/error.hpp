#pragma once

#ifndef AKBIT__SYSTEM__ERROR_HPP
#define AKBIT__SYSTEM__ERROR_HPP


namespace akbit::system::parsing
{
  enum class error_t
  {
    // Empty
    e_no_errors = 0,
    e_fatal_internal_error = 1,


    // Start up
    c_file_does_not_exist = 10,


    // Tokenization
    t_unexpected_eof = 1010,

    t_invalid_identifier = 1020,

    t_special_character_does_not_exist = 1030,
    t_misleading_character = 1031,


    // Parsing
    p_unexpected_token = 2010,


    // Optimization
    // ...


    // Translation
    // ...


    // Compilation
    // ...


    // Special
    s_not_implemented = 1000,
  };
}

#endif
