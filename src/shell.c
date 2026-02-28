#include "shell.h"
#include "interpreter.h"
#include "mt_scheduler.h"
#include "readyqueue.h"
#include "scheduler.h"
#include "shellmemory.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_t main_thread_id;
extern int request_quit;

int parseInput(const char ui[]);
int quit();
extern int multithreaded_mode;

// Start of everything
int main(int argc, char *argv[]) {
    printf("Shell version 1.5 created Dec 2025\n");
    fflush(stdout);

    char prompt = '$';               // Shell prompt
    char userInput[MAX_USER_INPUT];  // user's input stored here
    int errorCode = 0;               // zero means no error, default

    // init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }

    // we check if the stdin stream comes from a file or user input / terminal
    // (batch vs interactive mode)
    int interactiveMode = isatty(STDIN_FILENO);

    // init shell memory
    mem_init();
    prog_mem_init();
    ready_queue_init(&ready_queue);
    main_thread_id = pthread_self();

    while (1) {
        if (interactiveMode) {
            printf("%c ", prompt);
        }

        if (fgets(userInput, MAX_USER_INPUT - 1, stdin) == NULL) {  // fgets returns NULL when it reaches the end of the file (relevant for batch mode)
            int bye_already_printed = 0;

            if (multithreaded_mode) {
                // if EOF has been reached in multithreaded mode, it means
                // main thread has processed all input commands
                // so now we wait for the workers to finish processing the ready queue and exit
                handle_quit();
                pthread_mutex_lock(&ready_queue_lock);
                if (request_quit) {
                    bye_already_printed = 1;
                }
                pthread_mutex_unlock(&ready_queue_lock);
            }

            if (!bye_already_printed) {
                printf("Bye!\n");
            }
            return 0;
        }

        // one-liners logic starts here
        int end_idx = 0;
        int start_idx = 0;

        while (userInput[end_idx] != '\0') {
            // as long as we haven't reached the end of the input
            if (userInput[end_idx] != ';' && userInput[end_idx] != '\n') {  // if we haven't encountered a command separator
                end_idx++;
                continue;  // we just increment the user input index;
            }

            // else, we need to execute a command
            size_t command_size = end_idx - start_idx;
            char *curr_command = strndup(userInput + start_idx, command_size);

            errorCode = parseInput(curr_command);
            free(curr_command);

            if (errorCode == -1) {
                exit(99);
            }

            if (multithreaded_mode) {
                // in multithread mode, when a quit command has been processed by a worker thread,
                // we don't wait for EOF to exit, we just let both workers finish and exit
                pthread_mutex_lock(&ready_queue_lock);
                if (request_quit) {
                    pthread_mutex_unlock(&ready_queue_lock);
                    handle_quit();
                    exit(0);
                }
                pthread_mutex_unlock(&ready_queue_lock);
            }

            // if we are done with the line (one-liners or a single command)
            if (userInput[end_idx] == '\n') {
                break;
            } else {
                // if we only encountered a ';' separator, we need to keep iterating
                end_idx++;
                start_idx = end_idx;
            }
        }

        // one liner logic stops here;
        memset(userInput, 0, sizeof(userInput));
    }

    return 0;
}

int wordEnding(char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ';
}

int parseInput(const char inp[]) {
    char tmp[200];
    char *words[100];
    char *allocated_words[100];
    int ix = 0;
    int w = 0;
    int wordlen;
    int errorCode;

    while (inp[ix] != '\n' && inp[ix] != '\0' && ix < 1000) {
        // skip white spaces
        while (inp[ix] == ' ' && ix < 1000) {
            ix++;
        }

        // extract a word
        for (wordlen = 0; !wordEnding(inp[ix]) && ix < 1000; ix++, wordlen++) {
            tmp[wordlen] = inp[ix];
        }

        tmp[wordlen] = '\0';
        words[w] = strdup(tmp);
        allocated_words[w] = words[w];
        w++;

        if (inp[ix] == '\0') {
            break;
        }
        ix++;
    }
    errorCode = interpreter(words, w);

    for (int i = 0; i < w; i++) {
        free(allocated_words[i]);
    }

    return errorCode;
}
