/* iutf-import.c
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
 *
 * IUTF Import version 0.1
 */
#include "../includes/iutf-import.h"
#include "imm/IntMemoryManager.h"
#include "../includes/colors.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>

char* iutf_find_imported_file (const char* filename)
{
  if (filename == NULL) return NULL;
  // Check environment PATH
  const char* path_env = getenv ("IUTF_INCLUDE_PATH");
  if (!path_env) {
    path_env = "/usr/include";
  }

  // path: /usr/include/name/name.utext
  size_t len = strlen (path_env) + strlen (filename) * 2 + 32;
  char* full_path = MemoryAllocate (len);
  if (!full_path) return NULL;
  snprintf (full_path, len, "%s/%s/%s.utext", path_env, filename, filename);

  // check on file existing
  struct stat st;
  if (stat (full_path, &st) == 0) {
    return full_path;
  }

  // trying to find pst.utext
  snprintf (full_path, len, "%s/%s/pst.utext", path_env, filename);
  if (stat (full_path, &st) == 0) {
    return full_path;
  }

  cleanbit (full_path);
  return NULL;
}
