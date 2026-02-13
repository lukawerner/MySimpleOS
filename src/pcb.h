#ifndef PCB_H
#define PCB_H
typedef struct PCB PCB;
PCB *pcb_create(pid_t pid, int mem_idx, int prog_size);
void pcb_destroy(PCB *pcb);
void pcb_increment_pc(PCB *pcb);
void pcb_set_next(PCB *pcb, PCB *next);
PCB *pcb_get_next(PCB *pcb);
#endif