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
 * IUTF Main C SOURCE version 0.7.4
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include "../includes/iutf.h"

#define VERSION "0.6.3"
#define MAX_JSON_BUFFER 1024 * 1024  /* 1 MB max JSON request */
#define MAX_FILE_SIZE 10 * 1024 * 1024  /* 10 MB max file */

/* JSON helper functions */
typedef struct {
    char* buffer;
    size_t pos;
    size_t capacity;
} JsonBuilder;

JsonBuilder* json_builder_new(size_t initial_capacity) {
    JsonBuilder* jb = MemoryAllocate(sizeof(JsonBuilder));
    jb->buffer = MemoryAllocate(initial_capacity);
    jb->pos = 0;
    jb->capacity = initial_capacity;
    return jb;
}

void json_builder_append(JsonBuilder* jb, const char* str) {
    size_t len = strlen(str);
    while (jb->pos + len + 1 > jb->capacity) {
        jb->capacity *= 2;
        jb->buffer = MemoryReAllocate(jb->buffer, jb->capacity);
    }
    strcpy(jb->buffer + jb->pos, str);
    jb->pos += len;
}

void json_builder_append_char(JsonBuilder* jb, char c) {
    if (jb->pos + 1 >= jb->capacity) {
        jb->capacity *= 2;
        jb->buffer = MemoryReAllocate(jb->buffer, jb->capacity);
    }
    jb->buffer[jb->pos++] = c;
    jb->buffer[jb->pos] = '\0';
}

void json_builder_append_escaped(JsonBuilder* jb, const char* str) {
    json_builder_append_char(jb, '"');
    for (const char* p = str; *p; p++) {
        switch (*p) {
            case '"':  json_builder_append(jb, "\\\""); break;
            case '\\': json_builder_append(jb, "\\\\"); break;
            case '\n': json_builder_append(jb, "\\n");  break;
            case '\r': json_builder_append(jb, "\\r");  break;
            case '\t': json_builder_append(jb, "\\t");  break;
            default:
                json_builder_append_char(jb, *p);
        }
    }
    json_builder_append_char(jb, '"');
}

char* json_builder_finish(JsonBuilder* jb) {
    char* result = jb->buffer;
    cleanbit(jb);
    return result;
}

/* Simple JSON parser for stdin */
typedef struct {
    const char* str;
    size_t pos;
    size_t len;
    const char* error;
} JsonParser;

JsonParser* json_parser_new(const char* str) {
    JsonParser* jp = MemoryAllocate(sizeof(JsonParser));
    jp->str = str;
    jp->pos = 0;
    jp->len = strlen(str);
    jp->error = NULL;
    return jp;
}

void json_skip_whitespace(JsonParser* jp) {
    while (jp->pos < jp->len && isspace(jp->str[jp->pos])) {
        jp->pos++;
    }
}

int json_match(JsonParser* jp, const char* str) {
    json_skip_whitespace(jp);
    size_t len = strlen(str);
    if (jp->pos + len > jp->len) return 0;
    return strncmp(jp->str + jp->pos, str, len) == 0;
}

const char* json_parse_string(JsonParser* jp) {
    json_skip_whitespace(jp);
    if (jp->pos >= jp->len || jp->str[jp->pos] != '"') {
        jp->error = "Expected string";
        return NULL;
    }
    jp->pos++;

    size_t start = jp->pos;
    while (jp->pos < jp->len && jp->str[jp->pos] != '"') {
        if (jp->str[jp->pos] == '\\') jp->pos++;
        jp->pos++;
    }

    if (jp->pos >= jp->len) {
        jp->error = "Unterminated string";
        return NULL;
    }

    size_t len = jp->pos - start;
    char* result = MemoryAllocate(len + 1);
    strncpy(result, jp->str + start, len);
    result[len] = '\0';
    jp->pos++;

    return result;
}

const char* json_get_string_field(JsonParser* jp, const char* field_name) {
    (void)field_name;
    const char* str_value = json_parse_string(jp);
    if (!str_value) return NULL;
    return str_value;
}

void json_parser_free(JsonParser* jp) {
    cleanbit(jp);
}

/* Load entire file into memory */
char* load_file_content(const char* path, size_t* out_size) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size > MAX_FILE_SIZE) {
        fclose(f);
        return NULL;
    }

    char* buffer = MemoryAllocate(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    size_t read_size = fread(buffer, 1, size, f);
    fclose(f);

    if (read_size != (size_t)size) {
        cleanbit(buffer);
        return NULL;
    }

    buffer[size] = '\0';
    if (out_size) *out_size = size;
    return buffer;
}

