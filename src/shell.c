#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"

int parseInput(char ui[]);

// Start of everything
int main(int argc, char *argv[]) {
    printf("Shell version 1.5 created Dec 2025\n");
    fflush(stdout);


    char prompt = '$';  				// Shell prompt
    char userInput[MAX_USER_INPUT];		// user's input stored here
    int errorCode = 0;					// zero means no error, default

    //init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }

    //we check if the stdin stream comes from a file or user input / terminal (batch vs interactive mode)
    int interactiveMode = isatty(STDIN_FILENO);    // STDIN_FILENO is the file descriptor which 
                                                    // points to the current process' input object

    //init shell memory
    mem_init();
                
    while(1) { 
        if (interactiveMode) printf("%c ", prompt);
    
        if (fgets(userInput, MAX_USER_INPUT-1, stdin) == NULL) {    // fgets returns NULL when it reaches the end
            printf("Bye!\n");                                       // of the file (relevant for batch mode)
            return 0;                                               
        }

        //one-liners logic starts here
        int end_idx = 0;
        int start_idx = 0;
        while (userInput[end_idx] != '\0') { // as long as we haven't reached the end of the input
            if (userInput[end_idx] != ';' && userInput[end_idx] != '\n') { // if we haven't encountered a command separator
                end_idx++; 
                continue; // we just increment the user input index;
            }
            // else, we need to execute a command
            size_t command_size = end_idx-start_idx;
            char *curr_command = strndup(userInput + start_idx, command_size); // allocate and dup substring of userinput that corresponds to the separated command
            errorCode = parseInput(curr_command);
            free(curr_command);
            if (errorCode == -1) exit(99);	// ignore all other errors 
            
            if (userInput[end_idx] == '\n') { // if we are done with the line (one-liners or a single command)
                break; // we are done with the current input
            }

            else { // if we only encountered a ';' separator, we need to keep iterating
                end_idx++;
                start_idx = end_idx;
            }
        }
        //one liner logic stops here;
        memset(userInput, 0, sizeof(userInput));
    }

    return 0;
}

int wordEnding(char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ';
}

int parseInput(char inp[]) {
    char tmp[200], *words[100];                            
    int ix = 0, w = 0;
    int wordlen;
    int errorCode;
    while (inp[ix] != '\n' && inp[ix] != '\0' && ix < 1000) {
        for (ix; inp[ix] == ' ' && ix < 1000; ix++); // skip white spaces
        // extract a word
        for (wordlen = 0; !wordEnding(inp[ix]) && ix < 1000; ix++, wordlen++) {
            tmp[wordlen] = inp[ix];                        
        }
        tmp[wordlen] = '\0';
        words[w] = strdup(tmp);
        w++;
        if (inp[ix] == '\0') break;
        ix++; 
    }
    /*for (int i = 0; i<w; i++) {
        printf("%s\r\n", words[i]);
    }*/
    errorCode = interpreter(words, w);
    return errorCode;
}