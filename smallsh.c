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


//---------variables and functions
void command_prompt();
void expansion(char *source_string, char *find, char *exchange_with);
void clean_up();
int execute_commands();
void update_status(char *string);
void add_pid(int x);
int kill_kids();
void enter_foreground();
void handler_interrupt(int z);
int execute_commands_background();
//-------------------------------

/*
 * Below is the imperative struct that will be referenced throughout
 * this program.
 *
 * --members for command to hold the first command checked in
 * by built in statuses or to the library of programs if not built in,
 * --options list to hold arguments, or options for execution
 * --less will hold redirect for input
 * --greater holds redirect for greater than symbol
 * --amper will hold a value for when there is an '&' in the user input for background execution
 * --status holds status of previous child termination for calling by
 * built-in function 'status'
 * --pid array will hold child pid array
 * for checking by kill kids function dictating which processes will be reaped
 * --foreground member will hold value to indicate whether to initialize
 * foreground mode by signal control Z
 * --current holds value of the foreground child processes to dictated which children
 * will be reaped and also what process to interrupt with control C
 */
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


/*
 * Will take in the status of an exited child and save it for
 * calling from the built in 'status' command
 */
void update_status(char *string)
{
    memset(user_commands.status, '\0', strlen(user_commands.status));
    strcpy(user_commands.status, string);
}

/*
 * Add pid function will add child pids to array in
 * struct which will be checked on by the kill kids function
 * returning termination status and reaping children
 */
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
-------citation expansion function----
 date: 01/28/2022
 copied from /OR/ adapted from /OR/ based on:
 http://www.youtube.com/watch?v=yoa_mMmvlMc1
-------
 */
/*
 * Expansion function will take in the original string, character symbols,
 * and the pid to exchange it with, will parse and rebuild the string in
 * a character array, then copy it back to the original string for parsing
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

/*
 * Function will be called to clean out struct for new user input
 */
void clean_up()
{
    int i;
    memset(user_commands.command, '\0', 2048);
    memset(user_commands.less, '\0', 1000);
    memset(user_commands.greater, '\0', 1000);
    memset(user_commands.amper, '\0', 3);

    for (i = 0; i < sizeof (user_commands.options_list)/sizeof (user_commands.options_list[0]); i++)
    {

        free(user_commands.options_list[i]);
        user_commands.options_list[i] = NULL;
    }
}
/*
------citation for execute_commands function below
 date: 02/01/2022
 copied from /OR/ adapted from /OR/ based on :
 https://www.geeksforgeeks.org/difference-fork-exec/
 https://www.geeksforgeeks.org/wait-system-call-c/
 explorations: executing a new program, processes and I/O
 https://stackoverflow.com/questions/47441871/why-should-we-check-wifexited-after-wait-in-order-to-kill-child-processes-in-lin
 https://stackoverflow.com/questions/33508997/waitpid-wnohang-wuntraced-how-do-i-use-these/34845669
----------
 */
/*
 * Function will be called to execute fork and exec
 * will execute as a foreground child due to user input
 * takes in arguments and creates a new array specifically
 * for exec
 */
