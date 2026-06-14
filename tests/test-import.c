#include <glib-2.0/glib.h>
#include <glib-2.0/glib/gfileutils.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../subprojects/imm/src/IntMemoryManager.h"
#include <stddef.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include "../src/includes/iutf-import.h"

#define TEST_BASE_DIR "/tmp/test-iutf"
#define TEST_INCLUDE_PATH TEST_BASE_DIR "/include"
#define TRUE 1
#define FALSE 0

/* Helper: write file content */
static void write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "%s", content);
    fclose(f);
}

/* Helper: create directory recursively */
static void mkdir_recursive(const char* path) {
    char tmp[256];
    char* p = NULL;
    size_t len;
    
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    
    if (tmp[len - 1] == '/') tmp[len - 1] = 0;
    
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

/* Fixture setup */
static void test_import_setup(void) {
    /* Create test directory structure */
    mkdir_recursive(TEST_INCLUDE_PATH "/fpack");
    mkdir_recursive(TEST_INCLUDE_PATH "/networking");
    mkdir_recursive(TEST_INCLUDE_PATH "/invalid");

    /* Create valid extension files */
    write_file(
        TEST_INCLUDE_PATH "/fpack/fpack.utext",
        "iutf:extension:fpack { keywords: [fn, let] }"
    );
    
    write_file(
        TEST_INCLUDE_PATH "/networking/networking.utext",
        "iutf:extension:networking { operators: [+, -] }"
    );

    write_file(
        TEST_INCLUDE_PATH "/networking/pst.utext",
        "iutf:preset:network { types: [Int, String] }"
    );

    /* Set environment for test */
    setenv("IUTF_INCLUDE_PATH", TEST_INCLUDE_PATH, 1);
}

static void test_import_teardown(void) {
    /* Remove all test files and directories */
    char path[256];
    
    /* Clean networking */
    remove(TEST_INCLUDE_PATH "/networking/networking.utext");
    remove(TEST_INCLUDE_PATH "/networking/pst.utext");
    rmdir(TEST_INCLUDE_PATH "/networking");

    /* Clean fpack */
    remove(TEST_INCLUDE_PATH "/fpack/fpack.utext");
    rmdir(TEST_INCLUDE_PATH "/fpack");

    /* Clean invalid */
    rmdir(TEST_INCLUDE_PATH "/invalid");

    /* Clean include */
    rmdir(TEST_INCLUDE_PATH);
    rmdir(TEST_BASE_DIR);

    /* Unset environment */
    unsetenv("IUTF_INCLUDE_PATH");
}

/* Test: Find valid extension file */
static void test_import_find_file_valid(void) {
    char* path = iutf_find_imported_file("fpack");
    
    g_assert_nonnull(path);
    g_assert_true(g_file_test(path, G_FILE_TEST_EXISTS));
    g_assert_true(g_str_has_suffix(path, "fpack/fpack.utext"));
    
    cleanbit(path);
}

/* Test: Find file with pst.utext fallback */
static void test_import_find_file_pst_fallback(void) {
    char* path = iutf_find_imported_file("networking");
    
    g_assert_nonnull(path);
    g_assert_true(g_file_test(path, G_FILE_TEST_EXISTS));
    /* Should find either networking.utext or pst.utext */
    g_assert_true(
        g_str_has_suffix(path, "networking.utext") ||
        g_str_has_suffix(path, "pst.utext")
    );
    
    cleanbit(path);
}

/* Test: File not found returns NULL */
static void test_import_find_file_not_found(void) {
    char* path = iutf_find_imported_file("nonexistent");
    g_assert_null(path);
}

/* Test: Invalid directory returns NULL */
static void test_import_find_file_empty_dir(void) {
    char* path = iutf_find_imported_file("invalid");
    g_assert_null(path);
}

/* Test: Empty name returns NULL */
static void test_import_find_file_empty_name(void) {
    char* path = iutf_find_imported_file("");
    g_assert_null(path);
}

/* Test: NULL name returns NULL */
static void test_import_find_file_null_name(void) {
    char* path = iutf_find_imported_file(NULL);
    g_assert_null(path);
}

/* Test: Default IUTF_INCLUDE_PATH (if not set) */
static void test_import_find_file_default_path(void) {
    g_unsetenv("IUTF_INCLUDE_PATH");
    /* Should default to /usr/include, we don't test if files exist there */
    /* Just verify function doesn't crash */
    iutf_find_imported_file("nonexistent_pkg_definitely");
    g_assert_true(TRUE);
}

/* Test: Path with trailing slash */
static void test_import_find_file_trailing_slash(void) {
    g_setenv("IUTF_INCLUDE_PATH", TEST_INCLUDE_PATH "/", TRUE);
    char* path = iutf_find_imported_file("fpack");
    
    g_assert_nonnull(path);
    g_assert_true(g_file_test(path, G_FILE_TEST_EXISTS));
    
    cleanbit(path);
}

int main(int argc, char* argv[]) {
    g_test_init(&argc, &argv, NULL);

    /* Setup once for all tests */
    test_import_setup();

    /* Register tests */
    g_test_add_func("/import/find_file_valid", test_import_find_file_valid);
    g_test_add_func("/import/find_file_pst_fallback", test_import_find_file_pst_fallback);
    g_test_add_func("/import/find_file_not_found", test_import_find_file_not_found);
    g_test_add_func("/import/find_file_empty_dir", test_import_find_file_empty_dir);
    g_test_add_func("/import/find_file_empty_name", test_import_find_file_empty_name);
    g_test_add_func("/import/find_file_null_name", test_import_find_file_null_name);
    g_test_add_func("/import/find_file_default_path", test_import_find_file_default_path);
    g_test_add_func("/import/find_file_trailing_slash", test_import_find_file_trailing_slash);

    int result = g_test_run();
    
    /* Cleanup after all tests */
    test_import_teardown();

    return result;
}