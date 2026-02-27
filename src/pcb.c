#include "pcb.h"
#include "shellmemory.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

static pid_t pid_tracker = 1;

typedef struct PCB {
    pid_t pid;
    int memory_idx;
    int program_size;
    int pc;
    int job_length_score;
    PCB* next;
    int backgroundModeOn; // set to 1 if we are in background mode and pcb is a batch script, else 0
} PCB;


PCB* pcb_create(int memory_idx, int program_size) {
    PCB *pcb = malloc(sizeof(PCB));
    pcb->pid = pid_tracker;
    pid_tracker++;
    pcb->memory_idx = memory_idx;
    pcb->program_size = program_size;
    pcb->pc = memory_idx;
    pcb->job_length_score = program_size;
    pcb->next = NULL;
    pcb->backgroundModeOn = 0;
    return pcb;
}

void pcb_toggle_background_mode(PCB *pcb) {
    pcb->backgroundModeOn = !(pcb->backgroundModeOn);
}

int pcb_get_background_mode(PCB *pcb) {
    return pcb->backgroundModeOn;
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

void pcb_decrement_job_length_score(PCB *pcb) {
    if (pcb->job_length_score > 0) {
        pcb->job_length_score--;
    }
}

int pcb_get_job_length_score(PCB *pcb) {
    return pcb->job_length_score;
}








