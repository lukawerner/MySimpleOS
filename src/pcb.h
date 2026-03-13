#ifndef PCB_H
#define PCB_H

#include <sys/types.h>
typedef struct Program Program;
typedef struct PCB PCB;
PCB *pcb_create(Program *program);
void pcb_toggle_background_mode(PCB *pcb);
int pcb_get_background_mode(PCB *pcb);

void pcb_destroy(PCB *pcb);
void pcb_increment_pc(PCB *pcb);
void pcb_set_next(PCB *pcb, PCB *next);
PCB *pcb_get_next(PCB *pcb);
int pcb_get_pc(PCB *pcb);
Program *pcb_get_program(PCB *pcb);
int pcb_get_program_size(PCB *pcb);
void pcb_decrement_job_length_score(PCB *pcb);
int pcb_get_job_length_score(PCB *pcb);

#endif
