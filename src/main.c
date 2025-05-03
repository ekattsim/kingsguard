#include <stdio.h>
#include <stdbool.h>
#include <string.h> // For snprintf
#include "../inc/parse_exec.h"
#include "../inc/game_state.h" // Include game state header
#include "../inc/dialogue.h" // Include dialogue header
#include "../inc/locations.h" // Include locations header
#include "../inc/battle.h" // Include battle header (needed for end_battle check)
#include "../inc/kingsguard.h" // Need Kingsguard struct for stats in prompt

#ifdef DEBUG
extern void initialise_monitor_handles(void);
#endif

static char input[100] = ""; // Start with empty input so it waits for user first

static bool getInput(void)
{
     // Check if the game is in a terminal state (Game Over or Victory)
     GameState current_state = get_game_state();
     if (current_state == STATE_GAME_OVER || current_state == STATE_VICTORY) {
         return false; // Stop the main loop
     }

     // If in battle and player's active KG fainted, force a switch prompt
     if (current_state == STATE_BATTLE && game.active_battle != NULL) {
          BattleState *battle = (BattleState*)game.active_battle;
          if (battle->player_active_kg == NULL || !is_conscious(battle->player_active_kg)) { // Check if active KG is NULL *or* fainted
               printf("\nYour active Kingsguard has fainted! You must switch.\n");
               printf("Available Kingsguard:");
               bool first = true;
               for(int i=0; i < battle->player_party_size; ++i) {
                   if (battle->player_party[i] != NULL && is_conscious(battle->player_party[i])) {
                       if (!first) printf(",");
                       printf(" %s (HP: %d/%d)", battle->player_party[i]->name, battle->player_party[i]->current_hp, battle->player_party[i]->max_hp);
                       first = false;
                   }
               }
               printf("\n--> switch "); // Partial prompt, expects KG name
               fflush(stdout); // Ensure prompt is displayed
               char switch_param_buffer[100]; // Buffer for the switch target name
               if (fgets(switch_param_buffer, sizeof(switch_param_buffer), stdin) == NULL) return false;
               // Prepend "switch " to the input string
               snprintf(input, sizeof(input), "switch %s", switch_param_buffer);
               return true; // Got input for required switch

          }
           // In battle, display status before prompt
          if (battle->player_active_kg && battle->opponent_active_kg) { // Only show if both are active
              printf("\nYour %s (HP: %d/%d). Opponent's %s (HP: %d/%d).\n",
                     battle->player_active_kg->name, battle->player_active_kg->current_hp, battle->player_active_kg->max_hp,
                     battle->opponent_active_kg->name, battle->opponent_active_kg->current_hp, battle->opponent_active_kg->max_hp);
              printf("Available Moves for %s:", battle->player_active_kg->name);
              for(int i=0; i < battle->player_active_kg->num_moves; ++i) {
                  printf(" %s (PP: %d/%d)", battle->player_active_kg->moves[i].name, battle->player_active_kg->moves[i].pp, battle->player_active_kg->moves[i].max_pp);
                  if (i < battle->player_active_kg->num_moves - 1) printf(",");
              }
               printf("\n");
          } else if (battle->player_active_kg) {
              printf("\nYour %s (HP: %d/%d). Opponent has no active Kingsguard.\n", battle->player_active_kg->name, battle->player_active_kg->current_hp, battle->player_active_kg->max_hp);
               // List moves
               printf("Available Moves for %s:", battle->player_active_kg->name);
                for(int i=0; i < battle->player_active_kg->num_moves; ++i) {
                   printf(" %s (PP: %d/%d)", battle->player_active_kg->moves[i].name, battle->player_active_kg->moves[i].pp, battle->player_active_kg->moves[i].max_pp);
                   if (i < battle->player_active_kg->num_moves - 1) printf(",");
               }
                printf("\n");
          } else if (battle->opponent_active_kg) {
               // Should not happen if player has fainted KG but game is still active
               printf("\nOpponent's %s (HP: %d/%d). Your active Kingsguard has fainted.\n", battle->opponent_active_kg->name, battle->opponent_active_kg->current_hp, battle->opponent_active_kg->max_hp);
          }


     } else if (current_state == STATE_PUZZLE) {
          printf("\nWhat is your answer to Saber's riddle?\n--> solve ");
          fflush(stdout);
          char solve_param_buffer[100]; // Buffer for the answer
           if (fgets(solve_param_buffer, sizeof(solve_param_buffer), stdin) == NULL) return false;
           snprintf(input, sizeof(input), "solve %s", solve_param_buffer); // Prepend "solve"
           return true;
     }


     printf("\n--> ");
     fflush(stdout); // Ensure prompt is displayed immediately
     return fgets(input, sizeof(input), stdin) != NULL;
}

int main()
{
#ifdef DEBUG
    initialise_monitor_handles(); // required for semihosting
#endif

    // Initialize Game State
    initialize_game_state();

    // Starting Screen & Arrival Dialogue
    printf("Welcome to KingsGuard.\n");
    print_dialogue("arrival");

    // Initial state is Throne Room, initial interactions happen here
    printf("\nYou find yourself in the Throne Room.\n");

    // Game Loop
    // The loop continues as long as parse_execute returns true and getInput gets data
    while (getInput() && parse_execute(input)) {
         // The state machine inside parse_execute and the called functions
         // drive the game flow.
         // The check for game over/victory is now in getInput.
    }

    // Ending Screen (Handled by dialogue already, but print goodbye message if not in terminal state)
     GameState final_state = get_game_state();
    if (final_state != STATE_GAME_OVER && final_state != STATE_VICTORY) {
        printf("\nBye!\n");
    }


    // The original had while(1); - keeping for compatibility if needed,
    // but return 0 is standard for successful termination in console apps.
    // If you need to keep the console window open in some environments,
    // you might uncomment the while(1) or add a system("pause") or similar.
    // while(1);
    return 0;
}
