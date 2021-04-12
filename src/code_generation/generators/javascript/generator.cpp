#include <memory>
#include <type_traits>
#include <variant>
#include <string>
#include <iostream>
#include <fstream>

#include "generator.hpp"


namespace akbit::system::code_generation::js
{
  namespace
  {
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    // Context generation visitors
    std::string cg_visit(std::shared_ptr<Node> node, Settings s);

    std::string cg_visit_module(std::shared_ptr<Node> node, Node::module_t& val, Settings s);
    std::string cg_visit_declaration(std::shared_ptr<Node> node, Node::declaration_t& val, Settings s);
    std::string cg_visit_condition(std::shared_ptr<Node> node, Node::condition_t& val, Settings s);
    std::string cg_visit_block(std::shared_ptr<Node> node, Node::block_t& val, Settings s);
    std::string cg_visit_unary_operation(std::shared_ptr<Node> node, Node::unary_operation_t& val, Settings s);
    std::string cg_visit_binary_operation(std::shared_ptr<Node> node, Node::binary_operation_t& val, Settings s);
    std::string cg_visit_function_call(std::shared_ptr<Node> node, Node::function_call_t& val, Settings s);

    std::string cg_visit_value_function(std::shared_ptr<Node> node, Node::value_function_t& val, Settings s);
    std::string cg_visit_value_tuple(std::shared_ptr<Node> node, Node::value_tuple_t& val, Settings s);
    std::string cg_visit_value_variable(std::shared_ptr<Node> node, Node::value_variable_t& val, Settings s);
    std::string cg_visit_value_string(std::shared_ptr<Node> node, Node::value_string_t& val, Settings s);
    std::string cg_visit_value_character(std::shared_ptr<Node> node, Node::value_character_t& val, Settings s);
    std::string cg_visit_value_integer(std::shared_ptr<Node> node, Node::value_integer_t& val, Settings s);
    std::string cg_visit_value_decimal(std::shared_ptr<Node> node, Node::value_decimal_t& val, Settings s);
  }

  std::string generate(std::shared_ptr<Node> node, Settings settings)
  {
    return cg_visit(node, settings);
  }

  namespace
  {
    std::string cg_visit(std::shared_ptr<Node> node, Settings s)
    {
      if (nullptr == node) return "";

      return std::visit(overloaded {
        [&](auto                     & ) -> std::string { return "__UNKNOWN__";                         },
        [&](Node::module_t           &_) -> std::string { return cg_visit_module(node, _, s);           },
        [&](Node::declaration_t      &_) -> std::string { return cg_visit_declaration(node, _, s);      },
        [&](Node::condition_t        &_) -> std::string { return cg_visit_condition(node, _, s);        },
        [&](Node::block_t            &_) -> std::string { return cg_visit_block(node, _, s);            },
        [&](Node::unary_operation_t  &_) -> std::string { return cg_visit_unary_operation(node, _, s);  },
        [&](Node::binary_operation_t &_) -> std::string { return cg_visit_binary_operation(node, _, s); },
        [&](Node::function_call_t    &_) -> std::string { return cg_visit_function_call(node, _, s);    },
        [&](Node::value_function_t   &_) -> std::string { return cg_visit_value_function(node, _, s);   },
        [&](Node::value_tuple_t      &_) -> std::string { return cg_visit_value_tuple(node, _, s);      },
        [&](Node::value_variable_t   &_) -> std::string { return cg_visit_value_variable(node, _, s);   },
        [&](Node::value_string_t     &_) -> std::string { return cg_visit_value_string(node, _, s);     },
        [&](Node::value_character_t  &_) -> std::string { return cg_visit_value_character(node, _, s);  },
        [&](Node::value_integer_t    &_) -> std::string { return cg_visit_value_integer(node, _, s);    },
        [&](Node::value_decimal_t    &_) -> std::string { return cg_visit_value_decimal(node, _, s);    },
      }, node->value);
    }

