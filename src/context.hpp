#pragma once

#ifndef AKBIT__SYSTEM__CONTEXT_HPP
#define AKBIT__SYSTEM__CONTEXT_HPP


#include <cstdint>
#include <vector>


namespace akbit::system
{
  struct Node;

  struct Context
  {
  public:
    uint64_t const id;
    Context *parent;
    std::vector<Node*> declarations;
  
  private:
    static uint64_t generate_next_id()
    {
      static uint64_t next_id = 0;
      return ++next_id;
    }
    
  public:
    Context(Context *parent)
      : id(generate_next_id())
      , parent(parent)
    { }
  };
}

#endif
