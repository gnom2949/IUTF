#include <glib-2.0/glib.h>
#include <iutf/iutf-import.h>

static void test_import_find_file(void) {
    char* path = iutf_find_imported_file("fpackage");
    g_assert_nonnull(path);
    g_free(path);
}

int main(int argc, char* argv[]) {
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/import/find_file", test_import_find_file);
    return g_test_run();
}
