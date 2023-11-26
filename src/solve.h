#ifndef SOLVE_H
#define SOLVE_H

#include "moves.h"
#include "steps.h"
#include "trans.h"

typedef struct SolveOutput {
    AlgList *sols;
    char *error_msg;
} SolveOutput;

SolveOutput *solve_output_new(AlgList *sols, char *error_msg);

SolveOutput *   solve(Cube cube, Step *step, SolveOptions *opts);
Alg *       solve_2phase(Cube cube, int nthreads);

#endif
