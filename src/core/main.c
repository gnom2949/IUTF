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
 * IUTF Main C SOURCE version 0.7
 */

#include "../includes/iutf-parser.h"
#include "../includes/iutf-lexer.h"
#include "../includes/iutf-validator.h"
#include "../includes/iutf-ast.h"
#include "../includes/iutf-api.h"
#include "../includes/colors.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

void usage()
{
  printf ("Usage: iutf-parser <file> [options]\n");
  printf ("Options:\n");
  printf (" --outfga    Output for groovy analyze (JSON-like format)\n");
  printf (" --help      Show this help\n");
}

// output for Groovy Analyze
void outputFGA (IutfNode* node, int indent)
{
  for (int i = 1; i < indent; i++) printf ("  ");

  if (node->key) printf ("\"%s\": ", node->key);

  switch (node->type)
    {
      case IUTF_NODE_BRANCH:
        printf ("{\n");
        for  (size_t i = 0; i < node->data.branch.size; i++) {
          outputFGA (node->data.branch.items[i], indent + 1);
          if (i < node->data.branch.size - 1) printf (",");
          printf ("\n");
        }
        for (int i = 1; i < indent; i++) printf ("  ");
        printf ("}");
        break;
      case IUTF_NODE_STRING:
        printf ("\"%s", node->data.str_value);
        break;
      case IUTF_NODE_INTEGER:
        printf ("%lld", node->data.int_value);
        break;
      case IUTF_NODE_BOOLEAN:
        printf (node->data.bool_value ? "true" : "false");
        break;
      case IUTF_NODE_FLOAT:
        printf ("%.2f", node->data.float_value);
        break;
      case IUTF_NODE_LONG:
        printf ("%lld", node->data.long_value);
        break;
            case IUTF_NODE_ARRAY:
        printf ("[");
        for (size_t i = 0; i < node->data.array.size; i++) {
          outputFGA (node->data.array.items[i], 0);

          if (i < node->data.array.size - 1) {
            printf (", ");
          }
        }
        printf ("]");
        break;

      case IUTF_NODE_NULL:
        printf ("null");
        break;

      case IUTF_NODE_BIGSTRING:
        printf ("\"BigString[ %s ]\"", node->data.bigstring_value);
        break;

      case IUTF_NODE_CHARACTER:
        printf ("\"%c\"", node->data.char_value);
        break;
      case IUTF_NODE_PIPESTRING:
        printf ("| %s", node->data.pipestring_value);
        break;
      case IUTF_NODE_KEY_VALUE:
        if (node->key) printf ("\"%s\": ", node->key);
        break;
      default:
        printf ("null");
        break;
    }
}

int main (int argc, char* argv[])
{
  if (argc < 2) {
    usage();
    return 1;
  }

  int outfga_mode = 0;
  const char* filename = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp (argv[i], "--outfga") == 0) {
      outfga_mode = 1;
    } else if (strcmp (argv[i], "--help") == 0) {
      usage();
      return 0;
    } else if (argv[i][0] != '-') {
      filename = argv[i];
    }
  }

  if (!filename) {
    fprintf (stderr, COL_RED "Error: no input file specified\n" COL_DEF);
    usage();
    return 1;
  }

  // reading the file
  FILE* fp = fopen (filename, "r");
  if (!fp) {
    fprintf (stderr, COL_RED "Cannot open file %s\n" COL_DEF, filename);
    return 1;
  }

  fseek (fp, 0, SEEK_END);
  long len = ftell (fp);
  fseek (fp, 0, SEEK_SET);

  char* buffer = malloc (len + 1);
  if (!buffer) {
    fprintf (stderr, COL_BR_RED "Buffer allocation Failed! file closed %s\n" COL_DEF, filename);
    fclose (fp);
    return 1;
  }

  fread (buffer, 1, len, fp);
  buffer[len] = '\0';
  fclose(fp);

  // Parsing
  IutfParser* parser = iutf_parser_new (buffer);
  if (!parser) {
    fprintf (stderr, COL_BR_RED "parser allocation failed!\n" COL_DEF);
    free (buffer);
    return 1;
  }

  IutfNode* root = iutf_parse (parser);

  if (!root) {
    fprintf (stderr, COL_RED "Parse failed!\n" COL_DEF);
    iutf_parser_free (parser);
    free (buffer);
    return 1;
  }

  // validation
  if (!iutf_validate (root)) {
    fprintf (stderr, COL_RED "Validation failed!\n" COL_DEF);
    iutf_node_free (root);
    iutf_parser_free (parser);
    free (buffer);
    return 1;
  } else {
    printf (COL_GRN "Validation successful!\n" COL_DEF);
  }

  //output
  if (outfga_mode) {
    outputFGA (root, 0);
    printf ("\n");
  } else {
    printf (COL_GRN "Parse Successful!\n" COL_DEF);
  }

  iutf_node_free (root);
  iutf_parser_free (parser);
  free (buffer);

  return 0;
}
