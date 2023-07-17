#include "Parser.h"
#include "Ast.h"
#include "Lexer.h"

void parser_init(Parser *parser, Lexer *_lexer) { 
    assert(parser && _lexer);
    parser->lexer = _lexer;
    parser->current_token = NULL;
}

void parser_deinit(Parser *parser) { 
    assert(parser);
    parser->current_token = NULL;
}

f64 parse_factor(Parser *parser) { 
    assert(parser);
    assert(parser->current_token->type == Token_Type::TOKEN_INT ||
           parser->current_token->type == Token_Type::TOKEN_FLOAT);

    if (parser->current_token->type == Token_Type::TOKEN_INT) { 
        return parser->current_token->integer_value;
    }
    if (parser->current_token->type == Token_Type::TOKEN_FLOAT) { 
        return parser->current_token->f64_value;
    }
}

f64 parse_expression(Parser *parser) {
    parser->current_token = lexer_get_token(parser->lexer);

    f64 first = parse_factor(parser);
    
    parser->current_token = lexer_get_token(parser->lexer);
    
    while (parser->current_token->type != Token_Type::TOKEN_EOF) { 
        while (parser->current_token->type == '-') { 
            parser->current_token = lexer_get_token(parser->lexer);
            f64 second = parse_factor(parser);
            first = first - second;
        }
        while (parser->current_token->type == '+') { 
            parser->current_token = lexer_get_token(parser->lexer);
            f64 second = parse_factor(parser);
            first = first + second;
        }
        while (parser->current_token->type == '*') { 
            parser->current_token = lexer_get_token(parser->lexer);
            f64 second = parse_factor(parser);
            first = first * second;
        }
        while (parser->current_token->type == '/') { 
            parser->current_token = lexer_get_token(parser->lexer);
            f64 second = parse_factor(parser);
            first = first / second;
        }
        parser->current_token = lexer_get_token(parser->lexer);
    }
    return first;
        
}

f64 parser_parse(Parser *parser) { 
   assert(parser && parser->lexer);
   return parse_expression(parser);
}
