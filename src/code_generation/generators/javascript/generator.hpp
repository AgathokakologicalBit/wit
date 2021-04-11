#pragma once

#ifndef AKBIT__SYSTEM__CODE_GENERATION__JS_GENERATOR_HPP
#define AKBIT__SYSTEM__CODE_GENERATION__JS_GENERATOR_HPP

#include "../../../node.hpp"


namespace akbit::system::code_generation::js
{
  struct Settings
  {
    bool prettify = true;
    std::uint32_t indent = 0;
    bool vectorise_tuple = true;
  };

  std::string generate(std::shared_ptr<Node> node, Settings settings);
}

#endif
