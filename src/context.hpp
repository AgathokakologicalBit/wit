#pragma once

#ifndef AKBIT__SYSTEM__CONTEXT_HPP
#define AKBIT__SYSTEM__CONTEXT_HPP


#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

#include "node.hpp"


namespace akbit::system
{
  struct Node;
  struct Context;

  struct DeclarationRecord
  {
    std::weak_ptr<Context> context;
    std::string name;
    // TODO: Use dedicated type class instead
    Node::etype_t type;
  };

  struct Context
  {
  public:
    uint64_t const id;
    std::weak_ptr<Context> parent;
    std::vector<std::shared_ptr<DeclarationRecord>> declarations;
  
  private:
    static uint64_t generate_next_id()
    {
      static uint64_t next_id = 0;
      return ++next_id;
    }
    
  public:
    Context(std::shared_ptr<Context> parent)
      : id(generate_next_id())
      , parent(parent)
      , declarations{}
    { }

  public:
    std::shared_ptr<DeclarationRecord> add(std::shared_ptr<Context> self, std::string& name, Node::etype_t type)
    {
      return this->declarations.emplace_back(new DeclarationRecord{self, name, type});
    }

    std::vector<std::shared_ptr<DeclarationRecord>> get(std::string& name) const
    {
      std::vector<std::shared_ptr<DeclarationRecord>> results{};
      std::copy_if(this->declarations.begin(), this->declarations.end(),
                   std::back_inserter(results),
                   [&](auto& record) { return record->name == name; });
      return results;
    }

    std::vector<std::shared_ptr<DeclarationRecord>> find(std::string& name) const
    {
      std::vector<std::shared_ptr<DeclarationRecord>> results{};
      std::copy_if(this->declarations.begin(), this->declarations.end(),
                   std::back_inserter(results),
                   [&](auto& record) { return record->name == name; });
      if (!this->parent.lock()) return results;

      auto rest = this->parent.lock()->find(name);
      results.reserve(results.size() + rest.size());
      results.insert(results.end(), rest.begin(), rest.end());
      return results;
    }
  };
}

#endif
