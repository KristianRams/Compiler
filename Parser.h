#pragma once 

#include "Types.h"

struct Ast;
struct Token;
struct Lexer;

struct Parser { 
    Lexer *lexer;
    Token *current_token;
};

void parser_init(Parser *parser, Lexer *lexer);
void parser_deinit(Parser *parser);
f64 parser_parse(Parser *parser);
