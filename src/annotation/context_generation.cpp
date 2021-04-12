#include <iostream>

#include "../annotation.hpp"
#include "../node.hpp"
#include "../context.hpp"


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
    void cg_visit_condition(std::shared_ptr<Node> node, Node::condition_t &val, std::shared_ptr<Context> ctx, bool reg_vars);
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
        [&](Node::condition_t        &_) { cg_visit_condition(node, _, ctx, reg_vars);           },
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

    void cg_visit_declaration(std::shared_ptr<Node> node, Node::declaration_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      // Assumes that the assignee is a signle variable
      // TODO: Enhance the code to support more assignment types
      // TODO: Make a normal type checking
      cg_visit(val.type, ctx, false);
      auto t = Node::etype_t::unknown;
      if (val.type && val.type->value.index() == 12)
      {
        auto type_info = std::get<Node::value_variable_t>(val.type->value);
        if (!type_info.record.lock())
        {
          if (type_info.name == "int") t = Node::etype_t::integer;
          else if (type_info.name == "float") t = Node::etype_t::decimal;
          else if (type_info.name == "string") t = Node::etype_t::string;
          else if (type_info.name == "function") t = Node::etype_t::function;
        }
      }

      auto record = ctx->add(ctx, std::get<Node::value_variable_t>(val.variable->value).name, t);
      node->result_type = t;
      std::get<Node::value_variable_t>(val.variable->value).record = record;

      if (!val.value) return;
      cg_visit(val.value, ctx, reg_vars);
      auto rt = t != Node::etype_t::unknown ? t : val.value->result_type;
      if (t != Node::etype_t::unknown && rt != t)
      {
        // TODO: Handle error properly
        std::cerr << "Mismatch between declared and assigned value types.\n";
      }

      record->type = rt;
      node->result_type = rt;
    }

    void cg_visit_condition(std::shared_ptr<Node> node, Node::condition_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      cg_visit(val.expression, ctx, reg_vars);
      cg_visit(val.clause_true, ctx, reg_vars);
      cg_visit(val.clause_false, ctx, reg_vars);

      node->result_type = Node::etype_t::any;
      if (val.clause_false && val.clause_false->result_type != val.clause_true->result_type)
        node->result_type = val.clause_false->result_type;
    }

    void cg_visit_block(std::shared_ptr<Node>, Node::block_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      auto rt = Node::etype_t::unknown;
      for (auto stmt : val.code)
      {
        cg_visit(stmt, ctx, reg_vars);
        rt = stmt->result_type;
      }
    }

    void cg_visit_unary_operation(std::shared_ptr<Node> node, Node::unary_operation_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      cg_visit(val.expression, ctx, reg_vars);
      node->result_type = val.expression->result_type;
    }

    void cg_visit_binary_operation(std::shared_ptr<Node> node, Node::binary_operation_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      bool is_first = true;
      auto common_type = Node::etype_t::unknown;

      for (auto operand : val.operands)
      {
        cg_visit(operand, ctx, reg_vars && (val.operation != find_operator(":") || is_first));
        if (is_first) common_type = operand->result_type;
        else
        {
          auto rt = operand->result_type;
          if (common_type != rt && rt != Node::etype_t::unknown && rt != Node::etype_t::any)
          {
            // TODO: Report error
            std::cerr << "Binary operation type mismatch:\n  Expected <" << (int)common_type << ">, but <" << (int)rt << "> was given.\n";
            common_type = Node::etype_t::any;
          }
        }
        
        is_first = false;
      }

      node->result_type = common_type;
    }

    void cg_visit_function_call(std::shared_ptr<Node> node, Node::function_call_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      cg_visit(val.expression, ctx, reg_vars);
      cg_visit(val.arguments, ctx, reg_vars);
      node->result_type = Node::etype_t::any;
    }

    void cg_visit_value_function(std::shared_ptr<Node> node, Node::value_function_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      // TODO: Determine body return type
      node->result_type = Node::etype_t::function;
      auto sub_context = std::make_shared<Context>(ctx);
      val.owned_context = sub_context;
      for (auto param : val.parameters)
        cg_visit(param, sub_context, true);
      cg_visit(val.body, sub_context, reg_vars);
    }

    void cg_visit_value_tuple(std::shared_ptr<Node> node, Node::value_tuple_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      node->result_type = Node::etype_t::tuple;
      for (auto entry : val.entries)
        cg_visit(entry, ctx, reg_vars);
    }
    
    void cg_visit_value_variable(std::shared_ptr<Node> node, Node::value_variable_t &val, std::shared_ptr<Context> ctx, bool reg_vars)
    {
      // Assumes that the first found instance is the correct one
      // TODO: Enhance the type checking algorithm
      auto possible_values = ctx->find(val.name);
      val.record = possible_values.empty() ? nullptr : possible_values[0];
      if (val.record.lock()) node->result_type = val.record.lock()->type;
      if (!reg_vars) return;
      if (!ctx->get(val.name).empty()) return;
      val.record = ctx->add(ctx, val.name, node->result_type);
    }

    void cg_visit_value_string(std::shared_ptr<Node> node, Node::value_string_t&, std::shared_ptr<Context>, bool)
    { node->result_type = Node::etype_t::string; }
    
    void cg_visit_value_character(std::shared_ptr<Node> node, Node::value_character_t&, std::shared_ptr<Context>, bool)
    { node->result_type = Node::etype_t::character; }

    void cg_visit_value_integer(std::shared_ptr<Node> node, Node::value_integer_t&, std::shared_ptr<Context>, bool)
    { node->result_type = Node::etype_t::integer; }
    
    void cg_visit_value_decimal(std::shared_ptr<Node> node, Node::value_decimal_t&, std::shared_ptr<Context>, bool)
    { node->result_type = Node::etype_t::decimal; }
  }
}
