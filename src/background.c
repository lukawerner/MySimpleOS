#include "background.h"
#include "helper.h"
#include "readyqueue.h"
#include "scheduler.h"
#include "shellmemory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "program.h"
#include <pthread.h>

static int next_batch_pid = 0;
extern pthread_mutex_t shellmemory_lock;

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
    Program *bg_executable = create_background_program(batch_script, program_size);

    free_array(batch_script, program_size);
    PCB *pcb = pcb_create(bg_executable);
    pcb_toggle_background_mode(pcb);
    return pcb;
}

Program *create_background_program(char **background_script, int script_size) {
    char script_name[MAX_BACKGROUND_NAME_LENGTH];
    snprintf(script_name, sizeof(script_name), "BATCH%d", next_batch_pid);
    next_batch_pid++;
    Program* background_program = program_create(script_name);

    background_program_set_length(background_program, script_size);
    int n_frames = convert_length_to_pages(script_size);
    background_program_set_frames_idx(background_program, n_frames);
    load_background_program_pages(background_program, background_script, n_frames);
    return background_program;
}

int load_background_program_pages(Program *bp, char** background_script, int n_pages) {
    pthread_mutex_lock(&shellmemory_lock);
    for (int i=0; i<n_pages; i++) {
        int page_number = i;
        int errorCode = load_page_into_frame_store(bp, background_script, page_number);
        if (errorCode) {
            pthread_mutex_unlock(&shellmemory_lock);
            return 1;
        } 
    } 
    pthread_mutex_unlock(&shellmemory_lock);
    return 0;
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
