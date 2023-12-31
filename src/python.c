#include "alg.h"
#include "solve.h"

/* Functions that are designed to be called in python */

Step* getStep(char* stepName) {
    for (int i = 0; steps[i] != NULL; i++) {
        if (strcmp(stepName, steps[i]->shortname) == 0) {
            return steps[i];
        }
    }
    return NULL;
}

/* Similar to print_alg defined in alg.c except this returns a string */
char*
alg_to_string(Alg *alg) {
    char *result = malloc(1000); // Adjust the size as needed
    char fill[4];
    int i;
    bool niss = false;

    result[0] = '\0'; // Initialize result string

    for (i = 0; i < alg->len; i++) {
        if (!niss && alg->inv[i])
            strcpy(fill, i == 0 ? "(" : " (");
        if (niss && !alg->inv[i])
            strcpy(fill, ") ");
        if (niss == alg->inv[i])
            strcpy(fill, i == 0 ? "" : " ");

        strcat(result, fill);
        strcat(result, move_string(alg->move[i]));

        niss = alg->inv[i];
    }

    if (niss)
        strcat(result, ")");

    char alg_len[5];
    sprintf(alg_len, " (%d)", alg->len);
    strcat(result, alg_len);

    return result;
}

/* Similar to print_alglist defined in alg.c except it returns a list of strings */
char**
alglist_to_strings(AlgList *alglist) {
    char **result = malloc((alglist->len + 1) * sizeof(char*)); // Adjust the size as needed

    int resultLen = 0;
    for (AlgListNode *i = alglist->first; i != NULL; i = i->next, resultLen++) {
        char *alg_string = alg_to_string(i->alg);
        result[resultLen] = alg_string;
    }

    result[resultLen] = NULL; // Add NULL to the end of the array

    return result;
}

typedef struct StepData {
    char *key;
    char *value;
} StepData;


typedef struct SolveStep {
    char *name;
    char *shortname;
    struct StepData *data;
    int datalen;
} SolveStep;

typedef struct SolveArgs {
    struct SolveStep *steps;
    int num_steps;
    char *scramble;
    int nisstype;
} SolveArgs;

int count_bad_corners(int co) {
    int totalOrientation = 0;
    int badCorners = 0;

    for (int i = 0; i < 7; i++) {
        int ori = co % 3;
        co /= 3;
        totalOrientation += ori;
        if (ori != 0) {
            badCorners++;
        }
    }

    // The 8th corner needs to be inferred from the other 7.
    if (totalOrientation % 3 != 0) {
        badCorners++;
    }

    return badCorners;
}

int count_bad_edges(int epos) {
    int *ep = malloc(12 * sizeof(int));

    // This function will populate ep with 12 0s or 1s.
    // 0s are non-slice edges and 1s are slice edges.
    index_to_subset(epos / FACTORIAL4, 12, 4, ep);

    int count = 0;
    for (int i = 8; i < 12; i++) {
        count += ep[i];
    }

    free(ep);

    // Multiply by 2 because for every edge not in its slice,
    // that means there are 2 bad edges.
    return count * 2;
}

bool find_xexc(char *rzps, int e, int c) {
    int rzps_len = strlen(rzps);
    if (rzps_len % 4 != 0) {
        // Invalid rzps string.
        // rzps should be combinations of xexc
        return false;
    }

    for (int i = 0; i < rzps_len / 4; i++) {
        if (rzps[i] == e + '0' && rzps[i * 4 + 2] == c + '0') {
            return true;
        }
    }

    return false;
}

