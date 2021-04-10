#pragma once

#ifndef AKBIT__SYSTEM__CONTEXT_HPP
#define AKBIT__SYSTEM__CONTEXT_HPP


#include <algorithm>
#include <cstdint>
#include <vector>


namespace akbit::system
{
  struct Node;
  struct Context;

  struct DeclarationRecord
  {
    Context *context;
    std::string name;
    Node *type;
  };

  struct Context
  {
  public:
    uint64_t const id;
    Context *parent;
    std::vector<DeclarationRecord*> declarations;
  
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
      , declarations{}
    { }

  public:
    DeclarationRecord* add(std::string& name, Node *type)
    {
      return this->declarations.emplace_back(new DeclarationRecord{this, name, type});
    }

    std::vector<DeclarationRecord*> get(std::string& name) const
    {
      std::vector<DeclarationRecord*> results{};
      std::copy_if(this->declarations.begin(), this->declarations.end(),
                   std::back_inserter(results),
                   [&](DeclarationRecord const *record) { return record->name == name; });
      return results;
    }

    std::vector<DeclarationRecord*> find(std::string& name) const
    {
      std::vector<DeclarationRecord*> results{};
      std::copy_if(this->declarations.begin(), this->declarations.end(),
                   std::back_inserter(results),
                   [&](DeclarationRecord const *record) { return record->name == name; });
      if (nullptr == this->parent) return results;

      auto rest = this->parent->find(name);
      results.reserve(results.size() + rest.size());
      results.insert(results.end(), rest.begin(), rest.end());
      return results;
    }
  };
}

#endif
