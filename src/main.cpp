#include <memory>
#include <string>
#include <string.h>
#include <type_traits>
#include <variant>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>


#include "parsing/lexing.hpp"
#include "parsing/parsing.hpp"
#include "annotation.hpp"
#include "node.hpp"
#include "context.hpp"
#include "code_generation/generation.hpp"
#include "code_generation/generators/javascript/generator.hpp"


namespace
{
  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


  std::string get_node_type_name(akbit::system::Node const & node)
  {
    using sn = akbit::system::Node;
    return std::visit(overloaded {
      [](auto                  &) -> std::string { return "#???";             },
      [](sn::module_t           ) -> std::string { return "module";           },
      [](sn::declaration_t      ) -> std::string { return "declaration";      },
      [](sn::condition_t        ) -> std::string { return "condition";        },
      [](sn::binary_operation_t ) -> std::string { return "operation-binary"; },
      [](sn::unary_operation_t  ) -> std::string { return "operation-unary";  },
      [](sn::function_call_t    ) -> std::string { return "call";             },
      [](sn::block_t            ) -> std::string { return "block";            },
      [](sn::value_function_t   ) -> std::string { return "function";         },
      [](sn::value_tuple_t      ) -> std::string { return "tuple";            },
      [](sn::value_variable_t   ) -> std::string { return "variable";         },
      [](sn::value_string_t     ) -> std::string { return "string";           },
      [](sn::value_character_t  ) -> std::string { return "character";        },
      [](sn::value_integer_t    ) -> std::string { return "integer";          },
      [](sn::value_decimal_t    ) -> std::string { return "decimal";          },
    }, node.value);
  }

  void draw_p(std::uint32_t depth, std::uint64_t mask)
  {
    if (depth == ~(static_cast<std::uint32_t>(0)))
      return;

    std::wcout << L"\x1b[32m";
    for (std::uint32_t i = 0; i < depth; ++i)
    {
      if (mask & (static_cast<std::uint64_t>(1) << i))
        std::cout << "\x20\x20\x20";
      else
        std::cout << "\u2502\x20\x20";
    }

    if (mask & (static_cast<std::uint64_t>(1) << depth))
      std::cout << "\u2514\u2500 ";
    else
      std::cout << "\u251C\u2500 ";

    std::wcout << L"\x1b[37m";
  }
}

