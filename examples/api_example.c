/* api_example.c
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

#include "../src/includes/iutf-api.h"
#include <stdio.h>
#include <stdlib.h>

IutfNode* genStatTable (void)
{
  IutfNode* root = iutf_new_branch();

  // creating a iutf:init:main branch
  IutfNode* main_branch = iutf_new_branch();
  iutf_branch_add (main_branch, "StatVal", iutf_new_int(42));
  iutf_branch_add (main_branch, "enabled", iutf_new_bool(1));
  iutf_branch_add (main_branch, "name", iutf_new_str("Stats"));

  iutf_branch_add (root, "init", main_branch);

  return root;
}

int main()
{
  IutfNode* table = genStatTable();

  char* output = debug_print_string (table);
  if (output) {
    printf ("Generated IUTF:\n%s\n", output);
    free (output);
  }

  iutf_node_free (table);
  return 0;
}
