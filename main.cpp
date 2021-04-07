#include <string>
#include <vector>
#include <iostream>


#include "lexing.hpp"


int main(int argc, char* argv[])
{
  std::string source{}, line;
  while (getline(std::cin, line))
    source += line + '\n';
  
  auto result = akbit::system::parsing::tokenize(source);
  for (auto t : result)
    std::cout << t << std::endl;
  return 0;
}
