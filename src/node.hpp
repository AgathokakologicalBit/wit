#pragma once

#ifndef AKBIT__SYSTEM__NODE_HPP
#define AKBIT__SYSTEM__NODE_HPP


#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <memory>

#include "operators.hpp"
#include "context.hpp"


namespace akbit::system
{
  struct Node
  {
    struct unkown_t { };

    struct module_t
    {
      std::vector<std::shared_ptr<Node>> data;
      std::shared_ptr<Context> global_context;
      bool has_errors;
    };

    struct declaration_t
    {
      std::string name;
      std::shared_ptr<Node> type;
      std::shared_ptr<Node> value;
    };

    struct block_t
    {
      std::vector<std::shared_ptr<Node>> code;
    };

    struct binary_operation_t
    {
      operator_t const * operation;
      std::vector<std::shared_ptr<Node>> operands;
    };

    struct unary_operation_t
    {
      operator_t const * operation;
      std::shared_ptr<Node> expression;
    };
    
    struct function_call_t
    {
      std::shared_ptr<Node> expression;
      std::shared_ptr<Node> arguments;
    };

    struct value_string_t
    {
      std::string value;
    };
    
    struct value_character_t
    {
      std::uint32_t value;
    };
    
    struct value_integer_t
    {
      // TODO: Use special type
      std::string value;
    };

    struct value_decimal_t
    {
      // TODO: Use special type
      std::string value;
    };

    struct value_variable_t
    {
      std::string name;
      std::weak_ptr<DeclarationRecord> record;
    };

    struct value_function_t
    {
      std::vector<std::shared_ptr<Node>> parameters;
      std::shared_ptr<Node> body;
      std::shared_ptr<Context> owned_context;
    };

    struct value_tuple_t
    {
      std::vector<std::shared_ptr<Node>> entries;
    };


public:
    using node_variant_t = std::variant<
      unkown_t,

      module_t,
      declaration_t,
      block_t,
      binary_operation_t,
      unary_operation_t,
      function_call_t,
      

      value_string_t,
      value_character_t,
      value_integer_t,
      value_decimal_t,

      value_variable_t,
      value_function_t,
      value_tuple_t
    >;

  public:
    std::weak_ptr<Context> context;
    node_variant_t value;


  public:
    Node() = default;

    template <class T>
    Node(T&& value)
      : context{}
      , value(value)
    {}

    Node(Node&&) = default;
    
    
    ~Node() = default;

  public:
    Node& operator =(Node&) = default;
  };


  inline std::shared_ptr<Node> make_node_bop(std::shared_ptr<Node> left, std::shared_ptr<Node> right, operator_t const * operation)
  {
    return std::make_shared<Node>(Node::binary_operation_t{
      .operation = operation,
      .operands = {
        left,
        right,
      }
    });
  }
}

#endif
