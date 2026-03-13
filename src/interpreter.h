#ifndef INTERPRETER_H
#define INTERPRETER_H
int interpreter(char *command_args[], int args_size);
int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int exec(int argc, char *argv[]);
int echo(char *token);
int my_ls();
int my_mkdir(char *dirname);
int my_touch(char *filename);
int my_cd(char *dirname);
int run(char *args[]);
int badcommandFileDoesNotExist();
#endif