int execute_commands()
{
    char *args[512] = {NULL};
    int index_args;
    int index_arr_options;
    index_arr_options = 0;
    index_args = 1;

    args[0] = user_commands.command;
    while(1)
    {
        if(user_commands.options_list[index_arr_options] == NULL)
        {
            break;
        }
        args[index_args] = user_commands.options_list[index_arr_options];
        index_arr_options++;
        index_args++;
    }

    pid_t spawnPid;
    int childExitStatus;
    user_commands.pid_array[0] = 1;
    spawnPid = fork();
    switch(spawnPid)
    {
        case -1:
        {
            perror("fork failed!");
            fflush(stdout);
            clean_up();
            break;
        }
        case 0:
        {

            /*
            * Redirect input depending on if a '>' was found
            * in user input and parsed out
            */
            if ((strcmp(user_commands.greater, "") != 0))//not empty
            {
                int targetFD = open(user_commands.greater, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                if (targetFD == -1)
                {
                    printf("ERROR. cannot open");
                    fflush(stdout);
                    clean_up();
                    exit(1);
                }
                dup2(targetFD, 1);
                fcntl(targetFD, F_SETFD, FD_CLOEXEC);
                fflush(stdout);

            }
            /*
             * Redirect input depending on if a '<' was found
             * in user input and parsed out
             */
            if ((strcmp(user_commands.less, "") != 0))//not empty
            {
                int source_file = open(user_commands.less, O_RDONLY);
                if (source_file == -1)
                {
                    printf("error cannot open %s for input\n", user_commands.less);
                    fflush(stdout);
                    clean_up();
                    exit(1);
                }
                dup2(source_file, 0);
                fcntl(source_file, FD_CLOEXEC);
            }
            execvp(user_commands.command, args);

            perror(user_commands.command);
            clean_up();
            exit(2);
        }
        default:
        {

            fflush(stdout);
            user_commands.current = spawnPid;

            spawnPid = (waitpid(spawnPid, &childExitStatus, 0));

            if (WIFEXITED(childExitStatus))
            {
                char normal1[70] = {'\0'};
                sprintf(normal1, "exit status %d", WEXITSTATUS(childExitStatus));
                update_status(normal1);
                clean_up();
                fflush(stdout);
                break;

            }
            else
            {
                char abnormal[70] = {'\0'};
                sprintf(abnormal, "terminated by signal %d", WTERMSIG(childExitStatus));
                update_status(abnormal);
                clean_up();
                fflush(stdout);
                break;

            }
        }
    }
    return 0;

}
/*
 * Function will be called to execute fork and exec
 * will execute as a background child due to user input
 * takes in arguments and creates a new array specifically
 * for exec
 */
int execute_commands_background()
{
    char *args[512] = {NULL};
    int index_args;
    int index_arr_options;
    index_arr_options = 0;
    index_args = 1;

    args[0] = user_commands.command;
    while(1)
    {
        if(user_commands.options_list[index_arr_options] == NULL)
        {
            break;
        }
        args[index_args] = user_commands.options_list[index_arr_options];
        index_arr_options++;
        index_args++;
    }

    pid_t spawnPid;
    int childExitStatus;
    spawnPid = fork();
    switch(spawnPid)
    {
        case -1:
        {
            perror("fork failed!");
            fflush(stdout);
            clean_up();
            break;
        }
        case 0:
        {
            /*
             * For background processes where no redirection was
             * found, redirects will go to /dev/null
             */
            int targetFD = open("/dev/null", O_RDWR);
            if (targetFD == -1)
            {
                printf("could not open\n");
                fflush(stdout);
            }
            int source_file = open("/dev/null", O_RDWR);
            if (source_file == -1)
            {
                printf("could not open\n");
                fflush(stdout);
            }

            dup2(source_file, 0);
            dup2(targetFD, 1);
            printf("child (%d) \n", getpid());
            fflush(stdout);

            /*
            * Redirect input depending on if a '>' was found
            * in user input and parsed out
            */
            if ((strcmp(user_commands.greater, "") != 0))
            {
                targetFD = open(user_commands.greater, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                if (targetFD == -1)
                {
                    printf("ERROR. cannot open");
                    clean_up();
                    fflush(stdout);
                    exit(1);
                }
                dup2(targetFD, 1);
                fcntl(targetFD, F_SETFD, FD_CLOEXEC);
                fflush(stdout);

            }
            else

            /*
             * Redirect input depending on if a '<' was found
             * in user input and parsed out
             */
            if ((strcmp(user_commands.less, "") != 0))
            {
                source_file = open(user_commands.less, O_RDONLY);
                if (source_file == -1)
                {
                    printf("error cannot open %s for input\n", user_commands.less);
                    fflush(stdout);
                    clean_up();
                    exit(1);
                }
                dup2(source_file, 0);
                fcntl(source_file, FD_CLOEXEC);
            }


            //-----Passing ignore environment to forked child----------
            struct sigaction SIGINT_action = {0}, ignore_action = {0};
            SIGINT_action.sa_handler = SIG_IGN;
            ignore_action.sa_handler = SIG_IGN;
            sigaction(SIGINT, &ignore_action, NULL);
            sigaction(SIGINT, &SIGINT_action, NULL);
            //--------------------------------------------------------

            execvp(user_commands.command, args);
            perror("child: exec failed\n");
            clean_up();
            fflush(stdout);
            exit(2);
        }
        default:
        {
            fflush(stdout);
            user_commands.current = spawnPid;

            printf("background pid is %d\n", spawnPid);
            add_pid(spawnPid);
            fflush(stdout);

            spawnPid = (waitpid(spawnPid, &childExitStatus, WNOHANG));
        }
    }

    return 0;

}

/*
 * Function that will be called at the end of teh command prompt function and at
 * the beginning of the function to reap finished child processes and update
 * the status
 */
int kill_kids()
{
    int i;
    int result;
    int status;
    char stat[1000] = {0};



    for (i = 1; i < 512; i++)
    {
        if (user_commands.pid_array[i] == 0 || (user_commands.pid_array[i] == 1))
        {
            continue;
            // break;
        }
        //---Using waitpid to reap finished children
        result = waitpid(user_commands.pid_array[i], &status, WNOHANG);
        if (result != 0)
        {
            if(user_commands.current == user_commands.pid_array[i]){
                continue;
            }
            /*
             * Will check how the fork exited and update the status
             */
            if (WIFEXITED(status) != 0)
            {
                printf("back ground pid %d is done: exit value %d\n", user_commands.pid_array[i], WIFSIGNALED(status));
                fflush(stdout);
                sprintf(stat, "exit value %d", WIFSIGNALED(status));
                update_status(stat);
                clean_up();
                user_commands.pid_array[i] = 0;
            }
            else
            {

                printf("back ground pid %d is done: terminated by signal %d\n", user_commands.pid_array[i], WTERMSIG(status));
                sprintf(stat, "exit value %d", WTERMSIG(status));
                fflush(stdout);
                update_status(stat);
                clean_up();
                user_commands.pid_array[i] = 0;
            }

        }

    }
    return 0;
}
/*
 * Signal handling function for control Z, it will start foreground mode
 * sending the signal again will exit foreground mode, will switch foreground
 * member/element of the struct to indicate whether it is intialized for the
 * rest of the program execution
 */
void enter_foreground()
{
    if (user_commands.foreground == 0)
    {
        char *message = "Entering foregrond-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, message, 48);
        fflush(stdout);
        user_commands.foreground = 1;
        clean_up();
    }
    else if (user_commands.foreground == 1)
    {
        char *message = "Exiting foregrond-only mode\n";
        write(STDOUT_FILENO, message, 28);
        fflush(stdout);
        user_commands.foreground = 0;
        clean_up();
    }
}

/*
 * The main command prompt, called from main, will run indefinitely
 * until exiting the program
 */
void command_prompt()
{
    int compare;
    int counter;
    counter = 0;
    compare = 0;
    //-----getline---
    char *string_input = NULL;
    size_t string_input_len = 0;

    /*
     * Filling the struct with intiazl members to clear out
     * memory
     */
    memset(user_commands.command, '\0', 2048);
    memset(user_commands.less, '\0', 1000);
    memset(user_commands.greater, '\0', 1000);
    memset(user_commands.amper, '\0', 3);
    memset(user_commands.pid_array, 0, sizeof(user_commands.pid_array));
    memset(user_commands.status, '\0', sizeof(user_commands.status));
    strcpy(user_commands.status, "exit value 0");
    user_commands.foreground = 0; //zero/false foreground mode signal
    //----get pid change to string
    char string_pid[8] = {'\0'};
    /*
     * Directory handling for the built in Cd command
     */
    char buff[FILENAME_MAX];
    if (getcwd(buff, sizeof buff) == NULL)
    {
        perror("cwd error\n");
        fflush(stdout);
    }

    do

    {
        kill_kids();

        printf(": "); //the prompt
        fflush(stdout);
        int line_check = getline(&string_input, &string_input_len, stdin);
        if (line_check == -1)
        {
            clearerr(stdin);
            break;
        }
        string_input[strlen(string_input)-1] = '\0';
        if (*string_input == '\0' || string_input[0] == 35)
        {
            clean_up();
            continue;}
        if (strcmp(user_commands.command, "exit") == 0) {exit(0); }
        if((strstr(string_input, "$$")) != NULL)
        {
            pid_t pid = getpid();
            sprintf(string_pid, "%d", pid);
            expansion(string_input, "$$", string_pid);
        }

        /*
         * Separate the first word in the user input then moving to the rest
         * of the given line
         */
        char *save_pointer;
        char *token = strtok_r(string_input, " ", &save_pointer);
        strcpy(user_commands.command, token);

        /*
         * Loop will tokenize the given input and will redirect which
         * arguments are put in the regular argument array or in the
         * members for '<' '>' and '&'
         */
        do
        {
            token = strtok_r(NULL, " ", &save_pointer);

            if (token == 0)
            {
                break;
            }
            else if (*token == 60)
            {
                token = strtok_r(NULL, " ", &save_pointer);
                strcpy(user_commands.less, token);
                compare = 1;

            }
            else if (*token == 62)
            {
                token = strtok_r(NULL, " ", &save_pointer);
                strcpy(user_commands.greater, token);
                compare = 1;
            }
            else if (*token == 38)
            {
                if ((strcmp(user_commands.command, "status") == 0))
                {
                    break;
                }
                token ++;
                strcpy(user_commands.amper, "yes");
                compare = 1;
            }

            user_commands.options_list[counter] = malloc((strlen(token) +1) * sizeof (char));
            if (compare != 0) {
                user_commands.options_list[counter] = NULL;
            } else {
                strcpy(user_commands.options_list[counter], token);
            }
            compare = 0;
            counter++;
        } while(1);
        counter = 0; //reset counter to first argument position

        /*
         * Check the member of the struct holding the initial command
         * if it is not empty then go through built in commands for
         * execution
         */
        if ((strcmp(user_commands.command, "")) != 0)
        {
            //Get the present working directory
            if (strcmp(user_commands.command, "pwd") == 0) //strcmp return zero if equal
            {
                printf("%s\n", buff);
                fflush(stdout);
                clean_up();
                continue;

            }
            //Change directory to HOME
            else if (strcmp(user_commands.command, "cd") == 0 && user_commands.options_list[0] == NULL)
            {

                chdir(getenv("HOME"));
                if (chdir(getenv("HOME")) == 0)
                {
                    if (getcwd(buff, sizeof buff) == NULL)
                    {
                        perror("cwd error: could not change Dir\n");
                        fflush(stdout);
                    }
                }
                clean_up();
                continue;

            }
            //Changing the directory to given argument
            else if (strcmp(user_commands.command, "cd") == 0)
            {
                int change_result;
                change_result = chdir(user_commands.options_list[0]);
                if (change_result == 0)
                {

                    if (getcwd(buff, sizeof buff) == NULL)
                    {
                        perror("cwd error\n");
                        fflush(stdout);
                    }
                    clean_up();
                    continue;
                }
                else
                {
                    printf("Error, bad dir request\n");
                    fflush(stdout);
                    clean_up();
                    continue;
                }
            }
            //Check status of previous execution
            else if (strcmp(user_commands.command, "status") == 0)
            {
                printf("%s\n", user_commands.status);
                fflush(stdout);
                clean_up();
                continue;
            }
            //Exit the program
            else if (strcmp(user_commands.command, "exit") == 0)
            {
                exit(0);
            }
            /*
             * Logic to call foreground function or background function
             */
            if ((strcmp(user_commands.amper, "") != 0 && user_commands.foreground == 0))
            {
                execute_commands_background();
            }
            else
            {
                execute_commands();
            }
            kill_kids();
            clean_up();
            continue;
        }
    }while (1);

}
/*
 * Handler interrupt function to be assigned into the SIGTSTP interrupt
 */
void handler_interrupt(int z)
{

    if (user_commands.pid_array[0] == 1)
    {
        char message[] = "terminated by signal 2\n";
        fflush(stdout);
        write(STDOUT_FILENO, message, 23);
        fflush(stdout);
        kill(user_commands.current, SIGINT);
        clean_up();
    }
}


int main(int argc, char **argv)

{
    /*
     * Signal handling, for SIGTSTP and SIGINT, used information
     * from expplorations: signal api
     */
    struct sigaction SIGTSTP_action = {0}, SIGINT_action = {0};
    SIGTSTP_action.sa_handler = enter_foreground;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;char _pid[50] = {'\0'};
    sprintf(_pid, "%d", user_commands.current);

    SIGINT_action.sa_handler = handler_interrupt;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = SA_RESTART;

    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
    sigaction(SIGINT, &SIGINT_action, NULL);

    /*
     * Enter the main logic for command prompt
     */
    command_prompt();

    return 0;
}

