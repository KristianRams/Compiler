#pragma once

#include "Types.h"

/* There are 3 major types of nodes. These are the following ... 
   1) Expressions 
   2) Statements 
   3) Declarations
*/

struct Ast; 

enum Ast_Type : u16 {
    AST_EXPRESSION,
    AST_STATEMENT,
    AST_DECLARATION,
};

struct Ast { 
    Ast_Type ast_type;
};

struct Ast_Expression : public Ast { 
    
};

struct Ast_Statement : public Ast { 
};

struct Ast_Declaration : public Ast { 

}; 

struct Ast_Literal : public Ast_Expression { 
    char *string_value;
    u64 string_count;
    
    f64 float_value;
    u64 integer_value;
};
