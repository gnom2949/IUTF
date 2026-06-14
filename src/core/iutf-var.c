/* iutf-var.c
 *
 * Copyright 2026 Int Software, Aleksandr Silaev
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 * IUTF Variable System version 0.1
 */

#include "../includes/iutf.h"
#include <string.h>
#include <stdio.h>


#define VAR_TABLE_INIT_CAP 8

IutfVarTable* iutf_var_tab_new (void)
{
    IutfVarTable* tbl = MemoryAllocate (sizeof (IutfVarTable));
    if (!tbl) return NULL;
    tbl->entries  = MemoryAllocate (VAR_TABLE_INIT_CAP * sizeof (IutfVarEntry));
    if (!tbl->entries) { cleanbit (tbl); return NULL; }
    tbl->count    = 0;
    tbl->capacity = VAR_TABLE_INIT_CAP;
    return tbl;
}

void iutf_var_tab_free (IutfVarTable* tbl)
{
    if (!tbl) return;
    for (size i = 0; i < tbl->count; i++) {
        cleanbit (tbl->entries[i].name);
        iutf_node_free (tbl->entries[i].value);
    }
    cleanbit (tbl->entries);
    cleanbit (tbl);
}

int iutf_var_tab_set (IutfVarTable* tbl, ustring name, IutfNode* value)
{
    if (!tbl || !name || !value) return 0;

    for (size i = 0; i < tbl->count; i++) {
        if (strcmp (tbl->entries[i].name, name) == 0) {
            iutf_node_free (tbl->entries[i].value);
            tbl->entries[i].value = value;
            return 1;
        }
    }

    if (tbl->count >= tbl->capacity) {
        size_t new_cap = tbl->capacity * 2;
        IutfVarEntry* tmp = MemoryReAllocate (tbl->entries, new_cap * sizeof (IutfVarEntry));
        if (!tmp) return 0;
        tbl->entries  = tmp;
        tbl->capacity = new_cap;
    }

    char* name_dup = MemoryAllocate (strlen (name) + 1);
    if (!name_dup) return 0;
    strcpy (name_dup, name);

    tbl->entries[tbl->count].name  = name_dup;
    tbl->entries[tbl->count].value = value;
    tbl->count++;
    return 1;
}

IutfNode* iutf_var_tab_get (const IutfVarTable* tbl, ustring name)
{
    if (!tbl || !name) return NULL;
    for (size_t i = 0; i < tbl->count; i++) {
        if (strcmp (tbl->entries[i].name, name) == 0)
            return tbl->entries[i].value;
    }
    return NULL;
}


int iutf_parse_ref_string (ustring start, size len,
                           IutfRefScope* scope_out, char** name_out)
{
    if (!start || len == 0) return 0;

    ustring p = start;
    if (*p == '@') { p++; len--; }

    ustring dot = memchr (p, '.', len);

    size kw_len = dot ? (size)(dot - p) : len;

    if (kw_len == 6 && strncmp (p, "global", 6) == 0)
        *scope_out = IUTF_REF_GLOBAL;
    else if (kw_len == 5 && strncmp (p, "local", 5) == 0)
        *scope_out = IUTF_REF_LOCAL;
    else if (kw_len == 5 && strncmp (p, "share", 5) == 0)
        *scope_out = IUTF_REF_SHARE;
    else if (kw_len == 8 && strncmp (p, "getlenof", 8) == 0)
        *scope_out = IUTF_REF_GETLENOF;
    else {
        fprintf (stderr, "iutf-var: unknown ref scope '%.*s'\n", (int)kw_len, p);
        return 0;
    }

    if (!dot) {
        *name_out = NULL;
        return 1;
    }

    const char* name_start = dot + 1;
    size_t name_len = len - kw_len - 1;
    if (name_len == 0) {
        fprintf (stderr, "iutf-var: empty name after '.'\n");
        return 0;
    }

    char* name = MemoryAllocate (name_len + 1);
    if (!name) return 0;
    memcpy (name, name_start, name_len);
    name[name_len] = '\0';
    *name_out = name;
    return 1;
}

/* ------------------------------------------------------------------ */
/*  @getlenof                                                          */
/* ------------------------------------------------------------------ */

