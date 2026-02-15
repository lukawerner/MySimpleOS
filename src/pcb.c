#include "pcb.h"
#include "shellmemory.h"
#include <sys/types.h>
#include <stdlib.h>

static pid_t pid_tracker = 1;

typedef struct PCB {
    pid_t pid;
    int memory_idx;
    int program_size;
    int pc;
    PCB* next;
} PCB;


PCB* pcb_create(int memory_idx, int program_size) {
    PCB *pcb = malloc(sizeof(PCB));
    pcb->pid = pid_tracker;
    pid_tracker++;
    pcb->memory_idx = memory_idx;
    pcb->program_size = program_size;
    pcb->pc = memory_idx;
    pcb->next = NULL;
    return pcb;
}

void pcb_destroy(PCB *pcb) {
    prog_mem_free(pcb->memory_idx, pcb->program_size);
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

int pcb_get_pc(PCB *pcb) {
    return pcb->pc;
}

int pcb_get_memory_idx(PCB *pcb) {
    return pcb->memory_idx;
}

int pcb_get_program_size(PCB *pcb) {
    return pcb->program_size;
}








