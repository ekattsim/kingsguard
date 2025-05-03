#include "../inc/battle.h"
#include "../inc/game_state.h"
#include "../inc/kingsguard.h" // Need access to Kingsguard data and functions
#include "../inc/dialogue.h" // Need for printing battle messages
#include "../inc/locations.h" // Need for potential location changes after battle
#include <stdlib.h> // For malloc, free, rand, NULL
#include <stdio.h>
#include <string.h> // For strncpy, memset
#include <strings.h> // For strcasecmp
#include <time.h> // For seeding rand()


// Global Battle State instance
BattleState current_battle;

// Helper function to find the first conscious Kingsguard in a party
static Kingsguard* find_first_conscious(Kingsguard *party[], int party_size, int *index) {
    for (int i = 0; i < party_size; i++) {
        if (party[i] != NULL && is_conscious(party[i])) {
            if (index != NULL) *index = i;
            return party[i];
        }
    }
    if (index != NULL) *index = -1; // No conscious KG found
    return NULL;
}

// Helper function to calculate damage (simplified)
static int calculate_damage(const Kingsguard *attacker, const Kingsguard *defender, const Move *move) {
    if (move->type == MOVE_TYPE_STATUS || move->power <= 0) {
        return 0; // Status moves or healing don't deal damage this way
    }

    // Very simple damage calculation (Attacker's relevant stat vs Defender's relevant stat)
    int attack_stat = (move->type == MOVE_TYPE_PHYSICAL) ? attacker->attack : attacker->sp_attack;
    int defense_stat = (move->type == MOVE_TYPE_PHYSICAL) ? defender->defense : defender->sp_defense;

    // Prevent division by zero
    if (defense_stat == 0) defense_stat = 1;

    // Formula: ((Power * AttackerStat / DefenderStat) / 5) + 2 (Rough Pokémon approximation)
    // Scale it down a bit for smaller numbers
    int damage = ((move->power * attack_stat / defense_stat) / 10) + 1;

    // Add some randomness (optional)
    // damage = damage * (100 - (rand() % 16)) / 100; // ~0.85 to 1.0 variance

    // Ensure minimum damage
    if (damage < 1) damage = 1;

    return damage;
}

// Helper function for AI to choose a move (very basic)
// Returns a pointer to the Move struct *within the attacker's battle instance*
static Move* choose_opponent_move(Kingsguard *attacker, const Kingsguard *defender) { // Attacker needs to be non-const to check/decrease PP
    // Simple AI: Just pick a random move the attacker knows
    if (attacker == NULL || attacker->num_moves == 0) return NULL;

    Move *chosen_move = NULL;
    int attempts = 0;
    const int max_attempts = 10; // Prevent infinite loop if all moves have 0 PP

    while (attempts < max_attempts) {
        int move_index = rand() % attacker->num_moves;
        chosen_move = &attacker->moves[move_index]; // Get pointer to move in battle instance
        if (chosen_move->pp > 0) {
            return chosen_move;
        }
        attempts++;
    }

    // If all moves have 0 PP (unlikely with current setup, but safe)
    // Try to find any move with PP
    for(int i=0; i < attacker->num_moves; ++i) {
        if(attacker->moves[i].pp > 0) return &attacker->moves[i]; // Return pointer to move in battle instance
    }

    return NULL; // Should not happen if KG has moves with PP
}

// Helper function for AI to choose which Kingsguard to switch to
static Kingsguard* choose_opponent_switch(Kingsguard *party[], int party_size, int current_index, int *new_index) {
    // Simple AI: Switch to the first conscious Kingsguard who isn't the current one
    for (int i = 0; i < party_size; i++) {
        if (i != current_index && party[i] != NULL && is_conscious(party[i])) {
            *new_index = i;
            return party[i];
        }
    }
    *new_index = -1; // No valid switch target
    return NULL;
}