/* JSON-RPC response builders */
void respond_parse_ok(IutfNode* root) {
    JsonBuilder* jb = json_builder_new(4096);
    json_builder_append(jb, "{\"ok\":true,\"ast\":{");
    
    /* Simplified AST representation */
    if (root) {
        if (root->type == IUTF_NODE_BRANCH) {
            json_builder_append(jb, "\"type\":\"BRANCH\",\"items\":[");
            for (size i = 0; i < root->data.branch.size; i++) {
                if (i > 0) json_builder_append(jb, ",");
                json_builder_append(jb, "{\"key\":");
                json_builder_append_escaped(jb, root->data.branch.items[i]->key);
                json_builder_append(jb, "}");
            }
            json_builder_append(jb, "]");
        } else {
            json_builder_append(jb, "\"type\":\"VALUE\"");
        }
    }
    
    json_builder_append(jb, "}}");
    char* response = json_builder_finish(jb);
    printf("%s\n", response);
    fflush(stdout);
    cleanbit(response);
}

void respond_parse_error(const char* message) {
    JsonBuilder* jb = json_builder_new(1024);
    json_builder_append(jb, "{\"ok\":false,\"error\":");
    json_builder_append_escaped(jb, message);
    json_builder_append(jb, "}");
    char* response = json_builder_finish(jb);
    printf("%s\n", response);
    fflush(stdout);
    cleanbit(response);
}

void respond_validate_ok(int error_count) {
    JsonBuilder* jb = json_builder_new(256);
    json_builder_append(jb, "{\"ok\":true,\"errors\":");
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", error_count);
    json_builder_append(jb, buf);
    json_builder_append(jb, "}");
    char* response = json_builder_finish(jb);
    printf("%s\n", response);
    fflush(stdout);
    cleanbit(response);
}

void respond_invalid_request(const char* reason) {
    JsonBuilder* jb = json_builder_new(512);
    json_builder_append(jb, "{\"error\":\"Invalid request: ");
    json_builder_append_escaped(jb, reason);
    json_builder_append(jb, "\"}");
    char* response = json_builder_finish(jb);
    printf("%s\n", response);
    fflush(stdout);
    cleanbit(response);
}

/* JSON-RPC request handler */
void handle_json_rpc_request(const char* json_line) {
    JsonParser* jp = json_parser_new(json_line);

    /* Parse "action" field */
    if (!json_match(jp, "{")) {
        respond_invalid_request("Expected '{'");
        json_parser_free(jp);
        return;
    }
    jp->pos++;

    /* Look for "action" field */
    json_skip_whitespace(jp);
    
    const char* action = NULL;
    const char* content = NULL;
    const char* lang = NULL;

    /* Very simple field parsing — not a full JSON parser */
    while (jp->pos < jp->len && jp->str[jp->pos] != '}') {
        json_skip_whitespace(jp);

        if (jp->str[jp->pos] == '"') {
            const char* field = json_parse_string(jp);
            json_skip_whitespace(jp);

            if (jp->pos < jp->len && jp->str[jp->pos] == ':') {
                jp->pos++;
                json_skip_whitespace(jp);

                if (strcmp(field, "action") == 0) {
                    action = json_parse_string(jp);
                } else if (strcmp(field, "content") == 0) {
                    content = json_parse_string(jp);
                } else if (strcmp(field, "lang") == 0) {
                    lang = json_parse_string(jp);
                } else {
                    /* Skip unknown field */
                    if (jp->str[jp->pos] == '"') {
                        json_parse_string(jp);
                    } else if (jp->str[jp->pos] == '{' || jp->str[jp->pos] == '[') {
                        /* Skip nested object/array */
                        int depth = 1;
                        jp->pos++;
                        while (jp->pos < jp->len && depth > 0) {
                            if (jp->str[jp->pos] == '{' || jp->str[jp->pos] == '[') depth++;
                            else if (jp->str[jp->pos] == '}' || jp->str[jp->pos] == ']') depth--;
                            jp->pos++;
                        }
                    }
                }

                cleanbit((void*)field);
            }
        }

        json_skip_whitespace(jp);
        if (jp->pos < jp->len && jp->str[jp->pos] == ',') {
            jp->pos++;
        }
    }

    /* Validate request */
    if (!action) {
        respond_invalid_request("Missing 'action' field");
        json_parser_free(jp);
        return;
    }

    if (!content) {
        respond_invalid_request("Missing 'content' field");
        json_parser_free(jp);
        return;
    }

    /* Handle actions */
    if (strcmp(action, "parse") == 0) {
        IutfLexer lexer = {0};
        iutf_lexer_new(content);

        IutfParser parser = {0};
        iutf_parser_new(content);

        IutfNode* root = iutf_parse(&parser);

        if (root) {
            respond_parse_ok(root);
            iutf_node_free(root);
        } else {
            respond_parse_error(parser.Emessage ? parser.Emessage : "Parse error");
        }

        iutf_parser_free(&parser);
        iutf_lexer_corrupt(&lexer);

    } else if (strcmp(action, "validate") == 0) {
        IutfLexer lexer = {0};
        iutf_lexer_new(content);

        IutfParser parser = {0};
        iutf_parser_new(content);

        IutfNode* root = iutf_parse(&parser);

        if (root) {
            /* Run validator */
            int error_count = iutf_validate(root, &lexer);

            respond_validate_ok(error_count);

            iutf_node_free(root);
        } else {
            respond_parse_error(parser.Emessage ? parser.Emessage : "Parse error");
        }

        iutf_parser_free(&parser);
        iutf_lexer_corrupt(&lexer);

    } else {
        respond_invalid_request("Unknown action");
    }

    json_parser_free(jp);
    cleanbit((void*)action);
    cleanbit((void*)content);
    cleanbit((void*)lang);
}

