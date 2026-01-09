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
 */

#define _GNU_SOURCE

#include "../../includes/iutf-parser.h"
#include <assert.h>

static void advance(IutfParser* parser) {
    parser->current = iutf_lexer_next(parser->lexer);
}

static IutfNode* parse_value(IutfParser* parser);
static IutfNode* parse_branch(IutfParser* parser);

static char* safe_strndup(const char* s, size_t n) { // я ебал блять этот ебучий сегфолт
    char* dup = malloc(n + 1);
    if (!dup) return NULL;
    memcpy(dup, s, n);
    dup[n] = '\0';
    return dup;
}

static IutfNode* parse_string(IutfParser* parser) {
    IutfNode* node = iutf_node_new(IUTF_NODE_STRING);
    if (!node) return NULL;

    node->data.str_value = safe_strndup(parser->current.start, parser->current.length);
    if (!node->data.str_value) {
      fprintf(stderr, "Failed to allocate string!!\n");
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

static IutfNode* parse_character(IutfParser* parser) {
    IutfNode* node = iutf_node_new(IUTF_NODE_CHARACTER);
    if (!node) return NULL;

    // skip ''
    if (parser->current.length >= 3 && parser->current.start[1] == '\\') {
        node->data.char_value = parser->current.start[2];
    } else {
        if (parser->current.length < 3) {
            fprintf(stderr, "Invalid character literal\n");
            iutf_node_free(node);
            return NULL;
        }
        node->data.char_value = parser->current.start[1];
    }
    advance(parser);
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

        struct IutfNode** temp = realloc(node->data.array.items, (node->data.array.size + 1) * sizeof(struct IutfNode*));
        if (!temp) {
            fprintf(stderr, "Out of memory\n");
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
        fprintf(stderr, "Expected ']', got %s\n", iutf_token_type_to_string(parser->current.type));
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

    node->data.str_value = safe_strndup(parser->lexer->input + start, end - start);
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

    node->data.str_value = safe_strndup(parser->lexer->input + start, parser->lexer->pos - start);
    if (!node->data.str_value) {
        fprintf(stderr, "Failed to allocate pipe string\n");
        iutf_node_free(node);
        return NULL;
    }

    advance(parser); // skip closing |
    return node;
}

static IutfNode* parse_value(IutfParser* parser) {
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
        default:
            fprintf(stderr, "Unexpected token: %s\n", iutf_token_type_to_string(parser->current.type));
            return NULL;
    }
}

static IutfNode* parse_branch(IutfParser* parser) {
    IutfNode* node = iutf_node_new(IUTF_NODE_BRANCH);
    if (!node) return NULL;


    advance(parser); // skip '{'

    while (parser->current.type != IUTF_TOK_BRANCH_CLOSE && parser->current.type != IUTF_TOK_EOF) {
        if (parser->current.type == IUTF_TOK_IDENTIFIER) {
            char* key = safe_strndup(parser->current.start, parser->current.length);
            if (!key) {
                fprintf(stderr, "Failed to allocate key\n");
                iutf_node_free(node);
                return NULL;
            }
            advance(parser);

            if (parser->current.type == IUTF_TOK_COLON) {
                advance(parser);
                IutfNode* value = parse_value(parser);
                if (!value) {
                    free(key);
                    iutf_node_free(node);
                    return NULL;
                }
                value->key = key;

                struct IutfNode** temp = realloc(node->data.branch.items, (node->data.branch.size + 1) * sizeof(struct IutfNode*));
                if (!temp) {
                    fprintf(stderr, "Out of memory\n");
                    free(key);
                    iutf_node_free(value);
                    iutf_node_free(node);
                    return NULL;
                }
                node->data.branch.items = temp;
                node->data.branch.items[node->data.branch.size] = value;
                node->data.branch.size++;
            } else {
                fprintf(stderr, "Expected ':', got %s\n", iutf_token_type_to_string(parser->current.type));
                free(key);
                iutf_node_free(node);
                return NULL;
            }
        } else {
            fprintf(stderr, "Expected identifier, got %s\n", iutf_token_type_to_string(parser->current.type));
            iutf_node_free(node);
            return NULL;
        }

        if (parser->current.type == IUTF_TOK_COMMA) {
            advance(parser);
        }
    }

    if (parser->current.type != IUTF_TOK_BRANCH_CLOSE) {
        fprintf(stderr, "Expected '}', got %s\n", iutf_token_type_to_string(parser->current.type));
        iutf_node_free(node);
        return NULL;
    }
    advance(parser); // skip '}'
    return node;
}

IutfParser* iutf_parser_new(const char* input) {
    IutfParser* parser = malloc(sizeof(IutfParser));
    if (!parser) return NULL;

    parser->lexer = iutf_lexer_new(input);
    if (!parser->lexer) {
        free(parser);
        return NULL;
    }

    parser->current = iutf_lexer_next(parser->lexer);
    return parser;
}

void iutf_parser_free(IutfParser* parser) {
    if (parser) {
        iutf_lexer_corrupt (parser->lexer);
        free(parser);
    }
}

IutfNode* iutf_parse(IutfParser* parser) {
    if (parser->current.type != IUTF_TOK_IDENTIFIER) {
        fprintf(stderr, "Expected 'iutf', got %s\n", iutf_token_type_to_string(parser->current.type));
        return NULL;
    }

    advance(parser);
    if (parser->current.type != IUTF_TOK_COLON) {
        fprintf(stderr, "Expected ':', got %s\n", iutf_token_type_to_string(parser->current.type));
        return NULL;
    }

    advance(parser);
    if (parser->current.type != IUTF_TOK_IDENTIFIER) {
        fprintf(stderr, "Expected 'init', got %s\n", iutf_token_type_to_string(parser->current.type));
        return NULL;
    }

    advance(parser);
    if (parser->current.type != IUTF_TOK_COLON) {
        fprintf(stderr, "Expected ':', got %s\n", iutf_token_type_to_string(parser->current.type));
        return NULL;
    }

    advance(parser);
    if (parser->current.type != IUTF_TOK_IDENTIFIER) {
        fprintf(stderr, "Expected 'main', got %s\n", iutf_token_type_to_string(parser->current.type));
        return NULL;
    }

    advance(parser);
    if (parser->current.type != IUTF_TOK_BRANCH_OPEN) {
        fprintf(stderr, "Expected '{', got %s\n", iutf_token_type_to_string(parser->current.type));
        return NULL;
    }

    return parse_branch(parser);
}
