#pragma once

#ifndef AKBIT__SYSTEM__NODE_HPP
#define AKBIT__SYSTEM__NODE_HPP


#include <cstddef>
#include <string>
#include <vector>

#include "operators.hpp"
#include "context.hpp"


namespace akbit::system
{
  enum class NodeType
  {
    t_unknown,

    t_module,

    t_declaration,
    t_block,

    t_unary_operation,
    t_binary_operation,

    t_function_call,
    t_member_access,

    t_type,
    t_value,
  };

  enum class NodeValueType
  {
    t_unknown,

    t_type,
    t_function,
    t_tuple,
    t_array,
    
    t_variable,

    t_string,
    t_character,

    t_integer,
    t_decimal,
  };

  struct Node
  {
    NodeType type;
    Context *context;

    union
    {
      struct
      {
        std::vector<Node *> *data;
        bool has_errors;
      } module;

      struct
      {
        std::string *name;
        Node *type;
        Node *value;
      } declaration;

      struct
      {
        std::vector<Node *> *code;
      } block;

      struct
      {
        std::vector<Node *> *values;
      } array;

      struct
      {
        operator_t const *operation;
        std::vector<Node *> *operands;
      } binary_operation;

      struct
      {
        operator_t const *operation;
        Node *expression;
      } unary_operation;

      struct
      {
        Node *expression;
        Node *arguments;
      } call;

      struct
      {
        std::string * name;

        // flags
        bool is_const;
        // ...
      } typeinfo;

      struct
      {
        NodeValueType type;
        union
        {
          // TODO: use special type
          std::string *as_variable;
          std::string *as_string;
          std::uint32_t as_character;
          
          // TODO: use special type
          std::string *as_integer;
          std::string *as_decimal;

          struct
          {
            std::vector<Node *> *parameters;
            Node *body;
          } as_function;

          struct
          {
            // TODO: Fill it
          } as_type;

          struct
          {
            std::vector<Node *> *entries;
          } as_array;

          struct
          {
            std::vector<Node *> *entries;
          } as_tuple;
        };
      } value;
    };

    Node() = default;
    // TODO: Create a desctructor to free memory!
    // TODO: Create a desctructor to free memory!
  };

  inline Node * make_node_bop(Node * left, Node * right, operator_t const * operation)
  {
    Node *container = new Node;
    container->type = NodeType::t_binary_operation;
    container->binary_operation.operands = new std::vector<Node *>;

    container->binary_operation.operands->push_back(left);
    container->binary_operation.operands->push_back(right);

    container->binary_operation.operation = operation;
    return container;
  }
}

#endif
