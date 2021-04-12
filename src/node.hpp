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


namespace akbit::system
{
  struct Context;
  struct DeclarationRecord;

  struct Node
  {
    enum struct etype_t
    {
      unknown,

      string,
      character,
      integer,
      decimal,

      function,
      tuple,

      any,
    };

    struct unknown_t { };

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

    struct condition_t
    {
      std::shared_ptr<Node> expression;
      std::shared_ptr<Node> clause_true;
      std::shared_ptr<Node> clause_false;
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
      unknown_t,

      module_t,
      
      // statement-like expressions
      declaration_t,
      condition_t,

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
    etype_t result_type;


  public:
    Node() = default;

    template <class T>
    Node(T&& value)
      : context{}
      , value(value)
      , result_type(etype_t::unknown)
    {}

    Node(Node&&) = default;
    
    
    ~Node() = default;

  public:
    Node& operator =(Node&) = default;
  };


  inline std::string etype_to_str(Node::etype_t t)
  {
    switch (t)
    {
      case Node::etype_t::unknown: return "unknown";
      case Node::etype_t::string: return "string";
      case Node::etype_t::character: return "character";
      case Node::etype_t::integer: return "integer";
      case Node::etype_t::decimal: return "decimal";

      case Node::etype_t::function: return "function";
      case Node::etype_t::tuple: return "tuple";

      case Node::etype_t::any: return "any";
    } 
  }

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
