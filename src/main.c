#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/parse_exec.h"
#include "../inc/game_state.h"
#include "../inc/dialogue.h"
#include "../inc/locations.h"
#include "../inc/battle.h"
#include "../inc/kingsguard.h"

#ifdef DEBUG
extern void initialise_monitor_handles(void);
#endif

static char input[100] = "";

// --- Forward declarations for main menu and setup functions ---
static void display_ascii_greeting(void);
static void display_main_menu(void);
static bool handle_main_menu_input(void);
static void perform_character_creation(void);
static void perform_difficulty_selection(void);
static void display_how_to_play(void);
static void display_ascii_victory(void);


static bool getInput(void)
{
     GameState current_state_for_input = get_game_state();
     if (current_state_for_input == STATE_GAME_OVER || current_state_for_input == STATE_VICTORY) {
         return false;
     }

     // If in battle and player's active KG fainted, force a switch prompt
     if (current_state_for_input == STATE_BATTLE && game.active_battle != NULL) {
          BattleState *battle = (BattleState*)game.active_battle;
          if (battle->player_active_kg == NULL || !is_conscious(battle->player_active_kg)) {
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
               printf("\n--> switch ");
               fflush(stdout);
               char switch_param_buffer[100];
               if (fgets(switch_param_buffer, sizeof(switch_param_buffer), stdin) == NULL) return false;
               snprintf(input, sizeof(input), "switch %s", switch_param_buffer);
               return true;
          }
           // In battle, display status before prompt
          if (battle->player_active_kg && battle->opponent_active_kg) {
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
               printf("Available Moves for %s:", battle->player_active_kg->name);
                for(int i=0; i < battle->player_active_kg->num_moves; ++i) {
                   printf(" %s (PP: %d/%d)", battle->player_active_kg->moves[i].name, battle->player_active_kg->moves[i].pp, battle->player_active_kg->moves[i].max_pp);
                   if (i < battle->player_active_kg->num_moves - 1) printf(",");
               }
                printf("\n");
          } else if (battle->opponent_active_kg) {
               printf("\nOpponent's %s (HP: %d/%d). Your active Kingsguard has fainted.\n", battle->opponent_active_kg->name, battle->opponent_active_kg->current_hp, battle->opponent_active_kg->max_hp);
          }
          if (game.difficulty == DIFFICULTY_EASY && battle->player_active_kg != NULL && is_conscious(battle->player_active_kg)) {
               printf("(Hint: 'attack <move_name>', 'switch <kingsguard_name>')");
          }
          printf("\n--> ");
          fflush(stdout);
          return fgets(input, sizeof(input), stdin) != NULL;

     } else if (current_state_for_input == STATE_PUZZLE) {
          printf("\nWhat is your answer to Saber's riddle?");
          if (game.difficulty == DIFFICULTY_EASY) {
              printf(" (Hint: Saber values a deep bond, a core knightly virtue. Use 'solve <your_answer>')");
          }
          printf("\n--> solve ");
          fflush(stdout);
          char solve_param_buffer[100];
           if (fgets(solve_param_buffer, sizeof(solve_param_buffer), stdin) == NULL) return false;
           snprintf(input, sizeof(input), "solve %s", solve_param_buffer);
           return true;
     }

    // Default prompt for exploration and other states
    if (game.difficulty == DIFFICULTY_EASY) {
        bool is_exploration_state = (
            current_state_for_input == STATE_THRONE_ROOM ||
            current_state_for_input == STATE_COURTYARD ||
            current_state_for_input == STATE_SUMMONING_HALL ||
            current_state_for_input == STATE_COLOSSEUM
        );

        if (is_exploration_state) {
            printf("\n(Hint: try 'look', 'talk <person>')");

            // Specific 'go' hints based on current location and plot flags
            char go_hint_buffer[150] = "";
            bool first_dest = true;
            int current_loc_id = game.current_location;

            if (current_loc_id == LOCATION_THRONE_ROOM) {
                strcat(go_hint_buffer, location_names[LOCATION_COURTYARD]);
                first_dest = false;
                if (game.plot_flags.has_summoning_artifact) {
                    if (!first_dest) strcat(go_hint_buffer, ", ");
                    strcat(go_hint_buffer, location_names[LOCATION_SUMMONING_HALL]);
                    first_dest = false;
                }
                if (game.plot_flags.saber_recruited) {
                    if (!first_dest) strcat(go_hint_buffer, ", ");
                    strcat(go_hint_buffer, location_names[LOCATION_COLOSSEUM]);
                }
            } else if (current_loc_id == LOCATION_COURTYARD) {
                strcat(go_hint_buffer, location_names[LOCATION_THRONE_ROOM]);
            } else if (current_loc_id == LOCATION_SUMMONING_HALL) {
                strcat(go_hint_buffer, location_names[LOCATION_THRONE_ROOM]);
            }

            if (strlen(go_hint_buffer) > 0) {
                printf("\n(Hint: available 'go' destinations: %s)", go_hint_buffer);
            }
        }
    }

     printf("\n--> ");
     fflush(stdout);
     return fgets(input, sizeof(input), stdin) != NULL;
}