bool check_rzp(Cube cube, char *rzps) {
    // RZP is solved if both are satisfied:
    // 1. At least one of (eofb, eorl, eoud) is 0.
    // 2. One of the other COs matches a user-specified RZP.

    int badCornersUd = count_bad_corners(cube.coud);
    int badCornersRl = count_bad_corners(cube.corl);
    int badCornersFb = count_bad_corners(cube.cofb);

    int badEdgesE = count_bad_edges(cube.epose);
    int badEdgesM = count_bad_edges(cube.eposm);
    int badEdgesS = count_bad_edges(cube.eposs);

    bool rzpUd = find_xexc(rzps, badEdgesE, badCornersUd);
    bool rzpRl = find_xexc(rzps, badEdgesM, badCornersRl);
    bool rzpFb = find_xexc(rzps, badEdgesS, badCornersFb);

    if (cube.eofb == 0 && (rzpUd || rzpRl)) {
        return true;
    }

    if (cube.eorl == 0 && (rzpUd || rzpFb)) {
        return true;
    }

    if (cube.eoud == 0 && (rzpFb || rzpRl)) {
        return true;
    }

    return false;
}

bool jzp_corners(int co, int array[8]) {
    int totalOrientation = 0;
    int jzpCorners = 0;

    int i;
    int c;
    for (i = 0; i < 7; i++) {
        c = co % 3;
        totalOrientation += c;
        co /= 3;
        if (c == array[i]) {
            jzpCorners++;
        } else if (c == 0) {
            // Do nothing
        } else {
            // Not JZP
            return false;
        }
    }

    // The 8th corner needs to be inferred from the other 7.
    c = totalOrientation % 3;
    if (c == array[i]) {
        jzpCorners++;
    } else if (c == 0) {
        // Do nothing
    } else {
        // Not JZP
        return false;
    }

    bool evenJzpCorners = jzpCorners % 2 == 0;
    return evenJzpCorners;
}

bool jzp_edges(int epos, bool otherSlice) {
    int *ep = malloc(12 * sizeof(int));

    // This function will populate ep with 12 0s or 1s.
    // 0s are non-slice edges and 1s are slice edges.
    index_to_subset(epos / FACTORIAL4, 12, 4, ep);

    // If otherSlice is true, we check indices 1, 3, 5, 7.
    // Else, we check indices 0, 2, 4, 6.
    bool eSliceEdgeInMSlice = ep[0 + otherSlice] || ep[2 + otherSlice] || ep[4 + otherSlice] || ep[6 + otherSlice];
    free(ep);
    return !eSliceEdgeInMSlice;
}


/** Scrambles:
F B2 L2 D R2 U' R2 U' F2 R2 U2 R' F2 D B D' F L' F' R2
D2 B' L2 B2 U2 R2 U L2 D R2 U2 L2 F2 B' U' F U2 F2 L D U
R B2 D2 R2 B2 L' F2 U2 L2 F2 D L2 R F U' R2 D2 F2 L
U L2 B2 D2 R U B' L' U' B U L2 F2 R2 U D2 B2 D' F2 R2 U' F2
*/

/** FB EOs:
F B2 L2 D R2 U' R2 U' F2 R2 U2 R' F2 D B D' F L' F' R2 U' F U D' B
D2 B' L2 B2 U2 R2 U L2 D R2 U2 L2 F2 B' U' F U2 F2 L U L2 B F' U F
R B2 D2 R2 B2 L' F2 U2 L2 F2 D L2 R F U' R2 D2 F2 L R U' R' B
U L2 B2 D2 R U B' L' U' B U L2 F2 R2 U D2 B2 D' F2 R2 U' F2 R' D' B
*/

/** RL EOs:
F B2 L2 D R2 U' R2 U' F2 R2 U2 R' F2 D B D' F L' F' R2 L B' R2 D R
D2 B' L2 B2 U2 R2 U L2 D R2 U2 L2 F2 B' U' F U2 F2
R B2 D2 R2 B2 L' F2 U2 L2 F2 D L2 R F U' R2 D2 F2 L R U' R' B U L R D' R
U L2 B2 D2 R U B' L' U' B U L2 F2 R2 U D2 B2 D' F2 R2 U' F2 R' D' B L' B2 U D L
*/

