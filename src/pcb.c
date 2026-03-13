#include "pcb.h"
#include "shellmemory.h"
#include <stdlib.h>
#include <sys/types.h>
#include "program.h"

static pid_t pid_tracker = 1;

typedef struct PCB {
    pid_t pid;
    Program *program;
    int pc;
    int job_length_score;
    PCB *next;
    int backgroundModeOn;  // set to 1 if we are in background mode and pcb is a batch script, else 0
} PCB;

PCB *pcb_create(Program *program) {
    PCB *pcb = malloc(sizeof(PCB));
    pcb->pid = pid_tracker;
    pid_tracker++;
    pcb->program = program; 
    pcb->pc = program_get_base(program);
    pcb->job_length_score = program_get_bounds(program);
    pcb->next = NULL;
    pcb->backgroundModeOn = 0;
    return pcb;
}

void pcb_toggle_background_mode(PCB *pcb) { pcb->backgroundModeOn = !(pcb->backgroundModeOn); }

int pcb_get_background_mode(PCB *pcb) { return pcb->backgroundModeOn; }

void pcb_destroy(PCB *pcb) {
    int pcb_count = program_get_pcb_pointing(pcb->program);
    if (pcb_count == 1) {
        program_destroy(pcb->program);
    }
    else {
        program_dec_pcb_pointing(pcb->program);
    }
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

Program *pcb_get_program(PCB *pcb) {
    return pcb->program;
}

int pcb_get_program_size(PCB *pcb) {
    return program_get_bounds(pcb->program);
}

void pcb_decrement_job_length_score(PCB *pcb) {
    if (pcb->job_length_score > 0) {
        pcb->job_length_score--;
    }
}

int pcb_get_job_length_score(PCB *pcb) {
    return pcb->job_length_score;
}
