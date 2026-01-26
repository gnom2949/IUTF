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
#include "../includes/colors.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

char* iutf_find_imported_file (const char* filename)
{
  // Провека окружения переменной
  const char* path_env = getenv ("IUTF_INCLUDE_PATH");
  if (!path_env) {
    path_env = "/usr/include";
  }

  // формирование пути: /usr/include/name/name.utext
  size_t len = strlen (path_env) + strlen (filename) * 2 + 32;
  char* full_path = malloc (len);
  snprintf (full_path, len, "%s/%s/%s.utext", path_env, filename, filename);

  // проверка на существование файла
  struct stat st;
  if (stat (full_path, &st) == 0) {
    return full_path;
  }

  // пробуем найти файл pst.utext
  snprintf (full_path, len, "%s/%s/pst.utext", path_env, filename);
  if (stat (full_path, &st) == 0) {
    return full_path;
  }

  free (full_path);
  return NULL;
}
