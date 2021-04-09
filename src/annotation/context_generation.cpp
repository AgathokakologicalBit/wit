#include "../annotation.hpp"


namespace akbit::system::annotation
{
  namespace
  {
    // Context generation visitors
    void cg_visit(Node *node, Context *ctx);

    void cg_visit_module(Node *node, Context *ctx);
    void cg_visit_declaration(Node *node, Context *ctx);
    void cg_visit_block(Node *node, Context *ctx);
    void cg_visit_unary_operation(Node *node, Context *ctx);
    void cg_visit_binary_operation(Node *node, Context *ctx);
    void cg_visit_function_call(Node *node, Context *ctx);
    void cg_visit_member_access(Node *node, Context *ctx);
    void cg_visit_type(Node *node, Context *ctx);
    void cg_visit_value(Node *node, Context *ctx);

    void cg_visit_value_type(Node *node, Context *ctx);
    void cg_visit_value_function(Node *node, Context *ctx);
    void cg_visit_value_tuple(Node *node, Context *ctx);
    void cg_visit_value_array(Node *node, Context *ctx);
    void cg_visit_value_variable(Node *node, Context *ctx);
    void cg_visit_value_string(Node *node, Context *ctx);
    void cg_visit_value_character(Node *node, Context *ctx);
    void cg_visit_value_integer(Node *node, Context *ctx);
    void cg_visit_value_decimal(Node *node, Context *ctx);
  }

  void generate_context(Node *node)
  {
    if (nullptr == node) return;
    cg_visit(node, new Context(nullptr));
  }

  namespace
  {
    void cg_visit(Node *node, Context *ctx)
    {
      if (nullptr == node) return;

      node->context = ctx;
      switch (node->type)
      {
        case NodeType::t_module: return cg_visit_module(node, ctx);
        case NodeType::t_declaration: return cg_visit_declaration(node, ctx);
        case NodeType::t_block: return cg_visit_block(node, ctx);

        case NodeType::t_unary_operation: return cg_visit_unary_operation(node, ctx);
        case NodeType::t_binary_operation: return cg_visit_binary_operation(node, ctx);

        case NodeType::t_function_call: return cg_visit_function_call(node, ctx);
        case NodeType::t_member_access: return cg_visit_member_access(node, ctx);

        case NodeType::t_type: return cg_visit_type(node, ctx);
        case NodeType::t_value: return cg_visit_value(node, ctx);
      }
    }

    void cg_visit_value(Node *node, Context *ctx)
    {
      switch (node->value.type)
      {
        case NodeValueType::t_type: return cg_visit_value_type(node, ctx);
        case NodeValueType::t_function: return cg_visit_value_function(node, ctx);
        case NodeValueType::t_tuple: return cg_visit_value_tuple(node, ctx);
        case NodeValueType::t_array: return cg_visit_value_array(node, ctx);
    
        case NodeValueType::t_variable: return cg_visit_value_variable(node, ctx);

        case NodeValueType::t_string: return cg_visit_value_string(node, ctx);
        case NodeValueType::t_character: return cg_visit_value_character(node, ctx);

        case NodeValueType::t_integer: return cg_visit_value_integer(node, ctx);
        case NodeValueType::t_decimal: return cg_visit_value_decimal(node, ctx);
      }
    }

    void cg_visit_module(Node *node, Context *ctx)
    {
      for (auto d : *node->module.data)
        cg_visit(d, ctx);
    }

    void cg_visit_declaration(Node *node, Context *ctx)
    {
      ctx->declarations.push_back(node);
      cg_visit(node->declaration.type, ctx);
      cg_visit(node->declaration.value, ctx);
    }

    void cg_visit_block(Node *node, Context *ctx)
    {
      for (auto stmt : *node->block.code)
        cg_visit(stmt, ctx);
    }

    void cg_visit_unary_operation(Node *node, Context *ctx)
    {
      cg_visit(node->unary_operation.expression, ctx);
    }

    void cg_visit_binary_operation(Node *node, Context *ctx)
    {
      for (auto operand : *node->binary_operation.operands)
        cg_visit(operand, ctx);
    }

    void cg_visit_function_call(Node *node, Context *ctx)
    {
      cg_visit(node->call.expression, ctx);
      cg_visit(node->call.arguments, ctx);
    }

    void cg_visit_member_access([[maybe_unused]] Node *node, [[maybe_unused]] Context *ctx)
    {
      // non-existent
      // TODO: Implement
    }

    void cg_visit_type([[maybe_unused]] Node *node, [[maybe_unused]] Context *ctx)
    {
      // non-existent
      // TODO: Implement
    }

    void cg_visit_value_type([[maybe_unused]] Node *node, [[maybe_unused]] Context *ctx)
    {
      // non-existent
      // TODO: Implement
    }

    void cg_visit_value_function(Node *node, Context *ctx)
    {
      // TODO: Create new context!!!
      // TODO: Create new context!!!
      // TODO: Create new context!!!
      // TODO: Create new context!!!
      for (auto param : *node->value.as_function.parameters)
        cg_visit(param, ctx);
      cg_visit(node->value.as_function.body, ctx);
    }

    void cg_visit_value_tuple(Node *node, Context *ctx)
    {
      for (auto entry : *node->value.as_tuple.entries)
        cg_visit(entry, ctx);
    }

    void cg_visit_value_array(Node *node, Context *ctx)
    {
      for (auto entry : *node->value.as_array.entries)
        cg_visit(entry, ctx);
    }

    void cg_visit_value_variable([[maybe_unused]] Node *node, [[maybe_unused]] Context *ctx)
    {
      // TODO: Do something here as well
    }

    void cg_visit_value_string([[maybe_unused]] Node *node, [[maybe_unused]] Context *ctx) { }
    void cg_visit_value_character([[maybe_unused]] Node *node, [[maybe_unused]] Context *ctx) { }
    void cg_visit_value_integer([[maybe_unused]] Node *node, [[maybe_unused]] Context *ctx) { }
    void cg_visit_value_decimal([[maybe_unused]] Node *node, [[maybe_unused]] Context *ctx) { }
  }
}
