#include "../parsing.hpp"


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

    Node * parse_value(ParserState &state);
    Node * parse_unit(ParserState &state);
    Node * parse_composite_unit(ParserState &state);
    Node * parse_type(ParserState &state);

    // Node * parse_function_declaration(ParserState &state);

    Node * parse_expression(ParserState &state);
    Node * parse_expression(Node * left_operand, ParserState &state, uint32_t base_priority);

    Node * parse_declaration(ParserState &state);
    Node * parse_statement(ParserState &state);
    Node * parse_module(ParserState &state);
  }

  Node *parse(std::vector<Token> &tokens)
  {
    ParserState state(tokens);
    auto module = parse_module(state);

    if (state.is_failed())
    {
      module->module.has_errors = state.is_failed();
      log_error(state);
      return module;
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
      container.module.has_errors = false;

      while (!state.is_eof() && !state.is_failed())
      {
        container.module.data->push_back(parse_statement(state));

        if (state.is_failed())
          return &container;
      }

      return &container;
    }

    Node *parse_statement(ParserState &state)
    {
      if (state.peek().sub_type == TokenSubType::t_identifier && state.peek().value == "let")
        return parse_declaration(state);
      return parse_expression(state);
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
      auto unode = new Node;
      unode->type = NodeType::t_unknown;
      auto data = parse_expression(unode, state, 2);
      if (data != unode)
      {
        if (data->type == NodeType::t_binary_operation && data->binary_operation.operation == find_operator(":"))
        {
          delete unode;
          data = (*data->binary_operation.operands)[1];
        }
        else
        {
          delete data;
          data = unode;
        }
      }
      container.declaration.type = data;

      // if (state.peek().sub_type == TokenSubType::t_colon)
      // {
      //   state.move();
      //   container.declaration.type = parse_type(state);
      //   if (state.is_failed()) return &container;
      // }

      state.consume(TokenSubType::t_equal);
      if (state.is_failed()) return &container;

      container.declaration.value = parse_expression(state);
      return &container;
    }

    Node *parse_expression(ParserState &state)
    {
      Node * left_operand = parse_composite_unit(state);

      if (state.is_failed())
        return left_operand;

      return parse_expression(left_operand, state, 0);
    }

    Node *parse_expression(Node *left_operand, ParserState &state, uint32_t base_priority)
    {
      state.save();
      operator_t const * operation = &parse_operator(state);
      if (operation == &operator_unknown || operation->precedence < base_priority)
      {
        state.restore();
        return left_operand;
      }
      state.drop();

      Node *right_operand = parse_composite_unit(state);
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

          if (left_operand->type == NodeType::t_binary_operation && left_operand->binary_operation.operation == operation)
          {
            left_operand->binary_operation.operands->push_back(right_operand);
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

      if (left_operand->type == NodeType::t_binary_operation && left_operand->binary_operation.operation == operation)
      {
        left_operand->binary_operation.operands->push_back(right_operand);
      }
      else
      {
        left_operand = make_node_bop(left_operand, right_operand, operation);
      }

      return left_operand;
    }

    Node *convert_to_tuple(Node* node)
    {
      if (node->type == NodeType::t_value && node->value.type == NodeValueType::t_tuple)
        return node;
      
      auto &container = *(new Node);
      container.type = NodeType::t_value;
      container.value.type = NodeValueType::t_tuple;
      container.value.as_tuple.entries = new std::vector<Node*>();
      container.value.as_tuple.entries->push_back(node);
      return &container;
    }

    Node *parse_composite_unit(ParserState &state)
    {
      auto unit = parse_unit(state);
      if (state.is_failed()) return unit;

      // Function calls
      while (state.peek().sub_type == TokenSubType::t_brace_round_left)
      {
        auto &container = *(new Node);
        container.type = NodeType::t_function_call;
        container.call.expression = unit;
        container.call.arguments = convert_to_tuple(parse_unit(state));
        unit = &container;
      }

      return unit;
    }

    Node *parse_unit(ParserState &state)
    {
      if (state.peek().sub_type == TokenSubType::t_brace_round_left)
      {
        state.move();
        if (state.peek().sub_type == TokenSubType::t_brace_round_right)
        {
          Node &container = *(new Node);
          container.type = NodeType::t_value;
          container.value.type = NodeValueType::t_tuple;
          container.value.as_tuple.entries = new std::vector<Node*>();
          state.move();
          return &container;
        }

        auto res = parse_expression(state);
        if (not state.is_failed())
          state.consume(TokenSubType::t_brace_round_right);
        return res;
      }

      if (state.peek().sub_type == TokenSubType::t_brace_curly_left)
      {
        state.move();
        Node &container = *(new Node);
        container.type = NodeType::t_block;
        container.block.code = new std::vector<Node*>();
        while (!state.is_eof() && !state.is_failed() && state.peek().sub_type != TokenSubType::t_brace_curly_right)
        {
          auto line = parse_statement(state);
          container.block.code->push_back(line);
        }
        if (!state.is_failed()) state.consume(TokenSubType::t_brace_curly_right);
        return &container;
      }

      if (state.peek().sub_type == TokenSubType::t_dash || state.peek().sub_type == TokenSubType::t_plus)
      {
        Node &container = *(new Node);
        container.type = NodeType::t_unary_operation;
        container.unary_operation.operation = &parse_operator(state);
        container.unary_operation.expression = parse_composite_unit(state);
        return &container;
      }

      return parse_value(state);
    }

    Node *parse_value(ParserState &state)
    {
      Token tok;
      Node &container = *(new Node);
      container.type = NodeType::t_value;

      {
        state.save();
        tok = state.consume(TokenSubType::t_integer);
        if (not state.is_failed())
        {
          state.drop();
          container.value.type = NodeValueType::t_integer;
          container.value.as_integer = new std::string(tok.value);
          return &container;
        }
        state.restore();
      }

      {
        state.save();
        tok = state.consume(TokenSubType::t_decimal);
        if (not state.is_failed())
        {
          state.drop();
          container.value.type = NodeValueType::t_decimal;
          container.value.as_decimal = new std::string(tok.value);
          return &container;
        }
        state.restore();
      }

      {
        state.save();
        tok = state.consume(TokenSubType::t_identifier);
        if (not state.is_failed())
        {
          state.drop();
          container.value.type = NodeValueType::t_variable;
          container.value.as_variable = new std::string(tok.value);
          return &container;
        }
        state.restore();
      }

      {
        state.save();
        tok = state.consume(TokenSubType::t_string);
        if (not state.is_failed())
        {
          state.drop();
          container.value.type = NodeValueType::t_string;
          container.value.as_string = new std::string(tok.value);
          return &container;
        }
        state.drop();
      }
      
      return &container;
    }

    Node *parse_type(ParserState &state)
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