/** UD EOs:
F B2 L2 D R2 U' R2 U' F2 R2 U2 R' F2 D B D' F L' F' R D R B' F U
D2 B' L2 B2 U2 R2 U L2 D R2 U2 L2 F2 B' U' F U2 F2 L D U R B' D F U
R B2 D2 R2 B2 L' F2 U2 L2 F2 D L2 R F U' R2 D2 F2 U F R' D
U L2 B2 D2 R U B' L' U' B U L2 F2 R2 U D2 B2 D' F2 R2 U' F2 D R U2 R2 U
*/


bool check_jzp(Cube cube) {
    // Letters 1 & 2: EO axis
    // Letters 3 & 4: CO axis
    int fbud[8] = {2, 1, 2, 1, 1, 2, 1, 1};
    int rlud[8] = {1, 2, 1, 2, 2, 1, 2, 2};
    int fbrl[8] = {1, 2, 1, 2, 2, 1, 2, 2};
    int udrl[8] = {2, 1, 2, 1, 1, 2, 1, 1};
    int udfb[8] = {1, 2, 1, 2, 2, 1, 2, 2};
    int rlfb[8] = {2, 1, 2, 1, 1, 2, 1, 1};

    if (cube.eofb == 0 && ((jzp_corners(cube.coud, fbud) && jzp_edges(cube.epose, 0)) || (jzp_corners(cube.corl, fbrl) && jzp_edges(cube.eposm, 0)))) {
        return true;
    }

    if (cube.eorl == 0 && ((jzp_corners(cube.coud, rlud) && jzp_edges(cube.epose, 1)) || (jzp_corners(cube.cofb, rlfb) && jzp_edges(cube.eposs, 1)))) {
        return true;
    }

    if (cube.eoud == 0 && ((jzp_corners(cube.cofb, udfb) && jzp_edges(cube.eposs, 0)) || (jzp_corners(cube.corl, udrl) && jzp_edges(cube.eposm, 1)))) {
        return true;
    }

    return false;
}

bool moves_are_same_face(Move m1, Move m2) {
    // If (m1-1)/3) == (m2-1)/3, they are same face.
    // Integer division in C floors the result.
    int face1 = (m1 - 1) / 3;
    int face2 = (m2 - 1) / 3;
    return face1 == face2;
}

bool moves_are_opposite_face(Move m1, Move m2) {
    // If (m1-1)/6 == (m2-1)/6, they are either same face OR opposite face.
    int axis1 = (m1 - 1) / 6;
    int axis2 = (m2 - 1) / 6;
    return axis1 == axis2 && !moves_are_same_face(m1, m2);
}

bool alg_is_redundant(Alg *alg1, Alg *alg2) {
    if (alg1->len != alg2->len) {
        return false;
    }

    // The algs are redundant if both conditions are satisfied:
    // 1. All moves except the last are the same
    // 2. The last moves are on the same face

    // Example of redundant algs: R U and R U'.

    int len = alg1->len;
    for (int i = 0; i < len - 1; i++) {
        if (alg1->move[i] != alg2->move[i]) {
            return false;
        }
    }

    return moves_are_same_face(alg1->move[len - 1], alg2->move[len - 1]);
}

bool alg_is_redundant_in_algs(Alg *alg, AlgList *algs) {
    for (AlgListNode *i = algs->first; i != NULL; i = i->next) {
        if (alg_is_redundant(alg, i->alg)) {
            return true;
        }
    }
    return false;
}