long long getlenof (IutfNode* node)
{
    if (!node) return 0;
    if (node->type == IUTF_NODE_ARRAY)  return (long long) node->data.array.size;
    if (node->type == IUTF_NODE_STRING) return (long long) strlen (node->data.str_value);
    fprintf (stderr, "iutf-var: @getlenof applied to non-array/non-string node (type %d)\n",
             node->type);
    return 0;
}


IutfNode* resolve_one (IutfNode* ref, IutfResolveCtx* ctx)
{
    if (!ref || ref->type != IUTF_NODE_REF || !ctx) return NULL;

    IutfRefScope scope = ref->data.ref.scope;
    const char*  name  = ref->data.ref.name;

    if (scope == IUTF_REF_GETLENOF) {
        IutfNode* arr = NULL;
        if (ctx->global) arr = iutf_var_tab_get (ctx->global, name);
        if (!arr && ctx->local) arr = iutf_var_tab_get (ctx->local, name);
        if (!arr) {
            fprintf (stderr, "iutf-var: @getlenof: array '%s' not found\n", name);
            return NULL;
        }
        IutfNode* len_node = iutf_node_new (IUTF_NODE_INTEGER);
        if (!len_node) return NULL;
        len_node->data.int_value = getlenof (arr);
        return len_node;
    }

    IutfVarTable* tbl = NULL;
    switch (scope) {
        case IUTF_REF_GLOBAL: tbl = ctx->global; break;
        case IUTF_REF_LOCAL:  tbl = ctx->local;  break;
        case IUTF_REF_SHARE:  tbl = ctx->shared;  break;
        default: break;
    }

    if (!tbl) {
        fprintf (stderr, "iutf-var: var table for scope %d is NULL\n", scope);
        return NULL;
    }

    IutfNode* val = iutf_var_tab_get (tbl, name);
    if (!val) {
        fprintf (stderr, "iutf-var: variable '%s' not found in scope %d\n", name, scope);
    }
    return val;
}

int resolve_refs (IutfNode* root, IutfResolveCtx* ctx)
{
    if (!root || !ctx) return 1;

    switch (root->type) {
        case IUTF_NODE_BRANCH:
        case IUTF_NODE_UTEXT_SETUP: {
            struct IutfNode** items = (root->type == IUTF_NODE_BRANCH)
                                       ? root->data.branch.items
                                       : root->data.utext.items;
            size_t size = (root->type == IUTF_NODE_BRANCH)
                           ? root->data.branch.size
                           : root->data.utext.size;
            int ok = 1;
            for (size_t i = 0; i < size; i++) {
                IutfNode* child = items[i];
                if (child->type == IUTF_NODE_REF) {
                    IutfNode* resolved = resolve_one (child, ctx);
                    if (!resolved) { ok = 0; continue; }
                    string key_backup = child->key;
                    cleanbit (child->data.ref.name);
                    IutfNodeType saved_type = resolved->type;
                    char*        saved_key  = child->key;
                    *child = *resolved;
                    child->key  = saved_key;
                    (void)saved_type;
                    (void)key_backup;
                } else {
                    if (!resolve_refs (child, ctx)) ok = 0;
                }
            }
            return ok;
        }

        case IUTF_NODE_ARRAY:
            for (size_t i = 0; i < root->data.array.size; i++) {
                IutfNode* child = root->data.array.items[i];
                if (child->type == IUTF_NODE_REF) {
                    IutfNode* resolved = resolve_one (child, ctx);
                    if (!resolved) continue;
                    cleanbit (child->data.ref.name);
                    char* saved_key = child->key;
                    *child = *resolved;
                    child->key = saved_key;
                } else {
                    resolve_refs (child, ctx);
                }
            }
            return 1;

        case IUTF_NODE_ONLYTHIS:
            if (root->data.onlythis_value &&
                root->data.onlythis_value->type == IUTF_NODE_REF) {
                IutfNode* resolved = resolve_one (root->data.onlythis_value, ctx);
                if (resolved) {
                    cleanbit (root->data.onlythis_value->data.ref.name);
                    char* saved_key = root->data.onlythis_value->key;
                    *root->data.onlythis_value = *resolved;
                    root->data.onlythis_value->key = saved_key;
                }
            }
            return 1;

        default:
            return 1;
    }
}