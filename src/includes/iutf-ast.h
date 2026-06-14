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

#ifndef _IUTF_AST_H_
#define _IUTF_AST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

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
    IUTF_NODE_PIPESTRING,
    IUTF_NODE_VAR,
    IUTF_NODE_REF,
    IUTF_NODE_ONLYTHIS,
    IUTF_NODE_UTEXT,
    IUTF_NODE_UTEXT_SETUP
} IutfNodeType;

typedef enum {
  IUTF_REF_GLOBAL, /* @global.name - defined in file root. */
  IUTF_REF_LOCAL, /* @local.name - defined in local branch. */
  IUTF_REF_SHARE, /* @share.name - from imported file. */
  IUTF_REF_GETLENOF /* @getlenof.name - len of array. */
} IutfRefScope;

typedef struct IutfNode {
    IutfNodeType type;
    ustring key; // for key-value pairs
    union {
        ustring str_value;
        long long int_value;
        double float_value;
        long long long_value;
        char char_value;
        int bool_value;
        struct {
            struct IutfNode** items;
            size_t size;
        } array;
        struct {
            struct IutfNode** items;
            size_t size;
        } branch;
        ustring bigstring_value;
        ustring pipestring_value;

        struct {
          IutfRefScope scope;
          ustring name;
        } ref;

        struct IutfNode *onlythis_value;

        struct {
          ustring ext_name;
          struct IutfNode **items;
          size_t size;
        } utext;
    } data;
} IutfNode;

/* Variables */
typedef struct {
  ustring name;
  IutfNode *value;
} IutfVarEntry;

typedef struct {
  IutfVarEntry *entries;
  size_t count;
  size_t capacity;
} IutfVarTable;
    /* END */

typedef struct {
  char** names;
  size_t count;
  size_t capacity;
} IutfBrList;

IutfVarTable *iutf_var_tab_new (void);
void iutf_var_tab_free (IutfVarTable *ivt);
int iutf_var_tab_set (IutfVarTable *ivt, ustring name, IutfNode *value);
IutfNode *iutf_var_tab_get (const IutfVarTable *tbl, ustring name);

IutfBrList* iutf_branch_list_new (void);
void iutf_branch_list_free (IutfBrList* list);
void iutf_branch_list_add (IutfBrList* list, const char* name);
void iutf_collect_branches (IutfNode* node, IutfBrList* list);
int iutf_has_branch (IutfBrList *list, const char *name);
IutfBrList* iutf_branch_get_All (IutfNode* root);
IutfNode* iutf_node_new(IutfNodeType type);
void iutf_node_free(IutfNode* node);

#endif /* IUTF_AST_H */
