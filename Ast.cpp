#include "Ast.h"

#include <assert.h>

Ast *NEW_AST(Ast_Type ast_type) { 
    switch (ast_type) { 
        case Ast_Type::AST_EXPRESSION: { 
            Ast *ast = new Ast_Expression;
            ast->ast_type = ast_type;
            return ast;
        } 
    }
    return NULL;
}
