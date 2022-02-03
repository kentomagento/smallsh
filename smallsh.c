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


//---------variables and functions
void command_prompt();
char *get_line();
void expansion(char *source_string, char *find, char *exchange_with);
void clean_up();
//int execute_commands(char *first_command, char **arr_options );
int execute_commands();
//-------------------------------

struct Comms
{
    char command[2048];
    char *options_list[512];
    char less[1000];
    char greater[1000];
    char amper[3];

} user_commands;


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
    memset(user_commands.command, '\0', 2048);
    memset(user_commands.less, '\0', 1000);
    memset(user_commands.greater, '\0', 1000);
    memset(user_commands.amper, '\0', 3);

    for (i = 0; i < sizeof (user_commands.options_list)/sizeof (user_commands.options_list[0]); i++)
    {
        user_commands.options_list[i] = NULL;
        free(user_commands.options_list[i]);
    }
//    i = 0;
//    while(1)
//    {
//        if (user_commands.options_list[i] == NULL)
//        {
//            break;
//        }
//        if(user_commands.options_list[i] != NULL){
//            free(user_commands.options_list[i]);
//            user_commands.options_list[i] = NULL;
//
//        }
//        i++;
//    }
}

//int execute_commands(char *first_command, char **arr_options)
int execute_commands()
{
    char *args[512] = {NULL};
    int num_elements;
    int index_args;
    int index_arr_options;
    int i;
    printf("HOLLLLLLLLLOOOOWWW\n");
    index_arr_options = 0;
    num_elements = 0;
    index_args = 1;
//    args[0] = first_command;
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

    execvp(user_commands.command, args);
    perror(user_commands.command);
    clean_up();
    return 0;
//    exit(EXIT_FAILURE);
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
    //-----getline
    char *string_input = NULL;
    size_t string_input_len = 0;
    //------------fill struct
    memset(user_commands.command, '\0', 2048);
    memset(user_commands.less, '\0', 1000);
    memset(user_commands.greater, '\0', 1000);
    memset(user_commands.amper, '\0', 3);
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
    //do
    while(delete_count < delete_temp)
    {
        //-------------
        delete_count++;
        //-------------
        printf(": "); //the prompt
//        getchar();
        getline(&string_input, &string_input_len, stdin);
        string_input[strlen(string_input)-1] = '\0';
//        printf("%s\n", string_input);
        if (*string_input == '\0')
        {continue;}
        if((strstr(string_input, "$$")) != NULL)
        {
//            printf("found double dollas\n");
            pid_t pid = getpid();
            sprintf(string_pid, "%d", pid);
            expansion(string_input, "$$", string_pid);

//            printf("string pid worked --> %s\n", string_input);
        }

//        for (i = 0; i < strlen(string_input); i++)
//        {
//            printf("%c  \n", string_input[i]);
//            if (string_input[i] == 35)
//            {
//                break;
//            }
//            else if (string_input[i] == 60 && string_input[i-1] == 32 && string_input[i+1] == 32)
//            {
//                printf("found a LESS than symbole\n");
//                break;
//            }
//            else if (string_input[i] == 62 && string_input[i-1] == 32 && string_input[i+1] == 32)
//            {
//                printf("found a GREATER than symbole\n");
//                break;
//            }
//        }//end of for loop through input
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
                continue;
            }
            else if (*token == 62)
            {
                printf("great than found\n");
                token = strtok_r(NULL, " ", &save_pointer);
                strcpy(user_commands.greater, token);
                continue;
            }
//            if (token == 0)
//            {
//                //break;
//                continue;
//            }
            user_commands.options_list[counter] = malloc((strlen(token) +1) * sizeof (char));
            strcpy(user_commands.options_list[counter], token);
            counter++;
        } while(1);
        counter = 0; //reset counter to first argument position
        //checking if command is filled out
        printf("commands--> %s\n", user_commands.command);
        for (i = 0; i < 4; i++)
        {
            printf("args %d --> %s\n", i, user_commands.options_list[i]);
        }
        //---get pwd
        if ((strcmp(user_commands.command, "")) != 0)
        {
            if (strcmp(user_commands.command, "pwd") == 0) //strcmp return zero if equal
            {
                //printf("asking for directory\n");
                //---moves up to declarations; char buff[FILENAME_MAX];

                    printf("%s\n", buff);
//                    system("ls -l");

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
                }
                else
                {
                    printf("error, bad dir request\n");
                }
//                system("ls -l");
            }
            else if (strcmp(user_commands.command, "status") == 0)
            {
                printf("YOU NEED TO FILL OUT STATUS STUFF");
            }
            else if (strcmp(user_commands.command, "exit") == 0)
            {
                exit(0);
            }
            //----execute block

//            execute_commands(user_commands.command, user_commands.options_list);
            execute_commands();
            //---

        }//RUN EVERYTHING ABOVE THIS
//        execute_commands(user_commands.command, user_commands.options_list);

        //---
        printf("less than --> %s\n", user_commands.less);
        printf("greater than --> %s\n", user_commands.greater);
        printf("ampered up --> %s\n", user_commands.amper);
        clean_up();

        //--------confirm cleanup prints
        printf("commands--> %s\n", user_commands.command);
        for (i = 0; i < 5; i++)
        {
            printf("args %d --> %s\n", i, user_commands.options_list[i]);
        }
//        printf("less than --> %s\n", user_commands.less);
//        printf("greater than --> %s\n", user_commands.greater);
//        printf("ampered up --> %s\n", user_commands.amper);
        //-------------


        //call fflush at end after each print
    }//while (delete_count <= delete_temp);

}

int main(int argc, char **argv)

{
    command_prompt();

    return 0;
}

