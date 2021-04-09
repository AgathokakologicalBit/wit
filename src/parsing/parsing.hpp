#pragma once

#ifndef AKBIT__SYSTEM__PARSING_HPP
#define AKBIT__SYSTEM__PARSING_HPP


#include <vector>
#include <string>

#include "../error_handling.hpp"
#include "lexing.hpp"
#include "../operators.hpp"
#include "../node.hpp"


namespace akbit::system::parsing
{
  struct ParserState
  {
  public:
    std::vector<Token> &tokens;
    std::size_t index;

    struct error_info_t
    {
      error_t code;
      std::string message;
    } error;

  private:
    std::vector<std::size_t> saves;

  public:
    ParserState(std::vector<Token> &tokens_)
      : tokens(tokens_)
      , index(0)
      , error{error_t::e_no_errors, ""}
      , saves()
    { }


    inline bool is_eof() const noexcept { return index >= tokens.size() - 1; }
    inline bool is_failed() const noexcept { return error.code != error_t::e_no_errors; }


    inline Token peek() const noexcept { return tokens[index]; }

    inline void move() noexcept
    {
      if (index < tokens.size())
        ++index;
    }

    Token consume(TokenType const type, TokenSubType const subtype=TokenSubType::t_unknown);
    Token consume(TokenSubType const subtype);
    Token consume(TokenSubType const subtype, std::string const value);

    inline void save() { saves.push_back(index); }
    inline void drop() { saves.pop_back(); }
    inline void restore()
    {
      index = saves[saves.size() - 1];
      error.code = error_t::e_no_errors;
      error.message = "";
      saves.pop_back();
    }


    friend void ::akbit::system::log_error(ParserState &state);
  };

  Node *parse(std::vector<Token> &tokens);
}

#endif
