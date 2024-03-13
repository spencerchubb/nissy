#include "commands.h"

/* Arg parsing functions *****************************************************/

CommandArgs *           help_parse_args(int c, char **v);
CommandArgs *           parse_only_scramble(int c, char **v);
CommandArgs *           parse_no_arg(int c, char **v);
CommandArgs *           solve_parse_args(int c, char **v);
CommandArgs *           scramble_parse_args(int c, char **v);

/* Exec functions ************************************************************/

static char*             cleanup_exec(CommandArgs *args);
static char*             invert_exec(CommandArgs *args);
static char*             solve_exec(CommandArgs *args);
static char*             scramble_exec(CommandArgs *args);
static char*             steps_exec(CommandArgs *args);
static char*             commands_exec(CommandArgs *args);
static char*             twophase_exec(CommandArgs *args);
static char*             help_exec(CommandArgs *args);
static char*             unniss_exec(CommandArgs *args);
static char*             version_exec(CommandArgs *args);

/* Local functions ***********************************************************/

static bool             read_step(CommandArgs *args, char *str);
static bool             read_scrtype(CommandArgs *args, char *str);
static bool             read_scramble(int c, char **v, CommandArgs *args);

/* Commands ******************************************************************/

Command
solve_cmd = {
	.name        = "solve",
	.usage       = "solve STEP [OPTIONS] SCRAMBLE",
	.description = "Solve a step; see command steps for a list of steps",
	.parse_args  = solve_parse_args,
	.exec        = solve_exec
};

Command
scramble_cmd = {
	.name        = "scramble",
	.usage       = "scramble [TYPE] [-n N]",
	.description = "Get a random-position scramble",
	.parse_args  = scramble_parse_args,
	.exec        = scramble_exec,
};

Command
invert_cmd = {
	.name        = "invert",
	.usage       = "invert SCRAMBLE]",
	.description = "Invert a scramble",
	.parse_args  = parse_only_scramble,
	.exec        = invert_exec,
};

Command
steps_cmd = {
	.name        = "steps",
	.usage       = "steps",
	.description = "List available steps",
	.parse_args  = parse_no_arg,
	.exec        = steps_exec
};

Command
commands_cmd = {
	.name        = "commands",
	.usage       = "commands",
	.description = "List available commands",
	.parse_args  = parse_no_arg,
	.exec        = commands_exec
};

Command
help_cmd = {
	.name        = "help",
	.usage       = "help [COMMAND]",
	.description = "Display nissy manual page or help on specific command",
	.parse_args  = help_parse_args,
	.exec        = help_exec,
};

Command
twophase_cmd = {
	.name        = "twophase",
	.usage       = "twophase",
	.description = "Find a solution quickly using a 2-phase method",
	.parse_args  = parse_only_scramble,
	.exec        = twophase_exec,
};

Command
cleanup_cmd = {
	.name        = "cleanup",
	.usage       = "cleanup SCRAMBLE",
	.description = "Rewrite a scramble using only standard moves (HTM)",
	.parse_args  = parse_only_scramble,
	.exec        = cleanup_exec,
};

Command
unniss_cmd = {
	.name        = "unniss",
	.usage       = "unniss SCRAMBLE",
	.description = "Rewrite a scramble without NISS",
	.parse_args  = parse_only_scramble,
	.exec        = unniss_exec,
};

Command
version_cmd = {
	.name        = "version",
	.usage       = "version",
	.description = "print nissy version",
	.parse_args  = parse_no_arg,
	.exec        = version_exec,
};

Command *commands[] = {
	&commands_cmd,
	&help_cmd,
	&invert_cmd,
	&solve_cmd,
	&scramble_cmd,
	&steps_cmd,
	&twophase_cmd,
	&cleanup_cmd,
	&unniss_cmd,
	&version_cmd,
	NULL
};

/* Other constants ***********************************************************/

char *scrtypes[20] = { "eo", "corners", "edges", "fmc", "dr", "htr", NULL };

/* Arg parsing functions implementation **************************************/

