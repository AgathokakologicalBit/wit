#include "../operators.hpp"


namespace akbit::system
{
  std::array<operator_t const, 22> const operators_list
  {
    operator_t( 0, ""  ,    0, ASSOC_LEFT ),       //  0 - unknown


    operator_t( 1, "." , 1000, ASSOC_LEFT ),       //  1 - member access

    operator_t( 2, "^" ,  250, ASSOC_RIGHT),       //  2 - power

    operator_t( 3, "*" ,  200, ASSOC_LEFT ),       //  3 - multiplication
    operator_t( 4, "/" ,  200, ASSOC_LEFT ),       //  4 - division
    operator_t( 5, "%" ,  200, ASSOC_LEFT ),       //  5 - modulus

    operator_t( 6, "+" ,  100, ASSOC_LEFT ),       //  6 - addition
    operator_t( 7, "-" ,  100, ASSOC_LEFT ),       //  7 - subtraction


    operator_t( 8, ">=",   50, ASSOC_LEFT ),       //  8 - greater or equal
    operator_t( 9, "<=",   50, ASSOC_LEFT ),       //  9 - less or equal
    operator_t(10, ">" ,   50, ASSOC_LEFT ),       // 10 - greater than
    operator_t(11, "<" ,   50, ASSOC_LEFT ),       // 11 - less than

    operator_t(12, "==",   40, ASSOC_LEFT ),       // 12 - equality
    operator_t(13, "!=",   40, ASSOC_LEFT ),       // 13 - inequality


    operator_t(14, "&" ,   25, ASSOC_LEFT ),       // 14 - bitwise and
    operator_t(15, "|" ,   20, ASSOC_LEFT ),       // 15 - bitwise or

    operator_t(16, "&&",   15, ASSOC_LEFT ),       // 16 - logical and
    operator_t(17, "||",   10, ASSOC_LEFT ),       // 17 - logical or


    operator_t(18, "->",    4, ASSOC_RIGHT),       // 18 - lambda function
    operator_t(19, "," ,    3, ASSOC_LEFT ),       // 19 - tupling

    operator_t(20, ":" ,    2, ASSOC_LEFT ),       // 20 - type cast

    operator_t(21, "=" ,    1, ASSOC_RIGHT),       // 21 - assignment
    // TODO: Add assignments with operations
  };

  operator_t const & operator_unknown = operators_list[0];
  operator_t const & operator_type_cast = operators_list[20];

  operator_t const * find_operator(std::string representation)
  {
    for (auto& op : operators_list)
      if (op.representation == representation)
        return &op;
    return nullptr;
  }
}
