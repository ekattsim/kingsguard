#include "../inc/parse_exec.h"
#include "../inc/game_state.h"
#include "../inc/battle.h"
#include "../inc/locations.h" // Need location names array and get_location_id
#include "../inc/puzzle.h"
#include "../inc/dialogue.h"

#include <stdio.h>
#include <string.h>
#include <strings.h> // For strcasecmp

// --- State-specific command handlers ---
static bool handle_exploration_command(const char *verb, const char *noun);
static bool handle_battle_command(const char *verb, const char *noun);
static bool handle_puzzle_command(const char *verb, const char *noun);

bool parse_execute(char *input)
{
    // Use a copy of the input buffer because strtok modifies it
    char input_copy[100];
    strncpy(input_copy, input, sizeof(input_copy) - 1);
    input_copy[sizeof(input_copy) - 1] = '\0'; // Ensure null termination

    char *verb = strtok(input_copy, " \n");
    char *noun = strtok(NULL, "\n"); // Get the rest of the line as noun

    // Trim leading/trailing whitespace from noun
    if (noun != NULL) {
        // Find the first non-whitespace character
        char *trimmed_noun = noun;
        while (*trimmed_noun == ' ' || *trimmed_noun == '\t') trimmed_noun++;

        // Find the last non-whitespace character (from the end)
        char *end = trimmed_noun + strlen(trimmed_noun) - 1;
        while (end >= trimmed_noun && (*end == ' ' || *end == '\t')) {
            end--;
        }
        // Null-terminate the trimmed string
        *(end + 1) = '\0';
        noun = trimmed_noun; // Use the trimmed start

        if (*noun == '\0') noun = NULL; // If only whitespace was left
    }

    // --- Global Commands ---
    if (verb == NULL) {
        // Empty input, ignore
        return true;
    }

    if (strcasecmp(verb, "quit") == 0) {
        return false; // Signal to exit main loop
    }
    // Add other global commands here if any (e.g., "save", "load")

    // --- State-specific Command Dispatch ---
    GameState current_state = get_game_state();

    switch (current_state) {
        case STATE_THRONE_ROOM:
        case STATE_COURTYARD:
        case STATE_SUMMONING_HALL:
        case STATE_COLOSSEUM:
            // In exploration/location states, handle location-based commands
            return handle_exploration_command(verb, noun);
        case STATE_BATTLE:
            // In battle state, handle battle commands
            return handle_battle_command(verb, noun);
        case STATE_PUZZLE:
             // In puzzle state, handle puzzle commands
             return handle_puzzle_command(verb, noun);
        // Add cases for other states if they have unique command sets
        default:
            printf("Game is in an unexpected state (%d).\n", current_state);
            return true; // Continue loop, but something is wrong
    }

    return true; // Should not be reached
}

// --- State-specific command handler implementations ---

static bool handle_exploration_command(const char *verb, const char *noun)
{
    if (strcasecmp(verb, "look") == 0) {
        // Handle "look" (with or without noun)
        if (noun == NULL || strcasecmp(noun, "around") == 0) {
            look_around(); // Function from locations.c
        } else {
            // Handle looking at specific objects/people (future)
            printf("You look at the %s, but don't notice anything special.\n", noun);
        }
    } else if (strcasecmp(verb, "go") == 0) {
        // Handle "go <destination>"
        if (noun != NULL) {
            int dest_id = get_location_id(noun);
            if (dest_id == LOCATION_UNKNOWN) {
                 printf("You don't recognize that location.\n");
            } else {
                player_go_to_location(dest_id); // Use the new function
            }
        } else {
            printf("Go where?\n");
        }
    } else if (strcasecmp(verb, "talk") == 0) {
         // Handle "talk <target>" (future)
         if (noun != NULL) {
              // Add actual dialogue/interaction logic here later
              printf("You try talking to %s, but they don't seem to have anything to say right now.\n", noun);
              // TODO: Add specific dialogue triggers here, e.g., if noun is "Archer" and plot_flags.met_initial_kingsguard is false
               if (strcasecmp(noun, "Archer") == 0 || strcasecmp(noun, "Lancer") == 0) {
                   if (!game.plot_flags.met_initial_kingsguard && get_current_location() == LOCATION_THRONE_ROOM) {
                       print_dialogue("meet_kingsguard");
                       game.plot_flags.met_initial_kingsguard = true; // Set the flag
                   }
               }

         } else {
              printf("Talk to whom?\n");
         }
    }
    // Add other exploration commands here (e.g., "examine", "inventory", "stats")
    else {
        printf("I don't know how to '%s' here.\n", verb);
    }
    return true; // Always return true to continue game loop in exploration
}

