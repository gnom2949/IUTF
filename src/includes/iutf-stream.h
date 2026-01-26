/* iutf-stream.h
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
#ifndef IUTF_STREAM_H
#define IUTF_STREAM_H

#include "iutf-ast.h"
#include "iutf-lexer.h"
#include <stdio.h>

// read files by one entry
typedef struct {
  FILE* fp;
  IutfLexer* lexer;
  int finished;
} IutfStream;

// open the stream
IutfStream* stream_open (const char* filename);

// close the stream
void stream_corrupt (IutfStream* stream);

// read the next entry
IutfNode* stream_entry_next (IutfStream* stream);

#endif
