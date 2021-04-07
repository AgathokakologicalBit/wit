#include <iostream>

#include "error_handling.hpp"
#include "lexing.hpp"
// #include "parsing.pp"


namespace akbit::system
{
  template <typename T>
  static inline void log_error_common(T state)
  {
    std::cout << "Error #LC" << static_cast<int>(state.error.code) << ":\n";
    std::cout << state.error.message << "\n\n";
  }

  void log_error(parsing::LexerState state)
  {
    log_error_common(state);

    std::cout << "Line: " << state.line << "\n";
    std::cout << "Column: " << state.column << "\n\n";

    // throw std::runtime_error("halt");
  }

  // void log_error(parsing::ParserState state)
  // {
  //   log_error_common(state);

  //   std::cout << "Line: " << state.peek().line << "\n";
  //   std::cout << "Column: " << state.peek().column << "\n\n";
  //   // TODO: Implement specific error handling stuff
  //   // TODO: Implement specific error handling stuff
  //   // TODO: Implement specific error handling stuff
  //   throw std::runtime_error("halt");
  // }
}
