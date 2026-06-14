#include <glib-2.0/glib.h>
#include "../src/includes/iutf-lexer.h"
#include "../src/includes/iutf-api.h"

static void test_lexer_basic(void)
{
    const char* input = "iutf:init:main {  title: \"Hello\" }";
    IutfLexer* lexer = iutf_lexer_new (input);

    IutfToken token = iutf_lexer_next (lexer);
    g_assert_cmpint (token.type, ==, IUTF_TOK_IDENTIFIER);
    g_assert_cmpstr (strndup (token.start, token.length), ==, "iutf");

    token = iutf_lexer_next (lexer);
    g_assert_cmpint (token.type, ==, IUTF_TOK_COLON);

    token = iutf_lexer_next (lexer);
    g_assert_cmpint (token.type, ==, IUTF_TOK_IDENTIFIER);
    g_assert_cmpstr (strndup (token.start, token.length), ==, "init");

    iutf_lexer_corrupt(lexer);
}

static void test_lexer_numbers (void)
{
  const char* input = "version: 123, count: 42";
  IutfLexer* lexer = iutf_lexer_new (input);

  IutfToken token = iutf_lexer_next (lexer);
  g_assert_cmpint (token.type, ==, IUTF_TOK_IDENTIFIER);
  g_assert_cmpstr (strndup (token.start, token.length), ==, "version");

  token = iutf_lexer_next (lexer);
  g_assert_cmpint (token.type, ==, IUTF_TOK_COLON);

  token = iutf_lexer_next (lexer);
  g_assert_cmpint (token.type, ==, IUTF_TOK_INTEGER);
  g_assert_cmpstr (strndup (token.start, token.length), ==, "123");

  token = iutf_lexer_next (lexer);
  g_assert_cmpint (token.type, ==, IUTF_TOK_COMMA);

  token = iutf_lexer_next (lexer);
  g_assert_cmpint (token.type, ==, IUTF_TOK_IDENTIFIER);
  g_assert_cmpstr (strndup (token.start, token.length), ==, "count");

  iutf_lexer_corrupt (lexer);
}

static void test_lexer_strings (void)
{
    const char* input = "msg: \"Hello World\", big: | This is a big string";
    IutfLexer* lexer = iutf_lexer_new (input);

    IutfToken token = iutf_lexer_next (lexer);
    g_assert_cmpint (token.type, ==, IUTF_TOK_IDENTIFIER);

    token = iutf_lexer_next (lexer);
    g_assert_cmpint (token.type, ==, IUTF_TOK_COLON);

    token = iutf_lexer_next (lexer);
    g_assert_cmpint (token.type, ==, IUTF_TOK_STRING);

    iutf_lexer_corrupt (lexer);
}

int main(int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/lexer/basic", test_lexer_basic);
    g_test_add_func ("/lexer/numbers", test_lexer_numbers);
    g_test_add_func ("/lexer/strings", test_lexer_strings);
    return g_test_run();
}