//
// Created by kent480 on 1/25/22.
// 344 Operating Systems
// section 400
// Assignment3: smallsh
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>


//---------variables and functions
void command_prompt();
char *get_line();
void expansion(char *source_string, char *find, char *exchange_with);
void clean_up();
//int execute_commands(char *first_command, char **arr_options );
int execute_commands();
void update_status(char *string);
void add_pid(int x);
void clear_pids(); //did not write yet
//-------------------------------

struct Comms
{
    char command[2048];
    char *options_list[512];
    char less[1000];
    char greater[1000];
    char amper[3];
    char status[1000];
    int pid_array[200];

} user_commands;
//UPDATE
void update_status(char *string)
{
    memset(user_commands.status, '\0', strlen(user_commands.status));
    strcpy(user_commands.status, string);
}
void add_pid(int x)
{
    int i;
    for (i = 0; i < sizeof(user_commands.pid_array); i++)
    {
        if (user_commands.pid_array[i] == 0)
        {
            user_commands.pid_array[i] = x;
            break;
        }
    }
}

/*
-------citation expansion function
 date: 01/28/2022
 copied from /OR/ adapted from /OR/ based on:
 http://www.youtube.com/watch?v=yoa_mMmvlMc1
-------
 */
void expansion(char *source_string, char *find, char *exchange_with)
{
    char holder[10240] = {0};
    char *pointing_at = &holder[0];
    char *temp_string = source_string;
    size_t find_len = strlen(find);
    size_t exchange_len = strlen(exchange_with);

    while(1)
    {
        char *p = strstr(temp_string, find);
        if(p == NULL)
        {
            strcpy(pointing_at, temp_string);
            break;
        }
        memcpy(pointing_at, temp_string, p - temp_string);
        pointing_at += p -temp_string;

        memcpy(pointing_at, exchange_with, exchange_len);
        pointing_at +=exchange_len;

        temp_string = p +find_len;
    }
    strcpy(source_string, holder);
}
void clean_up()
{
    int i;
    fflush(stdout);
    fflush(stdin);
    memset(user_commands.command, '\0', 2048);
    memset(user_commands.less, '\0', 1000);
    memset(user_commands.greater, '\0', 1000);
    memset(user_commands.amper, '\0', 3);

    // for (i = 0; i < sizeof (user_commands.options_list)/sizeof (user_commands.options_list[0]); i++)
    // {
    //     user_commands.options_list[i] = NULL;
    //     free(user_commands.options_list[i]);
    // }
//UPDATE
    i = 0;
    while(1)
    {
        if (user_commands.options_list[i] == NULL)
        {
            break;
        }
        if(user_commands.options_list[i] != NULL){
            free(user_commands.options_list[i]);
            user_commands.options_list[i] = NULL;

        }
        i++;
    }
}
/*
------citation for execute_commands
 date: 02/01/2022
 copied from /OR/ adapted from /OR/ based on :
 https://www.geeksforgeeks.org/difference-fork-exec/
 https://www.geeksforgeeks.org/wait-system-call-c/
 explorations: executing a new program, processes and I/O
 https://stackoverflow.com/questions/47441871/why-should-we-check-wifexited-after-wait-in-order-to-kill-child-processes-in-lin
 https://stackoverflow.com/questions/33508997/waitpid-wnohang-wuntraced-how-do-i-use-these/34845669
----------
 */