void start_battle(Kingsguard *opponent_kingsguard_templates[], int num_opponents)
{
    // Seed random number generator (only once per program run usually, but fine here for testing)
    srand(time(NULL));

    // Clear previous battle state
    memset(&current_battle, 0, sizeof(BattleState));
    game.active_battle = &current_battle; // Point game state to the battle state


    // --- Setup Player Party (Temporary copies for battle) ---
    current_battle.player_party_size = 0;
    // Get templates from kingsguard.c
    Kingsguard *archer_template = get_kingsguard_by_name("Archer");
    Kingsguard *lancer_template = get_kingsguard_by_name("Lancer");
    Kingsguard *saber_template = get_kingsguard_by_name("Saber");

    if (game.player_kingsguard.has_archer && archer_template) {
        current_battle.player_party[current_battle.player_party_size] = malloc(sizeof(Kingsguard));
        memcpy(current_battle.player_party[current_battle.player_party_size], archer_template, sizeof(Kingsguard));
         // Reset HP/PP for battle (optional, could start with full HP/PP each battle)
         current_battle.player_party[current_battle.player_party_size]->current_hp = current_battle.player_party[current_battle.player_party_size]->max_hp;
         for(int i=0; i < current_battle.player_party[current_battle.player_party_size]->num_moves; ++i) {
              current_battle.player_party[current_battle.player_party_size]->moves[i].pp = current_battle.player_party[current_battle.player_party_size]->moves[i].max_pp;
         }
        current_battle.player_party_size++;
    }
    if (game.player_kingsguard.has_lancer && lancer_template) {
         current_battle.player_party[current_battle.player_party_size] = malloc(sizeof(Kingsguard));
        memcpy(current_battle.player_party[current_battle.player_party_size], lancer_template, sizeof(Kingsguard));
         current_battle.player_party[current_battle.player_party_size]->current_hp = current_battle.player_party[current_battle.player_party_size]->max_hp;
         for(int i=0; i < current_battle.player_party[current_battle.player_party_size]->num_moves; ++i) {
              current_battle.player_party[current_battle.player_party_size]->moves[i].pp = current_battle.player_party[current_battle.player_party_size]->moves[i].max_pp;
         }
        current_battle.player_party_size++;
    }
    if (game.player_kingsguard.has_saber && saber_template) {
         current_battle.player_party[current_battle.player_party_size] = malloc(sizeof(Kingsguard));
        memcpy(current_battle.player_party[current_battle.player_party_size], saber_template, sizeof(Kingsguard));
         current_battle.player_party[current_battle.player_party_size]->current_hp = current_battle.player_party[current_battle.player_party_size]->max_hp;
         for(int i=0; i < current_battle.player_party[current_battle.player_party_size]->num_moves; ++i) {
              current_battle.player_party[current_battle.player_party_size]->moves[i].pp = current_battle.player_party[current_battle.player_party_size]->moves[i].max_pp;
         }
        current_battle.player_party_size++;
    }


    if (current_battle.player_party_size == 0) {
         printf("ERROR: Player has no Kingsguard to battle with!\n");
         set_game_state(STATE_GAME_OVER); // Can't battle without KG
         end_battle(); // Clean up any allocated memory before exiting
         return;
    }

    // Set player's initial active Kingsguard (the first conscious one in the party list)
    current_battle.player_active_kg = find_first_conscious(current_battle.player_party, current_battle.player_party_size, &current_battle.player_active_index);
    if (current_battle.player_active_kg == NULL) {
        // This should not happen if player_party_size > 0, but defensive check
         printf("ERROR: Player Kingsguard are all fainted at start of battle!\n");
         set_game_state(STATE_GAME_OVER);
         end_battle();
         return;
    }
     printf("Your active Kingsguard is %s.\n", current_battle.player_active_kg->name);


    // --- Setup Opponent Party (Temporary copies for battle) ---
    current_battle.opponent_party_size = 0;
    for (int i = 0; i < num_opponents && i < 3; ++i) { // Max 3 opponents
        if (opponent_kingsguard_templates[i] != NULL) {
            current_battle.opponent_party[current_battle.opponent_party_size] = malloc(sizeof(Kingsguard));
            memcpy(current_battle.opponent_party[current_battle.opponent_party_size], opponent_kingsguard_templates[i], sizeof(Kingsguard));
             // Reset HP/PP for battle (optional)
            current_battle.opponent_party[current_battle.opponent_party_size]->current_hp = current_battle.opponent_party[current_battle.opponent_party_size]->max_hp;
            for(int j=0; j < current_battle.opponent_party[current_battle.opponent_party_size]->num_moves; ++j) {
                 current_battle.opponent_party[current_battle.opponent_party_size]->moves[j].pp = current_battle.opponent_party[current_battle.opponent_party_size]->moves[j].max_pp;
            }

            current_battle.opponent_party_size++;
        }
    }

     if (current_battle.opponent_party_size == 0) {
         printf("ERROR: Battle started with no opponents!\n");
         set_game_state(STATE_GAME_OVER); // Or return to previous state?
         end_battle(); // Clean up memory
         return;
    }

    // Set opponent's initial active Kingsguard
    current_battle.opponent_active_kg = find_first_conscious(current_battle.opponent_party, current_battle.opponent_party_size, &current_battle.opponent_active_index);
     if (current_battle.opponent_active_kg == NULL) {
        // This should not happen if opponent_party_size > 0 and templates have >0 HP, but defensive check
         printf("ERROR: Opponent Kingsguard are all fainted at start of battle!\n");
         set_game_state(STATE_GAME_OVER);
         end_battle();
         return;
    }
     printf("Opponent's active Kingsguard is %s.\n", current_battle.opponent_active_kg->name);


    // --- Start Battle ---
    current_battle.is_active = true;
    current_battle.player_won = false; // Assume loss until proven otherwise
    set_game_state(STATE_BATTLE);

    printf("\n--- BATTLE START! ---\n");
    // Display initial state?
    // print_kingsguard_stats(current_battle.player_active_kg);
    // print_kingsguard_stats(current_battle.opponent_active_kg);

    // The first turn input will be handled by the main loop calling parse_execute -> handle_battle_command
}

