#include "scheduler.h"
#include "shellmemory.h"
#include <stdio.h>

int load_program(char *script) {
    FILE *f = fopen(script, "rt");
    if (f == NULL) {
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    int counting = 1; // flag indicating we are counting the number of line in the script
    int file_read = 0; // flag indicating we have reach eof
    int script_length = 0;
    int idx;

    while (1) { // two states: counting (to get the length) or !counting (write program to memory)
        fgets(f, PROGRAM_MEM_SIZE, line);
        file_read = feof(f);

        if (counting && file_read) {
            counting = 0;
            idx = prog_mem_alloc(script_length);
            rewind(f);
        }
        else if (counting && !file_read) {
            script_length++;
        }
        else if (!counting && !file_read) {
            prog_write_line(idx, line);
            idx++;
        }
        else {
            return 0;
        }
    }
}