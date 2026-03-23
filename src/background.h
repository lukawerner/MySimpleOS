#ifndef BACKGROUND_H
#define BACKGROUND_H
#include "pcb.h"
PCB *parseBatchScript();
Program *create_background_program(char **background_script, int script_size);
int create_batch_script_pcb_and_enqueue();
int load_background_program_pages(Program *bp, char** background_script, int n_pages);
#endif
