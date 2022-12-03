#ifndef MATH_OP_DEFINES_H_INCLUDED
#define MATH_OP_DEFINES_H_INCLUDED


#endif // MATH_OP_DEFINES_H_INCLUDED

// #define MATH_OP_DEF(_enum, _enumval, _name, _pri, _ret, _diff)

MATH_OP_DEF(MATH_O_ADD  ,1        , "+"   , 1, a + b        )
MATH_OP_DEF(MATH_O_SUB  ,2        , "-"   , 1, a - b        )
MATH_OP_DEF(MATH_O_UMIN ,2  | 0x80, "-"   , 5, -b           )
MATH_OP_DEF(MATH_O_MUL  ,3        , "*"   , 2, a * b        )
MATH_OP_DEF(MATH_O_DIV  ,4        , "/"   , 2, a / b        )
MATH_OP_DEF(MATH_O_POW  ,5        , "^"   , 4, pow(a,b)     )
MATH_OP_DEF(MATH_O_LOG  ,6        , "log" , 3, log(b)/log(a))
MATH_OP_DEF(MATH_O_COMMA,7        , ","   , 0, NAN          )
MATH_OP_DEF(MATH_O_EXP  ,8  | 0x80, "exp" , 3, exp(b)       )
MATH_OP_DEF(MATH_O_LN   ,9  | 0x80, "ln"  , 3, log(b)       )
MATH_OP_DEF(MATH_O_SIN  ,10 | 0x80, "sin" , 3, sin(b)       )
MATH_OP_DEF(MATH_O_COS  ,11 | 0x80, "cos" , 3, cos(b)       )
MATH_OP_DEF(MATH_O_TG   ,12 | 0x80, "tan" , 3, tan(b)       )
MATH_OP_DEF(MATH_O_ASIN ,13 | 0x80, "asin", 3, asin(b)      )
MATH_OP_DEF(MATH_O_ACOS ,14 | 0x80, "acos", 3, acos(b)      )
MATH_OP_DEF(MATH_O_ATG  ,15 | 0x80, "atan", 3, atan(b)      )
MATH_OP_DEF(MATH_O_SH   ,16 | 0x80, "sinh", 3, sinh(a)      )
MATH_OP_DEF(MATH_O_CH   ,17 | 0x80, "cosh", 3, cosh(a)      )
MATH_OP_DEF(MATH_O_TH   ,18 | 0x80, "tanh", 3, tanh(a)      )
MATH_OP_DEF(MATH_O_SQRT ,19 | 0x80, "sqrt", 3, sqrt(a)      )
MATH_OP_DEF(MATH_O_o    ,20 | 0x80, "o"   , 3, 0            )
MATH_OP_DEF(MATH_O_d    ,21 | 0x80, "d"   , 3, 0            )

