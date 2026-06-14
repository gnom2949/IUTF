/* iutf-var.h
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
 * IUTF VAR HEADER VERSION 1.
 */
#ifndef _IUTF_VARIABLE_H_
#define _IUTF_VARIABLE_H_

#include "iutf-ast.h"

typedef struct {
    IutfVarTable *global;
    IutfVarTable *local;
    IutfVarTable *shared;
} IutfResolveCtx;

int resolve_refs(IutfNode *root, IutfResolveCtx *ctx);

IutfNode *resolve_one (IutfNode *ref_node, IutfResolveCtx *ctx);

int iutf_parse_ref_string (ustring token_start, size token_len, IutfRefScope *scope, char **name_out);

long long getlenof (IutfNode *array_node);

#endif /* _IUTF_VARIABLE_H */
