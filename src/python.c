#include "shell.h"

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

typedef struct {
    char *stepName;
    char *scrambleString;
    char *rzps;
    SolveOptions *opts;
} SolveArgs;

char* intToTernary(int num) {
    char* ternary = (char*)malloc(9); // 8 digits + null terminator

    // Convert the integer to ternary
    for (int i = 7; i >= 0; i--) {
        ternary[i] = (num % 3) + '0';
        num /= 3;
    }
    ternary[8] = '\0';

    return ternary;
}

static int
count_bad_corners(int co)
{
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

static int count_bad_edges(int epos) {
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

static bool
find_xexc(const char *rzps, int e, int c)
{
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

static bool
check_rzp(Cube cube, char *rzps)
{
    // RZP is solved if both are satisfied:
    // 1. At lesat one of (eofb, eorl, eoud) is 0.
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

    // The algs are redundant if both conditions are satisfied::
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
void append_rzp_sols(Alg *sol, AlgList *sols, Cube cube, char *rzps, int maxMoves, Move moves[], int numMoves) {
    if (maxMoves <= 0) {
        return;
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

        if (check_rzp(newCube, rzps) && !alg_is_redundant_in_algs(newSol, sols)) {
            append_alg(sols, newSol);
        }

        append_rzp_sols(newSol, sols, newCube, rzps, maxMoves - 1, moves, numMoves);
        i++;
    }

    return;
}

// Give a cube, return all RZP solutions up to maxMoves.
AlgList* solve_rzp(Cube cube, char *rzps, int maxMoves) {
    Alg *sol = new_alg("");
    AlgList *sols = new_alglist();

    // If eo is solved on fb axis
    if (cube.eofb == 0) {
        Move moves[] = {U, U2, U3, D, D2, D3, R, R2, R3, L, L2, L3, F2, B2};
        append_rzp_sols(sol, sols, cube, rzps, maxMoves, moves, 14);
    }

    // If eo is solved on rl axis
    if (cube.eorl == 0) {
        Move moves[] = {U, U2, U3, D, D2, D3, F, F2, F3, B, B2, B3, R2, L2};
        append_rzp_sols(sol, sols, cube, rzps, maxMoves, moves, 14);
    }

    // If eo is solved on ud axis
    if (cube.eoud == 0) {
        Move moves[] = {F, F2, F3, B, B2, B3, R, R2, R3, L, L2, L3, U2, D2};
        append_rzp_sols(sol, sols, cube, rzps, maxMoves, moves, 14);
    }

    return sols;
}

/*
* Based on 'solve_exec' from commands.c
* For opts, see struct solveoptions in cubetypes.h
*/
char**
python_solve(SolveArgs solveArgs) {
    char *stepName = solveArgs.stepName;
    char *scrambleString = solveArgs.scrambleString;
    char *rzps = solveArgs.rzps;
    SolveOptions *opts = solveArgs.opts;

    // Print statements for debugging
    // printf("stepName: %s\n", stepName);
    // printf("scrambleString: %s\n", scrambleString);
    // printf("rzps: %s\n", rzps);
    // printf("min_moves: %d\n", opts->min_moves);
    // printf("max_moves: %d\n", opts->max_moves);
    // printf("max_solutions: %d\n", opts->max_solutions);
    // printf("nthreads: %d\n", opts->nthreads);
    // printf("optimal: %d\n", opts->optimal);
    // printf("niss_type: %d\n", opts->nisstype);
    // printf("verbose: %d\n", opts->verbose);
    // printf("all: %d\n", opts->all);
    // printf("print_number: %d\n", opts->print_number);
    // printf("count_only: %d\n", opts->count_only);

    Alg *scramble = new_alg(scrambleString);

	init_all_movesets();
	init_symcoord();

    Cube c = apply_alg(scramble, (Cube){0});
    printf("epose: %d, eposs: %d, eposm: %d, eofb: %d, eorl: %d, eoud: %d, coud: %d, cofb: %d, corl: %d, cp: %d, cpos: %d\n", c.epose, c.eposs, c.eposm, c.eofb, c.eorl, c.eoud, c.coud, c.cofb, c.corl, c.cp, c.cpos);

    if (strcmp(stepName, "rzp") == 0) {
        AlgList *alglist = solve_rzp(c, rzps, 3);
        return alglist_to_strings(alglist);
    }

    Step *step = getStep(stepName);
    if (step == NULL) {
        printf("Error: Step '%s' not found\n", stepName);
        return NULL;
    }

    AlgList* alglist = solve(c, step, opts);

    // It is a lot more complicated to return the AlgList directly.
    // We would have to use ctypes to define AlgList, AlgNode, Alg, Move, and maybe even more structs.
    // That is why we are just returning strings.
    return alglist_to_strings(alglist);
}

/* Based on 'scramble_exec' from commands.c */
char*
python_scramble(char *scrtype) {
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
    scr = solve_2phase(cube, 1);

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