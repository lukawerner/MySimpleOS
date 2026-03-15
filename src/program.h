#ifndef PROGRAM_H
#define PROGRAM_H
typedef struct Program Program;
extern Program *program_table[];
extern int program_table_size;
Program *program_create(char *name); 
int program_destroy(Program *p);

int program_load_frames(Program * p, char **lines, int n_frames);
int program_get_frame(Program *p, int idx);
int program_get_num_of_pages(Program *p);
int program_get_pcb_pointing(Program *p); 
void program_inc_pcb_pointing(Program *p);
void program_dec_pcb_pointing(Program *p); 
const char* program_get_name(Program *p); 
int program_get_length(Program *p);
int load_program(Program *p);
int program_already_exists(char *name);
Program *find_program_in_table(char *name);
int remove_prog_from_table(Program *p);
int program_get_num_of_pages(Program *p);
int program_get_frame(Program *p, int idx);
int *program_get_frames_idx(Program *p); 
void background_program_set_length(Program *p, int script_length);
int convert_length_to_pages(int length);
#endif