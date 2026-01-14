/* example.c
 *
 * Copyright 2026 Int Software, Aleksandr Silaev
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../src/includes/iutf-lexer.h"
#include <stdio.h>

int main (int argc, char *argv[])
{
  const char* input = R"(
iutf:init:main {
  title: "IEEE 4"
  version: 1.2
  version_long: 123L
  rnd: 24.441
  char: 'x'
  #! this is a comment!!!
  authors: ["jack", "bob"]
  meta {
      created[tms]
      public: true
      tags: [dev, git, debug]
  }
  license: BigString[
    Copyright(C) 2026 example
    All rights reserved.
  ]
  readme: |
    Hello!
    This is a example!
  |
}
  )";

  IutfLexer* lexer = iutf_lexer_new (input);

  IutfToken token;
  do {
    token = iutf_lexer_next (lexer);
    printf ("[%d:%d] %s: '", token.line, token.col, iutf_token_type_to_string (token.type));
    if (token.start) {
      for (size_t i = 0; i < token.length; i++) {
        putchar (token.start[i]);
      }
    }
    printf ("'\n");
  } while (token.type != IUTF_TOK_EOF);

  iutf_lexer_free(lexer);
  return EXIT_SUCCESS;
}
