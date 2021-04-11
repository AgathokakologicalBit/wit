#include <memory>
#include <type_traits>
#include <variant>
#include <string.h>


#include "../annotation.hpp"
#include "../node.hpp"


namespace akbit::system::parsing { std::shared_ptr<Node> convert_to_tuple(std::shared_ptr<Node> node); }

namespace akbit::system::annotation
{
  namespace
  {
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    // Context generation visitors
    void tp_visit(std::shared_ptr<Node> node);

    void tp_visit_module(Node::module_t& node);
    void tp_visit_declaration(Node::declaration_t& node);
    void tp_visit_block(Node::block_t& node);
    void tp_visit_unary_operation(Node::unary_operation_t& node);
    void tp_visit_binary_operation(std::shared_ptr<Node> node);
    void tp_visit_function_call(Node::function_call_t& node);

    void tp_visit_value_function(Node::value_function_t& node);
    void tp_visit_value_tuple(Node::value_tuple_t& node);
    void tp_visit_value_variable(Node::value_variable_t& node);
    void tp_visit_value_string(Node::value_string_t& node);
    void tp_visit_value_character(Node::value_character_t& node);
    void tp_visit_value_integer(Node::value_integer_t& node);
    void tp_visit_value_decimal(Node::value_decimal_t& node);
  }

  void preprocess_ast(std::shared_ptr<Node> node)
  {
    tp_visit(node);
  }

  namespace
  {

    void tp_visit(std::shared_ptr<Node> node)
    {
      if (nullptr == node) return;
      std::visit(overloaded {
        [](auto                     & ) {                                  },
        [](Node::module_t           &_) { tp_visit_module(_);              },
        [](Node::declaration_t      &_) { tp_visit_declaration(_);         },
        [](Node::block_t            &_) { tp_visit_block(_);               },
        [](Node::unary_operation_t  &_) { tp_visit_unary_operation(_);     },
        [&](Node::binary_operation_t& ) { tp_visit_binary_operation(node); },
        [](Node::function_call_t    &_) { tp_visit_function_call(_);       },
        [](Node::value_function_t   &_) { tp_visit_value_function(_);      },
        [](Node::value_tuple_t      &_) { tp_visit_value_tuple(_);         },
        [](Node::value_variable_t   &_) { tp_visit_value_variable(_);      },
        [](Node::value_string_t     &_) { tp_visit_value_string(_);        },
        [](Node::value_character_t  &_) { tp_visit_value_character(_);     },
        [](Node::value_integer_t    &_) { tp_visit_value_integer(_);       },
        [](Node::value_decimal_t    &_) { tp_visit_value_decimal(_);       },
      }, node->value);
    }


    void tp_visit_module(Node::module_t& node)
    {
      for (auto d : node.data)
        tp_visit(d);
    }

    void tp_visit_declaration(Node::declaration_t& node)
    {
      tp_visit(node.type);
      tp_visit(node.value);
    }

    void tp_visit_block(Node::block_t& node)
    {
      for (auto stmt : node.code)
        tp_visit(stmt);
    }

    void tp_visit_unary_operation(Node::unary_operation_t& node)
    {
      tp_visit(node.expression);
    }

    void tp_visit_binary_operation(std::shared_ptr<Node> node_)
    {
      Node::binary_operation_t& node = std::get<Node::binary_operation_t>(node_->value);
      for (auto operand : node.operands)
        tp_visit(operand);

      if (node.operation == find_operator("->"))
      {
        auto function_node = node.operands.at(node.operands.size() - 1);

        // because i is unsigned, going towards 0 should be done using a special operator
        std::size_t i = node.operands.size() - 1;
        while (i --> 0)
        {
          auto tmp_tuple = parsing::convert_to_tuple(node.operands[i]);
          auto nfn = std::make_shared<Node>(Node::value_function_t{
            .parameters = std::get<Node::value_tuple_t>(tmp_tuple->value).entries,
          });
          std::get<Node::value_function_t>(nfn->value).body = function_node;
          function_node = nfn;
        }

        *node_ = *function_node;
      }
      else if (node.operation == find_operator(","))
      {
        Node tuple(Node::value_tuple_t{
          .entries = {},
        });

        for (auto op : node.operands)
          std::get<Node::value_tuple_t>(tuple.value).entries.push_back(op);
        
        *node_ = tuple;
      }
    }

    void tp_visit_function_call(Node::function_call_t& node)
    {
      tp_visit(node.expression);
      tp_visit(node.arguments);
    }


    void tp_visit_value_function(Node::value_function_t& node)
    {
      for (auto param : node.parameters)
        tp_visit(param);
      tp_visit(node.body);
    }

    void tp_visit_value_tuple(Node::value_tuple_t& node)
    {
      for (auto entry : node.entries)
        tp_visit(entry);
    }

    void tp_visit_value_variable(Node::value_variable_t&) { }
    void tp_visit_value_string(Node::value_string_t&) { }
    void tp_visit_value_character(Node::value_character_t&) { }
    void tp_visit_value_integer(Node::value_integer_t&) { }
    void tp_visit_value_decimal(Node::value_decimal_t&) { }
  }
}
