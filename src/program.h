#ifndef PROGRAM_H
#define PROGRAM_H
typedef struct Program Program;
extern Program *program_table[];
extern int program_table_size;
Program *program_create(char *name); 
int program_destroy(Program *p);


int load_page_into_frame_store(Program * p, char** lines, int page_number);

int load_program_page(Program *p, int page_number);
int find_free_page_table_entry(Program *p);

int program_get_frame(Program *p, int idx);
int program_get_num_of_frames(Program *p);
int program_get_pcb_pointing(Program *p); 
void program_inc_pcb_pointing(Program *p);
void program_dec_pcb_pointing(Program *p); 
const char* program_get_name(Program *p); 
int program_get_length(Program *p);
void program_dec_pages_stored(Program *p);
int program_get_pages_stored(Program *p);
int init_load_program(Program *p);
int program_already_exists(char *name);
Program *find_program_in_table(char *name);
int remove_prog_from_table(Program *p);
int program_update_page_table_entry(Program *p, int page_number, int frame_number);
int program_get_frame(Program *p, int idx);
int *program_get_frames_idx(Program *p); 
void background_program_set_length(Program *p, int script_length);
void background_program_set_frames_idx(Program *bg, int n_frames);
int convert_length_to_pages(int length);
#endif