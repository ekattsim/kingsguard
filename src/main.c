#include <stdio.h>
#include <stdbool.h>
#include "../inc/parse_exec.h"

#ifdef DEBUG
extern void initialise_monitor_handles(void);
#endif

static char input[100] = "look around";

static bool getInput(void)
{
     printf("\n--> ");
     return fgets(input, sizeof(input), stdin) != NULL;
}

int main()
{
#ifdef DEBUG
    initialise_monitor_handles(); // required for semihosting
#endif

    // Starting Screen
    printf("Welcome to KingsGuard.\n");

    // Main Loop
    while (parse_execute(input) && getInput());

    // Ending Screen
    printf("\nBye!\n");
    while(1);
}
