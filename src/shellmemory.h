#define MEM_SIZE 1000
#define PROGRAM_MEM_SIZE 1000
void mem_init();
void prog_mem_init();
int prog_mem_alloc(int size);
void prog_mem_free(int start_idx, int size);
void prog_write_line(int idx, const char *line);
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
char *prog_read_line(int idx);
