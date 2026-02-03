#include <glib-2.0/glib.h>
#include <iutf/iutf-parser.h>

static void test_parser_basic(void) {
    const char* input = "iutf:init:main { title: \"Hello\" }";
    IutfParser* parser = iutf_parser_new(input);
    IutfNode* root = iutf_parse(parser);

    g_assert_nonnull(root);
    g_assert_cmpint(root->type, ==, IUTF_NODE_BRANCH);

    iutf_node_free(root);
    iutf_parser_free(parser);
}

int main(int argc, char* argv[]) {
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/parser/basic", test_parser_basic);
    return g_test_run();
}
