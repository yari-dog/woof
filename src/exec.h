#ifndef EXEC_H
#define EXEC_H

#include "state.h"
#include <stdint.h>
uint8_t exec_cmd (char *cmd);
void run (state_t *state);

#endif
