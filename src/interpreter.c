#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellmemory.h"
#include "shell.h"
#include <dirent.h>
#include "helper.h"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

int MAX_ARGS_SIZE = 3;

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int echo(char *token);
int my_ls();
int my_mkdir(char *dirname);
int my_touch(char *filename);
int my_cd(char *dirname);
int run(char *args[]);
int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    /*for (i = 0; i < args_size; i++) {
        printf("%s\n", command_args[i]);
    }*/

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }



    if (strcmp(command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);

    } else if (strcmp(command_args[0], "echo") == 0) {
        if (args_size != 2)
            return badcommand();
        return echo(command_args[1]);

    } else if (strcmp(command_args[0], "my_ls") == 0) {
        if (args_size != 1)
            return 1;
        return my_ls();
    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        if (args_size != 2)
            return 1;
        return my_mkdir(command_args[1]);
    } else if (strcmp(command_args[0], "my_touch") == 0) {
        if (args_size != 2)
            return 1;
        return my_touch(command_args[1]);
    } else if (strcmp(command_args[0], "my_cd") == 0) {
        if (args_size != 2)
            return 1;
        return my_cd(command_args[1]);
    } else if (strcmp(command_args[0], "run") == 0) {
        if (args_size < 2)
            return 1;
        for (int i = 0; i < args_size - 1; i++) { // we shift the argument array by 1, because we don't need
            command_args[i] = command_args[i+1];  // to keep the first "run" entry, this allows us to set the last
        }                                         // element to NULL, which is necessary for execvp() inside run()
        command_args[args_size-1] = NULL;           // (execvp stops its argument list at NULL (sets the bound))
        return run(command_args);
    } 
    else
        return badcommand();
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}


int print(char *var) {
    printf("%s\n", mem_get_value(var));
    return 0;
}

int source(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, MAX_USER_INPUT - 1, p);
    while (1) {
        errCode = parseInput(line);     // which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
        fgets(line, MAX_USER_INPUT - 1, p);
    }

    fclose(p);

    return errCode;
}

int echo(char *token) {
    char *output = parseToken(token);


    if (!is_alphanumeric(output)) {
        printf("input or input value is not alphanumeric\n");
        return 1;
    }

    printf("%s\n", output);
    return 0;
}

int my_ls() {
    DIR *dir_stream = opendir("."); // opens a directory stream associated with the current dir
    struct dirent *entry; // dirent struct contains directory/file names and types

    int names_size = 8;
    int names_count = 0;
    char **names = malloc(names_size * sizeof(char *));
    

    if (dir_stream == NULL) {
        printf("couldn't access current directory");
        return 1;
    }

    while ((entry = readdir(dir_stream)) != NULL) {
        char *entry_name = strdup(entry->d_name);
        if (names_count >= names_size) {
            names_size *= 2;
            char **temp = realloc(names, names_size * sizeof(char *)); // reallocating to temp prevents memory leak in case it fails
            if (temp) names = temp;
            else {
                printf("couldn't reallocate space for the names array\n");
                free_array(names, names_count);
                free(entry_name);
                closedir(dir_stream);
                return 1;
            }
        }
        names[names_count] = entry_name;
        names_count ++;
    }
    bubble_sort_alphabetical(names, names_count);

    for (int i = 0; i<names_count; i++) {
        printf("%s\n", names[i]);
        free(names[i]);
    }

    free(names);
    closedir(dir_stream);
}

int my_mkdir(char *dirname) {

    char *output = parseToken(dirname);
    //printf("%s\n", output);
    int status = 1;

    if ((!is_alphanumeric(output)) || (status = mkdir(output, 0755) != 0)) { // 0755 corresponds to rwxr-xr-x permissions
        printf("Bad command: my_mkdir\n"); // the ordering of the if statement above prevents
    }                                    // creating the directory if the output isn't alphanumeric
    return status;
}

int my_touch(char *filename) {
    if (filename[0] == '\0' || !is_alphanumeric(filename)) {
        printf("Invalid file name\n");
        return 1;
    }
    FILE *new_file = fopen(filename, "w");
    if (!new_file) {
        printf("Couldn't create file\n");
        return 1;
    }
    fclose(new_file);
    return 0;
}

int my_cd(char *dirname) {
    if (dirname[0] == '\0' || !is_alphanumeric(dirname)) {
        printf("Invalid directory name\n");
        return 1;
    }
    if (chdir(dirname) == -1) { // chdir does "cd $dirname", returns -1 if it fails
        printf("Bad command: my_cd\n");
        return 1;
    }
    return 0;
}

int run(char *args[]) {

    pid_t pid = fork();

    if (pid == -1) {
        printf("Fork creation failed\n");
        return 1;
    }
    
    else if (pid) { // if we are in the parent process we wait

        if (waitpid(pid, NULL, 0) != pid) {             // waitpid waits specifically for the child process with id = pid, 2nd
            printf("Child process hasn't exited\n");  // arg serves to report the status of the child process, we pass NULL
            return 1;                                   // to not receive any status, 3rd argument is for options, we pass 0 to execute with none
        }
    } 
    else { // we are in the children process
        if (execvp(args[0], args) == -1) {                  //v -> vector, which allows to pass an array of argument as its 2nd argument, 
            printf("Child process failed to execute\n");  //p -> PATH, asks the OS to check the PATH env variable, PATH is a list
            exit(1);                                       //     containing all directories which have executable files --> allows to
        }                                                   //     simply pass the command name as 1st argument, instead of a hardcoded path
    }                                                       //execvp returns -1 only if it fails
    return 0;                      
}  
                                   
       

		
