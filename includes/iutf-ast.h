/* iutf-ast.h
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

#ifndef IUTF_AST_H
#define IUTF_AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    IUTF_NODE_BRANCH,
    IUTF_NODE_KEY_VALUE,
    IUTF_NODE_STRING,
    IUTF_NODE_INTEGER,
    IUTF_NODE_FLOAT,
    IUTF_NODE_LONG,
    IUTF_NODE_CHARACTER,
    IUTF_NODE_BOOLEAN,
    IUTF_NODE_NULL,
    IUTF_NODE_ARRAY,
    IUTF_NODE_BIGSTRING,
    IUTF_NODE_PIPESTRING
} IutfNodeType;

typedef struct IutfNode {
    IutfNodeType type;
    char* key; // for key-value pairs
    union {
        char* str_value;
        long long int_value;
        double float_value;
        long long long_value;
        char char_value;
        int bool_value;
        struct IutfNode** array_items;
        size_t array_size;
        struct IutfNode** branch_items;
        size_t branch_size;
    } data;
} IutfNode;

IutfNode* iutf_node_new(IutfNodeType type);
void iutf_node_free(IutfNode* node);

#endif /* IUTF_AST_H */
