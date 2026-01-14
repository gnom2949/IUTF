/* iutf-ast.c
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

#include "../includes/iutf-ast.h"

IutfNode* iutf_node_new(IutfNodeType type) {
    IutfNode* node = calloc(1, sizeof(IutfNode));
    if (!node) return NULL;
    node->type = type;
    node->key = NULL;
    return node;
}

void iutf_node_free(IutfNode* node) {
    if (!node) return;

    free(node->key);

    switch (node->type) {
        case IUTF_NODE_STRING:
        case IUTF_NODE_BIGSTRING:
        case IUTF_NODE_PIPESTRING:
            free(node->data.str_value);
            break;
        case IUTF_NODE_ARRAY:
            for (size_t i = 0; i < node->data.array.size; i++) {
                iutf_node_free(node->data.array.items[i]);
            }
            free(node->data.array.items);
            break;
        case IUTF_NODE_BRANCH:
            for (size_t i = 0; i < node->data.branch.size; i++) {
                iutf_node_free(node->data.branch.items[i]);
            }
            free(node->data.branch.items);
            break;
        default:
            break;
    }

    free(node);
}
