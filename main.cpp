#include <string>
#include <vector>
#include <iostream>


#include "lexing.hpp"
#include "parsing.hpp"


int main(int argc, char* argv[])
{
  std::string source{}, line;
  while (getline(std::cin, line))
    source += line + '\n';
  
  auto tokens = akbit::system::parsing::tokenize(source);
  for (auto t : tokens)
    std::cout << t << std::endl;

  auto ast = akbit::system::parsing::parse(tokens);
  if (ast)
  {
    std::cout << "Sucess!" << std::endl;
  }
  
  return 0;
}
