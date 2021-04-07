#pragma once
#ifndef AKBIT__SYSTEM__ERROR_HANDLING_HPP
#define AKBIT__SYSTEM__ERROR_HANDLING_HPP

namespace akbit::system
{
  namespace parsing
  {
    struct LexerState;
    struct ParserState;
  }

  void log_error(parsing::LexerState state);
  // void log_error(parsing::ParserState state);
}

#endif
