#include "Lexer.h"
#include "Hash.h"
#include "Common.h"

#include <stdio.h>
#include <stdarg.h>

// @Note: Look into https://c9x.me/compile/ for backend stuff.

char lexer_peek_next_character(Lexer *lexer);

// @Sync: lexer_keywords and lexer_token_table must be maintained in tandem.
// Modifications in one must be made to the other.
static const char *lexer_keywords[] = {
    "const", "if", "else", "switch", "case", "default", "break", 
    "return", "while", "for", "goto", "continue", "struct", "NULL", 
    "enum", "union", "true", "false", "float", "int", "char", "void", "static", 
    "do", "f32", "f64", "s8", "s16", "s32", "s64", "u8", "u16", "u32", "u64",
    "include", "main", NULL,
};

static const Token_Type lexer_token_table[] { 
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
};

void ASSERT(bool expr) { 
    if (!expr) { 
        exit(1);
    }
}

// Pulled out to replace new with an allocator of some sort
// Maybe Token Pool?
Token *NEW_TOKEN(Token_Type token_type=Token_Type::TOKEN_INVALID) { 
    Token *token = new Token;
    token->type = token_type;
    return token;
}

bool is_valid_keyword(char *string) { return true; }

void lexer_report_error(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt); 
  printf("\033[1;31m");
  vprintf(fmt, args); 
  printf("\033[0m");
  va_end(args); 
  exit(1);
}