// If we are using recursion, we have to do maxMoves instead of maxSolutions.
// Otherwise, we will only search deep and not wide.
// Returns a boolean indicating whether it has surpassed the time limit.
bool append_rzp_sols(struct timespec start, Alg *sol, AlgList *sols, Cube cube, char *rzps, int maxMoves, bool jzp, Move moves[], int numMoves) {
    if (elapsed_ms(start) > TIME_LIMIT) {
        return true;
    }
    
    if (maxMoves <= 0) {
        return false;
    }

    for (int i = 0; i < numMoves; i++) {
        // Look at the last two moves to see if this move will be redundant.
        int len = sol->len;
        if (len >= 1) {
            int prev = sol->move[len - 1];

            if (moves_are_same_face(prev, moves[i])) {
                continue;
            }

            if (len >= 2
                && moves_are_opposite_face(prev, moves[i])
                && moves_are_same_face(sol->move[len - 2], moves[i])) {
                continue;
            }
        }

        Cube newCube = apply_move(moves[i], cube);

        Alg *newSol = new_alg("");
        copy_alg(sol, newSol);
        append_move(newSol, moves[i], false);


        bool step_solved = check_rzp(newCube, rzps) || (jzp && check_jzp(newCube));
        if (step_solved && !alg_is_redundant_in_algs(newSol, sols)) {
            append_alg(sols, newSol);

            char *alg_string = alg_to_string(newSol);
            free(alg_string);
        }

        append_rzp_sols(start, newSol, sols, newCube, rzps, maxMoves - 1, jzp, moves, numMoves);
    }

    return false;
}

// Given a cube, return all RZP solutions up to maxMoves.
SolveOutput* solve_rzp(struct timespec start, Cube cube, SolveOptions *opts) {
    char *rzps = opts->rzps;
    int maxMoves = opts->max_moves;
    bool jzp = opts->jzp;

    Alg *sol = new_alg("");
    AlgList *sols = new_alglist();
    
    bool eofb_solved = cube.eofb == 0;
    bool eorl_solved = cube.eorl == 0;
    bool eoud_solved = cube.eoud == 0;

    if (!eofb_solved && !eorl_solved && !eoud_solved) {
        char *msg = malloc(100);
        sprintf(msg, "Cannot do RZP yet, EO must be solved on at least 1 axis\n");
        return solve_output_new(sols, msg);
    }

    if (eofb_solved) {
        Move moves[] = {U, U2, U3, D, D2, D3, R, R2, R3, L, L2, L3, F2, B2};
        bool timed_out = append_rzp_sols(start, sol, sols, cube, rzps, maxMoves, jzp, moves, 14);
        if (timed_out) {
            return solve_output_new(sols, TIMEOUT_MSG);
        }
    }

    if (eorl_solved) {
        Move moves[] = {U, U2, U3, D, D2, D3, F, F2, F3, B, B2, B3, R2, L2};
        bool timed_out = append_rzp_sols(start, sol, sols, cube, rzps, maxMoves, jzp, moves, 14);
        if (timed_out) {
            return solve_output_new(sols, TIMEOUT_MSG);
        }
    }

    if (eoud_solved) {
        Move moves[] = {F, F2, F3, B, B2, B3, R, R2, R3, L, L2, L3, U2, D2};
        bool timed_out = append_rzp_sols(start, sol, sols, cube, rzps, maxMoves, jzp, moves, 14);
        if (timed_out) {
            return solve_output_new(sols, TIMEOUT_MSG);
        }
    }

    return solve_output_new(sols, NULL);
}

char *get_step_value(SolveStep step, char *key) {
    for (int i = 0; i < step.datalen; i++) {
        if (strcmp(step.data[i].key, key) == 0) {
            return step.data[i].value;
        }
    }
    return NULL;
}

SolveOutput *solve_one_step(struct timespec start, Cube cube, char *shortname, SolveOptions *opts) {
    if (strcmp(shortname, "rzp") == 0) {
        return solve_rzp(start, cube, opts);
    }

    Step *step = getStep(shortname);
    if (step == NULL) {
        char *msg = malloc(100);
        sprintf(msg, "Error: Step '%s' not found\n", shortname);
        fprintf(stderr, "%s", msg);
        return solve_output_new(new_alglist(), msg);
    }

    return solve(start, cube, step, opts);
}

