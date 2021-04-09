#pragma once

#ifndef AKBIT__SYSTEM__ANNOTATION_HPP
#define AKBIT__SYSTEM__ANNOTATION_HPP

#include "node.hpp"


namespace akbit::system::annotation
{
  void preprocess_ast(Node *node);
  void generate_context(Node *node);
}

#endif