inline bool is_space(char c) {
    if (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v') { return true; }
    return false; 
}

void eat_character(Lexer *lexer) { 
    ASSERT(lexer && lexer->stream.cursor < lexer->stream.count);
    if (lexer->stream.data[lexer->stream.cursor] == '\n') {
        ++lexer->current_line_number;
        lexer->current_column_number = 1;
    } else { 
        ++lexer->current_column_number;
    }
    
    ++lexer->stream.cursor;
}

s32 get_hex_digit(Lexer *lexer) { 
    ASSERT(lexer);
    char c = lexer_peek_next_character(lexer);
    if      ((c >= 'a') && (c <= 'f')) { eat_character(lexer); return 10 + c - 'a'; } 
    else if ((c >= 'A') && (c <= 'F')) { eat_character(lexer); return 10 + c - 'A'; } 
    else if ((c >= '0') && (c <= '9')) { eat_character(lexer); return      c - '0'; } 
    else { return -1; }
}

inline char char_to_lower(char c) {
    return ('a' - 'A') | c; 
}

inline bool is_digit(char c) {
    return  ('0' <= c) && (c <= '9'); 
}

inline bool is_alpha(char c) {
    if (c  >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') { return true; }
    return false;
}

inline bool is_alpha_numeric(char c) {
    return is_alpha(c) || is_digit(c);
}

inline bool is_hex_digit(char c) {
    return (('0' <= c) && (c <= '9')) || (('a' <= char_to_lower(c)) && (char_to_lower(c) <= 'f')); 
}

bool is_digit(Lexer *lexer) { 
    return is_digit(lexer->stream.data[lexer->stream.cursor]);
}

inline u32 str_len(const char *str) { 
    u32 size = 0;
    while (str && *str != '\0') { ++str; ++size; }
    return size;
}

void intern_keywords(Lexer *lexer) { 
    ASSERT(lexer);

    u16 index = 0;
    const char **pointer = lexer_keywords;
    while (*pointer != NULL) { 
        // Hash the string keyword for fast lookups.
        u32 hash = murmur_32((void *)*pointer, strlen(*pointer));
        // Add the hash of the keyword to the corresponding token type
        table_add(&lexer->keywords, hash, lexer_token_table[index]);
        ++index;
        ++pointer;
    }
}

void eat_whitespace(Lexer *lexer) {
    ASSERT(lexer && lexer->stream.data);
    while (is_space(lexer->stream.data[lexer->stream.cursor])) { 
        eat_character(lexer);
    }
}

void skip_line_comment(Lexer *lexer) { 
    ASSERT(lexer && lexer->stream.data);
    if (lexer->stream.data[lexer->stream.cursor] == '/' && lexer->stream.data[lexer->stream.cursor + 1] == '/') { 
        while (lexer->stream.data[lexer->stream.cursor] != '\n') {
            eat_character(lexer); 
        }

        ASSERT(lexer->stream.data[lexer->stream.cursor] == '\n');
        // Eat the new line character
        eat_character(lexer);

        // eat any whitespaces after.
        eat_whitespace(lexer);
    }
}

void skip_block_comment(Lexer *lexer) {
    ASSERT(lexer && lexer->stream.data);
    if (lexer->stream.data[lexer->stream.cursor] == '/' && lexer->stream.data[lexer->stream.cursor + 1] == '*') { 
        // eat '/'
        eat_character(lexer);
        // eat '*'
        eat_character(lexer);

        while (lexer->stream.data[lexer->stream.cursor] != '*' && lexer->stream.data[lexer->stream.cursor + 1] != '/') {
            eat_character(lexer);
        }
    
        ASSERT(lexer->stream.data[lexer->stream.cursor] == '*' && lexer->stream.data[lexer->stream.cursor + 1] == '/');
    
        // eat the '*'
        eat_character(lexer);
    
        // eat the '/'
        eat_character(lexer);

        // eat any whitespaces after.
        eat_whitespace(lexer);
    }
}

void update_fields_if_lexer_keyword(Lexer *lexer, Token *token) { 
    ASSERT(lexer);

    u32 hash = murmur_32((void *)token->ident_name, token->ident_count);
    
    auto *found = table_find_pointer(&lexer->keywords, hash);
    if (found) { 
        token->type = *found;
    }
}

void lexer_init(Lexer *lexer) {
    ASSERT(lexer);
    table_init(&lexer->keywords, 0);
    
    // Intern all the keywords for amortized constant access in the lexer with a hash table.
    intern_keywords(lexer);
    
    lexer->stream = {};
    lexer->current_line_number   = 1;
    lexer->current_column_number = 0;
    lexer->owns_input_memory     = false;
}

void lexer_deinit(Lexer *lexer) {
    table_deinit(&lexer->keywords);
    if (lexer->owns_input_memory && lexer->stream.data) { delete[] lexer->stream.data; }
    lexer->owns_input_memory = false;
}

void lexer_set_input_from_file(Lexer *lexer, char *file_name) {
    ASSERT(lexer);
 
    s64 length = read_file(file_name, (void **)&lexer->stream.data);
    ASSERT(length > 0);

    lexer->stream.count  = length; 
    lexer->stream.cursor = 0;

    lexer->current_line_number   = 1; 
    lexer->current_column_number = 0; 
    
    lexer->owns_input_memory = true;
}

void lexer_set_input_from_memory(Lexer *lexer, char *_data) { 
    ASSERT(lexer);
    lexer->stream.data = _data;
    lexer->current_line_number   = 1;
    lexer->current_column_number = 0;
}

char lexer_peek_next_character(Lexer *lexer) { 
    ASSERT(lexer && lexer->stream.data);
    if (lexer->stream.cursor >= lexer->stream.count) { return NULL; }
    return lexer->stream.data[lexer->stream.cursor+1];
}

Token *lexer_peek_next_token(Lexer *lexer) { 
    u64 saved_stream_cursor = lexer->stream.cursor;
    u64 saved_line_number   = lexer->current_line_number;
    u64 saved_column_number = lexer->current_column_number;
    
    Token *token = lexer_get_token(lexer);
        
    lexer->stream.cursor         = saved_stream_cursor;
    lexer->current_line_number   = saved_line_number;
    lexer->current_column_number = saved_column_number;
    
    return token;
}

Token *scan_string_literal(Lexer *lexer) { 
    ASSERT(lexer && lexer->stream.cursor < lexer->stream.count);
    if (lexer->stream.data[lexer->stream.cursor] != '\"') { return NULL; }

    Token *token = NEW_TOKEN(Token_Type::TOKEN_STRING);
    token->position.line_start   = lexer->current_line_number;
    token->position.column_start = lexer->current_column_number;

    token->string_value.data  = NULL;
    token->string_value.count = 0;

    // eat the opening string quote
    eat_character(lexer);
    
    // If we have an empty string literal then fill out the token
    // and return out
    if (lexer->stream.data[lexer->stream.cursor] == '\"') { 
        token->position.line_end   = lexer->current_line_number;
        token->position.column_end = lexer->current_column_number;
        return token;
    }

    // We need to allocate memory to hold the string in case we free the
    // contents of this source file to parse another.
    // @Todo: Allocate this on the heap we our pointers aren't invalidated.
    token->string_value.data  = &lexer->stream.data[lexer->stream.cursor];
    token->string_value.count = 0;

    while (lexer->stream.data[lexer->stream.cursor] != '\"') { 
        if (lexer->stream.data[lexer->stream.cursor] == '\0') { 
            lexer_report_error("%s\n", "Failed to find closing \" for string");
        }
        
        // Bump the string count
        ++token->string_value.count;

        // eat the string character
        eat_character(lexer);
    }
    
    ASSERT(lexer->stream.data[lexer->stream.cursor] == '\"');

    // +1 for nul termination
    char *string_name = new char[token->string_value.count + 1];
    strncpy(string_name, token->string_value.data, token->string_value.count);
    
    token->string_value.data = string_name;
    token->string_value.data[token->string_value.count] = '\0';
    
    // eat the closing string quote
    eat_character(lexer);

    return token;
}

Token *scan_character_literal(Lexer *lexer) { 
    ASSERT(lexer && lexer->stream.cursor < lexer->stream.count);
    if (lexer->stream.data[lexer->stream.cursor] != '\'') { return NULL; }
    
    Token *token = NEW_TOKEN(Token_Type::TOKEN_CHAR);
    token->position.line_start   = lexer->current_line_number;
    token->position.column_start = lexer->current_column_number;

    //eat the opening character quote
    eat_character(lexer);
    
    if (lexer->stream.data[lexer->stream.cursor] == '\0') { 
        lexer_report_error("%s\n", "Character quote missing");
    } 
    
    if (lexer->stream.data[lexer->stream.cursor] == '\'') { 
        lexer_report_error("%s\n", "Character cannot be empty");
    }
    
    if (lexer->stream.data[lexer->stream.cursor] == '\\') { 
        eat_character(lexer);
        if (lexer->stream.data[lexer->stream.cursor] == 'n') { 
            lexer_report_error("%s\n", "Character cannot contain a new line");
        }
    }

    
    token->character_value = lexer->stream.data[lexer->stream.cursor];;

    // eat the character
    eat_character(lexer);
    
    token->position.line_end   = lexer->current_line_number;
    token->position.column_end = lexer->current_column_number;

    ASSERT(lexer->stream.data[lexer->stream.cursor] == '\'');

    // eat the closing character quote
    eat_character(lexer);

    return token;
}

Token *scan_identifier(Lexer *lexer) { 
    ASSERT(lexer && lexer->stream.data);
    ASSERT(is_alpha_numeric(lexer->stream.data[lexer->stream.cursor]) ||
           lexer->stream.data[lexer->stream.cursor] == '_');

    Token *token = NEW_TOKEN(Token_Type::TOKEN_IDENT);
    token->position.line_start   = lexer->current_line_number;
    token->position.column_start = lexer->current_column_number;

    token->ident_name = &lexer->stream.data[lexer->stream.cursor];
    token->ident_count = 0;

    while (is_alpha_numeric(lexer->stream.data[lexer->stream.cursor]) ||
           lexer->stream.data[lexer->stream.cursor] == '_') { 
        // Bump the ident count
        ++token->ident_count;

        // eat the character
        eat_character(lexer);
    }
    
    char *ident_name = new char[token->ident_count + 1];
    strncpy(ident_name, token->ident_name, token->ident_count);
           
    token->ident_name = ident_name;
    token->ident_name[token->ident_count] = '\0';
    
    token->position.line_end   = lexer->current_line_number;
    token->position.column_end = lexer->current_column_number;

    // Now we need to check our intered keywords to see if this 
    // is actually an identifier or a keyword
    // If it is a keyword then we need to update the Token_Type;
    update_fields_if_lexer_keyword(lexer, token);

    return token;
}

Token *scan_numeric_literal(Lexer *lexer) { 
    ASSERT(lexer && lexer->stream.data);
    bool digit = is_digit(lexer) || (lexer->stream.data[lexer->stream.cursor] == '.');
    if (!digit) { 
        ASSERT(false);
        return NULL; 
    }

    Token *token = NEW_TOKEN(Token_Type::TOKEN_INT);
    token->position.line_start   = lexer->current_line_number;
    token->position.column_start = lexer->current_column_number;

    // Scan the number as if it's just an integer.
    u64 whole_number = 0;
    while (lexer->stream.data[lexer->stream.cursor] >= '0' && 
           lexer->stream.data[lexer->stream.cursor] <= '9') {
        whole_number = whole_number * 10 + lexer->stream.data[lexer->stream.cursor] - '0';
        eat_character(lexer);
    }
    
    // If we encounter a '.' then this must be a float
    // Scan the decimal value.
    if (lexer->stream.data[lexer->stream.cursor] == '.') { 
        token->type = Token_Type::TOKEN_FLOAT;
        // eat the '.'
        eat_character(lexer);
        
        f64 yyy = 0.1;
        f64 fraction = 0.0;
        while (lexer->stream.data[lexer->stream.cursor] >= '0' && 
               lexer->stream.data[lexer->stream.cursor] <= '9') {
            fraction += (lexer->stream.data[lexer->stream.cursor] - '0') * yyy;
            eat_character(lexer);
            yyy *= 0.1;
        }
        
        // Put together the whole number plus the fraction part.
        token->f64_value = (f64)whole_number + fraction;
    } else { 
        token->integer_value = whole_number;
    }

    token->position.line_end   = lexer->current_line_number;
    token->position.column_end = lexer->current_column_number;

    return token;
}

Token *lexer_get_token(Lexer *lexer) {
    ASSERT(lexer && lexer->stream.data);

    // Skip all whitespaces.
    eat_whitespace(lexer);
    
    // Skip all lines comments.
    skip_line_comment(lexer);

    // Skip all block comments.
    skip_block_comment(lexer);
    
    switch (lexer->stream.data[lexer->stream.cursor]) { 
        case '\0': { 
            Token *token = NEW_TOKEN(Token_Type::TOKEN_EOF);
            return token;
        }
        case '0': case '1': case '2': case '3': case '4': 
        case '5': case '6': case '7': case '8': case '9': case '.': { 
            return scan_numeric_literal(lexer);
        }
        case '"': { 
            return scan_string_literal(lexer);
        }
        case '\'': { 
            return scan_character_literal(lexer);
        }
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': 
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': 
        case 'V': case 'W': case 'X': case 'Y': case 'Z': case 'a': case 'b': 
        case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': 
        case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': 
        case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': 
        case 'x': case 'y': case 'z': case '_': {
            return scan_identifier(lexer);
        }
        default: {
            Token *token = NEW_TOKEN((Token_Type)lexer->stream.data[lexer->stream.cursor]);
            token->position.line_start   = lexer->current_line_number;
            token->position.column_start = lexer->current_column_number;

            eat_character(lexer);

            token->position.line_end   = lexer->current_line_number;
            token->position.column_end = lexer->current_column_number;
            return token;
        }
    }
}
