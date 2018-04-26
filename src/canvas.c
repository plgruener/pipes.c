#include <config.h>

#include <stdlib.h>
#include "pipe.h"
#include "canvas.h"

struct pipe_cell_list *pipe_cell_list_alloc(void) {
    struct pipe_cell_list *p;
    p = malloc(sizeof(*p));

    pipe_cell_list_init(p);
    return p;
}

void pipe_cell_list_init(struct pipe_cell_list *list) {
    list->head = list->tail = NULL;
}

cpipes_errno pipe_cell_list_push(
        struct pipe_cell_list *list,
        const char *pipe_char, struct pipe *pipe) {

    struct pipe_cell *cell;
    cell = malloc(sizeof(*cell));
    if(!cell)
        return set_error(ERR_OUT_OF_MEMORY);

    cell->pipe_char = pipe_char;
    cell->color = pipe->color;
    cell->pipe = pipe;

    cell->next = list->head;
    cell->prev = NULL;

    if(!list->head)
        list->tail = cell;
    else
        list->head->prev = cell;
    list->head = cell;

    return 0;
}

void pipe_cell_list_remove(struct pipe_cell_list *list, struct pipe *pipe) {
    for(struct pipe_cell *c = list->tail; c; c = c->prev) {
        if(c->pipe == pipe) {
            if(c == list->head)
                list->head = c->next;
            if(c == list->tail)
                list->tail = c->prev;
            if(c->prev)
                c->prev->next = c->next;
            if(c->next)
                c->next->prev = c->prev;
            free(c);
            break;
        }
    }
}

void pipe_cell_list_free(struct pipe_cell_list *list) {
    for(struct pipe_cell *c = list->head; c; c = c->next)
        free(c->prev);
    free(list->tail);
}

/** The canvas "cells" contain a list of the pipe characters (and colors) that
 * have been written to that (x, y) coordinate. Cells are stored in a row-major
 * array.
 *
 * Each cell is a linked list (struct pipe_cell_list) of "struct pipe_cell"s.
 *
 * The cells array is resized using "realloc" to be "width * height" cells. If
 * the number of cells are reduced, then the linked lists are all destroyed
 * before the array is resized.
 */
cpipes_errno canvas_resize(
        struct canvas *canvas,
        unsigned int width, unsigned int height) {

    size_t old_sz = canvas->width * canvas->height;
    size_t new_sz = width * height;
    for(size_t i = new_sz; i < old_sz; i++)
        pipe_cell_list_free(&canvas->cells[i]);

    canvas->cells = realloc(canvas->cells, sizeof(*canvas->cells) * new_sz);
    if(!canvas->cells)
        return set_error(ERR_OUT_OF_MEMORY);

    canvas->width = width;
    canvas->height = height;
    for(size_t i = old_sz; i < new_sz; i++)
        pipe_cell_list_init(&canvas->cells[i]);
    return 0;
}

/// Initialise a blank canvas. The "cells" list for each position is empty.
cpipes_errno canvas_init(
        struct canvas *canvas,
        unsigned int width, unsigned int height) {
    canvas->width = canvas->height = 0;
    canvas->cells = NULL;
    return canvas_resize(canvas, width, height);
}

/// Free canvas and all pipe_cell_lists
void canvas_free(struct canvas *canvas) {
    for(size_t i = 0; i < canvas->width * canvas->height; i++)
        pipe_cell_list_free(&canvas->cells[i]);
    free(canvas->cells);
}
