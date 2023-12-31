#ifndef SOLVE_H
#define SOLVE_H

#include "moves.h"
#include "steps.h"
#include "trans.h"

typedef struct SolveOutput {
    AlgList *sols;
    char *error_msg;
} SolveOutput;

SolveOutput *   solve_output_new(AlgList *sols, char *error_msg);
void            solve_output_free(SolveOutput *so);
SolveOutput *   solve(struct timespec start, Cube cube, Step *step, SolveOptions *opts);
Alg *           solve_2phase(struct timespec start, Cube cube, int nthreads);

#endif