SolveOutput *solve_helper(struct timespec start, Alg *scramble, AlgList *sols, SolveStep *steps, int step_index, int num_steps, int nisstype) {
    if (step_index >= num_steps) {
        return solve_output_new(sols, NULL);
    }

    SolveStep step = steps[step_index];
    char *step_name = step.name;

    SolveOptions *opts = malloc(sizeof(SolveOptions));
    opts->min_moves = 0;
    opts->max_moves = 20;
    opts->max_solutions = 10000; // A large number
    opts->nthreads = 1;
    opts->optimal = -1; // Allow suboptimal solutions
    opts->nisstype = nisstype;
    opts->verbose = 0;
    opts->all = 0;
    opts->print_number = 0;
    opts->count_only = 0;

    // Set options based on step
    if (strcmp(step_name, "EO") == 0) {
        opts->max_solutions = atoi(get_step_value(step, "num_eos"));
    } else if (strcmp(step_name, "RZP") == 0) {
        opts->max_moves = atoi(get_step_value(step, "num_rzp_moves"));
        opts->rzps = get_step_value(step, "rzps");
        opts->jzp = strcmp(get_step_value(step, "jzp"), "true") == 0;
    } else if (strcmp(step_name, "DR") == 0) {
        opts->max_solutions = atoi(get_step_value(step, "num_drs"));
    } else if (strcmp(step_name, "HTR") == 0) {
        opts->max_solutions = atoi(get_step_value(step, "num_htrs"));
    } else if (strcmp(step_name, "Leave Slice") == 0) {
        opts->max_solutions = atoi(get_step_value(step, "num_leave_slice"));
    } else if (strcmp(step_name, "Finish") == 0) {
        opts->max_solutions = atoi(get_step_value(step, "num_finishes"));
    }

    AlgList *new_sols = new_alglist();
    
    for (AlgListNode *i = sols->first; i != NULL; i = i->next) {
        Alg *alg = i->alg;
        Cube cube = apply_alg(scramble, (Cube){0});
        cube = apply_alg(alg, cube);
        SolveOutput *solve_output = solve_one_step(start, cube, step.shortname, opts);

        if (solve_output->error_msg != NULL) {
            return solve_output;
        }

        for (AlgListNode *j = solve_output->sols->first; j != NULL; j = j->next) {
            Alg *sol = j->alg;
            Alg *combined_sol = new_alg("");
            copy_alg(alg, combined_sol);
            compose_alg(combined_sol, sol);
            append_alg(new_sols, combined_sol);
        }
    }

    return solve_helper(start, scramble, new_sols, steps, step_index + 1, num_steps, nisstype);
}

char** python_solve(SolveArgs solveArgs) {
    // Previously we implemented the timeout in python, but it was not reliable.
    // Probably has something to do with the Global Interpreter Lock.
    // So now we do the timeout in C.
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    fprintf(stderr, "Solve,%ld.%09ld\n", start.tv_sec, start.tv_nsec);

    struct SolveStep *steps = solveArgs.steps;
    int num_steps = solveArgs.num_steps;
    char *scrambleString = solveArgs.scramble;
    int nisstype = solveArgs.nisstype;

    // Print statements for debugging
    // printf("scrambleString: %s\n", scrambleString);
    // printf("nisstype: %d\n", nisstype);
    // printf("num_steps: %d\n", num_steps);
    // for (int i = 0; i < num_steps; i++) {
    //     struct SolveStep step = steps[i];
    //     printf("  %s:\n", step.name);
    //     for (int j = 0; j < step.datalen; j++) {
    //         char *key = step.data[j].key;
    //         char *value = step.data[j].value;
    //         printf("    %s: %s\n", key, value);
    //     }
    // }

    Alg *scramble = new_alg(scrambleString);

	init_all_movesets();
	init_symcoord();

    AlgList *sols = new_alglist();
    append_alg(sols, new_alg("")); // Add empty alg so the for loop works
    SolveOutput *solve_output = solve_helper(start, scramble, sols, steps, 0, num_steps, nisstype);

    if (solve_output->error_msg != NULL) {
        // Return an array of strings with the error message
        char **strings = malloc(2 * sizeof(char*));
        strings[0] = solve_output->error_msg;
        strings[1] = NULL;
        return strings;
    }

    sort_alglist(solve_output->sols);

    return alglist_to_strings(solve_output->sols);
}