void dump_ast(std::shared_ptr<akbit::system::Node> node_, std::uint32_t depth, std::uint64_t mask)
{
  using akbit::system::Node;

  std::cout << '\n';
  draw_p(depth, mask);
  ++depth;

  if (nullptr == node_)
  {
    std::cout << "\x1b[44mVOID*\x1b[49m";
    return;
  }

  auto &node = *node_;
  std::cout << get_node_type_name(node) << ": ";

  std::visit(overloaded {
    [&](auto&) { std::cout << "\x1b[44mUNKNOWN*\x1b[49m"; },
    
    [&](Node::module_t           &node) {
      for (std::size_t i = 0; i < node.data.size(); ++i)
        dump_ast(node.data[i], depth, mask);
    },

    [&](Node::declaration_t      &node) {
      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "variable: ";
      // std::cout << "\x1b[97m";
      dump_ast(node.variable, depth + 1u, mask);

      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "type: ";
      // std::cout << "\x1b[97m";
      dump_ast(node.type, depth + 1u, mask);

      dump_ast(node.value, depth, mask);
    },

    [&](Node::condition_t        &node) {
      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "expression: ";
      std::cout << "\x1b[97m";
      dump_ast(node.expression, depth + 1u, mask);

      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "clause_true: ";
      std::cout << "\x1b[97m";
      dump_ast(node.clause_true, depth + 1u, mask);

      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "clause_false: ";
      std::cout << "\x1b[97m";
      dump_ast(node.clause_false, depth + 1u, mask);
    },


    [&](Node::block_t            &node) {
      for (std::size_t i = 0; i < node.code.size(); ++i)
        dump_ast(node.code[i], depth, mask);
    },

    [&](Node::unary_operation_t  &node) {
      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "operator: ";
      std::wcout << L"\x1b[95m";
      std::cout << node.operation->representation;

      mask |= static_cast<std::uint64_t>(1) << depth;
      dump_ast(node.expression, depth, mask);
    },

    [&](Node::binary_operation_t &node) {
      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "operator: ";
      std::wcout << L"\x1b[95m";
      std::cout << node.operation->representation;

      for (std::size_t i = 0; i < node.operands.size(); ++i)
        dump_ast(node.operands[i], depth, mask);
    },

    [&](Node::function_call_t    &node) {
      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "expression: ";
      dump_ast(node.expression, depth + 1u, mask | (1u << (depth + 1u)));

      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "arguments: ";
      dump_ast(node.arguments, depth + 1u, mask | (1u << (depth + 1u)));
    },

    [&](Node::value_function_t   &node) {
      std::cout
        << "\x1b[39m\x1b[44mFUNCTION\x1b[49m"
        << "(\x1b[33m0x" << std::setw(4) << std::setfill('0') << std::hex << node.owned_context->id << "\x1b[39m)\n";
      draw_p(depth, mask | (0ul << (depth + 0u)));
      std::cout << "parameters:";
      for (std::size_t i = 0; i < node.parameters.size(); ++i)
        dump_ast(node.parameters[i], depth + 1, mask);

      dump_ast(node.body, depth, mask);
    },
    [&](Node::value_tuple_t      &node) {
      std::cout << "\x1b[39m\x1b[44mTUPLE\x1b[49m";
      for (std::size_t i = 0; i < node.entries.size(); ++i)
      {
        // if (i == node.entries.size() - 1)
        //   mask |= static_cast<std::uint64_t>(1) << depth;
        dump_ast(node.entries[i], depth, mask);
      }
    },

    [&](Node::value_variable_t   &node) {
      std::cout << node.name;
      auto record = node.record;
      if (record.lock())
      {
        std::cout << "\x1b[33m(0x" << std::setw(4) << std::setfill('0') << std::hex << record.lock()->context.lock()->id << ")\x1b[39m";
      }
      else
      {
        std::cout << "\x1b[33m(" << "------" << ")\x1b[39m";
      }
    },

    [&](Node::value_string_t     &node) { std::cout << node.value; },
    [&](Node::value_character_t  &node) { std::wcout << L'\'' << (wchar_t) node.value; },
    [&](Node::value_integer_t    &node) { std::cout << node.value; },
    [&](Node::value_decimal_t    &node) { std::cout << node.value; },
  }, node.value);

  std::cout << '\n';
  draw_p(depth, mask | (1u << depth));
  std::cout << "result_type: " << akbit::system::etype_to_str(node.result_type);
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    char const *name = (argc > 0 ? argv[0] : "witcc");
    std::cerr << "Usage: " << name << ' ' << "<filename>" << std::endl;
    return EXIT_FAILURE;
  }
  
  std::ifstream ifs(argv[1]);
  if (!ifs)
  {
    std::cerr << "File could not be opened!\n";
    std::cerr << "Reason: " << strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }
  std::string source((std::istreambuf_iterator<char>(ifs)),
                     (std::istreambuf_iterator<char>()   ));
  
  auto tokens = akbit::system::parsing::tokenize(source);
  // for (auto t : tokens)
  //   std::cout << t << std::endl;

  auto ast = akbit::system::parsing::parse(tokens);
  if (!ast) return EXIT_FAILURE;
  
  akbit::system::annotation::preprocess_ast(ast);
  akbit::system::annotation::generate_context(ast);

  dump_ast(ast, -1u, 0ul);
  std::cout << "\n\x1b[39mResult: "
    << (std::get<akbit::system::Node::module_t>(ast->value).has_errors ? "\x1b[01;41mFAILURE" : "\x1b[01;44mSUCCESS")
    << "\x1b[49m\x1b[00;39m" << std::endl;
  
  std::fstream js_output_file;
	js_output_file.open("program.out.js", std::ios::out);
  if (js_output_file)
  {
    js_output_file << akbit::system::code_generation::generate(ast, akbit::system::code_generation::GenerationTarget::Javascript, nullptr);
    js_output_file.close();
  }

  return EXIT_SUCCESS;
}