// This function is called by parse_execute when the game state is STATE_BATTLE
// It records the player's chosen action and then proceeds with the turn.
// Returns true if a valid action was chosen, false otherwise (e.g., bad command, fainted KG)
bool battle_handle_player_action(BattleAction action, const char *param)
{
    if (!current_battle.is_active) {
        // This case might happen if battle just ended and player entered another command quickly
        // Re-check battle over state and transition if necessary
         if (is_battle_over()) {
             // Battle ending logic is handled by end_battle, which sets the state.
             // Just return false here as input is irrelevant to a non-active battle.
             return false;
         }
         // Should not be reached if is_battle_over is checked
        printf("There is no active battle.\n");
        // set_game_state(game.current_location); // Should already be set by end_battle
        return false;
    }

    // Basic validation: Can the active KG act? If fainted, only switch is allowed.
    bool player_active_fainted = (current_battle.player_active_kg == NULL || !is_conscious(current_battle.player_active_kg));

    if (player_active_fainted && action != BATTLE_ACTION_SWITCH) {
         // Player needs to switch. Only accept switch command.
         printf("Your active Kingsguard has fainted! You must switch.\n");
          // Check if player has any conscious KG left (this check is also in process_turn/end_battle)
            int dummy_index;
            if (find_first_conscious(current_battle.player_party, current_battle.player_party_size, &dummy_index) == NULL) {
                // This state indicates a loss, which should be caught by is_battle_over
                 // and handled in end_battle. If we reach here, something might be slightly off
                 // in the state logic flow. For now, let's just state the loss.
                printf("You have no conscious Kingsguard left.\n");
                // Allow loop to continue, is_battle_over will end it.
            } else {
                printf("Enter 'switch <Kingsguard Name>'.\n");
            }
          return false; // Player needs to choose SWITCH
    }


    current_battle.player_action = action;
    // Store the parameter (move name or kingsguard name)
    // Use a small buffer for the parameter string
    static char player_param_buffer[50]; // static so it persists between calls
    strncpy(player_param_buffer, param, sizeof(player_param_buffer) - 1);
    player_param_buffer[sizeof(player_param_buffer) - 1] = '\0';
    current_battle.player_action_param = player_param_buffer;

    // --- Validate the specific action and parameter ---
    bool action_is_valid = false;

    switch (action) {
        case BATTLE_ACTION_ATTACK: {
             if (player_active_fainted) {
                 // Should have been caught by the check above, but defensive
                 printf("Your Kingsguard cannot attack right now.\n");
                 return false;
             }
            // Check if the active Kingsguard knows this move and has PP
            // Get pointer to the Move struct *within the battle instance's KG data*
            Move *chosen_move = NULL; // No longer const
            for (int i = 0; i < current_battle.player_active_kg->num_moves; ++i) {
                 // Corrected: Use current_battle
                if (strcasecmp(current_battle.player_active_kg->moves[i].name, param) == 0) {
                     // Corrected: Use &current_battle and get address of the move
                    chosen_move = &current_battle.player_active_kg->moves[i];
                    break;
                }
            }
            if (chosen_move == NULL) {
                printf("%s doesn't know the move '%s'.\n", current_battle.player_active_kg->name, param);
            } else if (chosen_move->pp <= 0) {
                 printf("%s is out of PP for %s!\n", current_battle.player_active_kg->name, param);
            }
            else {
                 action_is_valid = true;
                 // Don't decrease PP yet, do it when the move is used in process_turn
            }
            break;
        }
        case BATTLE_ACTION_SWITCH: {
            // Check if the Kingsguard exists in the player's party and is conscious
            Kingsguard *switch_target = NULL;
            int target_index = -1;
            for (int i = 0; i < current_battle.player_party_size; ++i) {
                if (current_battle.player_party[i] != NULL && strcasecmp(current_battle.player_party[i]->name, param) == 0) {
                    switch_target = current_battle.player_party[i];
                    target_index = i;
                    break;
                }
            }

            if (switch_target == NULL) {
                printf("You don't have a Kingsguard named '%s'.\n", param);
            } else if (!is_conscious(switch_target)) {
                 printf("%s has fainted and cannot be switched in.\n", switch_target->name);
            } else if (switch_target == current_battle.player_active_kg && is_conscious(current_battle.player_active_kg)) {
                 // Disallow switching to self if current KG is conscious
                 printf("%s is already your active Kingsguard.\n", switch_target->name);
             } else if (switch_target == current_battle.player_active_kg && !is_conscious(current_battle.player_active_kg)) {
                // Allow switching to self *only* if current KG is fainted AND they are the only option
                // (This case is covered by find_first_conscious returning the same KG if no others are available)
                 // However, the UI prompts for a different KG. Let's keep the logic simple: require switching to a *different* KG if possible.
                 // If no *other* KG is conscious, the battle should have ended.
                 // So if player_active_fainted is true, and find_first_conscious returned NULL, battle is over.
                 // If player_active_fainted is true, and find_first_conscious returned a *different* KG, that's the valid target.
                 // If player_active_fainted is true, and find_first_conscious returned the *same* KG (unlikely with 3 KG), that means no *other* conscious KG exist, battle is over.
                 // Simplify: Switching to self is only allowed implicitly if you *must* switch and no one else is available (i.e. battle ends).
                 // For player input, require switching to a different conscious KG.

                 // Let's check if there is ANY other conscious KG. If not, the battle IS over.
                  int dummy_index;
                  bool other_conscious_exists = false;
                  for(int i = 0; i < current_battle.player_party_size; ++i) {
                      if (current_battle.player_party[i] != NULL && is_conscious(current_battle.player_party[i]) && current_battle.player_party[i] != current_battle.player_active_kg) {
                          other_conscious_exists = true;
                          break;
                      }
                  }

                 if (!other_conscious_exists && player_active_fainted) {
                     // This means the user is trying to switch but has no conscious KG left.
                     // Battle should be over. is_battle_over check will handle this.
                     printf("You have no other Kingsguard to switch to.\n");
                 } else {
                      // Valid switch target found (must be different and conscious)
                      action_is_valid = true;
                 }
            }
            else {
                 action_is_valid = true;
                 // Switching consumes a turn. The actual switch happens based on speed.
            }
            break;
        }
        case BATTLE_ACTION_RUN:
            printf("This is a duel! There is no running!\n");
            break; // Cannot run in a duel
        default:
            printf("Unknown battle command.\n");
            break; // Should not happen if parse_execute filters correctly
    }

    // If the player's action is valid, decide opponent action and process turn
    if (action_is_valid) {
        // Opponent AI chooses action
        // First, check if opponent needs to switch because their active KG fainted
        if (current_battle.opponent_active_kg == NULL || !is_conscious(current_battle.opponent_active_kg)) {
             int new_opp_index;
             Kingsguard *next_opp = find_first_conscious(current_battle.opponent_party, current_battle.opponent_party_size, &new_opp_index);
             if (next_opp != NULL) {
                  // Opponent must switch
                  current_battle.opponent_action = BATTLE_ACTION_SWITCH;
                   static char opp_param_buffer_switch[50];
                  strncpy(opp_param_buffer_switch, next_opp->name, sizeof(opp_param_buffer_switch) - 1);
                  opp_param_buffer_switch[sizeof(opp_param_buffer_switch)-1] = '\0';
                  current_battle.opponent_action_param = opp_param_buffer_switch;
                  printf("Opponent is choosing their action...\n"); // Announce opponent is thinking
             } else {
                 // Opponent has no conscious KG left, battle is over (player wins)
                 // This condition will be caught by is_battle_over and end_battle will be called.
                  current_battle.opponent_action = BATTLE_ACTION_RUN; // Placeholder, they can't act
                 current_battle.opponent_action_param = NULL;
             }
        } else {
            // Opponent's active KG is conscious, choose attack
            current_battle.opponent_action = BATTLE_ACTION_ATTACK; // Basic AI always attacks
            Move* opp_move = choose_opponent_move(current_battle.opponent_active_kg, current_battle.player_active_kg); // Pass non-const attacker
             static char opp_param_buffer_attack[50]; // static for opponent parameter
             if (opp_move) {
                 strncpy(opp_param_buffer_attack, opp_move->name, sizeof(opp_param_buffer_attack) - 1);
             } else {
                  // If opponent can't attack (e.g., no PP), they struggle or do nothing
                  // For simplicity, let's assume they always have a move with PP for now.
                  strncpy(opp_param_buffer_attack, "Struggle", sizeof(opp_param_buffer_attack) - 1); // Placeholder
             }
             opp_param_buffer_attack[sizeof(opp_param_buffer_attack)-1] = '\0';
             current_battle.opponent_action_param = opp_param_buffer_attack;
             printf("Opponent is choosing their action...\n");
        }

        // Process the turn now that both actions are decided
        battle_process_turn();
        return true; // Action was valid and turn processed
    } else {
        // Action was invalid, player needs to input again
        return false;
    }
}


