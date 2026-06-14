/* iutf-lexer.h
 *
 * Copyright 2026 Int Software, Aleksandr Silaev
 *
 * License under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 * IUTF lexer version - 0.5
 */

#include "../includes/iutf-lexer.h"
#include "imm/IntMemoryManager.h"
#include <string.h>
#include <ctype.h>

Only IutfToken read_directive (IutfLexer *lexer, size_t start);

IutfToken ErrToken (IutfLexer* lex, const char* message)
{
    print_error_at (lex->input, lex->line, lex->col, 1, message);
    IutfToken tok;
    tok.type = IUTF_TOK_ERROR;
    tok.start = NULL;
    tok.length = 0;
    tok.line = lex->line;
    tok.col = lex->col;
    return tok;
}

int LexerGetErrLn (IutfLexer* lex)
{
  return lex->line;
}

int LexerGetErrCol (IutfLexer* lex)
{
  return lex->col;
}

ustring LexerGetErrMessage (IutfLexer* lex)
{
  return lex->message;
}

Only inline int is_ident_start (char c)
{
  return isalpha((unsigned char) c) || c == '_';
}

Only inline int is_ident_continue (char c)
{
  return isalnum(c) || c == '_' || c == '-';
}

Only inline void advance (IutfLexer* lexer) {
  if (lexer->pos < lexer->len) {
    if (lexer->input[lexer->pos] == '\n') {
      lexer->line++;
      lexer->col = 1;
    } else {
        lexer->col++;
    }
    lexer->pos++;
  }
}

Only inline char peek (IutfLexer* lexer, int offset)
{
  size_t index = lexer->pos + offset;
  if (index >= lexer->len) return '\0';
  return lexer->input[index];
}

Only inline char current (IutfLexer* lexer)
{
  return peek(lexer, 0);
}

Only inline int get_ln_start (const char* input, int Lnn/*Line number*/)
{
  if (!input || Lnn < 1) return -1;
  int curL = 1;
  const char* p = input;
  while (*p && curL < Lnn) {
    if (*p == '\0') break;
    if (*p == '\n') curL++;
    p++;
  }
  return p - input;
}

Only inline int get_ln_len (const char* input, int start_pos)
{
  if (!input || start_pos < 0) return -1;

  const char* p = input + start_pos;
  int len = 0;
  while (*p && *p != '\n') {
    p++;
    len++;
  }
  return len;
}

void print_error_at (ustring input, int ln, int col, int span, ustring msg)
{
  if (!input || !msg) {
    fprintf (stderr, COL_RED "Error: %s\n" COL_DEF, msg ? msg : "Unknown error");
    return;
  }

  fprintf (stderr, "error at line: \033[36m%d\033[0m, col: \033[36m%d\033[0m\n\033[31m%s\n\033[0m\n", ln, col, msg ? msg : "NULL ID!");

  int start_pos = get_ln_start (input, ln);
  int len = get_ln_len (input, start_pos);
  if (len <= 0 || start_pos < 0) {
    fprintf (stderr, COL_YLW "Cannot display line!\n" COL_DEF);
    return;
  }

  ustring ln_ptr = input + start_pos;
  fwrite (ln_ptr, 1, len, stderr);
  fprintf (stderr, "\n");

  if (span <=  0) { span = 1; }
  if (col + span - 1 > len) { span = len - col + 1; }

  // 1. print space to needed column (without color)
  for (int i = 1; i < col; i++) {
    fputc (' ', stderr);
  }

  // 2. print the red arrows and message (with red color)
  fputs (COL_RED, stderr);
  for (int i = 0; i < span; i++) {
    fputc ('^', stderr);
  }
  fputs ("\n" COL_DEF, stderr);
}

Only IutfToken make_token (IutfLexer* lexer, IutfTokenType type, size_t start)
{
  IutfToken token;
  token.type = type;
  token.start = lexer->input + start;
  token.length = lexer->pos - start;
  token.line = lexer->line;
  token.col = lexer->col - token.length;
  return token;
}