int main()
{
#ifdef DEBUG
    initialise_monitor_handles(); // required for semihosting
#endif

    initialize_game_state();

    display_ascii_greeting();

    // Main Menu Loop
    while(get_game_state() == STATE_MAIN_MENU) {
        display_main_menu();
        if (!handle_main_menu_input()) {
            printf("\nExiting game.\n");
            return 0;
        }
    }

    // Proceed to game loop only if "Start Game" was chosen and setup is complete
    if (get_game_state() != STATE_MAIN_MENU && get_game_state() != STATE_GAME_OVER && get_game_state() != STATE_VICTORY) {
        printf("\nWelcome to KingsGuard, %s %s.\n", game.player_gender, game.player_name);
        print_dialogue("arrival");
        // Initial state is Throne Room, initial interactions happen here
        printf("\nYou find yourself in the Throne Room.\n");
        look_around();


        // Game Loop
        // The loop continues as long as parse_execute returns true and getInput gets data
        while (getInput() && parse_execute(input)) {
             // The state machine inside parse_execute and the called functions
             // drive the game flow.
             // The check for game over/victory is now in getInput.
        }
    }

    // Ending Screen
    GameState final_state = get_game_state();
    if (final_state == STATE_VICTORY) {
        display_ascii_victory();
    } else if (final_state != STATE_GAME_OVER && get_game_state() != STATE_MAIN_MENU) {
        // Don't print "Bye!" if game ended with Game Over (it has its own message)
        // or if we quit from main menu (already printed "Exiting game.")
        printf("\nBye!\n");
    }

    return 0;
}

// --- Main Menu and Setup Function Definitions ---

static void display_ascii_greeting(void) {
    printf(
    "  _  __ _   _    _     _                           _          \n"
    " | |/ /(_) | |_ | | __| |  _ __ ___    __ _  _ __ (_)  __ _   ___ \n"
    " | ' / | | | __|| |/ /| | | '_ ` _ \\  / _` || '__|| | / _` | / __|\n"
    " | . \\ | | | |_ |   < | | | | | | | || (_| || |   | || (_| || (__ \n"
    " |_|\\_\\|_|  \\__||_|\_\\|_| |_| |_| |_| \\__,_||_|   |_| \\__,_| \\___|\n\n"
    "                 A Text-Based RPG Adventure\n");
}

static void display_main_menu(void) {
    printf("\n--- Main Menu ---\n");
    printf("1. Start New Game\n");
    printf("2. How to Play\n");
    printf("3. Quit\n");
}

static bool handle_main_menu_input(void) {
    char menu_choice_str[10];
    printf("Enter choice (1-3): ");
    if (fgets(menu_choice_str, sizeof(menu_choice_str), stdin) == NULL) {
        return false;
    }

    int choice = atoi(menu_choice_str);
    switch (choice) {
        case 1:
            perform_character_creation();
            perform_difficulty_selection();
            set_game_state(STATE_THRONE_ROOM);
            game.current_location = LOCATION_THRONE_ROOM;
            return true;
        case 2:
            display_how_to_play();
            return true;
        case 3:
            return false;
        default:
            printf("Invalid choice. Please try again.\n");
            return true;
    }
}

