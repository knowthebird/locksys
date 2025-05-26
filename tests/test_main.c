#include <stdio.h>

void test_template_example_one();
void test_template_example_two();

int main(void) {
    printf("Running LockSys unit tests...\n");

    test_template_example_one();
    test_template_example_two();

    return 0;
}
