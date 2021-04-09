#pragma once

#ifndef AKBIT__SYSTEM__LEXING_HPP
#define AKBIT__SYSTEM__LEXING_HPP


#include <cstddef>
#include <cctype>
#include <vector>
#include <string>

#include "../error.hpp"


namespace akbit::system::parsing
{
  enum class TokenType
  {
    t_unknown,

    t_commentary,

    t_identifier,
    t_number,
    t_string,
    t_character,

    t_operator,

    t_eof,
  };

  enum class TokenSubType
  {
    t_unknown,

    t_inline_commentary, t_multiline_commentary,

    t_identifier,
    t_integer, t_decimal,
    t_string,
    t_character,

    t_at_sign,

    t_brace_round_left, t_brace_round_right,
    t_brace_square_left, t_brace_square_right,
    t_brace_triangular_left, t_brace_triangular_right,
    t_brace_curly_left, t_brace_curly_right,

    t_plus, t_dash, t_tilde,
    t_star, t_slash, t_backslash, t_percent,
    t_dot, t_comma,

    t_vertical_bar, t_ampersand, t_caret,

    t_equal, t_exclamation_mark, t_question_mark,
    t_colon, t_semicolon,

    t_eof,
  };

  struct Token
  {
  public:
    std::uint32_t index, line, column;

    std::string value;
    TokenType type;
    TokenSubType sub_type;

    friend std::ostream &operator<<(std::ostream &, Token &);
  };

  std::string get_type_name(TokenType type);
  std::string get_sub_type_name(TokenSubType type);

  std::ostream &operator<<(std::ostream &out, parsing::Token &token);
  
  struct LexerState
  {
    std::uint32_t index, line, column;
    std::string &source;

    struct
    {
      error_t code;
      std::string message;
    } error;

  public:
    LexerState(std::string& source_)
      : index(0), line(1), column(1)
      , source(source_)
      , error{error_t::e_no_errors, ""}
    { }

  public:
    constexpr inline char peek() const { return source[index]; }
    void move();

    constexpr inline bool is_eof() const noexcept { return index >= source.size(); }
    constexpr inline bool is_error_occurred() const noexcept { return error.code != error_t::e_no_errors; }
  };

  /// Converts character to it's corresponding type
  /// \param c target character
  /// \return character type or ::t_unknown
  TokenSubType get_character_type(char c);

  /// Gets a next token from the byte stream
  /// \param state current tokenization state
  /// \return parsed valuable token
  Token get_next_token(parsing::LexerState &state);


  std::vector<Token> tokenize(std::string &source);
}

#endif
