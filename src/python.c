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
void
python_exec(char* line) {
    int i, shell_argc;
    char **shell_argv;

	shell_argv = malloc(MAXNTOKENS * sizeof(char *));
	for (i = 0; i < MAXNTOKENS; i++)
		shell_argv[i] = malloc((MAXTOKENLEN+1) * sizeof(char));

    shell_argc = parseline(line, shell_argv);

    if (shell_argc > 0) {
        exec_args(shell_argc, shell_argv);
    }

	for (i = 0; i < MAXNTOKENS; i++)
		free(shell_argv[i]);
	free(shell_argv);
}

// Impelement getStep function
Step* getStep(char* stepName) {
    for (int i = 0; steps[i] != NULL; i++) {
        if (strcmp(stepName, steps[i]->shortname) == 0) {
            return steps[i];
        }
    }
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

    char len_str[10];
    snprintf(len_str, sizeof(len_str), " (%d)", alg->len);
    strcat(result, len_str);

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