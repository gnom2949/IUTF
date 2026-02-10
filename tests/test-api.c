#include <glib-2.0/glib.h>
#include "../src/includes/iutf-api.h"

static void test_api_create_branch (void)
{
  IutfNode* branch = iutf_new_branch();
  g_assert_nonnull (branch);
  g_assert_cmpint (branch->type, ==, IUTF_NODE_BRANCH);

  IutfNode* str = iutf_new_str ("Hello");
  iutf_add_branch (branch, "title", str);

  g_assert_cmpstr (branch->data.branch.items[0]->key, ==, "title");

  iutf_node_free (branch);
}

int main (int argc, char* argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/api/create_branch", test_api_create_branch);
  return g_test_run ();
}
