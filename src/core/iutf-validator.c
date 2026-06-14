/* iutf-validator.c
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
 * IUTF Validator version 0.6
 */

#include "../includes/iutf-lexer.h"
#include "../includes/iutf-ast.h"
#include <stdio.h>

static int veln = 0; // validator error line
static int vecol = 0; // validator error column
static const char* vemsg = NULL; // validator error message

int ValidGetErrLn (void) { return veln; }
int ValidGetErrCol (void) { return vecol; }
const char* ValidGetErrMessage (void) { return vemsg; }

int iutf_validate(IutfNode* root, IutfLexer* lex)
{

  veln = 0;
  vecol = 0;
  vemsg = NULL;

    if (!root) {
        vemsg = "root node is NULL!!";
        return 0;
    }

    if (root->type != IUTF_NODE_BRANCH) {
      veln = lex->line;
      vecol = lex->col;
      vemsg = "root must be a branch.";
      return 0;
    }

    // Validation: Check that the root contains the required keys
    int has_title = 0;
    int has_version = 0;

    for (size_t i = 0; i < root->data.branch.size; i++) {
        IutfNode* item = root->data.branch.items[i];
        if (item->key && strcmp(item->key, "title") == 0) {
            if (item->type != IUTF_NODE_STRING) {
              veln = lex->line;
              vecol = lex->col;
              vemsg = "field 'title' must be a string!";
              return 0;
            }
            has_title = 1;
        } else if (item->key && strcmp(item->key, "version") == 0) {
            if (item->type != IUTF_NODE_FLOAT && item->type != IUTF_NODE_INTEGER) {
                veln = lex->line;
                vecol = lex->col;
                vemsg = "field 'version' must be a string!";
                return 0;
            }
            has_version = 1;
        }
    }

    if (!has_title || !has_version) {
        veln = lex->line;
        vecol = lex->col;
        vemsg = !has_title && !has_version ? "missing required fields: 'title' and 'version'."
                      : !has_title
                                ? "missing required field: 'title'."
                                : "missing required field: 'version'.";
        return 0;
    }

    return 1; // Validation complete
}
