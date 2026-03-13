#ifndef BACKGROUND_H
#define BACKGROUND_H
#include "pcb.h"
PCB *parseBatchScript();
Program *create_background_program(char **background_script, int script_size);
int create_batch_script_pcb_and_enqueue();
#endif
