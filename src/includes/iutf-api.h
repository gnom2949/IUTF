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
 * IUTF API Header version 0.2.3
 */
#ifndef _IUTF_API_H_
#define _IUTF_API_H_

#if defined(__cplusplus)
  extern "C" {
#endif

#define MAX_BUF_SIZE 4096
#define MIN_BUF_SIZE 1024
#define IUTF_PRETTY_PRINT (1 << 0)
#define IUTF_UNESCAPED_UNICODE (1 << 1)
#define IUTF_UNESCAPED_SLASHES (1 << 2)
#define IUTF_HEX_TAG (1 << 3)
#define IUTF_NUMERIC_CHECK (1 << 4)
#define IUTF_FORCE_BRANCH (1 << 5)

#include "iutf-ast.h"

typedef struct {
  char* data;
  size_t size;
  size_t capacity;
} IutfApiBuf;

typedef struct {
  char *data;
  size_t size;
  size_t capacity;
}JBuf;

//create root branch
IutfNode* iutf_new_branch (void);

// create another branch(not root branch)
void iutf_add_branch (IutfNode *branch, const char *key, IutfNode *value);

// add key-value in branch
void to_branch (IutfNode   *branch,
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

// do from IUTF to JSON
char* iutfToJSON (IutfNode* node);
// recursive function-helper
void debug_print_recursive (IutfNode* node, char** buf, size_t* size, int indent);

// init buffer
void bufInit (IutfApiBuf* buf);

// print to buffer (dynamic array)
void bufPrint (IutfApiBuf* buf, const char* fmt, ...);

int ApiGetLastErrLn (void);
int ApiGetLastErrCol (void);
const char* ApiGetLastErrMessage (void);
void iutf_free_string (char *ptr);

#if defined(__cplusplus)
  }
#endif /* __cplusplus */

#endif
