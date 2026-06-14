/* iutf-parser.c
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
 * IUTF Parser version 0.6.1
 */

//#define _GNU_SOURCE

#include "iutf.h"
#include <assert.h>
static char err_buf[512];

static void advance(IutfParser* parser)
{
    parser->current = iutf_lexer_next(parser->lexer);
}

static IutfNode* parse_value(IutfParser* parser);
static IutfNode* parse_branch(IutfParser* parser);

IutfNode* iutf_parse_from_file (ustring filename)
{
  FILE* fp = fopen (filename, "r");
  if (!fp) {
    fprintf (stderr, COL_RED "Cannot open file: " COL_DEF COL_CYAN "%s" COL_DEF "\n", filename);
    return NULL;
  }

  fseek (fp, 0, SEEK_END);
  long len = ftell (fp);
  fseek (fp, 0, SEEK_SET);

  char* buffer = MemoryAllocate (len + 1);
  if (!buffer) {
    fclose (fp);
    return NULL;
  }

  fread (buffer, 1, len, fp);
  buffer[len] = '\0';
  fclose (fp);

  IutfParser* parser = iutf_parser_new (buffer);
  if (!parser) {
    cleanbit (buffer);
    return NULL;
  }

  IutfNode* result = iutf_parse (parser);

  iutf_parser_free (parser);
  cleanbit (buffer);

  return result;
}

static char* sstrndup(const char* s, size_t n) { // я ебал блять этот ебучий сегфолт
    if (!s) return NULL;
    char* dup = MemoryAllocate (n + 1);
    if (!dup) return NULL;
    memcpy (dup, s, n);
    dup[n] = '\0';
    return dup;
}

static IutfNode* parse_string(IutfParser* parser)
{
    IutfNode* node = iutf_node_new(IUTF_NODE_STRING);
    if (!node) return NULL;

    node->data.str_value = sstrndup(parser->current.start, parser->current.length);
    if (!node->data.str_value) {
      fprintf(stderr, COL_RED "Failed to allocate string!!" COL_DEF "\n");
      iutf_node_free (node);
      return NULL;
    }
    advance(parser);
    return node;
}

static IutfNode* parse_number(IutfParser* parser) {
    IutfNode* node = NULL;
    if (parser->current.type == IUTF_TOK_INTEGER) {
        node = iutf_node_new(IUTF_NODE_INTEGER);
        node->data.int_value = atoll(parser->current.start);
        advance(parser);
    } else if (parser->current.type == IUTF_TOK_FLOAT) {
        node = iutf_node_new(IUTF_NODE_FLOAT);
        node->data.float_value = atof(parser->current.start);
        advance(parser);
    } else if (parser->current.type == IUTF_TOK_LONG) {
        node = iutf_node_new(IUTF_NODE_LONG);
        node->data.long_value = atoll(parser->current.start);
        advance(parser);
    }
    return node;
}

static IutfNode* parse_character(IutfParser* parser)
{
    IutfNode* node = iutf_node_new(IUTF_NODE_CHARACTER);
    if (!node) return NULL;

    // parser->current.start points to the first quote
    // Expected format: 'x' or '\x' where x is an escaped character

    if (parser->current.length < 3) {
      fprintf(stderr, COL_RED "Invalid character literal" COL_DEF "\n");
      iutf_node_free (node);
      return NULL;
    }

    if (parser->current.start[1] == '\\') {
        char esc = parser->current.start[2];
        switch (esc)
        {
          case 'n': node->data.char_value = '\n'; break;
          case 't': node->data.char_value = '\t'; break;
          case 'r': node->data.char_value = '\r'; break;
          case '\\': node->data.char_value = '\\'; break;
          case '\'': node->data.char_value = '\''; break;
          case '\"': node->data.char_value = '\"'; break;
        default:
            node->data.char_value = esc; // treat as literal
            break;
        }
    } else {
      node->data.char_value = parser->current.start[1];
    }
    advance (parser);
    return node;
}

static IutfNode* parse_boolean(IutfParser* parser) {
    IutfNode* node = iutf_node_new(IUTF_NODE_BOOLEAN);
    node->data.bool_value = (parser->current.type == IUTF_TOK_TRUE);
    advance(parser);
    return node;
}

