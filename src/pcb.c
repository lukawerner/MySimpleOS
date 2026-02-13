#include "pcb.h"
#include <sys/types.h>
#include <stdlib.h>

typedef struct PCB {
    pid_t pid;
    int memory_idx;
    int program_size;
    int pc;
    PCB* next;
} PCB;

PCB* pcb_create(pid_t pid, int memory_idx, int program_size) {
    PCB *pcb = malloc(sizeof(PCB));
    pcb->pid = pid;
    pcb->memory_idx = memory_idx;
    pcb->program_size = program_size;
    pcb->pc = memory_idx;
    pcb->next = NULL;
    return pcb;
}

void pcb_destroy(PCB *pcb) {
    if (pcb != NULL) free(pcb);
}

void pcb_increment_pc(PCB *pcb) {
    pcb->pc++;
}

void pcb_set_next(PCB *pcb, PCB *next) {
    pcb->next = next;
}

PCB *pcb_get_next(PCB *pcb) {
    return pcb->next;
}






