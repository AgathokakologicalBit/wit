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
    operator_t const & parse_operator(ParserState &state);

    std::shared_ptr<Node> parse_value(ParserState &state);
    std::shared_ptr<Node> parse_unit(ParserState &state);
    std::shared_ptr<Node> parse_composite_unit(ParserState &state);
    std::shared_ptr<Node> parse_type(ParserState &state);

    // Node * parse_function_declaration(ParserState &state);

    std::shared_ptr<Node> parse_expression(ParserState &state);
    std::shared_ptr<Node> parse_expression(std::shared_ptr<Node> left_operand, ParserState &state, uint32_t base_priority);

    std::shared_ptr<Node> parse_declaration(ParserState &state);
    std::shared_ptr<Node> parse_statement(ParserState &state);
    std::shared_ptr<Node> parse_module(ParserState &state);
  }

  std::shared_ptr<Node> parse(std::vector<Token> &tokens)
  {
    ParserState state(tokens);
    auto module = parse_module(state);

    if (state.is_failed())
    {
      std::get<Node::module_t>(module->value).has_errors = state.is_failed();
      log_error(state);
      return module;
    }

    return module;
  }

  std::shared_ptr<Node> convert_to_tuple(std::shared_ptr<Node> node)
  {
    if (nullptr == node) return nullptr;
    try
    {
      std::get<Node::value_tuple_t>(node->value);
      return node;
    }
    catch (...) { }
    
    auto container = std::make_shared<Node>(Node::value_tuple_t{
      .entries = {
        {node}
      }
    });
    return container;
  }


  namespace
  {
    std::shared_ptr<Node> parse_module(ParserState &state)
    {
      auto container = std::make_shared<Node>(Node::module_t{
        .data = {},
        .has_errors = false,
      });

      while (!state.is_eof() && !state.is_failed())
      {
        std::get<Node::module_t>(container->value).data.push_back(parse_statement(state));

        if (state.is_failed())
          return container;
      }

      return container;
    }

    std::shared_ptr<Node> parse_statement(ParserState &state)
    {
      if (state.peek().sub_type == TokenSubType::t_identifier && state.peek().value == "let")
        return parse_declaration(state);
      return parse_expression(state);
    }

    std::shared_ptr<Node> parse_declaration(ParserState &state)
    {
      state.consume(TokenSubType::t_identifier, "let");
      if (state.is_failed()) return nullptr;

      auto container = std::make_shared<Node>(Node::declaration_t{});
      auto idt = state.consume(TokenType::t_identifier);
      if (state.is_failed()) return container;

      std::get<Node::declaration_t>(container->value).name = idt.value;
      auto unode = std::make_shared<Node>();
      auto data = parse_expression(unode, state, 2);
      if (data != unode)
      {
        if (data->value.index() == 4 && std::get<Node::binary_operation_t>(data->value).operation == find_operator(":"))
        {
          data = std::get<Node::binary_operation_t>(data->value).operands[1];
        }
        else
        {
          data = unode;
        }
      }
      std::get<Node::declaration_t>(container->value).type = data;

      state.consume(TokenSubType::t_equal);
      if (state.is_failed()) return container;

      std::get<Node::declaration_t>(container->value).value = parse_expression(state);
      return container;
    }

    std::shared_ptr<Node> parse_expression(ParserState &state)
    {
      std::shared_ptr<Node> left_operand = parse_composite_unit(state);

      if (state.is_failed())
        return left_operand;

      return parse_expression(left_operand, state, 0);
    }

    std::shared_ptr<Node> parse_expression(std::shared_ptr<Node> left_operand, ParserState &state, uint32_t base_priority)
    {
      state.save();
      operator_t const * operation = &parse_operator(state);
      if (operation == &operator_unknown || operation->precedence < base_priority)
      {
        state.restore();
        return left_operand;
      }
      state.drop();

      std::shared_ptr<Node> right_operand = parse_composite_unit(state);
      if (state.is_failed())
        return left_operand;

      // Get next operator
      state.save();
      operator_t const * next_operation = &parse_operator(state);
      if (state.is_failed()) { state.restore(); }
      else state.drop();

      while (next_operation->precedence > base_priority)
      {
          if (next_operation->precedence > operation->precedence)
          {
            // If the following operator has the higher priority
            // parse that subexpression first
            state.index -= next_operation->representation.size();
            right_operand = parse_expression(right_operand, state, operation->precedence);
            if (state.is_failed())
              return left_operand;

            state.save();
            next_operation = &parse_operator(state);
            if (state.is_failed()) { state.restore(); break; }
            else state.drop();

            if (next_operation->precedence <= base_priority)
              break;
          }

          if (left_operand->value.index() == 4 && std::get<Node::binary_operation_t>(left_operand->value).operation == operation)
          {
            std::get<Node::binary_operation_t>(left_operand->value).operands.push_back(right_operand);
          }
          else
          {
            left_operand = make_node_bop(left_operand, right_operand, operation);
          }

          operation = next_operation;

          right_operand = parse_composite_unit(state);
          if (state.is_failed())
            return left_operand;

          state.save();
          next_operation = &parse_operator(state);
          if (state.is_failed()) { state.restore(); break; }
          else state.drop();
      }

      if (next_operation->precedence <= base_priority)
        state.index -= next_operation->representation.size();

      if (left_operand->value.index() == 4 && std::get<Node::binary_operation_t>(left_operand->value).operation == operation)
      {
        std::get<Node::binary_operation_t>(left_operand->value).operands.push_back(right_operand);
      }
      else
      {
        left_operand = make_node_bop(left_operand, right_operand, operation);
      }

      return left_operand;
    }

    std::shared_ptr<Node> parse_composite_unit(ParserState &state)
    {
      auto unit = parse_unit(state);
      if (state.is_failed()) return unit;

      // Function calls
      while (state.peek().sub_type == TokenSubType::t_brace_round_left)
      {
        auto container = std::make_shared<Node>(Node::function_call_t{
          .expression = unit,
          .arguments = convert_to_tuple(parse_unit(state))
        });
        unit = container;
      }

      return unit;
    }

    std::shared_ptr<Node> parse_unit(ParserState &state)
    {
      if (state.peek().sub_type == TokenSubType::t_brace_round_left)
      {
        state.move();
        if (state.peek().sub_type == TokenSubType::t_brace_round_right)
        {
          auto container = std::make_shared<Node>(Node::value_tuple_t{
            .entries = {},
          });
          state.move();
          return container;
        }

        auto res = parse_expression(state);
        if (not state.is_failed())
          state.consume(TokenSubType::t_brace_round_right);
        return res;
      }

      if (state.peek().sub_type == TokenSubType::t_brace_curly_left)
      {
        state.move();
        auto container = std::make_shared<Node>(Node::block_t{
          .code = {},
        });

        while (!state.is_eof() && !state.is_failed() && state.peek().sub_type != TokenSubType::t_brace_curly_right)
        {
          auto line = parse_statement(state);
          std::get<Node::block_t>(container->value).code.push_back(line);
        }
        
        if (!state.is_failed())
          state.consume(TokenSubType::t_brace_curly_right);

        return container;
      }

      if (state.peek().sub_type == TokenSubType::t_dash || state.peek().sub_type == TokenSubType::t_plus)
      {
        auto container = std::make_shared<Node>(Node::unary_operation_t{
          .operation = &parse_operator(state),
          .expression = parse_composite_unit(state),
        });
        return container;
      }

      return parse_value(state);
    }

    std::shared_ptr<Node> parse_value(ParserState &state)
    {
      Token tok;
      auto container = std::make_shared<Node>();

      {
        state.save();
        tok = state.consume(TokenSubType::t_integer);
        if (not state.is_failed())
        {
          state.drop();
          container->value = Node::value_integer_t({ .value = tok.value });
          return container;
        }
        state.restore();
      }

      {
        state.save();
        tok = state.consume(TokenSubType::t_decimal);
        if (not state.is_failed())
        {
          state.drop();
          container->value = Node::value_decimal_t({ .value = tok.value });
          return container;
        }
        state.restore();
      }

      {
        state.save();
        tok = state.consume(TokenSubType::t_identifier);
        if (not state.is_failed())
        {
          state.drop();
          container->value = Node::value_variable_t({ .name = tok.value });
          return container;
        }
        state.restore();
      }

      {
        state.save();
        tok = state.consume(TokenSubType::t_string);
        if (not state.is_failed())
        {
          state.drop();
          container->value = Node::value_string_t({ .value = tok.value });
          return container;
        }
        state.drop();
      }
      
      return container;
    }

    std::shared_ptr<Node> parse_type(ParserState &state)
    {
      state.error.code = error_t::e_fatal_internal_error;
      state.error.message = "Function 'parse_type' is not implemented";
      return nullptr;
    }

    operator_t const & parse_operator(ParserState &state)
    {
      state.save();
      Token t = state.consume(TokenType::t_operator);
      if (state.is_failed())
      {
        state.drop();
        return operator_unknown;
      }
      state.restore();

      operator_t const * operation = &operator_unknown;
      std::string representation = "";
      state.save();

      while (state.peek().type == TokenType::t_operator && representation.size() < 5)
      {
        representation += state.peek().value;
        state.move();

        for (std::size_t index = 0; index < operators_list.size(); ++index)
          if (operators_list[index].representation == representation)
            operation = &operators_list[index];
      }

      state.restore();
      for (std::size_t i = 0; i < operation->representation.size(); ++i)
          state.move();

      return *operation;
    }
  }
}