static IutfNode* parse_array(IutfParser* parser) {
    IutfNode* node = iutf_node_new(IUTF_NODE_ARRAY);
    if (!node) return NULL;

    advance(parser); // skip '['

    while (parser->current.type != IUTF_TOK_RBRACKET && parser->current.type != IUTF_TOK_EOF) {
        IutfNode* item = parse_value(parser);
        if (!item) break;

        struct IutfNode** temp = MemoryReAllocate (node->data.array.items, (node->data.array.size + 1) * sizeof(struct IutfNode*));
        if (!temp) {
            fprintf(stderr, COL_RED "Out of memory" COL_DEF "\n");
            iutf_node_free(item);
            iutf_node_free(node);
            return NULL;
        }
        node->data.array.items = temp;
        node->data.array.items[node->data.array.size] = item;
        node->data.array.size++;

        if (parser->current.type == IUTF_TOK_COMMA) {
            advance(parser);
        }
    }

    if (parser->current.type != IUTF_TOK_RBRACKET) {
        fprintf(stderr, COL_RED "Expected ']', got " "%s" COL_DEF "\n", iutf_token_type_to_string(parser->current.type));
        iutf_node_free(node);
        return NULL;
    }
    advance(parser); // skip ']'
    return node;
}

static IutfNode* parse_bigstring(IutfParser* parser) {
    IutfNode* node = iutf_node_new(IUTF_NODE_BIGSTRING);
    if (!node) return NULL;

    // skip "BigString["
    size_t start = (parser->current.start - parser->lexer->input) + 9; // "BigString" length
    if (start >= parser->lexer->len) {
        fprintf(stderr, "Invalid BigString start\n");
        iutf_node_free(node);
        return NULL;
    }

    int depth = 1;
    size_t pos = start;
    while (depth > 0 && pos < parser->lexer->len) {
        if (parser->lexer->input[pos] == '[') depth++;
        else if (parser->lexer->input[pos] == ']') depth--;
        pos++;
    }

    if (depth != 0) {
        fprintf(stderr, "Unterminated BigString\n");
        iutf_node_free(node);
        return NULL;
    }

    size_t end = pos - 1;
    if (end < start) {
        fprintf(stderr, "Invalid BigString range\n");
        iutf_node_free(node);
        return NULL;
    }

    node->data.str_value = sstrndup(parser->lexer->input + start, end - start);
    if (!node->data.str_value) {
        fprintf(stderr, "Failed to allocate BigString\n");
        iutf_node_free(node);
        return NULL;
    }

    while (parser->current.type != IUTF_TOK_BIGSTRING_START && parser->current.type != IUTF_TOK_EOF) {
        advance(parser);
    }
    advance(parser); // skip ']'
    return node;
}
static IutfNode* parse_pipe_string(IutfParser* parser) {
    IutfNode* node = iutf_node_new(IUTF_NODE_PIPESTRING);
    if (!node) return NULL;

    advance(parser); // skip opening |

    size_t start = parser->lexer->pos;
    while (parser->current.type != IUTF_TOK_PIPE && parser->current.type != IUTF_TOK_EOF) {
        advance(parser);
    }

    if (parser->current.type != IUTF_TOK_PIPE) {
        fprintf(stderr, "Unterminated pipe string\n");
        iutf_node_free(node);
        return NULL;
    }

    node->data.str_value = sstrndup(parser->lexer->input + start, parser->lexer->pos - start);
    if (!node->data.str_value) {
        fprintf(stderr, "Failed to allocate pipe string\n");
        iutf_node_free(node);
        return NULL;
    }

    advance(parser); // skip closing |
    return node;
}

