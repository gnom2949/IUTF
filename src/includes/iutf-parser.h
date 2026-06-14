/* iutf-parser.h
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
 * IUTF Parser Header version v0.6
 */

#ifndef _IUTF_PARSER_H_
#define _IUTF_PARSER_H_

#include "iutf-lexer.h"
#include "iutf-ast.h"
#include "colors.h"
#include <stdbool.h>

typedef struct {
    IutfLexer* lexer;
    IutfToken current;
    IutfVarTable *global_vars;
    IutfVarTable *local_vars;
    IutfVarTable *shared_vars;
    bool is_utext;
    int Eline;
    int Ecol;
    ustring Emessage;
    int line;
    int col;
    ustring message;
} IutfParser;

IutfParser* iutf_parser_new (ustring input);
void iutf_parser_free (IutfParser* parser);
IutfNode* iutf_parse (IutfParser* parser);
IutfNode *iutf_parse_utext (IutfParser *parser);
IutfNode* iutf_parse_from_file (ustring filename);
int ParserGetErrLn (IutfParser* IPS);
int ParserGetErrCol (IutfParser* IPS);
ustring ParserGetErrMessage (IutfParser* IPS);
#endif /* _IUTF_PARSER_H_ */
