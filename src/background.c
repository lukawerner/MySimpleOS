#include "background.h"
#include "helper.h"
#include "readyqueue.h"
#include "scheduler.h"
#include "shellmemory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PCB *parseBatchScript() {
    int program_size = 0;
    char line[MAX_LINE_LENGTH];
    int array_size = 2;
    char **batch_script = malloc(sizeof(char *) * array_size);

    // reading from stdin stream (the batch script file)
    while (fgets(line, MAX_LINE_LENGTH, stdin) != NULL) {
        batch_script[program_size] = strdup(line);
        //printf("batch script line %i: %s\n", program_size, batch_script[program_size]);
        program_size++;

        // dynamically reallocating memory if needed
        if (program_size >= array_size) {
            array_size *= 2;
            char **tmp = realloc(batch_script, sizeof(char *) * array_size);

            if (tmp == NULL) {
                free_array(batch_script, program_size);
                printf("Couldn't reallocate memory for batch script\n");
                return NULL;
            }
            batch_script = tmp;
        }
    }

    // allocate space for batch script
    int memory_start_idx = prog_mem_alloc(program_size);
    if (memory_start_idx == -1) {
        free_array(batch_script, program_size);
        printf("Couldn't allocate memory for batch script\n");
        return NULL;
    }

    // write batch script to program memory
    for (int i = 0; i < program_size; i++) {
        prog_write_line(memory_start_idx + i, batch_script[i]);
    }

    free_array(batch_script, program_size);
    PCB *pcb = pcb_create(memory_start_idx, program_size);
    pcb_toggle_background_mode(pcb);
    return pcb;
}

int create_batch_script_pcb_and_enqueue() {
    PCB *batch_script_pcb = parseBatchScript();

    if (batch_script_pcb == NULL) {
        return 1;
    }
    int errorCode = ready_queue_enqueue(batch_script_pcb, &ready_queue, NULL);
    if (errorCode) {
        pcb_destroy(batch_script_pcb);
        return 1;
    }
    return 0;
}
