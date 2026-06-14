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
 * IUTF Abstract Syntax Tree version 0.2.5
 */

#include "../includes/iutf-ast.h"
#include <IntMemoryManager.h>

IutfNode* iutf_node_new(IutfNodeType type) {
    IutfNode* node = MemoryAllocateAndFillZero (1, sizeof(IutfNode));
    if (!node) return NULL;
    node->type = type;
    node->key = NULL;
    return node;
}

IutfBrList* iutf_branch_list_new (void)
{
  IutfBrList* list = MemoryAllocate (sizeof(IutfBrList));
  if (!list) return NULL;

  list->names = NULL;
  list->count = 0;
  list->capacity = 0;

  return list;
}

void iutf_branch_list_free (IutfBrList* list)
{
  if (list) {
    if (list->names) {
      for (size_t i = 0; i < list->count; i++) {
        cleanbit (list->names[i]);
      }
      cleanbit (list->names);
    }
    cleanbit (list);
  }
}

void iutf_branch_list_add (IutfBrList* list, const char* name)
{
  if (!list || !name) return;

  if (list->count >= list->capacity) {
    size_t nCap = list->capacity == 0 ? 4 : list->capacity * 2;
    char** nNames = realloc (list->names, nCap * sizeof (char*));
    if (!nNames) return;

    list->names = nNames;
    list->capacity = nCap;
  }

  list->names[list->count] = strdup (name);
  if (list->names[list->count]) {
    list->count++;
  }
}

void iutf_collect_branches (IutfNode* node, IutfBrList* list)
{
  if (!node || !list) return;

  if (node->type == IUTF_NODE_BRANCH && node->key) {
    iutf_branch_list_add (list, node->key);
  }

  switch (node->type)
  {
    case IUTF_NODE_BRANCH:
      for (size_t i = 0; i < node->data.branch.size; i++) {
        iutf_collect_branches (node->data.branch.items[i], list);
      }
      break;
    case IUTF_NODE_ARRAY:
      for (size_t i = 0; i < node->data.array.size; i++) {
        iutf_collect_branches (node->data.array.items[i], list);
      }
      break;
    default:
      break;
  }
}

int iutf_has_branch(IutfBrList* list, const char* name) {
    if (!list || !name) return 0;

    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->names[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

IutfBrList* iutf_get_all_branches(IutfNode* root) {
    IutfBrList* list = iutf_branch_list_new();
    if (!list) return NULL;

    iutf_collect_branches(root, list);
    return list;
}

void iutf_node_free(IutfNode* node)
{
    if (!node) return;

    cleanbit (node->key);

    switch (node->type)
    {
        case IUTF_NODE_STRING:
        case IUTF_NODE_BIGSTRING:
        case IUTF_NODE_PIPESTRING:
            cleanbit (node->data.str_value);
            break;
        case IUTF_NODE_ARRAY:
            for (size_t i = 0; i < node->data.array.size; i++) {
                iutf_node_free(node->data.array.items[i]);
            }
            cleanbit (node->data.array.items);
            break;
        case IUTF_NODE_BRANCH:
            for (size_t i = 0; i < node->data.branch.size; i++) {
                iutf_node_free(node->data.branch.items[i]);
            }
            cleanbit (node->data.branch.items);
            break;
        default:
            break;
    }

    cleanbit (node);
}
