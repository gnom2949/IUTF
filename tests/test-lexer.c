#include <glib-2.0/glib.h>
#include <iutf/iutf-lexer.h>

static void test_lexer_basic(void) {
    const char* input = "iutf:init:main { title: \"Hello\" }";
    IutfLexer* lexer = iutf_lexer_new(input);

    IutfToken token = iutf_lexer_next(lexer);
    g_assert_cmpint(token.type, ==, IUTF_TOK_IDENTIFIER);
    g_assert_cmpstr(token.start, ==, "iutf");

    token = iutf_lexer_next(lexer);
    g_assert_cmpint(token.type, ==, IUTF_TOK_COLON);

    iutf_lexer_corrupt(lexer);
}

int main(int argc, char* argv[]) {
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/lexer/basic", test_lexer_basic);
    return g_test_run();
}
