#ifndef COMMANDS_H
#define COMMANDS_H

#include <time.h>

#include "solve.h"
#include "steps.h"

char*                   alg_to_string(Alg *alg);
char**                  alglist_to_strings(AlgList *alglist);
void                    free_args(CommandArgs *args);
CommandArgs *           new_args(void);

extern Command *        commands[];

#endif