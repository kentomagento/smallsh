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
#include <signal.h>

//UPDATE

//---------variables and functions
void command_prompt();
char *get_line();
void expansion(char *source_string, char *find, char *exchange_with);
void clean_up();
//int execute_commands(char *first_command, char **arr_options );
int execute_commands();
void update_status(char *string);
void add_pid(int x);
// void kill_kids();
int kill_kids();
void enter_foreground(int x);
void handler_interrupt(int z);
void handler_twp(int w);
int execute_commands_background();
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
    int foreground;
    int current;

} user_commands;

//UPDATE
void get_pids()
{
    int i;
    // i = 0;
    // for(i = 0; i < 4; i ++)
    // {
    //   printf("pid %d => %d\n", i, user_commands.pid_array[i]);
    // }
    // do
    // {
    //     if (user_commands.pid_array[i] != 0)
    //     {
    //         printf("pid #%d --> %d\n", i, user_commands.pid_array[i]);
    //     }
    //     i++;
    // } while (user_commands.pid_array[i] != 0);
}
void update_status(char *string)
{
    memset(user_commands.status, '\0', strlen(user_commands.status));
    strcpy(user_commands.status, string);
}
//ADDD PID
void add_pid(int x)
{
    int i;
    for (i = 1; i < sizeof(user_commands.pid_array); i++)
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
//        fcntl(targetFD, F_SETFD, FD_CLOEXEC);a
//    }
    //-----------

    pid_t spawnPid;
    int childExitStatus;
    user_commands.pid_array[0] = 1;
    spawnPid = fork();
    // int pid = (waitpid(-1, NULL, WNOHANG));
    // printf("pid result --> %d; the pide is --> %d\n", pid, spawnPid);
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
            // printf("child (%d) \n", getpid());
            // fflush(stdout);
            // usleep(5);
            // write(0, "\n", 1);
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
//EXECUTION
            execvp(user_commands.command, args);
            // perror("Foreground: exec failed\n");

            perror(user_commands.command);
            // exit(2);
            break;
            // return 2;
        }
            // else
        default:
        {
            // printf("FOREGROUND childs pid = %d\n", spawnPid);
            // write(0, "\n", 1);
            //-----adding pids to array

            //--------------
            fflush(stdout);

//        spawnPid = waitpid(spawnPid, &childExitStatus, WNOHANG);

            user_commands.current = spawnPid;
            // printf("FOREGROUND \n");
            // fflush(stdout);

            spawnPid = (waitpid(spawnPid, &childExitStatus, 0));

            if (WIFEXITED(childExitStatus))
            {
                // printf ("current FOREGROUND PID --> %d\n", user_commands.current);
                // printf("FOREGROUND exit status child--> %d\n", WIFEXITED(childExitStatus));
                // printf("FOREGROUND child %d exited normal with %d\n", spawnPid, WEXITSTATUS(childExitStatus));
                update_status("exit status 0");
                fflush(stdout);
                break;

            }
            else
            {
                // printf("FOREGROUND exit status child--> %d\n", WIFEXITED(childExitStatus));
                // printf("FOREGROUND child %d ABNORMALLY exited due to signal %d\n", spawnPid, WTERMSIG(childExitStatus));
                char abnormal[70] = {'\0'};
                sprintf(abnormal, "terminated by signal %d", WTERMSIG(childExitStatus));
                update_status(abnormal);
                fflush(stdout);
                break;

            }
            waitpid(spawnPid, NULL, WNOHANG);

            exit(0);

        }
            exit(0);
    }
    // exit();
    return 0;

}

int execute_commands_background()
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
    // int pid = (waitpid(-1, NULL, WNOHANG));
    // printf("pid result --> %d; the pide is --> %d\n", pid, spawnPid);
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
            int targetFD = open("/dev/null", O_RDWR);
            if (targetFD == -1)
            {
                printf("could not open\n");
            }
            int source_file = open("/dev/null", O_RDWR);
            if (source_file == -1)
            {
                printf("could not open\n");
            }

            dup2(source_file, 0);
            dup2(targetFD, 1);
            // dup2(open("/dev/null/", O_PATH),0);
            printf("child (%d) \n", getpid());
            // usleep(5);
            // write(0, "\n", 1);