    std::string cg_visit_module(std::shared_ptr<Node>, Node::module_t &val, Settings s)
    {
      std::string res = "/* auto-generated code */\n";

      // TODO: Handle errors
      std::fstream js_bootstrap_file;
      // TODO: Place it somewhere else
      js_bootstrap_file.open("src/code_generation/generators/javascript/bootstrap.js", std::ios::in);
      if (js_bootstrap_file)
      {
        std::string line;
        while (std::getline(js_bootstrap_file, line))
          res += line + '\n';

        js_bootstrap_file.close();
      } else { std::cerr << "Failed to read the boostrap.js file!" << std::endl; }

      for (auto d : val.data)
        res += cg_visit(d, s) + ";\n";

      return res;
    }

    std::string cg_visit_declaration(std::shared_ptr<Node>, Node::declaration_t &val, Settings s)
    {
      return std::string(s.indent, ' ') + "let u" + std::get<Node::value_variable_t>(val.variable->value).name + " = " + cg_visit(val.value, s);
    }

    std::string cg_visit_condition(std::shared_ptr<Node>, Node::condition_t &val, Settings s)
    {
      return "(() => { if (" + cg_visit(val.expression, s)
           + ") return " + cg_visit(val.clause_true, s) + "; else return "
           + (val.clause_false ? cg_visit(val.clause_false, s) : std::string("null"))
           + "; })()";
    }

    std::string cg_visit_block(std::shared_ptr<Node>, Node::block_t &val, Settings s)
    {
      std::string res = "(() => {\n";
      s.indent += 4;
      for (auto& stmt : val.code)
      {
        res += std::string(s.indent, ' ');
        if (&stmt == &val.code.back()) res += "return ";
        res += cg_visit(stmt, s) + ";\n";
      }
      res += std::string(s.indent - 4, ' ') + "})()";
      return res;
    }

    std::string cg_visit_unary_operation(std::shared_ptr<Node>, Node::unary_operation_t &val, Settings s)
    {
      return val.operation->representation + "(" + cg_visit(val.expression, s) + ")";
    }

    std::string cg_visit_binary_operation(std::shared_ptr<Node>, Node::binary_operation_t &val, Settings s)
    {
      std::string res = "so" + std::to_string(val.operation->id) + "(";
      for (auto& p : val.operands)
      {
        if (&p != &val.operands.front()) res += ", ";
        res += cg_visit(p, s);
      }

      return res + ")";
    }

    std::string cg_visit_function_call(std::shared_ptr<Node>, Node::function_call_t &val, Settings s)
    {
      std::string res = "(" + cg_visit(val.expression, s) + ")";
      s.vectorise_tuple = false;
      res += cg_visit(val.arguments, s);
      return res;
    }

    std::string cg_visit_value_function(std::shared_ptr<Node>, Node::value_function_t &val, Settings s)
    {
      std::string res = "((";
      
      for (auto& p : val.parameters)
      {
        if (&p != &val.parameters.front()) res += ", ";
        res += cg_visit(std::get<Node::declaration_t>(p->value).variable, s);
      }
      res += ") => " + cg_visit(val.body, s) + ")";

      return res;
    }

    std::string cg_visit_value_tuple(std::shared_ptr<Node>, Node::value_tuple_t &val, Settings s)
    {
      std::string res = s.vectorise_tuple ? "[" : "(";
      s.vectorise_tuple = true;

      for (auto& p : val.entries)
      {
        if (&p != &val.entries.front()) res += ", ";
        res += cg_visit(p, s);
      }

      res += (res[0] == '[') ? ']' : ')';
      return res;
    }
    
    std::string cg_visit_value_variable(std::shared_ptr<Node>, Node::value_variable_t &val, Settings)
    { return (val.record.lock() ? "u" : "s_") + val.name; }

    std::string cg_visit_value_string(std::shared_ptr<Node>, Node::value_string_t& val, Settings)
    { return val.value; }
    
    std::string cg_visit_value_character(std::shared_ptr<Node>, Node::value_character_t& val, Settings)
    { return std::string(1, val.value); }

    std::string cg_visit_value_integer(std::shared_ptr<Node>, Node::value_integer_t& val, Settings)
    { return val.value; }
    
    std::string cg_visit_value_decimal(std::shared_ptr<Node>, Node::value_decimal_t& val, Settings)
    { return val.value; }
  }
}
