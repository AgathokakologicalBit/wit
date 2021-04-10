#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>


#include "parsing/lexing.hpp"
#include "parsing/parsing.hpp"
#include "annotation.hpp"


std::string get_node_type_name(akbit::system::NodeType type)
{
  using nt = akbit::system::NodeType;
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

void dump_ast(akbit::system::Node *node_, std::uint32_t depth, std::uint64_t mask)
{
  using nt = akbit::system::NodeType;
  using nvt = akbit::system::NodeValueType;

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
    case nt::t_unknown:
    {
      std::cout << "\x1b[44mUNKNOWN*\x1b[49m";
    } break;

    case nt::t_module:
    {
      for (std::size_t i = 0; i < node.module.data->size(); ++i)
      {
        if (i == node.module.data->size() - 1)
          mask |= 1ul << 0u;
        dump_ast((*node.module.data)[i], depth, mask);
      }
    } break;

    case nt::t_declaration:
    {
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
    } break;

    case nt::t_value:
    {
      std::wcout << L"\x1b[95m";
      switch (node.value.type)
      {
        case nvt::t_character:
        {
          std::wcout << L'\'' << (wchar_t) node.value.as_character;
        } break;
        case nvt::t_integer:
        {
          std::cout << *node.value.as_integer;
        } break;
        case nvt::t_decimal:
        {
          std::cout << *node.value.as_decimal;
        } break;
        case nvt::t_string:
        {
          std::cout << *node.value.as_string;
        } break;
        case nvt::t_variable:
        {
          std::cout << *node.value.as_variable.name;
          auto record = node.value.as_variable.record;
          if (record != nullptr)
          {
            std::cout << "\x1b[33m(0x" << std::setw(4) << std::setfill('0') << std::hex << record->context->id << ")\x1b[39m";
          }
          else
          {
            std::cout << "\x1b[33m(" << "------" << ")\x1b[39m";
          }
        } break;
        
        case nvt::t_tuple:
        {
          std::cout << "\x1b[39m\x1b[44mTUPLE\x1b[49m";
          for (std::size_t i = 0; i < node.value.as_tuple.entries->size(); ++i)
          {
            if (i == node.value.as_tuple.entries->size() - 1)
              mask |= static_cast<std::uint64_t>(1) << depth;
            dump_ast((*node.value.as_tuple.entries)[i], depth, mask);
          }
        } break;
        
        case nvt::t_function:
        {
          std::cout
            << "\x1b[39m\x1b[44mFUNCTION\x1b[49m"
            << "(\x1b[33m0x" << std::setw(4) << std::setfill('0') << std::hex << node.context->id << "\x1b[39m)\n";
          draw_p(depth, mask | (0ul << (depth + 0u)));
          std::cout << "parameters:";
          for (std::size_t i = 0; i < node.value.as_function.parameters->size(); ++i)
          {
            auto sub_mask = mask;
            if (i == node.value.as_tuple.entries->size() - 1ul)
              sub_mask |= static_cast<std::uint64_t>(1) << (depth + 1u);
            dump_ast((*node.value.as_tuple.entries)[i], depth + 1, sub_mask);
          }

          dump_ast(node.value.as_function.body, depth, mask | (1ul << (depth + 0u)));
        } break;
      }
    } break;

    case nt::t_binary_operation:
    {
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
    } break;

    case nt::t_unary_operation:
    {
      std::cout << '\n';
      draw_p(depth, mask);
      std::cout << "operator: ";
      std::wcout << L"\x1b[95m";
      std::cout << node.unary_operation.operation->representation;

      mask |= static_cast<std::uint64_t>(1) << depth;

      dump_ast(node.unary_operation.expression, depth, mask);
    } break;

    case nt::t_function_call:
    {
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
    } break;

    case nt::t_block:
    {
      for (std::size_t i = 0; i < node.block.code->size(); ++i)
      {
        if (i == node.block.code->size() - 1)
          mask |= static_cast<std::uint64_t>(1) << depth;
        dump_ast((*node.block.code)[i], depth, mask);
      }
    } break;
  }
}

int main(int argc, char* argv[])
{
  // std::string source{}, line;
  // while (getline(std::cin, line))
  //   source += line + '\n';

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
  std::cout << "\n\x1b[39mResult: " << (ast->module.has_errors ? "\x1b[01;41mFAILURE" : "\x1b[01;44mSUCCESS") << "\x1b[49m" << std::endl;
  std::cout << std::endl;
  
  return EXIT_SUCCESS;
}