void battle_process_turn(void)
{
    if (!current_battle.is_active) return; // Should not happen if called correctly

    // Check if battle ended due to fainted KG prompts handled in handle_player_action
    if (is_battle_over()) {
         end_battle();
         return;
    }

    printf("\n--- Processing Turn ---\n");

    // Determine turn order based on Speed
    bool player_goes_first = false;
    Kingsguard *first_actor = NULL;
    Kingsguard *second_actor = NULL;
    BattleAction first_action;
    const char *first_param = NULL;
    BattleAction second_action;
    const char *second_param = NULL;


    // Only compare speeds if both active KG are conscious
    if (current_battle.player_active_kg != NULL && is_conscious(current_battle.player_active_kg) &&
        current_battle.opponent_active_kg != NULL && is_conscious(current_battle.opponent_active_kg))
    {
         printf("Determining turn order...\n");
        if (current_battle.player_active_kg->speed > current_battle.opponent_active_kg->speed) {
            player_goes_first = true;
        } else if (current_battle.player_active_kg->speed < current_battle.opponent_active_kg->speed) {
            player_goes_first = false;
        } else {
            // Tie: 50/50 chance
            player_goes_first = (rand() % 2 == 0);
        }

        if (player_goes_first) {
             first_actor = current_battle.player_active_kg;
             first_action = current_battle.player_action;
             first_param = current_battle.player_action_param;
             second_actor = current_battle.opponent_active_kg;
             second_action = current_battle.opponent_action;
             second_param = current_battle.opponent_action_param;
             printf("%s is faster!\n", first_actor->name);
        } else {
             first_actor = current_battle.opponent_active_kg;
             first_action = current_battle.opponent_action;
             first_param = current_battle.opponent_action_param;
             second_actor = current_battle.player_active_kg;
             second_action = current_battle.player_action;
             second_param = current_battle.player_action_param;
             printf("%s is faster!\n", first_actor->name);
        }

    } else {
         // One or both are fainted or NULL. The conscious one goes first if they exist.
         // If only player is conscious
         if (current_battle.player_active_kg != NULL && is_conscious(current_battle.player_active_kg)) {
             first_actor = current_battle.player_active_kg;
             first_action = current_battle.player_action;
             first_param = current_battle.player_action_param;
             second_actor = NULL; // Opponent is fainted/NULL
             player_goes_first = true;
             printf("%s is the only one able to act.\n", first_actor->name);
         }
         // If only opponent is conscious
         else if (current_battle.opponent_active_kg != NULL && is_conscious(current_battle.opponent_active_kg)) {
             first_actor = current_battle.opponent_active_kg;
             first_action = current_battle.opponent_action;
             first_param = current_battle.opponent_action_param;
             second_actor = NULL; // Player is fainted/NULL
             player_goes_first = false;
              printf("%s is the only one able to act.\n", first_actor->name);
         } else {
              // Both are fainted/NULL. No actions happen.
               printf("Neither side can act.\n");
               // Check battle end condition again just in case
               if (is_battle_over()) end_battle();
               return;
         }

         // The second actor is NULL or fainted, their action won't process.
         // Set their action/param to something safe.
         second_action = BATTLE_ACTION_RUN; // Placeholder
         second_param = NULL;
    }


    // --- Process FIRST ACTOR's Action ---
    if (first_actor != NULL && is_conscious(first_actor)) {
        if (first_action == BATTLE_ACTION_ATTACK) {
            // Get pointer to the Move struct *within the actor's battle instance*
            Move *f_move = NULL; // No longer const
            for(int i=0; i < first_actor->num_moves; ++i) {
                 if(strcasecmp(first_actor->moves[i].name, first_param) == 0) {
                      f_move = &first_actor->moves[i];
                      break;
                 }
            }

            if (f_move != NULL && f_move->pp > 0) {
                printf("%s uses %s!\n", first_actor->name, f_move->name);
                // Check accuracy (simple hit/miss)
                if ((rand() % 100) < f_move->accuracy) {
                    Kingsguard *target = (first_actor == current_battle.player_active_kg) ? current_battle.opponent_active_kg : current_battle.player_active_kg; // Determine target

                    if (f_move->type == MOVE_TYPE_STATUS && f_move->power <= 0) { // Healing/Buff (Applies to user)
                         printf("%s restores %d HP.\n", first_actor->name, -f_move->power);
                         first_actor->current_hp -= f_move->power; // power is negative for healing
                         if (first_actor->current_hp > first_actor->max_hp)
                             first_actor->current_hp = first_actor->max_hp;
                    } else if (f_move->type != MOVE_TYPE_STATUS && target != NULL && is_conscious(target)) { // Attack
                        int damage = calculate_damage(first_actor, target, f_move);
                        printf("It hits %s for %d damage!\n", target->name, damage);
                        target->current_hp -= damage;
                        if (target->current_hp < 0) target->current_hp = 0;
                    } else {
                         printf("But it had no effect.\n"); // e.g., attacking fainted target
                    }
                    f_move->pp--; // Decrease PP after use
                } else {
                    printf("%s's attack missed!\n", first_actor->name);
                }
            } else {
                 // This case implies an error or ran out of PP between action selection and turn execution
                 printf("%s is unable to use %s!\n", first_actor->name, first_param);
            }
        } else if (first_action == BATTLE_ACTION_SWITCH) {
            // Find the intended switch target within the actor's party
            Kingsguard **party = (first_actor == current_battle.player_active_kg) ? current_battle.player_party : current_battle.opponent_party;
            int party_size = (first_actor == current_battle.player_active_kg) ? current_battle.player_party_size : current_battle.opponent_party_size;
            int target_index = -1;
            Kingsguard *switch_target = NULL;

            for (int i = 0; i < party_size; ++i) {
                 if (party[i] != NULL && strcasecmp(party[i]->name, first_param) == 0) {
                      switch_target = party[i];
                      target_index = i;
                      break;
                 }
            }

            // Double-check validity (should be valid if handled in handle_player_action)
            if (switch_target != NULL && is_conscious(switch_target) && switch_target != first_actor) {
                 printf("%s switches out %s...\n", (first_actor == current_battle.player_active_kg) ? "You" : "Opponent", first_actor->name);
                 if (first_actor == current_battle.player_active_kg) {
                     current_battle.player_active_kg = switch_target;
                     current_battle.player_active_index = target_index;
                     printf("Go, %s!\n", current_battle.player_active_kg->name);
                 } else {
                      current_battle.opponent_active_kg = switch_target;
                      current_battle.opponent_active_index = target_index;
                      printf("Opponent sends out %s!\n", current_battle.opponent_active_kg->name);
                 }
            } else {
                 // Fallback: If intended switch target is invalid (e.g. fainted), find *any* conscious KG
                 int new_idx;
                 Kingsguard *any_conscious = find_first_conscious(party, party_size, &new_idx);
                 if (any_conscious != NULL && any_conscious != first_actor) {
                      printf("%s needs to switch but %s is unavailable. Switching...\n", (first_actor == current_battle.player_active_kg) ? "You" : "Opponent", first_param);
                       if (first_actor == current_battle.player_active_kg) {
                            current_battle.player_active_kg = any_conscious;
                            current_battle.player_active_index = new_idx;
                            printf("Go, %s!\n", current_battle.player_active_kg->name);
                        } else {
                             current_battle.opponent_active_kg = any_conscious;
                             current_battle.opponent_active_index = new_idx;
                            printf("Opponent sends out %s!\n", current_battle.opponent_active_kg->name);
                        }
                 } else {
                      printf("%s failed to switch.\n", (first_actor == current_battle.player_active_kg) ? "You" : "Opponent"); // Should only happen if no other KG are conscious
                 }
            }
        } else {
             // BATTLE_ACTION_RUN or other unexpected action
             printf("%s was unable to perform their action.\n", first_actor->name);
        }
    } else {
        printf("%s is unable to act.\n", first_actor ? first_actor->name : "A fainted Kingsguard");
    }


    // Check battle end after first action
    if (current_battle.is_active && is_battle_over()) {
        end_battle();
        return; // Exit after ending battle
    }

    // --- Process SECOND ACTOR's Action ---
    // Check if the second actor is still conscious after the first actor's move
    if (second_actor != NULL && is_conscious(second_actor)) {
         printf("\n%s prepares to act...\n", second_actor->name);
         if (second_action == BATTLE_ACTION_ATTACK) {
             // Get pointer to the Move struct *within the actor's battle instance*
              Move *s_move = NULL; // No longer const
              for(int i=0; i < second_actor->num_moves; ++i) {
                  if(strcasecmp(second_actor->moves[i].name, second_param) == 0) {
                       s_move = &second_actor->moves[i];
                       break;
                  }
              }

              if (s_move != NULL && s_move->pp > 0) {
                  printf("%s uses %s!\n", second_actor->name, s_move->name);
                   // Check accuracy
                  if ((rand() % 100) < s_move->accuracy) {
                      Kingsguard *target = (second_actor == current_battle.player_active_kg) ? current_battle.opponent_active_kg : current_battle.player_active_kg; // Determine target

                       if (s_move->type == MOVE_TYPE_STATUS && s_move->power <= 0) { // Healing/Buff
                            printf("%s restores %d HP.\n", second_actor->name, -s_move->power);
                            second_actor->current_hp -= s_move->power;
                            if (second_actor->current_hp > second_actor->max_hp)
                                second_actor->current_hp = second_actor->max_hp;
                      } else if (s_move->type != MOVE_TYPE_STATUS && target != NULL && is_conscious(target)) { // Attack
                           int damage = calculate_damage(second_actor, target, s_move);
                           printf("It hits %s for %d damage!\n", target->name, damage);
                           target->current_hp -= damage;
                           if (target->current_hp < 0) target->current_hp = 0;
                      } else {
                           printf("But it had no effect.\n");
                      }
                      s_move->pp--;
                  } else {
                      printf("%s's attack missed!\n", second_actor->name);
                  }
              } else {
                   printf("%s is unable to use a move!\n", second_actor->name);
              }
          } else if (second_action == BATTLE_ACTION_SWITCH) {
              // Find the intended switch target within the actor's party
               Kingsguard **party = (second_actor == current_battle.player_active_kg) ? current_battle.player_party : current_battle.opponent_party;
              int party_size = (second_actor == current_battle.player_active_kg) ? current_battle.player_party_size : current_battle.opponent_party_size;
               int target_index = -1;
              Kingsguard *switch_target = NULL;

               for (int i = 0; i < party_size; ++i) {
                   if (party[i] != NULL && strcasecmp(party[i]->name, second_param) == 0) {
                        switch_target = party[i];
                        target_index = i;
                        break;
                   }
               }

              // Double-check validity (should be valid if handled in handle_player_action)
               if (switch_target != NULL && is_conscious(switch_target) && switch_target != second_actor) {
                    printf("%s switches out %s...\n", (second_actor == current_battle.player_active_kg) ? "You" : "Opponent", second_actor->name);
                    if (second_actor == current_battle.player_active_kg) {
                        current_battle.player_active_kg = switch_target;
                        current_battle.player_active_index = target_index;
                        printf("Go, %s!\n", current_battle.player_active_kg->name);
                    } else {
                         current_battle.opponent_active_kg = switch_target;
                         current_battle.opponent_active_index = target_index;
                        printf("Opponent sends out %s!\n", current_battle.opponent_active_kg->name);
                    }
               } else {
                     int new_idx;
                    Kingsguard *any_conscious = find_first_conscious(party, party_size, &new_idx);
                    if (any_conscious != NULL && any_conscious != second_actor) {
                         printf("%s needs to switch but %s is unavailable. Switching...\n", (second_actor == current_battle.player_active_kg) ? "You" : "Opponent", second_param);
                          if (second_actor == current_battle.player_active_kg) {
                               current_battle.player_active_kg = any_conscious;
                               current_battle.player_active_index = new_idx;
                               printf("Go, %s!\n", current_battle.player_active_kg->name);
                           } else {
                                current_battle.opponent_active_kg = any_conscious;
                                current_battle.opponent_active_index = new_idx;
                               printf("Opponent sends out %s!\n", current_battle.opponent_active_kg->name);
                           }
                    } else {
                         printf("%s failed to switch.\n", (second_actor == current_battle.player_active_kg) ? "You" : "Opponent");
                    }
               }
         } else {
              // BATTLE_ACTION_RUN or other unexpected action
             printf("%s was unable to perform their action.\n", second_actor->name);
         }

    } else {
        printf("%s is unable to act.\n", second_actor ? second_actor->name : "A fainted Kingsguard");
    }


    // --- End of Turn Checks ---

    // If player's active KG fainted *during* this turn, handle it
    if (current_battle.is_active && current_battle.player_active_kg != NULL && !is_conscious(current_battle.player_active_kg)) {
        printf("\n%s fainted!\n", current_battle.player_active_kg->name);
        current_battle.player_active_kg = NULL; // Mark as fainted/inactive

        // Check if player has any conscious Kingsguard left
        int dummy_index;
        if (find_first_conscious(current_battle.player_party, current_battle.player_party_size, &dummy_index) == NULL) {
             // No conscious KG left! Player loses.
             current_battle.player_won = false;
             end_battle(); // This will change state to GAME_OVER
             return; // Exit after ending battle
        } else {
             // Player must switch. Next input *must* be a switch command.
             // The getInput function in main.c will detect this and prompt for switch.
             // We don't set the state here, remain in STATE_BATTLE but main loop
             // knows player_active_kg is NULL and enforces switch.
        }
    }

     // If opponent's active KG fainted *during* this turn, handle it
     if (current_battle.is_active && current_battle.opponent_active_kg != NULL && !is_conscious(current_battle.opponent_active_kg)) {
         printf("\nOpponent's %s fainted!\n", current_battle.opponent_active_kg->name);
         current_battle.opponent_active_kg = NULL; // Mark as fainted/inactive

         // Opponent finds next conscious Kingsguard
         int new_opp_index;
         Kingsguard *next_opp = find_first_conscious(current_battle.opponent_party, current_battle.opponent_party_size, &new_opp_index);

         if (next_opp == NULL) {
              // No conscious KG left for opponent! Player wins.
              current_battle.player_won = true;
              end_battle(); // This will change state to VICTORY or next plot point
              return; // Exit after ending battle
         } else {
              // Opponent switches automatically
              current_battle.opponent_active_kg = next_opp;
              current_battle.opponent_active_index = new_opp_index;
              printf("Opponent sends out %s.\n", current_battle.opponent_active_kg->name);
         }
     }

     // Final check if battle is over after all fainted checks and potential automatic switches
     if (current_battle.is_active && is_battle_over()) {
         end_battle();
         return; // Exit after ending battle
     } else if (current_battle.is_active) {
         // Battle continues. Display status and prompt for next turn.
         printf("\n--- Battle Status ---\n");
         if (current_battle.player_active_kg) print_kingsguard_stats(current_battle.player_active_kg);
         else printf("Your active Kingsguard slot is empty.\n"); // Indicate fainted

         if (current_battle.opponent_active_kg) print_kingsguard_stats(current_battle.opponent_active_kg);
         else printf("Opponent's active Kingsguard slot is empty.\n"); // Indicate fainted

         // Prompt for player action again (handled by main loop -> getInput -> parse_execute)
     }
}


