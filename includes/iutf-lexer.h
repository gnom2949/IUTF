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
 */

#ifndef IUTF_LEXER_H
#define IUTF_LEXER_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum {
  IUTF_TOK_EOF,
  IUTF_TOK_ERROR,
  IUTF_TOK_BRANCH_OPEN, // {
  IUTF_TOK_BRANCH_CLOSE, // }
  IUTF_TOK_LBRACKET, // [
  IUTF_TOK_RBRACKET, // ]
  IUTF_TOK_COLON, // :
  IUTF_TOK_EQUALS, // =
  IUTF_TOK_PIPE, // |
  IUTF_TOK_COMMA, // ,
  IUTF_TOK_STRING,
  IUTF_TOK_INTEGER, // 123
  IUTF_TOK_CHARACTER, //'c'
  IUTF_TOK_FLOAT, // 12.34
  IUTF_TOK_LONG, //21L
  IUTF_TOK_TRUE,
  IUTF_TOK_FALSE,
  IUTF_TOK_NULL,
  IUTF_TOK_IDENTIFIER,
  IUTF_TOK_BIGSTRING_START, // BigString[
  IUTF_TOK_COMMENT_LINE, // #!
  IUTF_TOK_COMMENT_CPP, // //
  IUTF_TOK_COMMENT_BLOCK_START, // /*
  IUTF_TOK_COMMENT_BLOCK_END, // */
} IutfTokenType;

typedef struct {
  IutfTokenType type;
  const char* start;
  size_t length;
  int line;
  int col;
} IutfToken;

typedef struct {
  const char* input;
  size_t pos;
  size_t len;
  int line;
  int col;
} IutfLexer;

IutfLexer* iutf_lexer_new (const char* input);
void iutf_lexer_free (IutfLexer* lexer);
IutfToken iutf_lexer_next (IutfLexer* lexer);

const char* iutf_token_type_to_string (IutfTokenType type);

#endif /* IUTF_LEXER_H */