static IutfToken read_number (IutfLexer* lexer, size_t start)
{
  while (isdigit(current(lexer)) || current(lexer) == '.') {
    advance(lexer);
  }
  return make_token(lexer, IUTF_TOK_INTEGER, start);
}

static IutfToken read_string (IutfLexer* lexer)
{
  size_t start = lexer->pos;
  advance(lexer);

  while (current(lexer) != '"' && current (lexer) != '\0') {
    if (current (lexer) == '\\') {
      advance (lexer);
        if (current (lexer) != '\0') {
          advance (lexer);
        } else {
            return ErrToken (lexer, "Unterminated escape sequence");
        }
    } else {
      advance (lexer);
    }
  }

  if (current (lexer) != '"') {
    return ErrToken (lexer, "Unterminated string");
  }
  advance (lexer);
  return make_token(lexer, IUTF_TOK_STRING, start);
}

static IutfToken read_identifier (IutfLexer* lexer, size_t start)
{
  while (is_ident_continue (current(lexer))) {
      advance (lexer);
  }

  size_t len = lexer->pos - start;
  const char* str = lexer->input + start;

  if (len == 4 && strncmp (str, "true", 4) == 0) return make_token (lexer, IUTF_TOK_TRUE, start);
  if (len == 5 && strncmp (str, "false", 5) == 0) return make_token (lexer, IUTF_TOK_FALSE, start);
  if (len == 4 && strncmp (str, "null", 4) == 0) return make_token (lexer, IUTF_TOK_NULL, start);
  if (len == 3 && strncmp (str, "var", 3) == 0) return make_token (lexer, IUTF_TOK_VAR, start);
  if (len == 7 && strncmp (str, "arrayof", 7) == 0) return make_token (lexer, IUTF_TOK_ARRAYOF, start);
  if (len == 5 && strncmp (str, "utext", 5) == 0) return make_token (lexer,  IUTF_TOK_UTEXT, start);

  return make_token (lexer, IUTF_TOK_IDENTIFIER, start);
}

static IutfToken read_bigstring (IutfLexer* lexer)
{
  size_t start = lexer->pos - 9;
  int depth = 1;
  while (depth > 0 && current (lexer) != '\0') {
    if (current (lexer) == '[') depth++;
    else if (current (lexer) == ']') depth--;
    advance (lexer);
  }
  if (depth != 0) {
    return ErrToken (lexer, "Unterminated BigString");
  }
  return make_token (lexer, IUTF_TOK_BIGSTRING_START, start);
}

static void skip_line_comment (IutfLexer* lexer)
{
  while (current (lexer) != '\n' && current (lexer) != '\0') {
    advance (lexer);
  }
}

static void skip_block_comment (IutfLexer* lexer)
{
  advance (lexer);
  while (current (lexer) != '\0') {
    if (current (lexer) == '*' && peek(lexer, 1) == '/') {
      advance (lexer);
      advance (lexer);
      return;
    }
    advance (lexer);
  }
  ErrToken (lexer, "Unterminated block comment");
}

IutfLexer* iutf_lexer_new (const char* input)
{
  IutfLexer* lexer = MemoryAllocate(sizeof(IutfLexer));
  if (!lexer) return NULL;

  lexer->input = input;
  lexer->len = strlen(input);
  lexer->pos = 0;
  lexer->line = 1;
  lexer->col = 1;

  return lexer;
}

void iutf_lexer_corrupt (IutfLexer* lexer)
{
  if (lexer) {
    cleanbit (lexer);
  }
}

