#include "alg.h"
#include "commands.h"
// #include "solve.h" TODO is this needed?

/* Functions that are designed to be called in python */

Step* getStep(char* stepName) {
    for (int i = 0; steps[i] != NULL; i++) {
        if (strcmp(stepName, steps[i]->shortname) == 0) {
            return steps[i];
        }
    }
    return NULL;
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

    // The number of bad edges is 8 - 2 * the number of slice edges.
    return 8 - 2 * count;
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_digit_or_ce(char c) {
    return is_digit(c) || c == 'c' || c == 'e';
}

bool find_xexc(char *rzps, int e, int c) {
    char edge = e + '0';
    char corner = c + '0';
    
    int len = strlen(rzps);
    int i = 0;
    while (i <= len - 4) {
        char c1 = rzps[i], c2 = rzps[i + 1], c3 = rzps[i + 2], c4 = rzps[i + 3];

        // Skip if these 4 characters are not the right format.
        if (!is_digit_or_ce(c1) || !is_digit_or_ce(c2) || !is_digit_or_ce(c3) || !is_digit_or_ce(c4)) {
            i++;
            continue;
        }

        if (c1 == edge && c2 == 'e' && c3 == corner && c4 == 'c') {
            return true;
        }

        if (c1 == corner && c2 == 'c' && c3 == edge && c4 == 'e') {
            return true;
        }

        i += 4;
    }

    return false;
}

bool check_rzp(Cube cube, char *rzps) {
    // RZP is solved if both are satisfied:
    // 1. At least one of (eofb, eorl, eoud) is 0.
    // 2. One of the other COs matches a user-specified RZP.

    int cornersUd = count_bad_corners(cube.coud);
    int cornersRl = count_bad_corners(cube.corl);
    int cornersFb = count_bad_corners(cube.cofb);

    int edgesE = count_bad_edges(cube.epose);
    int edgesM = count_bad_edges(cube.eposm);
    int edgesS = count_bad_edges(cube.eposs);

    bool rzpUd = find_xexc(rzps, edgesE, cornersUd);
    bool rzpRl = find_xexc(rzps, edgesM, cornersRl);
    bool rzpFb = find_xexc(rzps, edgesS, cornersFb);

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

// Create struct called RzpQueueNode with fields for cube, alg, and depth.
typedef struct RzpQueueNode {
    Cube cube;
    Alg *alg;
    int depth;
} RzpQueueNode;

int max(int a, int b) {
    return a > b ? a : b;
}

// Solve RZP on one axis. Append the solutions to sols.
void solve_rzp_on_axis(struct timespec start, AlgList *sols, Cube cube, char *rzps, int maxMoves, int maxSols, bool jzp, Move moves[], int numMoves) {
    int queueCap = max(32, sols->len);
    struct RzpQueueNode *queue = malloc(queueCap * sizeof(RzpQueueNode));
    queue[0] = (struct RzpQueueNode){cube, new_alg(""), 0};

    int queueIndex = 0;
    int queueLen = 1;
    int depth = 0;

    while (true) {
        RzpQueueNode node = queue[queueIndex];
        queueIndex++;

        if (node.depth > depth) {
            depth++;
        }

        Cube cube = node.cube; // Breaks without this line for some reason?
        bool step_solved = check_rzp(cube, rzps) || (jzp && check_jzp(cube));
        if (step_solved && !alg_is_redundant_in_algs(node.alg, sols)) {
            append_alg(sols, node.alg);
        }

        // Check for stopping condition AFTER checking if step is solved.
        // This allows the user to set 0 sols or 0 moves.
        if (depth >= maxMoves || sols->len >= maxSols || queueLen == 0 || elapsed_ms(start) > TIME_LIMIT) {
            break;
        }

        for (int i = 0; i < numMoves; i++) {
            // Look at the last two moves to see if this move will be redundant.
            int len = node.alg->len;
            if (len >= 1) {
                int prev = node.alg->move[len - 1];

                if (moves_are_same_face(prev, moves[i])) {
                    continue;
                }

                if (len >= 2
                    && moves_are_opposite_face(prev, moves[i])
                    && moves_are_same_face(node.alg->move[len - 2], moves[i])) {
                    continue;
                }
            }

            Cube newCube = apply_move(moves[i], cube);

            Alg *newAlg = new_alg("");
            copy_alg(node.alg, newAlg);
            append_move(newAlg, moves[i], false);

            if (queueLen == queueCap) {
                queueCap *= 2;
                queue = realloc(queue, queueCap * sizeof(RzpQueueNode));
            }
            
            queue[queueLen] = (struct RzpQueueNode){newCube, newAlg, depth + 1};
            queueLen++;
        }
    }

    for (int i = 0; i < queueLen; i++) {
        free_alg(queue[i].alg);
    }
    free(queue);
}

SolveOutput *solve_rzp(struct timespec start, Cube cube, SolveOptions *opts) {
    char *rzps = opts->rzps;
    int maxMoves = opts->max_moves;
    int maxSols = opts->max_solutions;
    bool jzp = opts->jzp;

    AlgList *sols = new_alglist();
    
    bool eofb_solved = cube.eofb == 0;
    bool eorl_solved = cube.eorl == 0;
    bool eoud_solved = cube.eoud == 0;

    if (!eofb_solved && !eorl_solved && !eoud_solved) {
        char *msg = malloc(100);
        sprintf(msg, "Cannot do RZP yet, EO must be solved on at least 1 axis\n");
        return solve_output_new(sols, msg);
    }

    Alg *sol = new_alg("");
    if (eofb_solved) {
        Move moves[] = {U, U2, U3, D, D2, D3, R, R2, R3, L, L2, L3, F2, B2};
        solve_rzp_on_axis(start, sols, cube, rzps, maxMoves, maxSols, jzp, moves, 14);
    }

    if (eorl_solved) {
        Move moves[] = {U, U2, U3, D, D2, D3, F, F2, F3, B, B2, B3, R2, L2};
        solve_rzp_on_axis(start, sols, cube, rzps, maxMoves, maxSols, jzp, moves, 14);
    }

    if (eoud_solved) {
        Move moves[] = {F, F2, F3, B, B2, B3, R, R2, R3, L, L2, L3, U2, D2};
        solve_rzp_on_axis(start, sols, cube, rzps, maxMoves, maxSols, jzp, moves, 14);
    }

    free_alg(sol);
    return solve_output_new(sols, NULL);
}

// Forcing a defaultValue helps sanitize inputs and avoid segfaults.
char *get_step_value(SolveStep step, char *key, char *defaultValue) {
    for (int i = 0; i < step.datalen; i++) {
        if (strcmp(step.data[i].key, key) == 0) {
            return step.data[i].value;
        }
    }
    return defaultValue;
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

/** Example: R U R --> R U R' */
Alg *complement_alg(Alg *alg) {
    Alg *comp = new_alg("");
    copy_alg(alg, comp);

    Move last_inv = inverse_move(comp->move[comp->len - 1]);
    comp->move[comp->len - 1] = last_inv;
    return comp;
}

/** Return an alglist with the original algs and their complements. */
AlgList *complement_algs(AlgList *algs) {
    AlgList *comps = new_alglist();
    for (AlgListNode *i = algs->first; i != NULL; i = i->next) {
        Alg *alg = i->alg;
        append_alg(comps, alg);

        // If alg is empty, it doesn't have a complement.
        if (alg->len == 0) continue;

        Alg *comp = complement_alg(i->alg);
        append_alg(comps, comp);
        free_alg(comp);
    }
    return comps;
}

bool empty_str(char *str) {
    return str[0] == '\0';
}

SolveOutput *solve_helper(struct timespec start, Alg *scramble, AlgList *sols, SolveStep *steps, int step_index, int num_steps, int nisstype) {
    if (step_index >= num_steps) {
        // Copy sols so it can be freed.
        AlgList *new_sols = new_alglist();
        for (AlgListNode *i = sols->first; i != NULL; i = i->next) {
            Alg *copy = new_alg("");
            copy_alg(i->alg, copy);
            append_alg(new_sols, copy);
            free_alg(copy);
        }
        return solve_output_new(new_sols, NULL);
    }

    SolveStep step = steps[step_index];
    char *step_name = step.name;

    SolveOptions *opts = malloc(sizeof(SolveOptions));
    opts->min_moves = 0;
    opts->nthreads = 1;
    opts->optimal = -1; // Allow suboptimal solutions
    opts->nisstype = nisstype;
    opts->verbose = 0;
    opts->all = 0;
    opts->print_number = 0;
    opts->count_only = 0;

    char *sols_value = get_step_value(step, "Sols", "1000");
    char *moves_value = get_step_value(step, "Moves", "1000");
    if (empty_str(sols_value)) sols_value = "1000";
    if (empty_str(moves_value)) moves_value = "1000";
    opts->max_solutions = atoi(sols_value);
    opts->max_moves = atoi(moves_value);

    if (opts->max_solutions == 1000 && opts->max_moves == 1000) {
        char *msg = malloc(100);
        sprintf(msg, "Error: Step %s has no Sols or Moves\n", step_name);
        return solve_output_new(new_alglist(), msg);
    }

    // Set options for specific steps
    if (strcmp(step_name, "RZP") == 0) {
        opts->rzps = get_step_value(step, "rzps", "4e4c");
        opts->jzp = strcmp(get_step_value(step, "jzp", "false"), "true") == 0;
    } else if (strcmp(step_name, "Leave Slice") == 0) {
        char *leave_slice_axis = get_step_value(step, "leave_slice_axis", "DR Axis");
        if (strcmp(leave_slice_axis, "M Axis") == 0) {
            step.shortname = "drrlslice";
        } else if (strcmp(leave_slice_axis, "E Axis") == 0) {
            step.shortname = "drudslice";
        } else if (strcmp(leave_slice_axis, "S Axis") == 0) {
            step.shortname = "drfbslice";
        }
    }

    AlgList *new_sols = new_alglist();

    for (AlgListNode *i = sols->first; i != NULL; i = i->next) {
        Alg *alg = i->alg;
        Cube cube = apply_alg(scramble, (Cube){0});
        cube = apply_alg(alg, cube);
        SolveOutput *solve_output = solve_one_step(start, cube, step.shortname, opts);

        // Check complements unless we are on the last step.
        if (step_index < num_steps - 1) {
            solve_output->sols = complement_algs(solve_output->sols);
        }

        for (AlgListNode *j = solve_output->sols->first; j != NULL; j = j->next) {
            Alg *sol = j->alg;

            // If last move of alg and first move of sol are the same face, skip.
            // When we check the complement, we will get the same alg.
            if (base_move(alg->move[alg->len - 1]) == base_move(sol->move[0])) {
                continue;
            }

            Alg *combined_sol = new_alg("");
            copy_alg(alg, combined_sol);
            compose_alg(combined_sol, sol);
            append_alg(new_sols, combined_sol);
            free_alg(combined_sol);
        }

        if (solve_output->error_msg != NULL) {
            free_alglist(new_sols);
            free(opts);
            return solve_output;
        }

        solve_output_free(solve_output);
    }

    SolveOutput *so = solve_helper(start, scramble, new_sols, steps, step_index + 1, num_steps, nisstype);
    free_alglist(new_sols);
    free(opts);
    return so;
}

char** python_solve(SolveArgs solveArgs) {
    init_all_movesets();
	init_symcoord();

    // Start time after initialization because initialization is slow.
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

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

    AlgList *sols = new_alglist();
    Alg *empty_alg = new_alg("");
    append_alg(sols, empty_alg); // Add empty alg so the for loop works
    free_alg(empty_alg);
    SolveOutput *solve_output = solve_helper(start, scramble, sols, steps, 0, num_steps, nisstype);

    char **strings;
    if (solve_output->error_msg != NULL) {
        // Return an array of strings with the error message
        strings = malloc(2 * sizeof(char*));
        strings[0] = solve_output->error_msg;
        strings[1] = NULL;
    } else {
        sort_alglist(solve_output->sols);
        strings = alglist_to_strings(solve_output->sols);
    }

    free_alg(scramble);
    free_alglist(sols);
    solve_output_free(solve_output);

    return strings;
}

/* Based on 'scramble_exec' from commands.c */
char* python_scramble(char *scrtype) {
    init_all_movesets();
	init_symcoord();

    // Start time after initialization because initialization is slow.
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    Cube cube;
	CubeArray *arr;
	Alg *scr, *ruf, *aux;
	int i, j, eo, ep, co, cp, a[12];
	int eparr[12] = { [8] = 8, [9] = 9, [10] = 10, [11] = 11 };
	uint64_t ui, uj, uk;

    // Used to be seeded with time in seconds.
    // Now we seed wih nanoseconds to get more variety.
	srand(start.tv_nsec);

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

#define MAXLINELEN          10000
#define MAXTOKENLEN         255
#define MAXNTOKENS          255

void cleanwhitespaces(char *line) {
	char *i;

	for (i = line; *i != 0; i++)
		if (*i == '\t' || *i == '\n')
			*i = ' ';
}

/* Parse tokens from line into v, and stop at maxtokens. Return number of tokens. */
int parseline(char *line, char **v, int maxtokens) {
	char *t;
	int n = 0;
	
	cleanwhitespaces(line);

	for (t = strtok(line, " "); t != NULL; t = strtok(NULL, " ")) {
		strcpy(v[n++], t);
        if (n == maxtokens)
            break;
    }

	return n;
}

char* exec_args(struct timespec start, int c, char **v) {
	int i;
	char line[MAXLINELEN];
	Command *cmd = NULL;
	CommandArgs *args;
	Alg *scramble;

	for (i = 0; commands[i] != NULL; i++)
		if (!strcmp(v[0], commands[i]->name))
			cmd = commands[i];

	if (cmd == NULL) {
        char *msg = malloc(100);
        sprintf(msg, "%s: command not found\n", v[0]);
        return msg;
	}

	args = cmd->parse_args(c-1, &v[1]);
    args->start = start;
	if (!args->success) {
        char *msg = malloc(100);
        sprintf(msg, "usage: %s\n", cmd->usage);
        free_args(args);
        return msg;
	}

	return cmd->exec(args);
}

char* python_shell(char *line) {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

	int i, shell_argc;
    char **shell_argv;

	shell_argv = malloc(MAXNTOKENS * sizeof(char *));
	for (i = 0; i < MAXNTOKENS; i++)
		shell_argv[i] = malloc((MAXTOKENLEN+1) * sizeof(char));

    shell_argc = parseline(line, shell_argv, MAXTOKENLEN);

    char *output = exec_args(start, shell_argc, shell_argv);

	for (i = 0; i < MAXNTOKENS; i++)
		free(shell_argv[i]);
	free(shell_argv);

    return output;
}

////////// For testing with valgrind //////////
// int main(int argc, char *argv[]) {
//     char *scrtype = "normal";
//     char *scramble = python_scramble(scrtype);

//     // Remove the length from scramble string.
//     scramble[strlen(scramble) - 5] = '\0';
//     printf("Scramble: %s\n", scramble);

//     SolveArgs solve_args = {
//         .steps = (struct SolveStep[]) {
//             {
//                 .name = "EO",
//                 .shortname = "eo",
//                 .data = (struct StepData[]) {
//                     {
//                         .key = "num_eos",
//                         .value = "10"
//                     }
//                 },
//                 .datalen = 1
//             },
//             {
//                 .name = "RZP",
//                 .shortname = "rzp",
//                 .data = (struct StepData[]) {
//                     {
//                         .key = "num_rzp_moves",
//                         .value = "3"
//                     },
//                     {
//                         .key = "rzps",
//                         .value = "xexc"
//                     },
//                     {
//                         .key = "jzp",
//                         .value = "true"
//                     }
//                 },
//                 .datalen = 3
//             },
//             {
//                 .name = "DR",
//                 .shortname = "dr",
//                 .data = (struct StepData[]) {
//                     {
//                         .key = "num_drs",
//                         .value = "1"
//                     }
//                 },
//                 .datalen = 1
//             },
//             {
//                 .name = "HTR",
//                 .shortname = "htr",
//                 .data = (struct StepData[]) {
//                     {
//                         .key = "num_htrs",
//                         .value = "1"
//                     }
//                 },
//                 .datalen = 1
//             },
//             {
//                 .name = "Finish",
//                 .shortname = "drfin",
//                 .data = (struct StepData[]) {
//                     {
//                         .key = "num_finishes",
//                         .value = "1"
//                     }
//                 },
//                 .datalen = 1
//             }
//         },
//         .num_steps = 5,
//         .scramble = scramble,
//         .nisstype = 1
//     };
//     char **sols = python_solve(solve_args);

//     free(scramble);
//     for (int i = 0; sols[i] != NULL; i++) {
//         printf("%s\n", sols[i]);
//         free(sols[i]);
//     }
//     free(sols);

//     return 0;
// }