/* Based on 'scramble_exec' from commands.c */
char* python_scramble(char *scrtype) {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    fprintf(stderr, "Scramble,%ld.%09ld\n", start.tv_sec, start.tv_nsec);

    Cube cube;
	CubeArray *arr;
	Alg *scr, *ruf, *aux;
	int i, j, eo, ep, co, cp, a[12];
	int eparr[12] = { [8] = 8, [9] = 9, [10] = 10, [11] = 11 };
	uint64_t ui, uj, uk;

	init_all_movesets();
	init_symcoord();

	srand(time(NULL));

    if (!strcmp(scrtype, "dr")) {
        /* Warning: cube is inconsistent because of side CO  *
            * and EO on U/D. But solve_2phase only solves drfin *
            * in this case, so it should be ok.                 *
            * TODO: check this properly                         *
            * Moreover we again need to fix parity after        *
            * generating epose manually                         */
        do {
            ui = rand() % FACTORIAL8;
            uj = rand() % FACTORIAL8;
            uk = rand() % FACTORIAL4;

            index_to_perm(ui, 8, eparr);
            arr = malloc(sizeof(CubeArray));
            arr->ep = eparr;
            cube = arrays_to_cube(arr, pf_ep);
            free(arr);

            cube.cp = uj;
            cube.epose = uk;
        } while (!is_admissible(cube));
    } else if (!strcmp(scrtype, "htr")) {
        /* antindex_htrfin() returns a consistent *
            * cube, except possibly for parity       */
        do {
            ui = rand() % (24*24/6);
            cube = (Cube){0};
            cube.cp = cornershtrfin_ant[ui];
            cube.epose = rand() % 24;
            cube.eposs = rand() % 24;
            cube.eposm = rand() % 24;
        } while (!is_admissible(cube));
    } else {
        eo = rand() % POW2TO11;
        ep = rand() % FACTORIAL12;
        co = rand() % POW3TO7;
        cp = rand() % FACTORIAL8;

        if (!strcmp(scrtype, "eo")) {
            eo = 0;
        } else if (!strcmp(scrtype, "corners")) {
            eo = 0;
            ep = 0;
            index_to_perm(cp, 8, a);
            if (perm_sign(a, 8) == 1) {
                swap(&a[0], &a[1]);
                cp = perm_to_index(a, 8);
            }
        } else if (!strcmp(scrtype, "edges")) {
            co = 0;
            cp = 0;
            index_to_perm(ep, 12, a);
            if (perm_sign(a, 12) == 1) {
                swap(&a[0], &a[1]);
                ep = perm_to_index(a, 12);
            }
        }
        cube = fourval_to_cube(eo, ep, co, cp);
    }

    /* TODO: can be optimized for htr and dr using htrfin, drfin */
    scr = solve_2phase(start, cube, 1);

    if (!strcmp(scrtype, "fmc")) {
        aux = new_alg("");
        copy_alg(scr, aux);
        /* Trick to rufify for free: rotate the scramble  *
            * so that it does not start with F or end with R */
        for (j = 0; j < NROTATIONS; j++) {
            if (base_move(scr->move[0]) != F &&
                base_move(scr->move[0]) != B &&
                base_move(scr->move[scr->len-1]) != R &&
                base_move(scr->move[scr->len-1]) != L)
                break;
            copy_alg(aux, scr);
            transform_alg(j, scr);
        }
        copy_alg(scr, aux);
        ruf = new_alg("R' U' F");
        copy_alg(ruf, scr);
        compose_alg(scr, aux);
        compose_alg(scr, ruf);
        free_alg(aux);
        free_alg(ruf);
    }
    char *alg_string = alg_to_string(scr);
    free_alg(scr);
    return alg_string;
}