//        sleep(10);
            if ((strcmp(user_commands.greater, "") != 0))//not empty
            {
                // int targetFD = open(user_commands.greater, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                targetFD = open(user_commands.greater, O_WRONLY | O_CREAT | O_TRUNC, 0777);
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
            else

            if ((strcmp(user_commands.less, "") != 0))//not empty
            {
                // int source_file = open(user_commands.less, O_RDONLY);
                source_file = open(user_commands.less, O_RDONLY);
                if (source_file == -1)
                {
//                perror("cannot open for input");
                    printf("error cannot open %s for input\n", user_commands.less);
                    exit(1);
                }
                dup2(source_file, 0);
                fcntl(source_file, FD_CLOEXEC);
            }

//EXECUTION
            //  DIR *currDir = opendir("/dev/null");
            // printf("changing stdout\n");
            // fflush(stdout);
            // int dev_null_out = open("/dev/null", O_RDONLY);
            // // open("/dev/null", O_RDONLY);
            // dup2(dev_null_out, 0);
            // fcntl(dev_null_out, F_SETFD, FD_CLOEXEC);
            //------------------------
            struct sigaction SIGINT_action = {0}, ignore_action = {0};
            SIGINT_action.sa_handler = SIG_IGN;
            ignore_action.sa_handler = SIG_IGN;
            sigaction(SIGINT, &ignore_action, NULL);
            sigaction(SIGINT, &SIGINT_action, NULL);
            //---------------------------
            execvp(user_commands.command, args);
            perror("child: exec failed\n");
            // exit(2);
            break;
            // return 2;
        }
            // else
        default:
        {
            // printf("childs pid = %d\n", spawnPid);
            // write(0, "\n", 1);
            //-----adding pids to array

            //--------------
            fflush(stdout);
//        spawnPid = waitpid(spawnPid, &childExitStatus, WNOHANG);

            user_commands.current = spawnPid;
            // printf("BACKGROUND\n");
            printf("background pid is %d\n", spawnPid);
            add_pid(spawnPid);
            fflush(stdout);

            spawnPid = (waitpid(spawnPid, &childExitStatus, WNOHANG));

            // if (WIFEXITED(childExitStatus))
            // {
            //     // printf ("current FOREGROUND PID --> %d\n", user_commands.current);
            //     printf("BACKGROUND exit status child--> %d\n", WIFEXITED(childExitStatus));
            //     printf("BACKGROUND child %d exited normal\n", spawnPid, WEXITSTATUS(childExitStatus));
            //     fflush(stdout);
            //     break;

            // }
            // else
            // {
            //     printf("BACKGROUND exit status child--> %d\n", WIFEXITED(childExitStatus));
            //     printf("BACKGROUND child %d ABNORMALLY exited due to signal %d\n", spawnPid, WTERMSIG(childExitStatus));
            //     fflush(stdout);
            //     break;

            // }
            // exit(0);

        }
    }

    return 0;

}


