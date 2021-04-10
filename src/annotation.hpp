#pragma once

#ifndef AKBIT__SYSTEM__ANNOTATION_HPP
#define AKBIT__SYSTEM__ANNOTATION_HPP

#include "node.hpp"


namespace akbit::system::annotation
{
  void preprocess_ast(std::shared_ptr<Node> node);
  void generate_context(std::shared_ptr<Node> node);
}

#endif
