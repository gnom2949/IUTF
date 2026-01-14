/* iutf-api.c
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
#include "../includes/iutf-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IutfNode* iutf_new_branch (void)
{
  IutfNode* node = iutf_node_new (IUTF_NODE_BRANCH);
  if (!node) return NULL;
  node->data.branch.items = NULL;
  node->data.branch.size = 0;
  return node;
}

void iutf_branch_add (IutfNode* branch, const char* key, IutfNode* value)
{
  if (!branch || !key || !value) return;

  struct IutfNode** temp = realloc (branch->data.branch.items, (branch->data.branch.size + 1) * sizeof (struct IutfNode*));
  if (!temp) return;

  branch->data.branch.items = temp;
  branch->data.branch.items[branch->data.branch.size] = value;
  value->key = strdup (key);
  branch->data.branch.size++;
}

IutfNode* iutf_new_str (const char* value)
{
  IutfNode* node = iutf_node_new (IUTF_NODE_STRING);
  if (!node) return NULL;
  node->data.str_value = value ? strdup(value) : NULL;
  return node;
}

IutfNode* iutf_new_int (long long value)
{
  IutfNode* node = iutf_node_new (IUTF_NODE_INTEGER);
  if (!node) return NULL;
  node->data.int_value = value;
  return node;
}

IutfNode* iutf_new_float (double value)
{
  IutfNode* node = iutf_node_new (IUTF_NODE_FLOAT);
  if (!node) return NULL;
  node->data.float_value = value;
  return node;
}

IutfNode* iutf_new_long (long long value)
{
  IutfNode* node = iutf_node_new (IUTF_NODE_LONG);
  if (!node) return NULL;
  node->data.long_value = value;
  return node;
}

IutfNode* iutf_new_char (char value)
{
  IutfNode* node = iutf_node_new (IUTF_NODE_CHARACTER);
  if (!node) return NULL;
  node->data.char_value = value;
  return node;
}

IutfNode* iutf_new_bool (int value)
{
  IutfNode* node = iutf_node_new (IUTF_NODE_BOOLEAN);
  if (!node) return NULL;
  node->data.bool_value = value;
  return node;
}

IutfNode* iutf_new_null (void)
{
  return iutf_node_new (IUTF_NODE_NULL);
}

IutfNode* iutf_new_array (void)
{
  IutfNode* node = iutf_node_new (IUTF_NODE_ARRAY);
  if (!node) return NULL;
  node->data.array.items = NULL;
  node->data.array.size = 0;
  return node;
}

void add_to_array (IutfNode* array, IutfNode* item)
{
  if (!array || !item) return;

  struct IutfNode** temp = realloc (array->data.array.items, (array->data.array.size + 1) * sizeof(struct IutfNode*));
  if (!temp) return;

  array->data.array.items = temp;
  array->data.array.items[array->data.array.size] = item;
  array->data.array.size++;
}

IutfNode* iutf_new_BigString (const char* value)
{
  IutfNode* node = iutf_node_new (IUTF_NODE_BIGSTRING);
  if (!node) return NULL;
  node->data.str_value = value ? strdup (value) : NULL;
  return node;
}

IutfNode* iutf_new_PipeStr (const char* value)
{
  IutfNode* node = iutf_node_new (IUTF_NODE_PIPESTRING);
  if (!node) return NULL;
  node->data.str_value = value ? strdup (value) : NULL;
  return node;
}

//TODO: реализовать debug_print_string
char* debug_print_string (IutfNode* node)
{
  return NULL;
}