CommandArgs *
solve_parse_args(int c, char **v)
{
	int i;
	bool infinitesols, fixedmsols;
	long val;

	CommandArgs *a = new_args();

	a->opts->min_moves     = 0;
	a->opts->max_moves     = 20;
	a->opts->max_solutions = 1;
	a->opts->nthreads      = 1;
	a->opts->optimal       = -1;
	a->opts->nisstype      = NORMAL;
	a->opts->verbose       = false;
	a->opts->all           = false;
	a->opts->print_number  = true;
	a->opts->count_only    = false;

	fixedmsols = false;
	infinitesols = false;

	for (i = 0; i < c; i++) {
		if (!strcmp(v[i], "-m") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 0 || val > 100) {
				fprintf(stderr,
					"Invalid min number of moves"
					"(0 <= N <= 100).\n");
				return a;
			}
			a->opts->min_moves = val;
		} else if (!strcmp(v[i], "-M") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 0 || val > 100) {
				fprintf(stderr,
					"Invalid max number of moves"
					"(0 <= N <= 100).\n");
				return a;
			}
			a->opts->max_moves = val;
			infinitesols = true;
		} else if (!strcmp(v[i], "-t") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 1 || val > 64) {
				fprintf(stderr,
					"Invalid number of threads."
					"1 <= t <= 64\n");
				return a;
			}
			a->opts->nthreads = val;
		} else if (!strcmp(v[i], "-n") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 1 || val > 1000000) {
				fprintf(stderr,
					"Invalid number of solutions.\n");
				return a;
			}
			a->opts->max_solutions = val;
			fixedmsols = true;
		} else if (!strcmp(v[i], "-o")) {
			a->opts->optimal = 0;
			infinitesols = true;
		} else if (!strcmp(v[i], "-O") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 0 || val > 100 ||
			    (val == 0 && strcmp("0", v[i]))) {
				fprintf(stderr,
					"Invalid max number of moves"
					" (0 <= N <= 100).\n");
				return a;
			}
			a->opts->optimal = val;
			infinitesols = true;
		} else if (!strcmp(v[i], "-N")) {
			a->opts->nisstype = NISS;
		} else if (!strcmp(v[i], "-L")) {
			if (a->opts->nisstype != NISS)
				a->opts->nisstype = LINEAR;
		} else if (!strcmp(v[i], "-i")) {
			a->scrstdin = true;
		} else if (!strcmp(v[i], "-v")) {
			a->opts->verbose = true;
		} else if (!strcmp(v[i], "-a")) {
			a->opts->all = true;
		} else if (!strcmp(v[i], "-p")) {
			a->opts->print_number = false;
		} else if (!strcmp(v[i], "-c")) {
			a->opts->count_only = true;
		} else if (!read_step(a, v[i])) {
			break;
		}
	}

	if (infinitesols && !fixedmsols)
		a->opts->max_solutions = 1000000; /* 1M = +infty */

	a->success = (a->scrstdin && i == c) || read_scramble(c-i, &v[i], a);
	return a;
}

CommandArgs *
scramble_parse_args(int c, char **v)
{
	int i;
	long val;

	CommandArgs *a = new_args();

	a->success = true;
	a->n       = 1;

	for (i = 0; i < c; i++) {
		if (!strcmp(v[i], "-n") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 1 || val > 1000000) {
				fprintf(stderr,
					"Invalid number of scrambles.\n");
				a->success = false;
				return a;
			}
			a->n = val;
		} else if (!read_scrtype(a, v[i])) {
			a->success = false;
			return a;
		}
	}

	return a;
}

CommandArgs *
gen_parse_args(int c, char **v)
{
	int val;
	CommandArgs *a = new_args();

	a->opts->nthreads = 64;
	a->success  = false;

	if (c == 0) {
		a->success = true;
	} else {
		if (!strcmp(v[0], "-t") && c > 1) {
			val = strtol(v[1], NULL, 10);
			if (val < 1 || val > 64) {
				fprintf(stderr,
					"Invalid number of threads."
					"1 <= t <= 64\n");
				return a;
			}
			a->opts->nthreads = val;
			a->success = true;
		}
	}
	
	return a;
}

CommandArgs *
help_parse_args(int c, char **v)
{
	int i;
	CommandArgs *a = new_args();

	if (c == 1) {
		for (i = 0; commands[i] != NULL; i++)
			if (!strcmp(v[0], commands[i]->name))
				a->command = commands[i];
		if (a->command == NULL)
			fprintf(stderr, "%s: command not found\n", v[0]);
	}

	a->success = c == 0 || (c == 1 && a->command != NULL);
	return a;
}

