#pragma once

#include "Types.h"
#include "Hash_Table.h"

/**
   This lexer lexs on demand instead of doing it all it one shot.
   
   Doing it this way we can compartmentalize the lexer and the parser in a way such that
   the parser is separate from the lexer. If we want to make relatively large changes to the lexer
   the parser should not care.
   
   This makes it easier in the future if we want to have the lexer running in it's own thread
   scanning the input file and caching all of the tokens in an array (insert fast data structure here, 
   preferably a data structure which is packed contiguously so we pull in tokens at cache line at a 
   time and get speed benefits.) and the parser can pull from this prefetched/precached (I know these are two 
   different things) piece of memory and have performance benefits. (Well that's the thought anyway)
   
   I've seen lexers which output tokens in a linked-list (not a fan due to potential cache misses everytime 
   the parser wants a token (which it will do many many times))
**/

enum Token_Type { 
    /*
        0 -  31 is reversed for ascii control   codes.
       32 - 127 is reversed for ascii printable codes.
      128 - 255 is reversed for ascii extended  codes.
    */

    TOKEN_IDENT  = 256, 
    TOKEN_INT    = 257, 
    TOKEN_FLOAT  = 258, 
    TOKEN_CHAR   = 259, 
    TOKEN_STRING = 260,  
    TOKEN_BOOL   = 261,
    TOKEN_VOID   = 262,

    TOKEN_PLUS_EQUALS  = 263,
    TOKEN_MINUS_EQUALS = 264, 
    TOKEN_TIMES_EQUALS = 265, 
    TOKEN_DIV_EQUALS   = 266, 
    TOKEN_MOD_EQUALS   = 267, 

    TOKEN_IS_EQUAL            = 268, 
    TOKEN_IS_NOT_EQUAL        = 269,  
    TOKEN_LESS_THAN           = 270,  
    TOKEN_LESS_THAN_EQUALS    = 271, 
    TOKEN_GREATER_THAN        = 272,  
    TOKEN_GREATER_THAN_EQUALS = 273, 
    TOKEN_ASSIGN              = 274,
  
    TOKEN_LOGICAL_AND  = 275,
    TOKEN_LOGICAL_OR   = 276,
  
    TOKEN_SHIFT_LEFT   = 277, 
    TOKEN_SHIFT_RIGHT  = 278,
  
    TOKEN_BITWISE_AND_EQUALS = 279, 
    TOKEN_BITWISE_OR_EQUALS  = 280, 
    TOKEN_BITWISE_XOR_EQUALS = 281, 
  
    // Start of keywords
    TOKEN_KEYWORD_CONST,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_SWITCH,
    TOKEN_KEYWORD_CASE,
    TOKEN_KEYWORD_DEFAULT,
    TOKEN_KEYWORD_BREAK,
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_FOR,
    TOKEN_KEYWORD_GOTO,
    TOKEN_KEYWORD_CONTINUE,
    TOKEN_KEYWORD_STRUCT,
    TOKEN_KEYWORD_NULL,
    TOKEN_KEYWORD_ENUM,
    TOKEN_KEYWORD_UNION,
    TOKEN_KEYWORD_TRUE,
    TOKEN_KEYWORD_FALSE,
    TOKEN_KEYWORD_FLOAT,
    TOKEN_KEYWORD_INT,
    TOKEN_KEYWORD_CHAR,
    TOKEN_KEYWORD_VOID,
    TOKEN_KEYWORD_STATIC,
    TOKEN_KEYWORD_DO,
    TOKEN_KEYWORD_F32,
    TOKEN_KEYWORD_F64,
    TOKEN_KEYWORD_S8,
    TOKEN_KEYWORD_S16,
    TOKEN_KEYWORD_S32,
    TOKEN_KEYWORD_S64,
    TOKEN_KEYWORD_U8,
    TOKEN_KEYWORD_U16,
    TOKEN_KEYWORD_U32,
    TOKEN_KEYWORD_U64,
    TOKEN_KEYWORD_INCLUDE,
    TOKEN_KEYWORD_MAIN,
  
    TOKEN_EOF,
    TOKEN_INVALID,
};

// Lines   will start at 1.
// Columns will start at 0.
struct Token_Position { 
    u64 line_start;
    u64 line_end;
    u64 column_start;
    u64 column_end;
};

struct Token { 
    Token_Type type;

    Token_Position position;

    // Name of the identifier
    // Kept as char so it's nul terminated when doing keyword lookups.
    char *ident_name;
    
    // The length of identifier name
    // So we don't have to do a bunch of strlens
    u16 ident_count;

    // Holds the value of the type.
    union { 
        char character_value;
        u64  integer_value;  // Unsigned because the parser will handle negativity.
        f32  f32_value; 
        f64  f64_value; 
        struct { char *data; u64 count; } string_value;
    };
};

struct Stream { 
    // Where we are currently in the stream.
    u64 cursor;
    // The size in bytes of the stream.
    u64 count;
    // The actual contents of the stream.
    char *data;
};

struct Lexer { 
    // Holds the actual content of the source files
    Stream stream;
    
    u64 current_line_number;
    u64 current_column_number;
    
    bool owns_input_memory;
    
    // Interned Keyword to length
    Hash_Table<u32, Token_Type> keywords;
};


// Exported functions will be ones which start with lexer_###
void lexer_init(Lexer *lexer);
void lexer_deinit(Lexer *lexer);
void lexer_set_input_from_file(Lexer *lexer, char *file_name);
void lexer_set_input_from_memory(Lexer *lexer, char *_data);
char lexer_peek_next_character(Lexer *lexer);
Token *lexer_peek_next_token(Lexer *lexer);
Token *lexer_get_token(Lexer *lexer);
