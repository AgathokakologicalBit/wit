#pragma once

#ifndef AKBIT__SYSTEM__OPERATORS_HPP
#define AKBIT__SYSTEM__OPERATORS_HPP


#include <array>
#include <string>
#include <cstdint>


namespace akbit::system::parsing
{
  enum Associativity : bool
  {
    ASSOC_LEFT  = false,
    ASSOC_RIGHT = true,
  };

  struct operator_t
  {
    uint32_t precedence;
    Associativity associativity;
    std::string representation;

    operator_t(std::string representation_, uint32_t precedence_, Associativity associativity_)
      : precedence(precedence_)
      , associativity(associativity_)
      , representation(representation_)
    { }
  };

  extern std::array<operator_t const, 23> const operators_list;

  extern operator_t const & operator_unknown;
  extern operator_t const & operator_type_cast;
}

#endif