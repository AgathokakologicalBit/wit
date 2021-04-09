#include <string>
#include <vector>
#include <iostream>
#include <fstream>


#include "lexing.hpp"
#include "parsing.hpp"


std::string get_node_type_name(akbit::system::parsing::NodeType type)
{
  using nt = akbit::system::parsing::NodeType;
  switch (type)
  {
    case nt::t_module: return "module";
    case nt::t_declaration: return "declaration";
    case nt::t_type: return "type";
    case nt::t_value: return "value";
    case nt::t_binary_operation: return "operation-binary";
    case nt::t_unary_operation: return "operation-unary";
    case nt::t_function_call: return "call";
    case nt::t_block: return "block";
    case nt::t_member_access: return "member-access";
  }

  return "#???";
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

void dump_ast(akbit::system::parsing::Node *node_, std::uint32_t depth, std::uint64_t mask)
{
  using nt = akbit::system::parsing::NodeType;
  using nvt = akbit::system::parsing::NodeValueType;

  std::cout << '\n';
  draw_p(depth, mask);
  ++depth;

  if (nullptr == node_)
  {
    std::cout << "\x1b[44mVOID*\x1b[49m";
    return;
  }

  auto &node = *node_;
  std::cout << get_node_type_name(node.type) << ": ";

  switch (node.type)
  {
    case nt::t_module:
      for (std::size_t i = 0; i < node.module.data->size(); ++i)
      {
        if (i == node.module.data->size() - 1)
          mask |= 1ul << 0u;
        dump_ast((*node.module.data)[i], depth, mask);
      }
      break;

    case nt::t_declaration:
      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "name: ";
      std::cout << "\x1b[97m";
      std::cout << *node.declaration.name;

      std::cout << '\n';
      draw_p(depth, mask );
      std::cout << "type: ";
      std::cout << "\x1b[97m";
      dump_ast(node.declaration.type, depth + 1u, mask | (1ul << (depth + 1u)));

      mask |= static_cast<std::uint64_t>(1) << depth;

      dump_ast(node.declaration.value, depth, mask);
      break;

    case nt::t_value:
      std::wcout << L"\x1b[95m";
      switch (node.value.type)
      {
        case nvt::t_character:
          std::wcout << L'\'' << (wchar_t) node.value.as_character;
          break;
        case nvt::t_integer:
          std::cout << *node.value.as_integer;
          break;
        case nvt::t_decimal:
          std::cout << *node.value.as_decimal;
          break;
        case nvt::t_string:
          std::cout << *node.value.as_string;
          break;
        case nvt::t_variable:
          std::cout << *node.value.as_variable;
          break;
        
        case nvt::t_tuple:
          std::cout << "\x1b[39m\x1b[44mTUPLE\x1b[49m";
          for (std::size_t i = 0; i < node.value.as_tuple.entries->size(); ++i)
          {
            if (i == node.value.as_tuple.entries->size() - 1)
              mask |= static_cast<std::uint64_t>(1) << depth;
            dump_ast((*node.value.as_tuple.entries)[i], depth, mask);
          }
          break;
      }
      break;

    case nt::t_binary_operation:
      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "operator: ";
      std::wcout << L"\x1b[95m";
      std::cout << node.binary_operation.operation->representation;

      for (std::size_t i = 0; i < node.binary_operation.operands->size(); ++i)
      {
        if (i == node.binary_operation.operands->size() - 1)
          mask |= static_cast<std::uint64_t>(1) << depth;
        dump_ast((*node.binary_operation.operands)[i], depth, mask);
      }
      break;

    case nt::t_unary_operation:
      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "operator: ";
      std::wcout << L"\x1b[95m";
      std::cout << node.unary_operation.operation->representation;

      mask |= static_cast<std::uint64_t>(1) << depth;

      dump_ast(node.unary_operation.expression, depth, mask);
      break;

    case nt::t_function_call:
      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "expression: ";

      mask |= static_cast<std::uint64_t>(1) << (depth + 1);
      dump_ast(node.call.expression, depth + 1u, mask);

      mask |= static_cast<std::uint64_t>(1) << depth;

      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "arguments: ";
      dump_ast(node.call.arguments, depth + 1u, mask);
      break;

    case nt::t_block:
      for (std::size_t i = 0; i < node.block.code->size(); ++i)
      {
        if (i == node.block.code->size() - 1)
          mask |= static_cast<std::uint64_t>(1) << depth;
        dump_ast((*node.block.code)[i], depth, mask);
      }
      break;
  }
}

int main(int argc, char* argv[])
{
  // std::string source{}, line;
  // while (getline(std::cin, line))
  //   source += line + '\n';
  
  std::ifstream ifs("test_input.ws");
  std::string source((std::istreambuf_iterator<char>(ifs)),
                     (std::istreambuf_iterator<char>()   ));
  
  auto tokens = akbit::system::parsing::tokenize(source);
  for (auto t : tokens)
    std::cout << t << std::endl;

  auto ast = akbit::system::parsing::parse(tokens);
  if (ast)
  {
    std::cout << "result: " << (ast->module.has_errors ? "failure" : "success") << std::endl;
    dump_ast(ast, -1u, 0ul);
    std::cout << std::endl;
  }
  
  return 0;
}