// Battle and Puzzle handlers remain mostly the same, but they rely on game state
// transitions and the updated Kingsguard/Battle logic. No changes needed here
// for the compiler errors, but they will interact with the new plot flags.

static bool handle_battle_command(const char *verb, const char *noun)
{
    // Example Battle Commands: "attack <move>", "switch <kingsguard>"
     // Ensure battle is actually active before processing commands
    if (!game.active_battle || !((BattleState*)game.active_battle)->is_active) {
         printf("You are not currently in battle.\n");
         // Transition back to location state if somehow stuck here
         if (get_game_state() == STATE_BATTLE) set_game_state(get_current_location()); // Return to location
         return false; // Invalid state/command
    }


    if (strcasecmp(verb, "attack") == 0) {
        if (noun != NULL) {
            // Attempt to use a move. The battle system handles validation, turns, etc.
            if (battle_handle_player_action(BATTLE_ACTION_ATTACK, noun)) {
                 // Action was valid, battle turn proceeds within battle_handle_player_action
                 // The game loop will then get the next input.
            } else {
                 // Action was invalid (e.g., unknown move, move not available)
                 // battle_handle_player_action should print error
            }
        } else {
            printf("Attack with what move? (e.g., 'attack Fire_Arrow')\n");
        }
    } else if (strcasecmp(verb, "switch") == 0) {
         if (noun != NULL) {
              // Attempt to switch Kingsguard
              if (battle_handle_player_action(BATTLE_ACTION_SWITCH, noun)) {
                  // Action was valid, battle turn proceeds
              } else {
                  // Invalid switch (e.g., unknown KG, KG fainted)
              }
         } else {
             printf("Switch to which Kingsguard? (e.g., 'switch Lancer')\n");
         }
    }
    // Add other battle commands like "use item" (future)
    else {
        printf("You are in battle. You can 'attack <move>' or 'switch <kingsguard>'.\n");
         // Also list available moves/kingsguard? Needs access to battle state.
         // This info is printed by getInput when in battle state.
    }

    // After a valid action, the battle turn should process and check for end conditions
    // This happens synchronously within battle_handle_player_action -> battle_process_turn.

    // If battle is over, the state will be changed within the battle module.
    // We always return true from the parser while in battle, the state change
    // will affect which handler is called next loop.
    return true;
}

static bool handle_puzzle_command(const char *verb, const char *noun)
{
     // Ensure puzzle is active
     if (get_game_state() != STATE_PUZZLE) {
          printf("You are not currently in a puzzle state.\n");
          // Transition back? Or just state it?
          return false; // Invalid state/command
     }

     // Commands specific to the puzzle state
     if (strcasecmp(verb, "solve") == 0) {
          if (noun != NULL) {
               // Attempt to solve the puzzle with the given answer
               if (check_puzzle_answer(noun)) { // Function from puzzle.c
                   // Puzzle solved! State should transition in puzzle.c
               } else {
                   // Wrong answer. check_puzzle_answer should handle feedback.
               }
          } else {
               printf("Solve what? Provide an answer.\n");
          }
     } else if (strcasecmp(verb, "look") == 0) {
          // Allow looking around during the puzzle
          if (noun == NULL || strcasecmp(noun, "around") == 0) {
              look_around(); // This might show puzzle hints
          } else {
              printf("You examine the %s, but it offers no clues.\n", noun); // Or add specific examine logic
          }
     }
     // Add other relevant puzzle commands (e.g., "read inscription", "examine artifact")
     else {
          printf("You are faced with a puzzle. You can try to 'solve <answer>'.\n");
     }
     return true; // Always return true in puzzle state
}
