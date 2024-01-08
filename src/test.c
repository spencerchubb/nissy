#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "python.h"

int main(int argc, char **argv) {
    int num_correct = 0;
    int num_tests = 0;

    // Single group
    num_tests++;
    if (find_xexc("4e4c", 4, 4)) num_correct++;

    num_tests++;
    if (!find_xexc("4e4c", 3, 3)) num_correct++;

    // No separator
    num_tests++;
    if (find_xexc("4e4c2e3c2e4c", 4, 4)) num_correct++;

    num_tests++;
    if (find_xexc("4e4c2e3c2e4c", 2, 3)) num_correct++;

    num_tests++;
    if (find_xexc("4e4c2e3c2e4c", 2, 4)) num_correct++;

    num_tests++;
    if (!find_xexc("4e4c2e3c2e4c", 3, 3)) num_correct++;

    // Comma separator
    num_tests++;
    if (find_xexc("4e4c,2e3c,2e4c", 4, 4)) num_correct++;

    num_tests++;
    if (find_xexc("4e4c,2e3c,2e4c", 2, 3)) num_correct++;

    num_tests++;
    if (find_xexc("4e4c,2e3c,2e4c", 2, 4)) num_correct++;

    num_tests++;
    if (!find_xexc("4e4c,2e3c,2e4c", 3, 3)) num_correct++;

    // Space separator
    num_tests++;
    if (find_xexc("4e4c 2e3c 2e4c", 4, 4)) num_correct++;

    num_tests++;
    if (find_xexc("4e4c 2e3c 2e4c", 2, 3)) num_correct++;

    num_tests++;
    if (find_xexc("4e4c 2e3c 2e4c", 2, 4)) num_correct++;

    num_tests++;
    if (!find_xexc("4e4c 2e3c 2e4c", 3, 3)) num_correct++;

    // Command and space separator
    num_tests++;
    if (find_xexc("4e4c, 2e3c, 2e4c", 4, 4)) num_correct++;

    num_tests++;
    if (find_xexc("4e4c, 2e3c, 2e4c", 2, 3)) num_correct++;

    num_tests++;
    if (find_xexc("4e4c, 2e3c, 2e4c", 2, 4)) num_correct++;

    num_tests++;
    if (!find_xexc("4e4c, 2e3c, 2e4c", 3, 3)) num_correct++;

    printf("Tests passed: %d/%d\n", num_correct, num_tests);
}
