#pragma once

#ifndef AKBIT__SYSTEM__CODE_GENERATION__GENERATION_HPP
#define AKBIT__SYSTEM__CODE_GENERATION__GENERATION_HPP

#include "../node.hpp"
#include "generators/javascript/generator.hpp"


namespace akbit::system::code_generation
{
  enum struct GenerationTarget
  {
    Javascript,
  };

  inline std::string generate(std::shared_ptr<Node> node, GenerationTarget target, void* settings)
  {
    auto ags = ((nullptr == settings) ? js::Settings() : *((js::Settings*) settings));
    return js::generate(node, ags);
  }
}

#endif
