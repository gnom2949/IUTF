/* iutf-validator.c
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

#include "../includes/iutf-validator.h"
#include <stdio.h>

int iutf_validate(IutfNode* root) {
    if (!root) {
        fprintf(stderr, "Root node is NULL\n");
        return 0;
    }

    if (root->type != IUTF_NODE_BRANCH) {
        fprintf(stderr, "Root must be a branch\n");
        return 0;
    }

    // Validation: Check that the root contains the required keys
    int has_title = 0;
    int has_version = 0;

    for (size_t i = 0; i < root->data.branch.size; i++) {
        IutfNode* item = root->data.branch.items[i];
        if (item->key && strcmp(item->key, "title") == 0) {
            if (item->type != IUTF_NODE_STRING) {
                fprintf(stderr, "Field 'title' must be a string\n");
                return 0;
            }
            has_title = 1;
        } else if (item->key && strcmp(item->key, "version") == 0) {
            if (item->type != IUTF_NODE_FLOAT && item->type != IUTF_NODE_INTEGER) {
                fprintf(stderr, "Field 'version' must be a number\n");
                return 0;
            }
            has_version = 1;
        }
    }

    if (!has_title || !has_version) {
        fprintf(stderr, "Missing required fields: 'title' and 'version'\n");
        return 0;
    }

    return 1; // Validation complete
}
