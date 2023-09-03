// #include "commands.c"
#include "shell.h"

/* Functions that are designed to be called in python */

/* Copied from shell.c */
void
cleanwhitespaces(char *line)
{
	char *i;

	for (i = line; *i != 0; i++)
		if (*i == '\t' || *i == '\n')
			*i = ' ';
}

/*
* Copied from shell.c
* This function assumes that **v is large enough
*/
int
parseline(char *line, char **v)
{
	char *t;
	int n = 0;
	
	cleanwhitespaces(line);

	for (t = strtok(line, " "); t != NULL; t = strtok(NULL, " "))
		strcpy(v[n++], t);

	return n;
}

/* Based on 'launch' from shell.c */
// void
// python_exec(char* line) {
//     int i, shell_argc;
//     char **shell_argv;

// 	shell_argv = malloc(MAXNTOKENS * sizeof(char *));
// 	for (i = 0; i < MAXNTOKENS; i++)
// 		shell_argv[i] = malloc((MAXTOKENLEN+1) * sizeof(char));

//     shell_argc = parseline(line, shell_argv);

//     if (shell_argc > 0) {
//         exec_args(shell_argc, shell_argv);
//     }

// 	for (i = 0; i < MAXNTOKENS; i++)
// 		free(shell_argv[i]);
// 	free(shell_argv);
// }

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

char*
append_len(Alg *alg, char *alg_string) {
    char len_str[10];
    snprintf(len_str, sizeof(len_str), " (%d)", alg->len);
    strcat(alg_string, len_str);
}

/* Similar to print_alglist defined in alg.c except it returns a list of strings */
char**
alglist_to_strings(AlgList *alglist) {
    char **result = malloc((alglist->len + 1) * sizeof(char*)); // Adjust the size as needed

    int resultLen = 0;
    for (AlgListNode *i = alglist->first; i != NULL; i = i->next, resultLen++) {
        char *alg_string = alg_to_string(i->alg);
        append_len(i->alg, alg_string);
        result[resultLen] = alg_string;
    }

    result[resultLen] = NULL; // Add NULL to the end of the array

    return result;
}

typedef struct {
    char *stepName;
    char *scrambleString;
    SolveOptions *opts;
} SolveArgs;

/*
* Based on 'solve_exec' from commands.c
* For opts, see struct solveoptions in cubetypes.h
*/
char**
python_solve(SolveArgs solveArgs) {
    char *stepName = solveArgs.stepName;
    char *scrambleString = solveArgs.scrambleString;
    SolveOptions *opts = solveArgs.opts;

    // Print statements for debugging
    // printf("stepName: %s\n", stepName);
    // printf("scrambleString: %s\n", scrambleString);
    // printf("min_moves: %d\n", opts->min_moves);
    // printf("max_moves: %d\n", opts->max_moves);
    // printf("max_solutions: %d\n", opts->max_solutions);
    // printf("nthreads: %d\n", opts->nthreads);
    // printf("optimal: %d\n", opts->optimal);
    // printf("can_niss: %d\n", opts->can_niss);
    // printf("verbose: %d\n", opts->verbose);
    // printf("all: %d\n", opts->all);
    // printf("print_number: %d\n", opts->print_number);
    // printf("count_only: %d\n", opts->count_only);

    Step *step = getStep(stepName);
    if (step == NULL) {
        printf("Error: Step '%s' not found\n", stepName);
        return NULL;
    }

    Alg *scramble = new_alg(scrambleString);

	init_all_movesets();
	init_symcoord();

    Cube c = apply_alg(scramble, (Cube){0});
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