//BOOKMARK
// void kill_kids()
int kill_kids()
{
    int i;
    //  i = 0;
    int result;
    int run;
    int status;
    char stat[1000] = {0};
    //  pid_t chek_pid = wait(&status);
    // printf("%d\n", user_commands.current);
    get_pids();
    if (user_commands.pid_array[0] == 1)
    {
        user_commands.pid_array[0] = 0;

        return 0;
    }
    if(user_commands.current == user_commands.pid_array[i]){
        return 0;
    }

    for (i = 1; i < 512; i++)
    {
        if (user_commands.pid_array[i] == 0 || (user_commands.pid_array[i] == 1))
        {
            continue;
            // break;
        }

        result = waitpid(user_commands.pid_array[i], &status, WNOHANG); //checks if any completed and return 0 if they're still running
        if (result != 0)
        {
            if(user_commands.current == user_commands.pid_array[i]){
                continue;
            }
            if (WIFEXITED(status) != 0)
            {
//                printf("extra\n");
                printf("back ground pid %d is done: exit value %d\n", user_commands.pid_array[i], WIFSIGNALED(status));

                sprintf(stat, "exit value %d", WIFSIGNALED(status));
                update_status(stat);
                user_commands.pid_array[i] = 0;
            }
            else
            {

                // printf("pid --> %d, ABNORMALLY EXITED --> %d\n", user_commands.pid_array[i], WTERMSIG(status));
                printf("back ground pid %d is done: terminated by signal %d\n", user_commands.pid_array[i], WTERMSIG(status));
                sprintf(stat, "exit value %d", WTERMSIG(status));
                update_status(stat);
                user_commands.pid_array[i] = 0;
            }

        }

    }
    memset(stat, '\0', strlen(stat));
    return 0;
}
void enter_foreground(int x)
{
    if (user_commands.foreground == 0)
    {
        char *message = "Entering foregrond-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, message, 48);
        fflush(stdout);
        user_commands.foreground = 1;
    }
    else if (user_commands.foreground == 1)
    {
        char *message = "Exiting foregrond-only mode\n";
        write(STDOUT_FILENO, message, 28);
        fflush(stdout);
        user_commands.foreground = 0;
    }

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
    user_commands.foreground = 0; //zero/false foreground mode
    //----get pid change to string
    char string_pid[8] = {'\0'};
    //---directory handling
    char buff[FILENAME_MAX];
    if (getcwd(buff, sizeof buff) == NULL)
    {
        perror("cwd error\n");
    }
    //-------

    //-----temp input
//    string_input = "ls -la";
    //-----
    do
//    while(delete_count < delete_temp)
    {



        //-------------
        delete_count++;
        //-------------


        kill_kids();

        // kill_kids();
        usleep(2900);

        // sleep(0.99);

        printf(":: "); //the prompt
        int line_check = getline(&string_input, &string_input_len, stdin);
        // getline(&string_input, &string_input_len, stdin);
        if (line_check == -1)
        {
            clearerr(stdin);
            break;
        }
        string_input[strlen(string_input)-1] = '\0';
//        printf("%s\n", string_input);
        if (*string_input == '\0' || string_input[0] == 35)
        {
            continue;}
        if (strcmp(user_commands.command, "exit") == 0) {exit(0); }
        if((strstr(string_input, "$$")) != NULL)
        {
//            printf("found double dollas\n");
            pid_t pid = getpid();
            sprintf(string_pid, "%d", pid);
            expansion(string_input, "$$", string_pid);

//            printf("string pid worked --> %s\n", string_input);
        }
        //MAYBE MAYBE MAYBE
        // if (string_input[strlen(string_input) -1] == 38 && string_input[strlen(string_input) -2] == 32 )
        // {
        //     printf("found & at the end\n");
        // }


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
                // printf("less than found\n");
                token = strtok_r(NULL, " ", &save_pointer);
                strcpy(user_commands.less, token);
//                token = strtok_r(NULL, " ", &save_pointer);
//                continue;
                compare = 1;

            }
            else if (*token == 62)
            {
                // printf("great than found\n");
                token = strtok_r(NULL, " ", &save_pointer);
                strcpy(user_commands.greater, token);
//                continue;
                compare = 1;

            }
            else if (*token == 38)
            {
                if ((strcmp(user_commands.command, "status") == 0))
                {
                    break;
                }
                //  printf("& FOUND\n");
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
        //  printf("commands--> %s\n", user_commands.command);
        //  for (i = 0; i < 4; i++)
        //  {
        //      printf("args %d --> %s\n", i, user_commands.options_list[i]);
        //  }
        //  printf("less than --> %s\n", user_commands.less);
        //        if (strcmp(user_commands.less, "") == 0){printf("its empty\n");}
        //  printf("greater than --> %s\n", user_commands.greater);
        //  printf("ampered up --> %s\n", user_commands.amper);
//         printf("foreground mode --> %d\n", user_commands.foreground);
//         printf("current child pid --> %d\n", user_commands.current);
//         getpid();
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
                        perror("cwd error: could not change Dir\n");
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
                    printf("Error, bad dir request\n");
                    clean_up();
                    continue;
                }
//                system("ls -l");
            }
            else if (strcmp(user_commands.command, "status") == 0)
            {
//                printf("YOU NEED TO FILL OUT STATUS STUFF");
                printf("%s\n", user_commands.status);
                fflush(stdout);
                // get_pids();
                // get_pids();
                // kill_kids();
                // get_pids();
                clean_up();
                continue;
            }
            else if (strcmp(user_commands.command, "exit") == 0)
            {
                exit(0);
                break;
            }
            //----execute block

//            execute_commands(user_commands.command, user_commands.options_list);
            if ((strcmp(user_commands.amper, "") != 0 && user_commands.foreground == 0))
            {
                execute_commands_background();
            }
            else
            {

                execute_commands();
            }
            // execute_commands();
            kill_kids();
            clean_up();
            // printf("less than --> %s\n", user_commands.less);
            // printf("greater than --> %s\n", user_commands.greater);
            // printf("ampered up --> %s\n", user_commands.amper);
            // printf("current child pid --> %d\n", user_commands.current);
            // getpid();
            continue;
            //---

        }//RUN EVERYTHING ABOVE THIS
//        execute_commands(user_commands.command, user_commands.options_list);

    }while (1);

}
void handler_interrupt(int z)
{

    // if (user_commands.current != 0)
    if (user_commands.pid_array[0] == 1)
    {
        char message[] = "terminated by signal 2\n";
        // update_status(message);
        write(STDOUT_FILENO, message, 23);
        fflush(stdout);
        kill(user_commands.current, SIGINT);
    }
}


int main(int argc, char **argv)

{
    struct sigaction SIGTSTP_action = {0}, SIGINT_action = {0}, ignore_action = {0}, SIGUSR2_action = {0};
    // struct sigaction SIGTSTP_action = {0}, ignore_action = {0};
    SIGTSTP_action.sa_handler = enter_foreground;
    sigfillset(&SIGTSTP_action.sa_mask);
    // SIGTSTP_action.sa_flags = 0;
    SIGTSTP_action.sa_flags = SA_RESTART;char _pid[50] = {'\0'};
    sprintf(_pid, "%d", user_commands.current);

    SIGINT_action.sa_handler = handler_interrupt;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = SA_RESTART;

    //---------------
    // ignore_action.sa_handler = SIG_IGN;

    // sigaction(SIGTSTP, &ignore_action, NULL);
    //---------------



    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
    sigaction(SIGINT, &SIGINT_action, NULL);

    command_prompt();

    return 0;
}

//