IutfToken iutf_lexer_next (IutfLexer* lexer) {
  while (current(lexer) != '\0') {
    size_t start = lexer->pos;

    char c = current (lexer);
    advance (lexer);

    switch (c)
    {
    case '{': return make_token (lexer, IUTF_TOK_BRANCH_OPEN, start);
    case '}': return make_token (lexer, IUTF_TOK_BRANCH_CLOSE, start);
    case '[': {
      if (lexer->pos >= 9 && strncmp(lexer->input + lexer->pos - 9, "BigString", 9) == 0) {
          return read_bigstring (lexer);
        }
        return make_token (lexer, IUTF_TOK_LBRACKET, start);
    }
    case ']': return make_token (lexer, IUTF_TOK_RBRACKET, start);
    case ':': 
      if (current (lexer) == ':')
      {
        advance(lexer);
        return make_token (lexer, IUTF_TOK_DOUBLE_COLON, start);
      }
      return make_token (lexer, IUTF_TOK_COLON, start);

    case '=': return make_token (lexer, IUTF_TOK_EQUALS, start);
    case '|': return make_token (lexer, IUTF_TOK_PIPE, start);
    case ',': return make_token (lexer, IUTF_TOK_COMMA, start);
    case '@': return read_directive (lexer, start);
    case '#':
      if (current (lexer) == '!') {
        advance (lexer);
        skip_line_comment (lexer);
        continue;
      } else {
        return make_token (lexer, IUTF_TOK_IDENTIFIER, start);
      }
    case '/':
      if (current (lexer) == '/') {
        advance (lexer);
        skip_line_comment (lexer);
        continue;
      } else if (current (lexer)) {
        advance (lexer);
        skip_block_comment (lexer);
        continue;
      } else {
        return make_token (lexer, IUTF_TOK_IDENTIFIER, start);
      }
    case '"': return read_string (lexer);
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        continue;
    case '.': return make_token(lexer, IUTF_TOK_DOT, start);
    default:
      if (isdigit(c)) {
        return read_number (lexer, start);
      } else if (is_ident_start(c)) {
        return read_identifier (lexer, start);
      } else {
        return ErrToken (lexer, "Unexpected character");
      }
    }
  }

  IutfToken token;
  token.type = IUTF_TOK_EOF;
  token.start = lexer->input + lexer->pos;
  token.length = 0;
  token.line = lexer->line;
  token.col = lexer->col;
  return token;
}

static IutfToken read_directive (IutfLexer *lexer, size_t start)
{
  if (!is_ident_start (current(lexer))) return ErrToken (lexer, "Expected identifier after '@'");

  while (is_ident_continue (current (lexer))) advance (lexer);

  return make_token (lexer, IUTF_TOK_DIRECTIVE, start);
}

const char* iutf_token_type_to_string (IutfTokenType type)
{
  switch (type)
  {
  case IUTF_TOK_EOF: return "EOF";
  case IUTF_TOK_ERROR: return "ERROR";
  case IUTF_TOK_BRANCH_OPEN: return "{";
  case IUTF_TOK_BRANCH_CLOSE: return "}";
  case IUTF_TOK_LBRACKET: return "[";
  case IUTF_TOK_RBRACKET: return "]";
  case IUTF_TOK_COLON: return ":";
  case IUTF_TOK_EQUALS: return "=";
  case IUTF_TOK_PIPE: return "|";
  case IUTF_TOK_COMMA: return ",";
  case IUTF_TOK_STRING: return "STRING";
  case IUTF_TOK_INTEGER: return "NUMBER";
  case IUTF_TOK_TRUE: return "TRUE";
  case IUTF_TOK_FALSE: return "FALSE";
  case IUTF_TOK_NULL: return "NULL";
  case IUTF_TOK_FLOAT: return "FLOAT";
  case IUTF_TOK_CHARACTER: return "CHARACTER";
  case IUTF_TOK_LONG: return "LONG";
  case IUTF_TOK_IDENTIFIER: return "IDENTIFIER";
  case IUTF_TOK_BIGSTRING_START: return "BIGSTRING";
  case IUTF_TOK_COMMENT_LINE: return "COMMENT_LINE";
  case IUTF_TOK_COMMENT_CPP: return "COMMENT_CPP";
  case IUTF_TOK_COMMENT_BLOCK_START: return "COMMENT_BLOCK_START";
  case IUTF_TOK_COMMENT_BLOCK_END: return "COMMENT_BLOCK_END";
  case IUTF_TOK_DOT: return ".";
  case IUTF_TOK_DOUBLE_COLON: return "::";
  case IUTF_TOK_VAR: return "var";
  case IUTF_TOK_ARRAYOF: return "arrayof";
  case IUTF_TOK_UTEXT: return "utext";
  case IUTF_TOK_DIRECTIVE: return "@";
  default: return "UNKNOWN";
  }
}

