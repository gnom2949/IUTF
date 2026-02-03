#include <glib-2.0/glib.h>
#include <glib/gstdio.h>
#include <iutf/iutf-import.h>

static void test_import_find_file(void) {
    // create the temporary dir for test
    g_assert_true (g_mkdir_with_parents ("/tmp/test-iutf/fpack", 0755) == 0);
    g_assert_true (g_file_set_contents ("/tmp/test-iutf/fpack/fpack.utext", "iutf:extension:test {}", -1, NULL));

    // change IUTF_INCLUDE_PATH
    g_setenv ("IUTF_INCLUDE_PATH", "/tmp/test-iutf", TRUE);

    char* path = iutf_find_imported_file ("fpack");
    g_assert_nonnull (path);
    g_free (path);

    // delete the dir after test
    if (g_remove ("/tmp/test-iutf/fpack/fpack.utext") != 0) g_debug ("File not found or removed");
    if (g_rmdir ("/tmp/test-iutf/fpack") != 0) g_warning ("Failed to delete dir: %s !", g_strerror(errno));
    if (g_rmdir ("/tmp/test-iutf") != 0) g_warning ("Failed to delete dir: %s!", g_strerror(errno));
}

int main(int argc, char* argv[]) {
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/import/find_file", test_import_find_file);
    return g_test_run();
}