static void perform_character_creation(void) {
    char local_player_name_buffer[50];
    char local_player_gender_buffer[20];

    printf("\n--- Character Creation ---\n");
    printf("Enter your character's name: ");
    if (fgets(local_player_name_buffer, sizeof(local_player_name_buffer), stdin) != NULL) {
        local_player_name_buffer[strcspn(local_player_name_buffer, "\n")] = 0;
        if (strlen(local_player_name_buffer) == 0) {
            strcpy(game.player_name, "Player");
        } else {
            strcpy(game.player_name, local_player_name_buffer);
        }
    } else {
        strcpy(game.player_name, "Player");
    }

    printf("Enter your title/gender (e.g., King, Queen, Ruler): ");
     if (fgets(local_player_gender_buffer, sizeof(local_player_gender_buffer), stdin) != NULL) {
        local_player_gender_buffer[strcspn(local_player_gender_buffer, "\n")] = 0;
        if (strlen(local_player_gender_buffer) == 0) {
            strcpy(game.player_gender, "Ruler");
        } else {
            strcpy(game.player_gender, local_player_gender_buffer);
        }
    } else {
        strcpy(game.player_gender, "Ruler");
    }
    printf("Welcome, %s %s!\n", game.player_gender, game.player_name);
}

static void perform_difficulty_selection(void) {
    char diff_choice_str[10];
    printf("\n--- Difficulty Selection ---\n");
    while (true) {
        printf("Choose difficulty (1-Easy, 2-Hard): ");
        if (fgets(diff_choice_str, sizeof(diff_choice_str), stdin) == NULL) {
            game.difficulty = DIFFICULTY_EASY;
            printf("Defaulting to Easy difficulty due to input error.\n");
            break;
        }
        int choice = atoi(diff_choice_str);
        if (choice == 1) {
            game.difficulty = DIFFICULTY_EASY;
            printf("Easy difficulty selected. Hints will be provided.\n");
            break;
        } else if (choice == 2) {
            game.difficulty = DIFFICULTY_HARD;
            printf("Hard difficulty selected. No hints.\n");
            break;
        } else {
            printf("Invalid choice. Please enter 1 or 2.\n");
        }
    }
}

static void display_how_to_play(void) {
    printf("\n--- How to Play ---\n");
    printf("KingsGuard is a text-based adventure RPG.\n");
    printf("You interact with the world by typing commands, usually a verb and optionally a noun.\n");
    printf("Examples: 'look around', 'go Courtyard', 'talk Archer'.\n\n");
    printf("Common Exploration Commands:\n");
    printf("  look / look around  - Describes your current location and notable things.\n");
    printf("  go <location_name>  - Attempts to move to the specified location.\n");
    printf("                        (e.g., 'go Courtyard')\n");
    printf("  talk <target_name>  - Initiates dialogue with a character.\n");
    printf("                        (e.g., 'talk Lancer')\n\n");
    printf("Battle Commands (when in combat):\n");
    printf("  attack <move_name>  - Uses one of your active Kingsguard's moves.\n");
    printf("                        (e.g., 'attack Slash', 'attack Fire_Arrow')\n");
    printf("  switch <kg_name>    - Switches your active Kingsguard to another conscious one.\n");
    printf("                        (e.g., 'switch Lancer')\n\n");
    printf("Puzzle Commands (when solving a riddle):\n");
    printf("  solve <your_answer> - Submits your answer to the puzzle.\n");
    printf("                        (e.g., 'solve Loyalty')\n\n");
    printf("Global Command:\n");
    printf("  quit                - Exits the game at any time.\n\n");
    printf("Your goal is to recruit your Kingsguard, navigate the castle, and prepare for\n");
    printf("the final duel to protect your kingdom. Pay attention to dialogues for clues!\n");
    printf("If playing on Easy difficulty, contextual hints will appear before prompts.\n");
}

static void display_ascii_victory(void) {
    printf(
    "\n\n"
    " __   __           _            _ _ _ \n"
    " \\ \\ / /__ _ _ __ | | ___ _   _(_) | | __ _ \n"
    "  \\ V / _ \\ '_ \\| |/ _ \\ | | | | | |/ _` |\n"
    "   | |  __/ | | | |  __/ |_| | | | | (_| |\n"
    "   |_|\\___|_| |_|_|\\___|\\__,_|_|_|_|\\__,_|\n\n");
}
