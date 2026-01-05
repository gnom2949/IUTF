/* main.c
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

#include "../../includes/iutf-parser.h"
#include "../../includes/iutf-validator.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file.iutf>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Cannot open file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(len + 1);
    if (!buffer) {
        fprintf(stderr, "Out of memory\n");
        fclose(file);
        return 1;
    }

    fread(buffer, 1, len, file);
    buffer[len] = '\0';
    fclose(file);

    IutfParser* parser = iutf_parser_new(buffer);
    if (!parser) {
        fprintf(stderr, "Failed to create parser\n");
        free(buffer);
        return 1;
    }

    IutfNode* ast = iutf_parse(parser);
    if (!ast) {
        fprintf(stderr, "Parse failed\n");
        iutf_parser_free(parser);
        free(buffer);
        return 1;
    }

    printf("Parse successful!\n");

    if (iutf_validate(ast)) {
        printf("Validation passed!\n");
    } else {
        printf("Validation failed!\n");
    }

    iutf_node_free(ast);
    iutf_parser_free(parser);
    free(buffer);
    return 0;
}
