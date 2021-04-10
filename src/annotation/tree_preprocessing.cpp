#include <string.h>

#include "../annotation.hpp"
#include "../node.hpp"


namespace akbit::system::parsing { Node * convert_to_tuple(Node* node); }

namespace akbit::system::annotation
{
  namespace
  {
    // Context generation visitors
    void tp_visit(Node *node);

    void tp_visit_module(Node *node);
    void tp_visit_declaration(Node *node);
    void tp_visit_block(Node *node);
    void tp_visit_unary_operation(Node *node);
    void tp_visit_binary_operation(Node *node);
    void tp_visit_function_call(Node *node);
    void tp_visit_member_access(Node *node);
    void tp_visit_type(Node *node);
    void tp_visit_value(Node *node);

    void tp_visit_value_type(Node *node);
    void tp_visit_value_function(Node *node);
    void tp_visit_value_tuple(Node *node);
    void tp_visit_value_array(Node *node);
    void tp_visit_value_variable(Node *node);
    void tp_visit_value_string(Node *node);
    void tp_visit_value_character(Node *node);
    void tp_visit_value_integer(Node *node);
    void tp_visit_value_decimal(Node *node);
  }

  void preprocess_ast(Node *node)
  {
    tp_visit(node);
  }

  namespace
  {
    void tp_visit(Node *node)
    {
      if (nullptr == node) return;

      switch (node->type)
      {
        case NodeType::t_module: return tp_visit_module(node);
        case NodeType::t_declaration: return tp_visit_declaration(node);
        case NodeType::t_block: return tp_visit_block(node);

        case NodeType::t_unary_operation: return tp_visit_unary_operation(node);
        case NodeType::t_binary_operation: return tp_visit_binary_operation(node);

        case NodeType::t_function_call: return tp_visit_function_call(node);
        case NodeType::t_member_access: return tp_visit_member_access(node);

        case NodeType::t_type: return tp_visit_type(node);
        case NodeType::t_value: return tp_visit_value(node);
      }
    }

    void tp_visit_value(Node *node)
    {
      switch (node->value.type)
      {
        case NodeValueType::t_type: return tp_visit_value_type(node);
        case NodeValueType::t_function: return tp_visit_value_function(node);
        case NodeValueType::t_tuple: return tp_visit_value_tuple(node);
        case NodeValueType::t_array: return tp_visit_value_array(node);
    
        case NodeValueType::t_variable: return tp_visit_value_variable(node);

        case NodeValueType::t_string: return tp_visit_value_string(node);
        case NodeValueType::t_character: return tp_visit_value_character(node);

        case NodeValueType::t_integer: return tp_visit_value_integer(node);
        case NodeValueType::t_decimal: return tp_visit_value_decimal(node);
      }
    }

    void tp_visit_module(Node *node)
    {
      for (auto d : *node->module.data)
        tp_visit(d);
    }

    void tp_visit_declaration(Node *node)
    {
      tp_visit(node->declaration.type);
      tp_visit(node->declaration.value);
    }

    void tp_visit_block(Node *node)
    {
      for (auto stmt : *node->block.code)
        tp_visit(stmt);
    }

    void tp_visit_unary_operation(Node *node)
    {
      tp_visit(node->unary_operation.expression);
    }

    void tp_visit_binary_operation(Node *node)
    {
      for (auto operand : *node->binary_operation.operands)
        tp_visit(operand);

      if (node->binary_operation.operation == find_operator("->"))
      {
        auto function_node = node->binary_operation.operands->at(node->binary_operation.operands->size() - 1);

        // because i is unsigned, going towards 0 should be done using a special operator
        std::size_t i = node->binary_operation.operands->size() - 1;
        while (i --> 0)
        {
          auto nfn = new Node;
          nfn->type = NodeType::t_value;
          nfn->value.type = NodeValueType::t_function;
          nfn->context = nullptr;
          auto tmp_tuple = parsing::convert_to_tuple((*node->binary_operation.operands)[i]);
          nfn->value.as_function.parameters = tmp_tuple->value.as_tuple.entries;
          delete tmp_tuple;
          nfn->value.as_function.body = function_node;
          function_node = nfn;
        }

        memcpy(node, function_node, sizeof(*node));
      }
      else if (node->binary_operation.operation == find_operator(","))
      {
        auto tuple = Node{};
        tuple.type = NodeType::t_value;
        tuple.value.type = NodeValueType::t_tuple;
        tuple.value.as_tuple.entries = new std::vector<Node*>();
        for (auto op : *node->binary_operation.operands)
          tuple.value.as_tuple.entries->push_back(op);
        
        memcpy(node, &tuple, sizeof(tuple));
      }
    }

    void tp_visit_function_call(Node *node)
    {
      tp_visit(node->call.expression);
      tp_visit(node->call.arguments);
    }

    void tp_visit_member_access([[maybe_unused]] Node *node)
    {
      // non-existent
      // TODO: Implement
    }

    void tp_visit_type([[maybe_unused]] Node *node)
    {
      // non-existent
      // TODO: Implement
    }

    void tp_visit_value_type([[maybe_unused]] Node *node)
    {
      // non-existent
      // TODO: Implement
    }

    void tp_visit_value_function(Node *node)
    {
      for (auto param : *node->value.as_function.parameters)
        tp_visit(param);
      tp_visit(node->value.as_function.body);
    }

    void tp_visit_value_tuple(Node *node)
    {
      for (auto entry : *node->value.as_tuple.entries)
        tp_visit(entry);
    }

    void tp_visit_value_array(Node *node)
    {
      for (auto entry : *node->value.as_array.entries)
        tp_visit(entry);
    }

    void tp_visit_value_variable([[maybe_unused]] Node *node) { }
    void tp_visit_value_string([[maybe_unused]] Node *node) { }
    void tp_visit_value_character([[maybe_unused]] Node *node) { }
    void tp_visit_value_integer([[maybe_unused]] Node *node) { }
    void tp_visit_value_decimal([[maybe_unused]] Node *node) { }
  }
}
