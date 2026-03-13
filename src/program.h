#ifndef PROGRAM_H
#define PROGRAM_H
typedef struct Program Program;
extern Program *program_table[];
extern int program_table_size;
Program *program_create(char *name, int base, int bounds); 
int program_destroy(Program *p);
int program_get_bounds(Program* p);
int program_get_pcb_pointing(Program *p); 
void program_inc_pcb_pointing(Program *p);
void program_dec_pcb_pointing(Program *p); 
const char* program_get_name(Program *p); 
int program_get_base(Program *p);
int load_program(Program *p);
int program_already_exists(char *name);
Program *find_program_in_table(char *name);
int remove_prog_from_table(Program *p);
#endif