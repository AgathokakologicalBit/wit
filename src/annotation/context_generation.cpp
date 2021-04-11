#include "../annotation.hpp"


namespace akbit::system::annotation
{
  namespace
  {
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    // Context generation visitors
    void cg_visit(std::shared_ptr<Node> node, std::shared_ptr<Context> ctx, bool reg_vars);

    void cg_visit_module(std::shared_ptr<Node> node, Node::module_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    void cg_visit_declaration(std::shared_ptr<Node> node, Node::declaration_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    void cg_visit_block(std::shared_ptr<Node> node, Node::block_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    void cg_visit_unary_operation(std::shared_ptr<Node> node, Node::unary_operation_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    void cg_visit_binary_operation(std::shared_ptr<Node> node, Node::binary_operation_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    void cg_visit_function_call(std::shared_ptr<Node> node, Node::function_call_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    
    void cg_visit_value_function(std::shared_ptr<Node> node, Node::value_function_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    void cg_visit_value_tuple(std::shared_ptr<Node> node, Node::value_tuple_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    void cg_visit_value_variable(std::shared_ptr<Node> node, Node::value_variable_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    void cg_visit_value_string(std::shared_ptr<Node> node, Node::value_string_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    void cg_visit_value_character(std::shared_ptr<Node> node, Node::value_character_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    void cg_visit_value_integer(std::shared_ptr<Node> node, Node::value_integer_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
    void cg_visit_value_decimal(std::shared_ptr<Node> node, Node::value_decimal_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
  }

  void generate_context(std::shared_ptr<Node> node)
  {
    if (nullptr == node) return;
    cg_visit(node, std::make_shared<Context>(nullptr), false);
  }

  namespace
  {
    void cg_visit(std::shared_ptr<Node> node, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      if (nullptr == node) return;

      node->context = ctx;
      std::visit(overloaded {
        [&](auto                     & ) {                                                       },
        [&](Node::module_t           &_) { cg_visit_module(node, _, ctx, reg_vars);              },
        [&](Node::declaration_t      &_) { cg_visit_declaration(node, _, ctx, reg_vars);         },
        [&](Node::block_t            &_) { cg_visit_block(node, _, ctx, reg_vars);               },
        [&](Node::unary_operation_t  &_) { cg_visit_unary_operation(node, _, ctx, reg_vars);     },
        [&](Node::binary_operation_t &_) { cg_visit_binary_operation(node, _, ctx, reg_vars);    },
        [&](Node::function_call_t    &_) { cg_visit_function_call(node, _, ctx, reg_vars);       },
        [&](Node::value_function_t   &_) { cg_visit_value_function(node, _, ctx, reg_vars);      },
        [&](Node::value_tuple_t      &_) { cg_visit_value_tuple(node, _, ctx, reg_vars);         },
        [&](Node::value_variable_t   &_) { cg_visit_value_variable(node, _, ctx, reg_vars);      },
        [&](Node::value_string_t     &_) { cg_visit_value_string(node, _, ctx, reg_vars);        },
        [&](Node::value_character_t  &_) { cg_visit_value_character(node, _, ctx, reg_vars);     },
        [&](Node::value_integer_t    &_) { cg_visit_value_integer(node, _, ctx, reg_vars);       },
        [&](Node::value_decimal_t    &_) { cg_visit_value_decimal(node, _, ctx, reg_vars);       },
      }, node->value);
    }

    void cg_visit_module(std::shared_ptr<Node>, Node::module_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      val.global_context = ctx;
      for (auto d : val.data)
        cg_visit(d, ctx, reg_vars);
    }

    void cg_visit_declaration(std::shared_ptr<Node>, Node::declaration_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      // Assumes that the assignee is a signle variable
      // TODO: Enhance the code to support more assignment types
      ctx->add(ctx, val.name, val.type);
      cg_visit(val.type, ctx, reg_vars);
      cg_visit(val.value, ctx, reg_vars);
    }

    void cg_visit_block(std::shared_ptr<Node>, Node::block_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      for (auto stmt : val.code)
        cg_visit(stmt, ctx, reg_vars);
    }

    void cg_visit_unary_operation(std::shared_ptr<Node>, Node::unary_operation_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      cg_visit(val.expression, ctx, reg_vars);
    }

    void cg_visit_binary_operation(std::shared_ptr<Node>, Node::binary_operation_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      bool is_first = true;
      for (auto operand : val.operands)
      {
        cg_visit(operand, ctx, reg_vars && (val.operation != find_operator(":") || is_first));
        is_first = false;
      }
    }

    void cg_visit_function_call(std::shared_ptr<Node>, Node::function_call_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      cg_visit(val.expression, ctx, reg_vars);
      cg_visit(val.arguments, ctx, reg_vars);
    }

    void cg_visit_value_function(std::shared_ptr<Node>, Node::value_function_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      auto sub_context = std::make_shared<Context>(ctx);
      val.owned_context = sub_context;
      for (auto param : val.parameters)
        cg_visit(param, sub_context, true);
      cg_visit(val.body, sub_context, reg_vars);
    }

    void cg_visit_value_tuple(std::shared_ptr<Node>, Node::value_tuple_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      for (auto entry : val.entries)
        cg_visit(entry, ctx, reg_vars);
    }
    
    void cg_visit_value_variable(std::shared_ptr<Node>, Node::value_variable_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      // Assumes that the first found instance is the correct one
      // TODO: Enhance the type checking algorithm
      auto possible_values = ctx->find(val.name);
      val.record = possible_values.empty() ? nullptr : possible_values[0];
      if (!reg_vars) return;
      if (!ctx->get(val.name).empty()) return;
      val.record = ctx->add(ctx, val.name, nullptr);
    }

    void cg_visit_value_string(std::shared_ptr<Node>, Node::value_string_t&, std::shared_ptr<Context>, bool) { }
    void cg_visit_value_character(std::shared_ptr<Node>, Node::value_character_t&, std::shared_ptr<Context>, bool) { }
    void cg_visit_value_integer(std::shared_ptr<Node>, Node::value_integer_t&, std::shared_ptr<Context>, bool) { }
    void cg_visit_value_decimal(std::shared_ptr<Node>, Node::value_decimal_t&, std::shared_ptr<Context>, bool) { }
  }
}
