#include "../inc/parse_exec.h"
#include "../inc/game_state.h"
#include "../inc/battle.h"
#include "../inc/locations.h"
#include "../inc/puzzle.h"
#include "../inc/dialogue.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>

// --- State-specific command handlers ---
static bool handle_exploration_command(const char *verb, const char *noun);
static bool handle_battle_command(const char *verb, const char *noun);
static bool handle_puzzle_command(const char *verb, const char *noun);

bool parse_execute(char *input)
{
    // Use a copy of the input buffer because strtok modifies it
    char input_copy[100];
    strncpy(input_copy, input, sizeof(input_copy) - 1);
    input_copy[sizeof(input_copy) - 1] = '\0';

    char *verb = strtok(input_copy, " \n");
    char *noun = strtok(NULL, "\n");

    // Trim leading/trailing whitespace from noun
    if (noun != NULL) {
        char *trimmed_noun = noun;
        while (*trimmed_noun == ' ' || *trimmed_noun == '\t') trimmed_noun++;

        char *end = trimmed_noun + strlen(trimmed_noun) - 1;
        while (end >= trimmed_noun && (*end == ' ' || *end == '\t')) {
            end--;
        }
        *(end + 1) = '\0';
        noun = trimmed_noun;

        if (*noun == '\0') noun = NULL;
    }

    // --- Global Commands ---
    if (verb == NULL) {
        return true;
    }

    if (strcasecmp(verb, "quit") == 0) {
        return false;
    }
    // Add other global commands here if any (e.g., "save", "load")

    // --- State-specific Command Dispatch ---
    GameState current_state = get_game_state();

    switch (current_state) {
        case STATE_THRONE_ROOM:
        case STATE_COURTYARD:
        case STATE_SUMMONING_HALL:
        case STATE_COLOSSEUM:
            return handle_exploration_command(verb, noun);
        case STATE_BATTLE:
            return handle_battle_command(verb, noun);
        case STATE_PUZZLE:
             return handle_puzzle_command(verb, noun);
        // Add cases for other states if they have unique command sets
        default:
            printf("Game is in an unexpected state (%d).\n", current_state);
            return true;
    }

    return true;
}

// --- State-specific command handler implementations ---

static bool handle_exploration_command(const char *verb, const char *noun)
{
    if (strcasecmp(verb, "look") == 0) {
        if (noun == NULL || strcasecmp(noun, "around") == 0) {
            look_around();
        } else {
            // Handle looking at specific objects/people (future)
            printf("You look at the %s, but don't notice anything special.\n", noun);
        }
    } else if (strcasecmp(verb, "go") == 0) {
        if (noun != NULL) {
            int dest_id = get_location_id(noun);
            if (dest_id == LOCATION_UNKNOWN) {
                 printf("You don't recognize that location.\n");
            } else {
                player_go_to_location(dest_id);
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
                       game.plot_flags.met_initial_kingsguard = true;
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
    return true;
}


static bool handle_battle_command(const char *verb, const char *noun)
{
    // Example Battle Commands: "attack <move>", "switch <kingsguard>"
    if (!game.active_battle || !((BattleState*)game.active_battle)->is_active) {
         printf("You are not currently in battle.\n");
         if (get_game_state() == STATE_BATTLE) set_game_state(game.pre_battle_state);
         return false;
    }


    if (strcasecmp(verb, "attack") == 0) {
        if (noun != NULL) {
            if (battle_handle_player_action(BATTLE_ACTION_ATTACK, noun)) {
            } else {
            }
        } else {
            printf("Attack with what move? (e.g., 'attack Fire_Arrow')\n");
        }
    } else if (strcasecmp(verb, "switch") == 0) {
         if (noun != NULL) {
              if (battle_handle_player_action(BATTLE_ACTION_SWITCH, noun)) {
              } else {
              }
         } else {
             printf("Switch to which Kingsguard? (e.g., 'switch Lancer')\n");
         }
    }
    // Add other battle commands like "use item" (future)
    else {
        printf("You are in battle. You can 'attack <move>' or 'switch <kingsguard>'.\n");
    }
    return true;
}

static bool handle_puzzle_command(const char *verb, const char *noun)
{
     if (get_game_state() != STATE_PUZZLE) {
          printf("You are not currently in a puzzle state.\n");
          return false;
     }

     if (strcasecmp(verb, "solve") == 0) {
          if (noun != NULL) {
               if (check_puzzle_answer(noun)) {
               } else {
               }
          } else {
               printf("Solve what? Provide an answer.\n");
          }
     } else if (strcasecmp(verb, "look") == 0) {
          if (noun == NULL || strcasecmp(noun, "around") == 0) {
              look_around();
          } else {
              printf("You examine the %s, but it offers no clues.\n", noun);
          }
     }
     // Add other relevant puzzle commands (e.g., "read inscription", "examine artifact")
     else {
          printf("You are faced with a puzzle. You can try to 'solve <answer>'.\n");
     }
     return true;
}
