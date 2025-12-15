#ifndef EXEC_H
#define EXEC_H

#include "state.h"
#include <stdint.h>
void run (state_t *state);

void get_results (state_t *state);

void move_hover (state_t *state, int8_t count);
#endif
