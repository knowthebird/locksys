#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

void test_template_example_one() {
    printf("test_template_example_one passes.\n");
    assert(true);
}

void test_template_example_two() {
    printf("test_template_example_two fails.\n");
    assert(false);
}