Only int parse_var (IutfParser *parser, int scope)
{
    advance(parser);
    if (parser->current.type != IUTF_TOK_IDENTIFIER)
    {
        snprintf(err_buf, sizeof(err_buf), "Expected variable name after 'var', got %s", iutf_token_type_to_string(parser->current.type));
        parser->Emessage = err_buf;
        parser->Eline = parser->current.line;
        parser->Ecol = parser->current.col;
        return 0; 
    }

    string name = sstrndup (parser->current.start, parser->current.length);
    if (!name) return 0;
    advance (parser);

    if (parser->current.type != IUTF_TOK_COLON)
    {
        snprintf (err_buf, sizeof (err_buf), "Expected ':' after var name, got %s", iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        parser->Eline = parser->current.line;
        parser->Ecol  = parser->current.col;
        cleanbit (name);
        return 0;
    }
    advance(parser);

    IutfNode *value = parse_value(parser);
    if (!value)
    {
        cleanbit(name);
        return 0;
    }

    IutfVarTable *tab = (scope == 0) ? parser->global_vars : parser->local_vars;
    if (!tab)
    {
        fprintf (stderr, "parser: var table is NULL (scope=%d)\n", scope);
        cleanbit(name);
        iutf_node_free(value);
        return 0;
    }

    if (!iutf_var_tab_set (tab, name, value))
    {
        fprintf(stderr, "parser: failed to store var '%s'\n", name);
        cleanbit(name);
        iutf_node_free(value);
        return 0;
    }

    cleanbit (name);
    return 1;
}

Only IutfNode *parse_ref (IutfParser *parser)
{
    ustring start = parser->current.start;
    size length = parser->current.length;

    if (length == 9 && strncmp (start, "@onlythis", 9) == 0)
    {
        advance (parser);

        IutfNode *inner = parse_value(parser);
        if (!inner) return null;

        IutfNode *node = iutf_node_new (IUTF_NODE_ONLYTHIS);
        if (!node) { iutf_node_free(inner); return null; }
        node->data.onlythis_value = inner;
        return node;
    }

    IutfRefScope scope;
    string name = null;
    
    if (!iutf_parse_ref_string(start, length, &scope, &name))
    {
        snprintf (err_buf, sizeof(err_buf), "Invalid reference: '%.*s'", (int)length, start);
        parser->Emessage = err_buf;
        parser->Eline = parser->current.line;
        parser->Ecol = parser->current.col;
        return null;
    }

    advance (parser);

    if (!name && parser->current.type == IUTF_TOK_DOT)
    {
        advance(parser);
        if (parser->current.type != IUTF_TOK_IDENTIFIER)
        {
            snprintf(err_buf, sizeof(err_buf), "Expected name after '.', got %s", iutf_token_type_to_string(parser->current.type));
            parser->Emessage = err_buf;
            parser->Eline = parser->current.line;
            parser->Ecol = parser->current.col;
            return null;
        }
        name = sstrndup(parser->current.start, parser->current.length);
        advance(parser);
    } else if (name) {

    } else
    {
        snprintf (err_buf, sizeof(err_buf), "Expected '.' after scope in reference");
        parser->Emessage = err_buf;
        parser->Eline = parser->current.line;
        parser->Ecol = parser->current.col;
        return null;
    }

    IutfNode *node = iutf_node_new(IUTF_NODE_REF);
    if (!node) { cleanbit(name); return null; }
    node->data.ref.scope = scope;
    node->data.ref.name = name;
    return node;
}

Only IutfNode* parse_value(IutfParser* parser) {
    switch (parser->current.type) {
        case IUTF_TOK_STRING:
            return parse_string(parser);
        case IUTF_TOK_INTEGER:
        case IUTF_TOK_FLOAT:
        case IUTF_TOK_LONG:
            return parse_number(parser);
        case IUTF_TOK_CHARACTER:
            return parse_character(parser);
        case IUTF_TOK_TRUE:
        case IUTF_TOK_FALSE:
            return parse_boolean(parser);
        case IUTF_TOK_NULL:
            advance(parser);
            return iutf_node_new(IUTF_NODE_NULL);
        case IUTF_TOK_LBRACKET:
            return parse_array(parser);
        case IUTF_TOK_BIGSTRING_START:
            return parse_bigstring(parser);
        case IUTF_TOK_PIPE:
            return parse_pipe_string(parser);
        case IUTF_TOK_BRANCH_OPEN:
            return parse_branch(parser);
        case IUTF_TOK_DIRECTIVE:
            return parse_ref(parser);
        case IUTF_TOK_ARRAYOF:
            advance(parser);
            if (parser->current.type != IUTF_TOK_COLON) 
            {
                fprintf (stderr, "Parser: Expected ':' at %d", parser->current.line);
                return null;
            }
            advance(parser);
            if (parser->current.type == IUTF_TOK_NULL) {
                advance(parser);
                return iutf_node_new(IUTF_NODE_NULL);
            }
            if (parser->current.type != IUTF_TOK_LBRACKET) { fprintf (stderr, "Parser: Expected '[' at %d", parser->current.line); return null; }
            return parse_array(parser);
        default:
            fprintf(stderr, "Unexpected token: %s\n", iutf_token_type_to_string(parser->current.type));
            return NULL;
    }
}

IutfNode *iutf_parse_utext(IutfParser *parser)
{
    if (parser->current.type != IUTF_TOK_UTEXT)
    {
        snprintf (err_buf, sizeof (err_buf), "Expected 'utext', got %s", iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        parser->Eline = parser->current.line;
        parser->Ecol  = parser->current.col;
        return null;
    }
    advance(parser);

    if (parser->current.type != IUTF_TOK_DOUBLE_COLON)
    {
        snprintf (err_buf, sizeof (err_buf), "Expected '::', got %s", iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        parser->Eline = parser->current.line;
        parser->Ecol  = parser->current.col;
        return null;
    }
    advance(parser);

    if (parser->current.type != IUTF_TOK_IDENTIFIER || strncmp (parser->current.start, "setup", 5) != 0)
    {
        snprintf (err_buf, sizeof (err_buf), "Expected 'setup' after '::', got %s",
                  iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        parser->Eline = parser->current.line;
        parser->Ecol  = parser->current.col;
        return null;
    }
    advance(parser);

    if (parser->current.type != IUTF_TOK_BRANCH_OPEN)
    {
        snprintf (err_buf, sizeof (err_buf), "Expected '{' after 'setup', got %s",
                  iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        parser->Eline = parser->current.line;
        parser->Ecol  = parser->current.col;
        return null;
    }
    advance(parser);

    if (parser->current.type != IUTF_TOK_IDENTIFIER)
    {
        snprintf (err_buf, sizeof (err_buf), "Expected extension name, got %s",
                  iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        parser->Eline = parser->current.line;
        parser->Ecol  = parser->current.col;
        return null;
    }
    
    string ext_name = sstrndup(parser->current.start, parser->current.length);
    if (!ext_name) return null;
    advance(parser);

    if (parser->current.type != IUTF_TOK_BRANCH_OPEN)
    {
        snprintf (err_buf, sizeof (err_buf), "Expected '{' after '%s', got %s", ext_name,
                  iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        parser->Eline = parser->current.line;
        parser->Ecol  = parser->current.col;
        cleanbit(ext_name);
        return null;
    }

    IutfNode *body = parse_branch(parser);
    if (!body) { cleanbit(ext_name); return null; }

    if (parser->current.type != IUTF_TOK_BRANCH_CLOSE)
    {
        snprintf (err_buf, sizeof (err_buf), "Expected '}' to close branch 'setup', got %s",
                  iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        parser->Eline = parser->current.line;
        parser->Ecol  = parser->current.col;
        cleanbit(ext_name);
        return null;
    }
    advance (parser);

    IutfNode *node = iutf_node_new(IUTF_NODE_UTEXT_SETUP);
    if (!node)
    {
        cleanbit(ext_name);
        iutf_node_free(body);
        return null;
    }

    node->data.utext.ext_name = ext_name;
    node->data.utext.items = body->data.branch.items;
    node->data.utext.size = body->data.branch.size;
    
    body->data.branch.items = null;
    body->data.branch.size = 0;
    iutf_node_free(body);

    return node;
}

Only IutfNode* parse_branch(IutfParser* parser) {
    IutfNode* node = iutf_node_new(IUTF_NODE_BRANCH);
    if (!node) return NULL;

    advance(parser); // skip '{'

    while (parser->current.type != IUTF_TOK_BRANCH_CLOSE && parser->current.type != IUTF_TOK_EOF) {

        // === waiting identifier (key) ===
        if (parser->current.type != IUTF_TOK_IDENTIFIER &&
            parser->current.type != IUTF_TOK_DIRECTIVE &&
            parser->current.type != IUTF_TOK_ARRAYOF) {
            parser->Eline = parser->current.line;
            parser->Ecol = parser->current.col;
            parser->Emessage = "Expected identifier, got %s";
            iutf_node_free(node);
            return NULL;
        }

        // var declaration
        if (parser->current.type == IUTF_TOK_VAR)
        {
            int scope = (parser->global_vars && parser->local_vars && parser->local_vars->count == 0) ? 0 : 1;
            if (!parse_var(parser, scope)) { iutf_node_free(node); return null; }
            continue;
        }

        // reading key
        char* key = sstrndup(parser->current.start, parser->current.length);
        if (!key) {
            fprintf(stderr, "Failed to allocate key\n");
            iutf_node_free(node);
            return NULL;
        }
        advance(parser);

        // === Processing @import ===
        if (strncmp(key, "@import", 7) == 0) {
            cleanbit(key);  // freeing key

            char* ext_name = NULL;

            // Support: @import name or @import "name"
            if (parser->current.type == IUTF_TOK_IDENTIFIER) {
                ext_name = sstrndup(parser->current.start, parser->current.length);
                advance(parser);
            }
            else if (parser->current.type == IUTF_TOK_STRING) {
                if (parser->current.length >= 2) {
                    ext_name = sstrndup(parser->current.start + 1, parser->current.length - 2);
                }
                advance(parser);
            }
            else {
                parser->Eline = parser->current.line;
                parser->Ecol = parser->current.col;
                parser->Emessage = "Expected package name after @import";
                iutf_node_free(node);
                return NULL;
            }

            // Skipping 'from <source>' if exists
            if (parser->current.type == IUTF_TOK_IDENTIFIER && strncmp(parser->current.start, "from", 4) == 0) {
                advance(parser);  // skip 'from'
                if (parser->current.type == IUTF_TOK_IDENTIFIER) {
                    advance(parser);  // skip source (local/sys/global)
                }
            }

            // Search and load extension
            char* file_path = iutf_find_imported_file(ext_name);
            if (file_path) {
                IutfNode* ext = iutf_parse_from_file(file_path);
                if (ext)
                {
                  for (size_t i = 0; i < ext->data.branch.size; i++)
                  {
                    IutfNode *item = ext->data.branch.items[i];
                    iutf_add_branch (node, item->key, item);
                  }

                  cleanbit (ext->data.branch.items);
                  cleanbit (ext);
                }
                cleanbit(file_path);
            } else {
                fprintf(stderr, COL_YLW "Extension '" COL_CYAN "%s" COL_YLW "' not found\n" COL_DEF, ext_name);
            }

            cleanbit(ext_name);
            fprintf (stderr, "[DEBUG] after @import: token=%s", iutf_token_type_to_string (parser->current.type));
            continue;
        }

        // === colon processing ===
        if (parser->current.type != IUTF_TOK_COLON) {
            parser->Eline = parser->current.line;
            parser->Ecol = parser->current.col;
            snprintf(err_buf, sizeof(err_buf), "Expected ':', got %s", iutf_token_type_to_string(parser->current.type));
            parser->Emessage = err_buf;
            cleanbit(key);
            iutf_node_free(node);
            return NULL;
        }
        advance(parser);  // skip ':'

        // === if: key:identifier { ... } ===
        // for example: deps:init { @import "x" }
        if (parser->current.type == IUTF_TOK_IDENTIFIER) {
            char* subkey = sstrndup(parser->current.start, parser->current.length);
            if (!subkey) {
                cleanbit(key);
                iutf_node_free(node);
                return NULL;
            }
            advance(parser);  // skip identifier

            if (parser->current.type == IUTF_TOK_BRANCH_OPEN) {
                //advance(parser);  // skip '{'

                IutfNode* inner = parse_branch(parser);
                if (!inner) {
                    cleanbit(key);
                    cleanbit(subkey);
                    iutf_node_free(node);
                    return NULL;
                }

                // Creating wrap: { "init": { inner } }
                IutfNode* wrapper = iutf_new_branch();
                if (!wrapper) {
                    cleanbit(key);
                    cleanbit(subkey);
                    iutf_node_free(inner);
                    iutf_node_free(node);
                    return NULL;
                }

                iutf_add_branch(wrapper, subkey, inner);
                cleanbit(subkey);

                // Adding in main branch: { "deps": { wrapper } }
                iutf_add_branch(node, key, wrapper);
                cleanbit(key);
                continue;
            } else {
                // identifier not followed by { - error
                cleanbit(subkey);
                parser->Eline = parser->current.line;
                parser->Ecol = parser->current.col;
                parser->Emessage = "Expected '{' after identifier";
                cleanbit(key);
                iutf_node_free(node);
                return NULL;
            }
        }

        // === Default: key: value ===
        // parse_value proceed: strings, nums, branches, arrays, null etc.
        IutfNode* value = parse_value(parser);
        if (!value) {
            cleanbit(key);
            iutf_node_free(node);
            return NULL;
        }

        // Adding value in branch
        struct IutfNode** temp = MemoryReAllocate(
            node->data.branch.items,
            (node->data.branch.size + 1) * sizeof(struct IutfNode*)
        );
        if (!temp) {
            fprintf(stderr, "Out of memory\n");
            cleanbit(key);
            iutf_node_free(value);
            iutf_node_free(node);
            return NULL;
        }
        node->data.branch.items = temp;
        node->data.branch.items[node->data.branch.size] = value;
        value->key = key;
        node->data.branch.size++;
        //cleanbit(key) Trash!

        // Skip comma if it exist
        if (parser->current.type == IUTF_TOK_COMMA) {
            advance(parser);
        }
    }

    // Check on closing branch
    if (parser->current.type != IUTF_TOK_BRANCH_CLOSE) {
        parser->Eline = parser->current.line;
        parser->Ecol = parser->current.col;
        snprintf(err_buf, sizeof(err_buf), "Expected '}', got %s", iutf_token_type_to_string(parser->current.type));
        parser->Emessage = err_buf;
        iutf_node_free(node);
        return NULL;
    }

    advance(parser); // skip '}'
    return node;
}

IutfParser* iutf_parser_new(ustring input) {
    IutfParser* parser = MemoryAllocate (sizeof(IutfParser));
    if (!parser) return NULL;

    parser->lexer = iutf_lexer_new(input);
    parser->global_vars = iutf_var_tab_new();
    parser->local_vars = iutf_var_tab_new();
    parser->shared_vars = iutf_var_tab_new();
    parser->is_utext = 0;
    if (!parser->lexer) {
        cleanbit (parser);
        return NULL;
    }

    parser->current = iutf_lexer_next(parser->lexer);
    return parser;
}

void iutf_parser_free(IutfParser* parser) {
    if (parser) {
        iutf_lexer_corrupt (parser->lexer);
        iutf_var_tab_free(parser->global_vars);
        iutf_var_tab_free(parser->shared_vars);
        iutf_var_tab_free(parser->local_vars);
        cleanbit (parser);
    }
}

IutfNode* iutf_parse(IutfParser* parser) 
{
    if (parser->current.type == IUTF_TOK_UTEXT)
    {
        parser->is_utext = 1;
        return iutf_parse_utext(parser);
    }

    if (parser->current.type != IUTF_TOK_IDENTIFIER) {
        parser->Eline = parser->current.line;
        parser->Ecol = parser->current.col;
        snprintf (err_buf, sizeof (err_buf), "Expected 'iutf', got %s", iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        return NULL;
    }

    advance(parser);
    if (parser->current.type != IUTF_TOK_COLON) {
        parser->Eline = parser->current.line;
        parser->Ecol = parser->current.col;
        snprintf (err_buf, sizeof (err_buf), "Expected ':', got %s", iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        return NULL;
    }

    advance(parser);
    if (parser->current.type != IUTF_TOK_IDENTIFIER) {
        parser->Eline = parser->current.line;
        parser->Ecol = parser->current.col;
        snprintf (err_buf, sizeof (err_buf), "Expected 'init', got %s", iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        return NULL;
    }

    advance(parser);
    if (parser->current.type != IUTF_TOK_COLON) {
        parser->Eline = parser->current.line;
        parser->Ecol = parser->current.col;
        snprintf (err_buf, sizeof (err_buf), "Expected ':', got %s", iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        return NULL;
    }

    advance(parser);
    if (parser->current.type != IUTF_TOK_IDENTIFIER) {
        parser->Eline = parser->current.line;
        parser->Ecol = parser->current.col;
        snprintf (err_buf, sizeof (err_buf), "Expected 'main', got %s", iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        return NULL;
    }

    advance(parser);
    if (parser->current.type != IUTF_TOK_BRANCH_OPEN) {
        parser->Eline = parser->current.line;
        parser->Ecol = parser->current.col;
        snprintf (err_buf, sizeof (err_buf), "Expected '{', got %s", iutf_token_type_to_string (parser->current.type));
        parser->Emessage = err_buf;
        return NULL;
    }

    return parse_branch(parser);
}

int ParserGetErrLn (IutfParser* parser)
{
  return parser->Eline;
}

int ParserGetErrCol (IutfParser* parser)
{
  return parser->Ecol;
}

const char* ParserGetErrMessage (IutfParser* parser)
{
  return parser->Emessage;
}
