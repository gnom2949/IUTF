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

static void append_to_buf (char** buf, size_t* size, const char* format, ...)
{
  va_list args;
  va_start (args, format);

  // determine the required size
  va_list args_copy;
  va_copy (args_copy, args);
  int len = vsnprintf (NULL, 0, format, args_copy);
  va_end (args_copy);

  if (len < 0) return;

  char* new_buf = realloc (*buf, *size + len + 1);
  if (!new_buf) return;

  *buf = new_buf;
  vsprintf (*buf + *size, format, args);
  *size += len;
  va_end (args);
}

IutfNode* iutf_new_branch (void)
{
  IutfNode* node = iutf_node_new (IUTF_NODE_BRANCH);
  if (!node) return NULL;
  node->data.branch.items = NULL;
  node->data.branch.size = 0;
  return node;
}

void to_branch (IutfNode* branch, const char* key, IutfNode* value)
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

static void debug_print_recursive (IutfNode* node, char** buf, size_t* size, int indent)
{
  if (!node) return;

  // spacing
  for (int i = 0; i < indent; i++) append_to_buf (buf, size, "  ");

  //If the node has a key (for branches)
  if (node->key) append_to_buf (buf, size, "\"%s\": ", node->key);

  switch (node->type) {
    case IUTF_NODE_BRANCH:
      append_to_buf (buf, size, "{\n");
      for (size_t i = 0; i < node->data.branch.size; i++) {
        debug_print_recursive (node->data.branch.items[i], buf, size, indent + 1);
        append_to_buf (buf, size, (i == node->data.branch.size - 1) ? "\n" : ",\n");
      }
      for (int i = 0; i < indent; i++) append_to_buf (buf, size, "  ");
      append_to_buf (buf, size, "}");
      break;

    case IUTF_NODE_ARRAY:
      append_to_buf (buf, size, "[\n");
      for (size_t i = 0; i < node->data.array.size; i++) {
        debug_print_recursive (node->data.array.items[i], buf, size, indent + 1);
        append_to_buf (buf, size, (i == node->data.array.size - 1) ? "\n" : ",\n");
      }
      for (int i = 0; i < indent; i++) append_to_buf (buf, size, "  ");
      append_to_buf (buf, size, "]");
      break;

    case IUTF_NODE_STRING:
    case IUTF_NODE_BIGSTRING:
    case IUTF_NODE_PIPESTRING:
      append_to_buf (buf, size, "\"%s\"", node->data.str_value ? node->data.str_value : "null");
      break;

    case IUTF_NODE_INTEGER:
    case IUTF_NODE_LONG:
      append_to_buf (buf, size, "%lld", node->data.int_value);
      break;

    case IUTF_NODE_FLOAT:
      append_to_buf(buf, size, "%f", node->data.float_value);
      break;

    case IUTF_NODE_CHARACTER:
      append_to_buf(buf, size, "'%c'", node->data.char_value);
      break;

    case IUTF_NODE_BOOLEAN:
      append_to_buf(buf, size, node->data.bool_value ? "true" : "false");
      break;

    case IUTF_NODE_NULL:
      append_to_buf(buf, size, "null");
      break;
  }
}


char* debug_print_string (IutfNode* node)
{
  if (!node) return NULL;

  size_t size = 0;
  char* buf = malloc(1);
  if (!buf) return NULL;
  buf[0] = '\0';

  debug_print_recursive (node, &buf, &size, 0);

  return buf;
}
