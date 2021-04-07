#include "parsing.hpp"


namespace akbit::system::parsing
{
  Token ParserState::consume(TokenType const type, TokenSubType const subtype)
  {
    Token tok = this->peek();
    if (type != tok.type || (subtype != TokenSubType::t_unknown && subtype != tok.sub_type))
    {
      std::string expected = (subtype != TokenSubType::t_unknown ? get_sub_type_name(subtype) : get_type_name(type));
      this->error.code = error_t::p_unexpected_token;
      this->error.message = "[somewhere]: <" + expected + "> expected, but <"
      + get_sub_type_name(tok.sub_type) + "> was given";
      return tok;
    }

    this->move();
    return tok;
  }

  Token ParserState::consume(TokenSubType const subtype)
  {
    Token tok = this->peek();
    if (subtype != tok.sub_type)
    {
      std::string expected = get_sub_type_name(subtype);
      this->error.code = error_t::p_unexpected_token;
      this->error.message = "[somewhere]: <" + expected + "> expected, but <"
      + get_sub_type_name(tok.sub_type) + ">(" + tok.value +  ") was given";
      return tok;
    }

    this->move();
    return tok;
  }

  Token ParserState::consume(TokenSubType const subtype, std::string const value)
  {
    Token tok = this->peek();
    if (subtype != tok.sub_type || tok.value != value)
    {
      std::string expected = get_sub_type_name(subtype);
      this->error.code = error_t::p_unexpected_token;
      this->error.message = "[somewhere]: <" + expected + ">(" + value + ") expected, but <"
      + get_sub_type_name(tok.sub_type) + ">(" + tok.value +  ") was given";
      return tok;
    }

    this->move();
    return tok;
  }


  namespace
  {
    // operator_t const & parse_operator(ParserState &state);

    Node * parse_value(ParserState &state);
    Node * parse_unit(ParserState &state);
    Node * parse_type(ParserState &state);

    // Node * parse_function_declaration(ParserState &state);

    Node * parse_expression(ParserState &state);
    // Node * parse_expression(Node * left_operand, ParserState &state, uint32_t base_priority);

    Node * parse_declaration(ParserState &state);
    Node * parse_module(ParserState &state);
  }

  Node *parse(std::vector<Token> &tokens)
  {
    ParserState state(tokens);
    auto module = parse_module(state);

    if (state.is_failed())
    {
      log_error(state);
      return nullptr;
    }

    return module;
  }


  namespace
  {
    Node *parse_module(ParserState &state)
    {
      auto &container = *(new Node);
      container.type = NodeType::t_module;
      container.module.data = new std::vector<Node*>;

      while (!state.is_eof() && !state.is_failed())
      {
        container.module.data->push_back(parse_declaration(state));

        if (state.is_failed())
          return &container;
      }

      return &container;
    }

    Node *parse_declaration(ParserState &state)
    {
      state.consume(TokenSubType::t_identifier, "let");
      if (state.is_failed()) return nullptr;

      Node &container = *(new Node);
      container.type = NodeType::t_declaration;

      auto idt = state.consume(TokenType::t_identifier);
      if (state.is_failed()) return &container;
      container.declaration.name = new std::string(idt.value);
      container.declaration.type = nullptr;

      if (state.peek().sub_type == TokenSubType::t_colon)
      {
        container.declaration.type = parse_type(state);
        if (state.is_failed()) return &container;
      }

      state.consume(TokenSubType::t_equal);
      if (state.is_failed()) return &container;

      container.declaration.value = parse_expression(state);
      return &container;
    }

    Node *parse_expression(ParserState &state)
    {
      return parse_unit(state);
    }

    Node *parse_unit(ParserState &state)
    {
      return parse_value(state);
    }

    Node *parse_value(ParserState &state)
    {
      auto it = state.consume(TokenSubType::t_integer);
      if (state.is_failed()) return nullptr;

      Node &container = *(new Node);
      container.type = NodeType::t_value;
      container.value.as_integer = new std::string(it.value);

      return &container;
    }

    Node *parse_type(ParserState &state)
    {
      state.error.code = error_t::e_fatal_internal_error;
      state.error.message = "Function 'parse_type' is not implemented";
      return nullptr;
    }
  }
}
