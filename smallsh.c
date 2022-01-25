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

//---------variables and functions
void command_prompt();
char *get_line();
//-------------------------------
void command_prompt()
{
    int delete_temp = 100;
    int delete_count = 0;

    do
    {
        //-------------
        delete_count++;
        //-------------
        printf(": "); //the prompt


        //call fflush at end after each print
    }while (delete_count <= delete_temp);
}

int main(int argc, char **argv)

{

    command_prompt();

    return 0;
}