//int execute_commands(char *first_command, char **arr_options)
int execute_commands()
{
    char *args[512] = {NULL};
    int num_elements;
    int index_args;
    int index_arr_options;
    int i;
    index_arr_options = 0;
    num_elements = 0;
    index_args = 1;
//    args[0] = first_command;
    pid_t first_pid;
    pid_t second_pid;
    int state;
    args[0] = user_commands.command;
    while(1)
    {
//        if (arr_options[index_arr_options] == NULL)
        if(user_commands.options_list[index_arr_options] == NULL)
        {
            break;
        }
//        args[index_args] = arr_options[index_arr_options];
        args[index_args] = user_commands.options_list[index_arr_options];
        index_arr_options++;
        index_args++;
    }
    //---------handle redirections
//    if ((strcmp(user_commands.greater, "") != 0))//not empty
//    {
//        int targetFD = open(user_commands.greater, O_WRONLY | O_CREAT | O_TRUNC, 0777);
//        if (targetFD == -1)
//        {
//            perror("target open()");
//            exit(1);
//        }
//        dup2(targetFD, 1);
//        fcntl(targetFD, F_SETFD, FD_CLOEXEC);
//    }
    //-----------

    pid_t spawnPid;
    int childExitStatus;

    spawnPid = fork();
    switch(spawnPid)
    {
        // if (spawnPid == -1)
        case -1:
        {
            perror("fork failed!");
            break;
            // exit(1);
        }
            // else if (spawnPid == 0)
        case 0:
        {
            printf("child (%d) \n", getpid());
//        sleep(10);
            if ((strcmp(user_commands.greater, "") != 0))//not empty
            {
                int targetFD = open(user_commands.greater, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                if (targetFD == -1)
                {
//                perror("target open()");
                    printf("ERROR. cannot open");
                    exit(1);
                }
                dup2(targetFD, 1);
                fcntl(targetFD, F_SETFD, FD_CLOEXEC);
                fflush(stdout);

            }
            if ((strcmp(user_commands.less, "") != 0))//not empty
            {
                int source_file = open(user_commands.less, O_RDONLY);
                if (source_file == -1)
                {
//                perror("cannot open for input");
                    printf("error cannot open %s for input\n", user_commands.less);
                    exit(1);
                }
                dup2(source_file, 0);
                fcntl(source_file, FD_CLOEXEC);
            }
            execvp(user_commands.command, args);
            perror("child: exec failed\n");
            // exit(2);
            break;
            // return 2;
        }
            // else
        default:
        {
            printf("childs pid = %d\n", spawnPid);
            fflush(stdout);

//        spawnPid = waitpid(spawnPid, &childExitStatus, WNOHANG);
            if (strcmp(user_commands.amper, "") == 0)
            {
                printf("FOREGROUND\n");
                fflush(stdout);

                spawnPid = (waitpid(spawnPid, &childExitStatus, 0));
                if (WIFEXITED(childExitStatus))
                {
                    printf("exit status child--> %d\n", WIFEXITED(childExitStatus));
                    printf("child %d exited normal\n", spawnPid, WEXITSTATUS(childExitStatus));
                    fflush(stdout);
                    // break;

                }
                else
                {
                    printf("exit status child--> %d\n", WIFEXITED(childExitStatus));
                    printf("child %d ABNORMALLY exited due to signal\n", spawnPid, WTERMSIG(childExitStatus));
                    fflush(stdout);
                    // break;

                }
            }
            else if (strcmp(user_commands.amper, "") != 0)
            {
                printf("BACKGROUND\n");
                fflush(stdout);
                sleep(1);
                spawnPid = (waitpid(spawnPid, &childExitStatus, WNOHANG));


                if (WIFEXITED(childExitStatus))
                {
                    printf("exit status child--> %d\n", WIFEXITED(childExitStatus));
                    printf("child %d exited normal\n", spawnPid, WEXITSTATUS(childExitStatus));
                    fflush(stdout);
                    // break;

                }
                else
                {
                    printf("exit status child--> %d\n", WIFEXITED(childExitStatus));
                    printf("child %d ABNORMALLY exited due to signal %d\n", spawnPid, WTERMSIG(childExitStatus));
                    fflush(stdout);
                    // break;

                }
                // break;
                return 0;

                // exit(0);
            }

            break;

        }
    }

    return 0;

}

void command_prompt()
{
    int delete_temp = 100;
    int delete_count = 0;
    int condition;
    condition = 0;
    int i;
    int compare;
    int counter;
    counter = 0;
    compare = 0;
    //-----getline
    char *string_input = NULL;
    size_t string_input_len = 0;
//UPDATE
    //------------fill struct
    memset(user_commands.command, '\0', 2048);
    memset(user_commands.less, '\0', 1000);
    memset(user_commands.greater, '\0', 1000);
    memset(user_commands.amper, '\0', 3);
    memset(user_commands.pid_array, 0, sizeof(user_commands.pid_array));
    memset(user_commands.status, '\0', sizeof(user_commands.status));
    strcpy(user_commands.status, "exit value 0");
    //----get pid change to string
    char string_pid[8] = {'\0'};
    //---directory handling
    char buff[FILENAME_MAX];
    if (getcwd(buff, sizeof buff) == NULL)
    {
        perror("cwd error\n");
    }
    //-------
    //-------kill children
    waitpid(-1, NULL, WNOHANG);
    //------------------

    //-----temp input
//    string_input = "ls -la";
    //-----
    do
//    while(delete_count < delete_temp)
    {
        //-------------
        delete_count++;
        //-------------

        printf("> "); //the prompt
        getline(&string_input, &string_input_len, stdin);
        string_input[strlen(string_input)-1] = '\0';
//        printf("%s\n", string_input);
        if (*string_input == '\0' || string_input[0] == 35)
        {continue;}
        if((strstr(string_input, "$$")) != NULL)
        {
//            printf("found double dollas\n");
            pid_t pid = getpid();
            sprintf(string_pid, "%d", pid);
            expansion(string_input, "$$", string_pid);

//            printf("string pid worked --> %s\n", string_input);
        }

        if (string_input[strlen(string_input) -1] == 38 && string_input[strlen(string_input) -2] == 32 )
        {
            printf("found & at the end\n");
        }
        printf("fill the struct like normal\n");

        char *save_pointer;
        char *token = strtok_r(string_input, " ", &save_pointer);
        strcpy(user_commands.command, token);

        do
        {
            token = strtok_r(NULL, " ", &save_pointer);

            if (token == 0)
            {
                break;
            }
            else if (*token == 60)
            {
                printf("less than found\n");
                token = strtok_r(NULL, " ", &save_pointer);
                strcpy(user_commands.less, token);
//                token = strtok_r(NULL, " ", &save_pointer);
//                continue;
                compare = 1;

            }
            else if (*token == 62)
            {
                printf("great than found\n");
                token = strtok_r(NULL, " ", &save_pointer);
                strcpy(user_commands.greater, token);
//                continue;
                compare = 1;

            }
            else if (*token == 38)
            {
                printf("& FOUND\n");
//                token = strtok_r(NULL, " ", &save_pointer);
                token ++;
                strcpy(user_commands.amper, "yes");
                compare = 1;
            }
//            if (token == 0)
//            {
//                //break;
//                continue;
//            }
            user_commands.options_list[counter] = malloc((strlen(token) +1) * sizeof (char));
            if (compare != 0) {
                user_commands.options_list[counter] = NULL;
            } else {
                strcpy(user_commands.options_list[counter], token);
            }
//            strcpy(user_commands.options_list[counter], token);
            compare = 0;
            counter++;
        } while(1);
        counter = 0; //reset counter to first argument position
        //checking if command is filled out-----------
        printf("commands--> %s\n", user_commands.command);
        for (i = 0; i < 4; i++)
        {
            printf("args %d --> %s\n", i, user_commands.options_list[i]);
        }
        printf("less than --> %s\n", user_commands.less);
//        if (strcmp(user_commands.less, "") == 0){printf("its empty\n");}
        printf("greater than --> %s\n", user_commands.greater);
        printf("ampered up --> %s\n", user_commands.amper);
        //---------------------------------------
        //---get pwd
        if ((strcmp(user_commands.command, "")) != 0)
        {
            if (strcmp(user_commands.command, "pwd") == 0) //strcmp return zero if equal
            {
                //printf("asking for directory\n");
                //---moves up to declarations; char buff[FILENAME_MAX];

                printf("%s\n", buff);
//                    system("ls -l");
                clean_up();
                continue;

            }
            else if (strcmp(user_commands.command, "cd") == 0 && user_commands.options_list[0] == NULL)
            {

//                printf("%s\n", buff);
//                system("ls -l");

//                printf("getenv--> %s\n", getenv("HOME"));
                chdir(getenv("HOME"));
                if (chdir(getenv("HOME")) == 0)
                {
                    if (getcwd(buff, sizeof buff) == NULL)
                    {
                        perror("cwd error\n");
                    }
                }
                clean_up();
                continue;

            }
            else if (strcmp(user_commands.command, "cd") == 0)
            {
                int change_result;
                change_result = chdir(user_commands.options_list[0]);
                if (change_result == 0)
                {
                    printf("dir changed\n");
                    if (getcwd(buff, sizeof buff) == NULL)
                    {
                        perror("cwd error\n");
                    }
                    clean_up();
                    continue;
                }
                else
                {
                    printf("error, bad dir request\n");
                    clean_up();
                    continue;
                }
//                system("ls -l");
            }
            else if (strcmp(user_commands.command, "status") == 0)
            {
//                printf("YOU NEED TO FILL OUT STATUS STUFF");
                printf("%s\n", user_commands.status);
                clean_up();
                continue;
            }
            else if (strcmp(user_commands.command, "exit") == 0)
            {
                exit(0);
            }
            //----execute block

//            execute_commands(user_commands.command, user_commands.options_list);
            execute_commands();
            clean_up();
            continue;
            //---

        }//RUN EVERYTHING ABOVE THIS
//        execute_commands(user_commands.command, user_commands.options_list);

        //---
//        printf("less than --> %s\n", user_commands.less);
//        printf("greater than --> %s\n", user_commands.greater);
//        printf("ampered up --> %s\n", user_commands.amper);
//        clean_up();

        //--------confirm cleanup prints
//        printf("commands--> %s\n", user_commands.command);
//        for (i = 0; i < 5; i++)
//        {
//            printf("args %d --> %s\n", i, user_commands.options_list[i]);
//        }
//        printf("less than --> %s\n", user_commands.less);
//        printf("greater than --> %s\n", user_commands.greater);
//        printf("ampered up --> %s\n", user_commands.amper);
        //-------------


        //call fflush at end after each print
    }while (delete_count <= delete_temp);

}

int main(int argc, char **argv)

{
    command_prompt();

    return 0;
}

//