CommandArgs *
parse_only_scramble(int c, char **v)
{
	CommandArgs *a = new_args();

	if (!strcmp(v[0], "-i")) {
		a->scrstdin = true;
		a->success = c == 1;
	} else {
		a->success = read_scramble(c, v, a);
	}

	return a;
}

CommandArgs *
parse_no_arg(int c, char **v)
{
	CommandArgs *a = new_args();

	a->success = true;

	return a;
}

void append_str(char **str1, const char *str2) {
    size_t len1 = strlen(*str1);
    size_t len2 = strlen(str2);

    // Check if more memory is needed
    if (len1 + len2 + 1 >= strlen(*str1)) {
        *str1 = realloc(*str1, (len1 + len2 + 1) * sizeof(char));
        if (*str1 == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
    }

    // Append str2 to str1
    strcat(*str1, str2);
}

/* Exec functions implementation *********************************************/

static char*
solve_exec(CommandArgs *args)
{
	Cube c;

	init_all_movesets();
	init_symcoord();

    // I don't think the timeout is working so here's a workaround.
    args->opts->max_solutions = 100;

	c = apply_alg(args->scramble, (Cube){0});
	SolveOutput *solve_output = solve(args->start, c, args->step, args->opts);

    if (solve_output->error_msg != NULL) {
        char *msg = solve_output->error_msg;
        strcat(msg, "\n");
        solve_output_free(solve_output);
        return msg;
    } else {
        sort_alglist(solve_output->sols);
        char **strings = alglist_to_strings(solve_output->sols);
        int memory_needed = 0;
        for (int i = 0; i < solve_output->sols->len; i++)
            memory_needed += strlen(strings[i]) + 1;
        char *output = malloc(memory_needed * sizeof(char));
        output[0] = '\0';  // Initialize output as an empty string
        for (int i = 0; i < solve_output->sols->len; i++) {
            // printf("%s\n", strings[i]);
            strcat(output, strings[i]);
            strcat(output, "\n");
            free(strings[i]);
        }
        free(strings);
        return output;
    }
}

static char*
scramble_exec(CommandArgs *args)
{
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

    if (!strcmp(args->scrtype, "dr")) {
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
    } else if (!strcmp(args->scrtype, "htr")) {
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

        if (!strcmp(args->scrtype, "eo")) {
            eo = 0;
        } else if (!strcmp(args->scrtype, "corners")) {
            eo = 0;
            ep = 0;
            index_to_perm(cp, 8, a);
            if (perm_sign(a, 8) == 1) {
                swap(&a[0], &a[1]);
                cp = perm_to_index(a, 8);
            }
        } else if (!strcmp(args->scrtype, "edges")) {
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

    if (!strcmp(args->scrtype, "fmc")) {
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
    strcat(alg_string, "\n");
    free_alg(scr);
    return alg_string;
}

static char*
invert_exec(CommandArgs *args)
{
	Alg *inv = inverse_alg(args->scramble);
    char *alg_string = alg_to_string(inv);
    strcat(alg_string, "\n");
	free_alg(inv);
    return alg_string;
}

static char*
steps_exec(CommandArgs *args)
{
    char *output = malloc(1024 * sizeof(char));
    output[0] = '\0';  // Initialize output as an empty string
    for (int i = 0; steps[i] != NULL; i++) {
        printf("%d ", i);
        strcat(output, steps[i]->shortname);
        strcat(output, "\t ");
        strcat(output, steps[i]->name);
        strcat(output, "\n");
    }
    return output;
}

static char*
commands_exec(CommandArgs *args)
{
    char *output = malloc(1024 * sizeof(char));
    output[0] = '\0';  // Initialize output as an empty string
    for (int i = 0; commands[i] != NULL; i++) {
        strcat(output, commands[i]->usage);
        strcat(output, "\n");
    }
    
    return output;
}

static char*
twophase_exec(CommandArgs *args)
{
	Cube c;
	Alg *sol;

	init_all_movesets();
	init_symcoord();

	c = apply_alg(args->scramble, (Cube){0});
	sol = solve_2phase(args->start, c, 1);

    char *alg_string = alg_to_string(sol);
	free_alg(sol);
    strcat(alg_string, "\n");
    return alg_string;
}

static char*
help_exec(CommandArgs *args)
{
    char *output = malloc(1024 * sizeof(char));
	if (args->command == NULL) {
		sprintf(output,
		       "Use the nissy command \"help COMMAND\" for a short "
		       "description of a specific command.\n"
		       "Use the nissy command \"commands\" for a list of "
		       "available commands.\n"
		       "See the manual page for more details. The manual"
		       " page is available with \"man nissy\" on a UNIX"
		       " system (such as Linux or MacOS) or in pdf and html"
		       " format in the docs folder.\n"
		       "Nissy is available for free at "
		       "https://github.com/sebastianotronto/nissy\n"
		      );
	} else {
		sprintf(output, "Command %s: %s\nusage: %s\n", args->command->name,
		        args->command->description, args->command->usage);
	}
    return output;
}

static char*
cleanup_exec(CommandArgs *args)
{
	init_moves();

	Alg *alg = cleanup(args->scramble);
    char *alg_string = alg_to_string(alg);
    strcat(alg_string, "\n");
	free_alg(alg);
    return alg_string;
}

static char*
unniss_exec(CommandArgs *args)
{
	Alg *unnissed = unniss(args->scramble);
    char *alg_string = alg_to_string(unnissed);
    strcat(alg_string, "\n");
    free_alg(unnissed);
    return alg_string;
}

static char*
version_exec(CommandArgs *args)
{
    return VERSION"\n";
}

/* Local functions implementation ********************************************/

/* Similar to print_alg defined in alg.c except this returns a string */
char* alg_to_string(Alg *alg) {
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
char** alglist_to_strings(AlgList *alglist) {
    char **result = malloc((alglist->len + 1) * sizeof(char*)); // Adjust the size as needed

    int resultLen = 0;
    for (AlgListNode *i = alglist->first; i != NULL; i = i->next, resultLen++) {
        char *alg_string = alg_to_string(i->alg);
        result[resultLen] = alg_string;
    }

    result[resultLen] = NULL; // Add NULL to the end of the array

    return result;
}

static bool
read_scramble(int c, char **v, CommandArgs *args)
{
	int i, k, n;
	unsigned int j;
	char *algstr;

	if (c < 1) {
		fprintf(stderr, "Error: no scramble given?\n");
		return false;
	}

	for(n = 0, i = 0; i < c; i++)
		n += strlen(v[i]);

	algstr = malloc((n + 1) * sizeof(char));
	k = 0;
	for (i = 0; i < c; i++)
		for (j = 0; j < strlen(v[i]); j++)
			algstr[k++] = v[i][j];
	algstr[k] = 0;

	args->scramble = new_alg(algstr);
	free(algstr);

	if (args->scramble->len == 0)
		fprintf(stderr, "Error reading scramble\n");

	return args->scramble->len > 0;
}

static bool
read_scrtype(CommandArgs *args, char *str)
{
	int i;

	for (i = 0; scrtypes[i] != NULL; i++) {
		if (!strcmp(scrtypes[i], str)) {
			strcpy(args->scrtype, scrtypes[i]);
			return true;
		}
	}

	return false;
}

static bool
read_step(CommandArgs *args, char *str)
{
	int i;

	for (i = 0; steps[i] != NULL; i++) {
		if (!strcmp(steps[i]->shortname, str)) {
			args->step = steps[i];
			return true;
		}
	}

	return false;
}

/* Public functions implementation *******************************************/

void
free_args(CommandArgs *args)
{
	if (args == NULL)
		return;

	if (args->scramble != NULL)
		free_alg(args->scramble);
	if (args->opts != NULL)
		free(args->opts);

	/* step and command must not be freed, they are static! */

	free(args);
}

CommandArgs *
new_args(void)
{
	CommandArgs *args = malloc(sizeof(CommandArgs));

	args->success = false;
	args->scrstdin = false;
	args->scramble = NULL; /* initialized in read_scramble */
	args->opts = malloc(sizeof(SolveOptions));

	/* step and command are static */
	args->step = steps[0]; /* default: first step in list */
	args->command = NULL;
	args->pd = NULL;

	return args;
}