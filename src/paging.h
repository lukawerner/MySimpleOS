#ifndef PAGING_H
#define PAGING_H
typedef struct PCB PCB;
typedef struct Program Program;

int handle_page_fault(PCB *process);
int find_frame_in_prog_page_table(Program *program, int frame_number);
Program *find_victim_program(int frame_number);
int evict_program_frame(Program *p, int frame_idx);
int evict_random_frame();
int evict_lru_frame();
int print_victim_lines(Program *p, int page_num);
#endif