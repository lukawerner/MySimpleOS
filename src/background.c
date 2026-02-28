#include "background.h"
#include <stdio.h>
#include "scheduler.h"
#include <stdlib.h>
#include "helper.h"
#include <string.h>
#include "shellmemory.h"
#include "readyqueue.h"

PCB *parseBatchScript() {
    int program_size = 0;
    char line[MAX_LINE_LENGTH];
    int array_size = 2;

    char **batch_script = malloc(sizeof(char*)*array_size);

    while (fgets(line, MAX_LINE_LENGTH, stdin) != NULL) { // reading from stdin stream (the batch script file)

        batch_script[program_size] = strdup(line);

        //printf("batch script line %i: %s\n", program_size, batch_script[program_size]);
        program_size++;
        if (program_size >= array_size) {// dynamically reallocating memory if needed
            array_size *= 2;
            char **tmp = realloc(batch_script, sizeof(char*)*array_size);

            if (tmp == NULL) {
                free_array(batch_script, program_size);
                printf("Couldn't reallocate memory for batch script\n");
                return NULL;
            }
            batch_script = tmp;
        }
    }
    int memory_start_idx = prog_mem_alloc(program_size); // allocate space for batch script
    if (memory_start_idx == -1) {
        free_array(batch_script, program_size);
        printf("Couldn't allocate memory for batch script\n");
        return NULL;
    }
    for (int i = 0; i<program_size; i++) { // write batch script to program memory
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
