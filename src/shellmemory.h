#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H
typedef struct Program Program;
void mem_init();
void frame_store_init();
void store_frame(int frame_number, char *script[], int script_length, int page_number);
int alloc_frame();
void mem_free_frame(int frame_idx);
void prog_mem_free(Program *p);
int search_free_frame(int start_idx, int end_idx);
void prog_write_line(int idx, const char *line);
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
char *prog_read_line(int idx);
char *prog_read_line_unlocked(int idx);
#endif