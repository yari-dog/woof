#ifndef STATE_H
#define STATE_H
#include "config.h"
#include "util.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct result_t
{
    char *title;
    char *path;
    char *entry_path; // this is the path of the .desktop file
    char *comment;
    char *exec_cmd;
    bool terminal;
    bool visible;
    bool hovered;
    struct result_t *next;
    struct result_t *prev;
    uint8_t pos;
} result_t;

typedef struct state_t
{
    // window servers
    struct wlc_t *wlc;

    // key handling
    struct xkb_t *xkb;
    size_t cursor;

    // various window server agnostic bits
    struct render_context_t *render_context;

    struct result_t **results;
    struct result_t *head_result;
    struct result_t *hovered_result;
    uint16_t result_count;

    char *title;
    char *current_command_string;
    int32_t ccs_buffsize;
    bool close;
    bool update;
} state_t;

state_t *init_state ();

#endif
