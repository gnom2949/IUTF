/* iutf-api.h
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
#ifndef IUTF_API_H
#define IUTF_API_H

#include "iutf-ast.h"

//create branch
IutfNode* iutf_new_branch (void);

// add key-value in branch
void iutf_branch_add (IutfNode   *branch,
                      const char *key,
                      IutfNode   *value);

// create string
IutfNode* iutf_new_str (const char* value);

// create Integer number
IutfNode* iutf_new_int (long long value);

// create floating point number(float)
IutfNode* iutf_new_float (double value);

// create long
IutfNode* iutf_new_long (long long value);

// create character
IutfNode* iutf_new_char (char value);

// create boolean
IutfNode* iutf_new_bool (int value);

// create null
IutfNode* iutf_new_null (void);

// create array
IutfNode* iutf_new_array (void);

// add element into array
void add_to_array (IutfNode *array,
                   IutfNode *item);

// create bigstring
IutfNode* iutf_new_BigString (const char* value);

// create PipeString
IutfNode* iutf_new_PipeStr (const char* value);

// Print IUTF to string (for debugging)
char* debug_print_string (IutfNode* node);

#endif