bool is_battle_over(void)
{
    // Battle is over if either side cannot continue (no conscious KG left)
    return !current_battle.is_active || // Battle already ended explicitly
           find_first_conscious(current_battle.player_party, current_battle.player_party_size, NULL) == NULL ||
           find_first_conscious(current_battle.opponent_party, current_battle.opponent_party_size, NULL) == NULL;
}

void end_battle(void)
{
    if (!current_battle.is_active) {
        // Free memory even if battle somehow wasn't marked active? Defensive.
         for(int i=0; i < current_battle.player_party_size; ++i) { free(current_battle.player_party[i]); current_battle.player_party[i] = NULL; }
         for(int i=0; i < current_battle.opponent_party_size; ++i) { free(current_battle.opponent_party[i]); current_battle.opponent_party[i] = NULL; }
         current_battle.player_active_kg = NULL;
         current_battle.opponent_active_kg = NULL;
         game.active_battle = NULL;
        return; // Already ended
    }

    // Determine result *before* setting is_active to false or freeing memory
    bool player_has_conscious = find_first_conscious(current_battle.player_party, current_battle.player_party_size, NULL) != NULL;
    bool opponent_has_conscious = find_first_conscious(current_battle.opponent_party, current_battle.opponent_party_size, NULL) != NULL;

    current_battle.is_active = false; // Set battle inactive first

    // Set the win flag based on checks
    current_battle.player_won = player_has_conscious && !opponent_has_conscious;
    // If both ran out, it's a loss for player
     if (!player_has_conscious && !opponent_has_conscious) current_battle.player_won = false;


    printf("\n--- BATTLE END ---\n");

    if (current_battle.player_won) {
        printf("You were victorious!\n");
        // Trigger post-battle event based on which battle it was
        // The location in game.current_location should be the one where battle started.
        int battle_location = game.current_location;

        if (battle_location == LOCATION_COURTYARD) {
             print_dialogue("heir_battle_win");
             game.plot_flags.has_summoning_artifact = true; // Player gets the artifact
             // Transition state and location after event
             set_game_state(STATE_THRONE_ROOM); // Return to throne room
             game.current_location = LOCATION_THRONE_ROOM; // Explicitly set location
             printf("You return to the Throne Room.\n");
        } else if (battle_location == LOCATION_COLOSSEUM) {
             print_dialogue("game_victory");
             set_game_state(STATE_VICTORY); // Final victory state
        } else {
             // Won a battle in an unexpected location? Just return to that location.
             set_game_state(battle_location);
             // game.current_location remains the same
        }

    } else { // Player lost
        printf("You were defeated...\n");
        print_dialogue("game_over_loss");
        set_game_state(STATE_GAME_OVER); // Game over state
    }

    // Free the temporary Kingsguard copies used in battle
    for(int i=0; i < current_battle.player_party_size; ++i) {
        free(current_battle.player_party[i]);
        current_battle.player_party[i] = NULL; // Prevent double free
    }
     for(int i=0; i < current_battle.opponent_party_size; ++i) {
        free(current_battle.opponent_party[i]);
        current_battle.opponent_party[i] = NULL; // Prevent double free
    }

    current_battle.player_active_kg = NULL; // Clear pointers
    current_battle.opponent_active_kg = NULL;
    game.active_battle = NULL; // Clear battle state pointer in game state
}