/* JSON-RPC server loop */
void json_rpc_server_loop(void) {
    char buffer[MAX_JSON_BUFFER];

    fprintf(stderr, "[iutf-parser] JSON-RPC mode started, reading from stdin\n");
    fflush(stderr);

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        /* Remove trailing newline */
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        if (strlen(buffer) == 0) continue;

        handle_json_rpc_request(buffer);
    }

    fprintf(stderr, "[iutf-parser] stdin closed, shutting down\n");
    fflush(stderr);
}

/* CLI Mode */

typedef struct {
    int check_ext;
    int setup_ext;
    const char* filename;
} CliArgs;

CliArgs parse_cli_args(int argc, char* argv[]) {
    CliArgs args = {0, 0, NULL};

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--check-ext") == 0) {
            args.check_ext = 1;
        } else if (strcmp(argv[i], "--setup-ext") == 0) {
            args.setup_ext = 1;
        } else if (strcmp(argv[i], "--json-rpc") == 0) {
            /* JSON-RPC mode is handled separately */
            continue;
        } else if (argv[i][0] != '-') {
            args.filename = argv[i];
        }
    }

    return args;
}

char* find_ext_name_in_ast(IutfNode* root) {
    if (!root || root->type != IUTF_NODE_BRANCH) {
        return NULL;
    }

    for (size i = 0; i < root->data.branch.size; i++) {
        IutfNode* item = root->data.branch.items[i];
        if (strcmp(item->key, "extension") == 0 && 
            item->key and item->type == IUTF_NODE_BRANCH) {
            
            /* extension: { name: ... } */
            if (item->data.branch.size > 0) {
                return item->data.branch.items[0]->key;
            }
        }
    }

    return NULL;
}

void cmd_check_ext(const char* filename) {
    printf("Checking extension: %s\n", filename);

    char* content = load_file_content(filename, NULL);
    if (!content) {
        fprintf(stderr, "Error: Cannot read file %s\n", filename);
        return;
    }

    IutfLexer lexer = {0};
    iutf_lexer_new(content);

    IutfParser parser = {0};
    iutf_parser_new(content);

    IutfNode* root = iutf_parse(&parser);

    if (!root) {
        fprintf(stderr, "Parse error: %s\n", parser.Emessage);
        iutf_parser_free(&parser);
        iutf_lexer_corrupt(&lexer);
        cleanbit(content);
        return;
    }

    /* Validate */
    int error_count = iutf_validate(root, &lexer);

    
        /* Find extension name */
        char* ext_name = find_ext_name_in_ast(root);
        if (ext_name) {
            printf("Extension: %s\n", ext_name);
            printf("Status: VALID\n");
        } else {
            fprintf(stderr, "Warning: Could not find extension name in AST\n");
            printf("Status: VALID (but no extension found)\n");
        }

    iutf_node_free(root);
    iutf_parser_free(&parser);
    iutf_lexer_corrupt(&lexer);
    cleanbit(content);
}

void cmd_setup_ext(ustring filename) {
    printf("Setting up extension from: %s\n", filename);

    string content = load_file_content(filename, NULL);
    if (!content) {
        fprintf(stderr, "Error: Cannot read file %s\n", filename);
        return;
    }

    IutfLexer lexer = {0};
    iutf_lexer_new(content);

    IutfParser parser = {0};
    iutf_parser_new(content);

    IutfNode* root = iutf_parse(&parser);

    if (!root) {
        fprintf(stderr, "Parse error: %s\n", parser.Emessage);
        iutf_parser_free(&parser);
        iutf_lexer_corrupt(&lexer);
        cleanbit(content);
        return;
    }

    /* Validate */
    int error_count = iutf_validate(root, &lexer);

    if (error_count > 0) {
        fprintf(stderr, "Validation failed, cannot install\n");
        iutf_node_free(root);
        iutf_parser_free(&parser);
        iutf_lexer_corrupt(&lexer);
        cleanbit(content);
        return;
    }

    /* Find extension name and copy to IUTF_INCLUDE_PATH */
    string ext_name = find_ext_name_in_ast(root);
    if (!ext_name) {
        fprintf(stderr, "Error: Could not find extension name\n");
        iutf_node_free(root);
        iutf_parser_free(&parser);
        iutf_lexer_corrupt(&lexer);
        cleanbit(content);
        return;
    }

    ustring include_path = getenv("IUTF_INCLUDE_PATH");
    if (!include_path) {
        include_path = "/usr/include";
    }

    /* Create target directory */
    char target_dir[512];
    snprintf(target_dir, sizeof(target_dir), "%s/%s", include_path, ext_name);
    mkdir(target_dir, 0755);

    char target_file[512];
    snprintf(target_file, sizeof(target_file), "%s/%s.utext", target_dir, ext_name);

    /* Copy file */
    if (rename(filename, target_file) != 0) {
        /* Fallback: copy + delete */
        FILE* src = fopen(filename, "rb");
        FILE* dst = fopen(target_file, "wb");

        if (src && dst) {
            char buf[4096];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
                fwrite(buf, 1, n, dst);
            }
            fclose(src);
            fclose(dst);
            unlink(filename);
            printf("✓ Extension installed to %s\n", target_file);
        } else {
            fprintf(stderr, "Error: Cannot copy file\n");
            if (src) fclose(src);
            if (dst) fclose(dst);
        }
    } else {
        printf("✓ Extension installed to %s\n", target_file);
    }

    iutf_node_free(root);
    iutf_parser_free(&parser);
    iutf_lexer_corrupt(&lexer);
    cleanbit(content);
}

void show_help(void) {
    printf("iutf-parser" COL_CYAN " v%s\n\n" COL_DEF, VERSION);
    printf(COL_PRPL "Usage" COL_DEF ": iutf-parser <file> [options]\n");
    printf("  iutf-parser --json-rpc                  JSON-RPC mode\n");
    printf("  iutf-parser --check-ext <file>          Check extension syntax\n");
    printf("  iutf-parser --setup-ext <file>          Install extension\n");
    printf("  iutf-parser --version                   Show version\n");
    printf("  iutf-parser --help                      Show this help\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        show_help();
        return 1;
    }

    /* Check for JSON-RPC mode */
    if (strcmp(argv[1], "--json-rpc") == 0) {
        json_rpc_server_loop();
        return 0;
    }

    /* Parse CLI arguments */
    CliArgs args = parse_cli_args(argc, argv);

    if (strcmp(argv[1], "--version") == 0) {
        printf("iutf-parser " COL_CYAN "v%s\n" COL_DEF, VERSION);
        return 0;
    }

    if (strcmp(argv[1], "--help") == 0) {
        show_help();
        return 0;
    }

    if (args.check_ext && args.filename) {
        cmd_check_ext(args.filename);
        return 0;
    }

    if (args.setup_ext && args.filename) {
        cmd_setup_ext(args.filename);
        return 0;
    }

    fprintf(stderr, COL_RED "Error" COL_DEF ": Invalid arguments\n");
    show_help();
    return 1;
}