#include "../inc/battle.h"
#include "../inc/game_state.h"
#include "../inc/kingsguard.h"
#include "../inc/dialogue.h"
#include "../inc/locations.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>


BattleState current_battle;

static Kingsguard* find_first_conscious(Kingsguard *party[], int party_size, int *index) {
    for (int i = 0; i < party_size; i++) {
        if (party[i] != NULL && is_conscious(party[i])) {
            if (index != NULL) *index = i;
            return party[i];
        }
    }
    if (index != NULL) *index = -1;
    return NULL;
}

static int calculate_damage(const Kingsguard *attacker, const Kingsguard *defender, const Move *move) {
    if (move->type == MOVE_TYPE_STATUS || move->power <= 0) {
        return 0; // Status moves or healing don't deal damage this way
    }

    int attack_stat = (move->type == MOVE_TYPE_PHYSICAL) ? attacker->attack : attacker->sp_attack;
    int defense_stat = (move->type == MOVE_TYPE_PHYSICAL) ? defender->defense : defender->sp_defense;

    if (defense_stat == 0) defense_stat = 1;
    // Formula: ((Power * AttackerStat / DefenderStat) / 5) + 2 (Rough Pokémon approximation)
    // Scale it down a bit for smaller numbers
    int damage = ((move->power * attack_stat / defense_stat) / 10) + 1;

    // Add some randomness (optional)
    // damage = damage * (100 - (rand() % 16)) / 100; // ~0.85 to 1.0 variance

    if (damage < 1) damage = 1;

    return damage;
}

// Returns a pointer to the Move struct *within the attacker's battle instance*
static Move* choose_opponent_move(Kingsguard *attacker, const Kingsguard *defender) {
    if (attacker == NULL || attacker->num_moves == 0) return NULL;

    Move *chosen_move = NULL;
    int attempts = 0;
    const int max_attempts = 10;

    while (attempts < max_attempts) {
        int move_index = rand() % attacker->num_moves;
        chosen_move = &attacker->moves[move_index];
        if (chosen_move->pp > 0) {
            return chosen_move;
        }
        attempts++;
    }

    for(int i=0; i < attacker->num_moves; ++i) {
        if(attacker->moves[i].pp > 0) return &attacker->moves[i];
    }

    return NULL;
}

static Kingsguard* choose_opponent_switch(Kingsguard *party[], int party_size, int current_index, int *new_index) {
    for (int i = 0; i < party_size; i++) {
        if (i != current_index && party[i] != NULL && is_conscious(party[i])) {
            *new_index = i;
            return party[i];
        }
    }
    *new_index = -1;
    return NULL;
}


void start_battle(Kingsguard *opponent_kingsguard_templates[], int num_opponents)
{
    // Seed random number generator (only once per program run usually, but fine here for testing)
    srand(time(NULL));

    memset(&current_battle, 0, sizeof(BattleState));
    game.active_battle = &current_battle;


    // --- Setup Player Party (Temporary copies for battle) ---
    current_battle.player_party_size = 0;
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
         set_game_state(STATE_GAME_OVER);
         end_battle();
         return;
    }
    current_battle.player_active_kg = find_first_conscious(current_battle.player_party, current_battle.player_party_size, &current_battle.player_active_index);
    if (current_battle.player_active_kg == NULL) {
         printf("ERROR: Player Kingsguard are all fainted at start of battle!\n");
         set_game_state(STATE_GAME_OVER);
         end_battle();
         return;
    }
     printf("Your active Kingsguard is %s.\n", current_battle.player_active_kg->name);


    // --- Setup Opponent Party (Temporary copies for battle) ---
    current_battle.opponent_party_size = 0;
    for (int i = 0; i < num_opponents && i < 3; ++i) {
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
         set_game_state(STATE_GAME_OVER);
         end_battle();
         return;
    }
    current_battle.opponent_active_kg = find_first_conscious(current_battle.opponent_party, current_battle.opponent_party_size, &current_battle.opponent_active_index);
     if (current_battle.opponent_active_kg == NULL) {
         printf("ERROR: Opponent Kingsguard are all fainted at start of battle!\n");
         set_game_state(STATE_GAME_OVER);
         end_battle();
         return;
    }
     printf("Opponent's active Kingsguard is %s.\n", current_battle.opponent_active_kg->name);


    // --- Start Battle ---
    game.pre_battle_state = get_game_state();
    current_battle.is_active = true;
    current_battle.player_won = false;
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
             return false;
         }
        printf("There is no active battle.\n");
        // set_game_state(game.current_location); // Should already be set by end_battle
        return false;
    }

    // Basic validation: Can the active KG act? If fainted, only switch is allowed.
    bool player_active_fainted = (current_battle.player_active_kg == NULL || !is_conscious(current_battle.player_active_kg));

    if (player_active_fainted && action != BATTLE_ACTION_SWITCH) {
         printf("Your active Kingsguard has fainted! You must switch.\n");
            int dummy_index;
            if (find_first_conscious(current_battle.player_party, current_battle.player_party_size, &dummy_index) == NULL) {
                printf("You have no conscious Kingsguard left.\n");
            } else {
                printf("Enter 'switch <Kingsguard Name>'.\n");
            }
          return false;
    }


    current_battle.player_action = action;
    static char player_param_buffer[50];
    strncpy(player_param_buffer, param ? param : "", sizeof(player_param_buffer) - 1);
    player_param_buffer[sizeof(player_param_buffer) - 1] = '\0';
    current_battle.player_action_param = player_param_buffer;

    // --- Validate the specific action and parameter ---
    bool action_is_valid = false;

    switch (action) {
        case BATTLE_ACTION_ATTACK: {
             if (player_active_fainted) {
                 printf("Your Kingsguard cannot attack right now.\n");
                 return false;
             }
            Move *chosen_move = NULL;
            for (int i = 0; i < current_battle.player_active_kg->num_moves; ++i) {
                if (strcasecmp(current_battle.player_active_kg->moves[i].name, param) == 0) {
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
            }
            break;
        }
        case BATTLE_ACTION_SWITCH: {
            Kingsguard *switch_target = NULL;
            for (int i = 0; i < current_battle.player_party_size; ++i) {
                if (current_battle.player_party[i] != NULL && strcasecmp(current_battle.player_party[i]->name, param) == 0) {
                    switch_target = current_battle.player_party[i];
                    break;
                }
            }

            if (switch_target == NULL) {
                printf("You don't have a Kingsguard named '%s'.\n", param);
            } else if (!is_conscious(switch_target)) {
                 printf("%s has fainted and cannot be switched in.\n", switch_target->name);
            } else if (switch_target == current_battle.player_active_kg && !player_active_fainted) {
                 printf("%s is already your active Kingsguard.\n", switch_target->name);
             }
            else {
                 action_is_valid = true;
            }
            break;
        }
        case BATTLE_ACTION_RUN:
            printf("This is a duel! There is no running!\n");
            break;
        default:
            printf("Unknown battle command.\n");
            break;
    }

    if (action_is_valid) {
        if (current_battle.opponent_active_kg == NULL || !is_conscious(current_battle.opponent_active_kg)) {
             int new_opp_index;
             Kingsguard *next_opp = find_first_conscious(current_battle.opponent_party, current_battle.opponent_party_size, &new_opp_index);
             if (next_opp != NULL) {
                  current_battle.opponent_action = BATTLE_ACTION_SWITCH;
                   static char opp_param_buffer_switch[50];
                  strncpy(opp_param_buffer_switch, next_opp->name, sizeof(opp_param_buffer_switch) - 1);
                  opp_param_buffer_switch[sizeof(opp_param_buffer_switch)-1] = '\0';
                  current_battle.opponent_action_param = opp_param_buffer_switch;
                  printf("Opponent must switch...\n");
             } else {
                  current_battle.opponent_action = BATTLE_ACTION_RUN;
                  current_battle.opponent_action_param = NULL;
             }
        } else {
            current_battle.opponent_action = BATTLE_ACTION_ATTACK;
            Move* opp_move = choose_opponent_move(current_battle.opponent_active_kg, current_battle.player_active_kg);
             static char opp_param_buffer_attack[50];
             if (opp_move) {
                 strncpy(opp_param_buffer_attack, opp_move->name, sizeof(opp_param_buffer_attack) - 1);
             } else {
                  strncpy(opp_param_buffer_attack, "Struggle", sizeof(opp_param_buffer_attack) - 1);
             }
             opp_param_buffer_attack[sizeof(opp_param_buffer_attack)-1] = '\0';
             current_battle.opponent_action_param = opp_param_buffer_attack;
             printf("Opponent is choosing their action...\n");
        }

        battle_process_turn();
        return true;
    } else {
        return false;
    }
}


void battle_process_turn(void)
{
    if (!current_battle.is_active) return;
    if (is_battle_over()) { end_battle(); return; }

    printf("\n--- Processing Turn ---\n");

    Kingsguard *actor1 = NULL, *actor2 = NULL;
    BattleAction action1 = BATTLE_ACTION_RUN, action2 = BATTLE_ACTION_RUN;
    const char *param1 = NULL, *param2 = NULL;
    bool player_acted_via_forced_switch = false;

    bool player_starts_fainted = (current_battle.player_active_kg == NULL || !is_conscious(current_battle.player_active_kg));

    // 1. Handle Player's forced switch (if fainted and action is switch)
    if (player_starts_fainted && current_battle.player_action == BATTLE_ACTION_SWITCH) {
        Kingsguard *switch_target = NULL;
        int target_index = -1;
        for (int i = 0; i < current_battle.player_party_size; ++i) {
            if (current_battle.player_party[i] != NULL &&
                current_battle.player_action_param &&
                strcasecmp(current_battle.player_party[i]->name, current_battle.player_action_param) == 0) {
                switch_target = current_battle.player_party[i];
                target_index = i;
                break;
            }
        }
        if (switch_target != NULL && is_conscious(switch_target)) {
            printf("You had to switch. ");
            current_battle.player_active_kg = switch_target;
            current_battle.player_active_index = target_index;
            printf("Go, %s!\n", current_battle.player_active_kg->name);
            player_acted_via_forced_switch = true;

            actor1 = current_battle.opponent_active_kg;
            action1 = current_battle.opponent_action;
            param1 = current_battle.opponent_action_param;
            actor2 = NULL;
            if(actor1 && is_conscious(actor1)) printf("%s prepares to act against your new Kingsguard!\n", actor1->name);
            else if (actor1) printf("%s is unable to act (fainted).\n", actor1->name);
            else printf("Opponent has no conscious Kingsguard to act.\n");
        } else {
            printf("Error during forced switch: '%s' is not a valid switch target now.\n", current_battle.player_action_param ? current_battle.player_action_param : "Unknown");
            if (is_battle_over()) end_battle();
            return;
        }
    }

    // 2. If player didn't do a forced switch, determine normal turn order
    if (!player_acted_via_forced_switch) {
        bool player_can_act_now = (current_battle.player_active_kg != NULL && is_conscious(current_battle.player_active_kg));
        bool opponent_can_act_now = (current_battle.opponent_active_kg != NULL && is_conscious(current_battle.opponent_active_kg));

        if (player_can_act_now && opponent_can_act_now) {
            printf("Determining turn order...\n");
            bool player_goes_first_regular = false;
            if (current_battle.player_active_kg->speed > current_battle.opponent_active_kg->speed) player_goes_first_regular = true;
            else if (current_battle.player_active_kg->speed < current_battle.opponent_active_kg->speed) player_goes_first_regular = false;
            else player_goes_first_regular = (rand() % 2 == 0);

            if (player_goes_first_regular) {
                actor1 = current_battle.player_active_kg; action1 = current_battle.player_action; param1 = current_battle.player_action_param;
                actor2 = current_battle.opponent_active_kg; action2 = current_battle.opponent_action; param2 = current_battle.opponent_action_param;
            } else {
                actor1 = current_battle.opponent_active_kg; action1 = current_battle.opponent_action; param1 = current_battle.opponent_action_param;
                actor2 = current_battle.player_active_kg; action2 = current_battle.player_action; param2 = current_battle.player_action_param;
            }
             printf("%s is faster!\n", actor1->name);
        } else if (player_can_act_now) {
            actor1 = current_battle.player_active_kg; action1 = current_battle.player_action; param1 = current_battle.player_action_param;
            actor2 = NULL;
            printf("%s is the only one able to act.\n", actor1->name);
        } else if (opponent_can_act_now) {
            actor1 = current_battle.opponent_active_kg; action1 = current_battle.opponent_action; param1 = current_battle.opponent_action_param;
            actor2 = NULL;
            printf("%s is the only one able to act.\n", actor1->name);
        } else {
            printf("Neither side can act this turn.\n");
            if (is_battle_over()) end_battle();
            return;
        }
    }

    // --- Process Actor 1's Action ---
    if (actor1 != NULL && is_conscious(actor1)) {
        Kingsguard* target_for_actor1 = NULL;
        if (actor1 == current_battle.player_active_kg) target_for_actor1 = current_battle.opponent_active_kg;
        else if (actor1 == current_battle.opponent_active_kg) target_for_actor1 = current_battle.player_active_kg;

        if (action1 == BATTLE_ACTION_ATTACK) {
            Move *f_move = NULL;
            for(int i=0; i < actor1->num_moves; ++i) {
                 if(param1 && strcasecmp(actor1->moves[i].name, param1) == 0) { f_move = &actor1->moves[i]; break; }
            }
            if (f_move != NULL && f_move->pp > 0) {
                printf("%s uses %s!\n", actor1->name, f_move->name);
                if ((rand() % 100) < f_move->accuracy) {
                    if (f_move->type == MOVE_TYPE_STATUS && f_move->power <= 0) {
                         printf("%s restores %d HP.\n", actor1->name, -f_move->power);
                         actor1->current_hp -= f_move->power;
                         if (actor1->current_hp > actor1->max_hp) actor1->current_hp = actor1->max_hp;
                    } else if (f_move->type != MOVE_TYPE_STATUS && target_for_actor1 != NULL && is_conscious(target_for_actor1)) {
                        int damage = calculate_damage(actor1, target_for_actor1, f_move);
                        printf("It hits %s for %d damage!\n", target_for_actor1->name, damage);
                        target_for_actor1->current_hp -= damage;
                        if (target_for_actor1->current_hp < 0) target_for_actor1->current_hp = 0;
                    } else { printf("But it had no effect.\n"); }
                    f_move->pp--;
                } else { printf("%s's attack missed!\n", actor1->name); }
            } else { printf("%s is unable to use %s!\n", actor1->name, param1 ? param1 : "a move"); }
        } else if (action1 == BATTLE_ACTION_SWITCH) {
            Kingsguard **party = (actor1 == current_battle.player_active_kg) ? current_battle.player_party : current_battle.opponent_party;
            int party_size = (actor1 == current_battle.player_active_kg) ? current_battle.player_party_size : current_battle.opponent_party_size;
            int target_idx_actor1 = -1; Kingsguard *switch_target_actor1 = NULL;
            for (int i = 0; i < party_size; ++i) {
                 if (party[i] != NULL && param1 && strcasecmp(party[i]->name, param1) == 0) {
                      switch_target_actor1 = party[i]; target_idx_actor1 = i; break;
                 }
            }
            if (switch_target_actor1 != NULL && is_conscious(switch_target_actor1) && switch_target_actor1 != actor1) {
                 printf("%s switches out %s...\n", (actor1 == current_battle.player_active_kg) ? "You" : "Opponent", actor1->name);
                 if (actor1 == current_battle.player_active_kg) {
                     current_battle.player_active_kg = switch_target_actor1; current_battle.player_active_index = target_idx_actor1;
                     printf("Go, %s!\n", current_battle.player_active_kg->name);
                 } else {
                      current_battle.opponent_active_kg = switch_target_actor1; current_battle.opponent_active_index = target_idx_actor1;
                      printf("Opponent sends out %s!\n", current_battle.opponent_active_kg->name);
                 }
            } else { printf("%s failed to switch.\n", (actor1 == current_battle.player_active_kg) ? "You" : "Opponent"); }
        } else if (action1 != BATTLE_ACTION_RUN) { printf("%s was unable to perform their action.\n", actor1->name); }
    } else if (actor1 != NULL && !is_conscious(actor1)) { printf("%s is unable to act (fainted).\n", actor1->name); }


    if (current_battle.is_active && is_battle_over()) { end_battle(); return; }

    // --- Process Actor 2's Action ---
    if (actor2 != NULL && is_conscious(actor2)) {
        // Check if actor2 fainted from actor1's move
        if (!is_conscious(actor2)) {
            printf("%s fainted before it could act!\n", actor2->name);
        } else {
            printf("\n%s prepares to act...\n", actor2->name);
            Kingsguard* target_for_actor2 = NULL;
            if (actor2 == current_battle.player_active_kg) target_for_actor2 = current_battle.opponent_active_kg;
            else if (actor2 == current_battle.opponent_active_kg) target_for_actor2 = current_battle.player_active_kg;

            if (action2 == BATTLE_ACTION_ATTACK) {
                Move *s_move = NULL;
                for(int i=0; i < actor2->num_moves; ++i) {
                    if(param2 && strcasecmp(actor2->moves[i].name, param2) == 0) { s_move = &actor2->moves[i]; break; }
                }
                if (s_move != NULL && s_move->pp > 0) {
                    printf("%s uses %s!\n", actor2->name, s_move->name);
                    if ((rand() % 100) < s_move->accuracy) {
                        if (s_move->type == MOVE_TYPE_STATUS && s_move->power <= 0) {
                            printf("%s restores %d HP.\n", actor2->name, -s_move->power);
                            actor2->current_hp -= s_move->power;
                            if (actor2->current_hp > actor2->max_hp)
                                actor2->current_hp = actor2->max_hp;
                        } else if (s_move->type != MOVE_TYPE_STATUS && target_for_actor2 != NULL && is_conscious(target_for_actor2)) {
                           int damage = calculate_damage(actor2, target_for_actor2, s_move);
                           printf("It hits %s for %d damage!\n", target_for_actor2->name, damage);
                           target_for_actor2->current_hp -= damage;
                           if (target_for_actor2->current_hp < 0) target_for_actor2->current_hp = 0;
                        } else { printf("But it had no effect.\n"); }
                        s_move->pp--;
                    } else { printf("%s's attack missed!\n", actor2->name); }
                } else { printf("%s is unable to use %s!\n", actor2->name, param2 ? param2 : "a move"); }
            } else if (action2 == BATTLE_ACTION_SWITCH) {
               Kingsguard **party2 = (actor2 == current_battle.player_active_kg) ? current_battle.player_party : current_battle.opponent_party;
              int party_size2 = (actor2 == current_battle.player_active_kg) ? current_battle.player_party_size : current_battle.opponent_party_size;
               int target_idx_actor2 = -1;
              Kingsguard *switch_target_actor2 = NULL;

               for (int i = 0; i < party_size2; ++i) {
                   if (party2[i] != NULL && param2 && strcasecmp(party2[i]->name, param2) == 0) {
                        switch_target_actor2 = party2[i];
                        target_idx_actor2 = i;
                        break;
                   }
               }
               if (switch_target_actor2 != NULL && is_conscious(switch_target_actor2) && switch_target_actor2 != actor2) {
                    printf("%s switches out %s...\n", (actor2 == current_battle.player_active_kg) ? "You" : "Opponent", actor2->name);
                    if (actor2 == current_battle.player_active_kg) {
                        current_battle.player_active_kg = switch_target_actor2;
                        current_battle.player_active_index = target_idx_actor2;
                        printf("Go, %s!\n", current_battle.player_active_kg->name);
                    } else {
                         current_battle.opponent_active_kg = switch_target_actor2;
                         current_battle.opponent_active_index = target_idx_actor2;
                        printf("Opponent sends out %s!\n", current_battle.opponent_active_kg->name);
                    }
               } else { printf("%s failed to switch.\n", (actor2 == current_battle.player_active_kg) ? "You" : "Opponent");}
            } else if (action2 != BATTLE_ACTION_RUN) { printf("%s was unable to perform their action.\n", actor2->name); }
        }
    } else if (actor2 != NULL && !is_conscious(actor2)) { printf("%s is unable to act (fainted).\n", actor2->name); }


    // --- End of Turn Faint Checks & Auto-Switches ---
    if (current_battle.is_active && current_battle.player_active_kg != NULL && !is_conscious(current_battle.player_active_kg)) {
        printf("\n%s fainted!\n", current_battle.player_active_kg->name);
        int dummy_index;
        if (find_first_conscious(current_battle.player_party, current_battle.player_party_size, &dummy_index) == NULL) {
             current_battle.player_won = false; end_battle(); return;
        }
    }

     if (current_battle.is_active && current_battle.opponent_active_kg != NULL && !is_conscious(current_battle.opponent_active_kg)) {
         printf("\nOpponent's %s fainted!\n", current_battle.opponent_active_kg->name);
         int new_opp_index;
         Kingsguard *next_opp = find_first_conscious(current_battle.opponent_party, current_battle.opponent_party_size, &new_opp_index);
         if (next_opp == NULL) {
              current_battle.player_won = true; end_battle(); return;
         } else {
              current_battle.opponent_active_kg = next_opp;
              current_battle.opponent_active_index = new_opp_index;
              printf("Opponent sends out %s.\n", current_battle.opponent_active_kg->name);
         }
     }

     if (current_battle.is_active && is_battle_over()) { end_battle(); return; }
}


bool is_battle_over(void)
{
    return !current_battle.is_active ||
           find_first_conscious(current_battle.player_party, current_battle.player_party_size, NULL) == NULL ||
           find_first_conscious(current_battle.opponent_party, current_battle.opponent_party_size, NULL) == NULL;
}

void end_battle(void)
{
    if (!current_battle.is_active && game.active_battle == NULL) {
        return;
    }

    bool player_has_conscious = find_first_conscious(current_battle.player_party, current_battle.player_party_size, NULL) != NULL;
    bool opponent_has_conscious = find_first_conscious(current_battle.opponent_party, current_battle.opponent_party_size, NULL) != NULL;

    current_battle.is_active = false;

    current_battle.player_won = player_has_conscious && !opponent_has_conscious;
    if (!player_has_conscious) current_battle.player_won = false;


    printf("\n--- BATTLE END ---\n");

    // Trigger post-battle event based on which battle it was
    if (current_battle.player_won) {
        printf("You were victorious!\n");
        if (game.pre_battle_state == STATE_COURTYARD) {
             print_dialogue("heir_battle_win");
             game.plot_flags.has_summoning_artifact = true;
             set_game_state(STATE_THRONE_ROOM);
             game.current_location = LOCATION_THRONE_ROOM;
             printf("You return to the Throne Room.\n");
             look_around();
        } else if (game.pre_battle_state == STATE_COLOSSEUM) {
             print_dialogue("game_victory");
             set_game_state(STATE_VICTORY);
        } else {
             set_game_state(game.pre_battle_state);
             if (game.pre_battle_state == STATE_THRONE_ROOM) game.current_location = LOCATION_THRONE_ROOM;
             else if (game.pre_battle_state == STATE_COURTYARD) game.current_location = LOCATION_COURTYARD;
             else if (game.pre_battle_state == STATE_SUMMONING_HALL) game.current_location = LOCATION_SUMMONING_HALL;
             else if (game.pre_battle_state == STATE_COLOSSEUM) game.current_location = LOCATION_COLOSSEUM;
             // else { // Potentially an issue if pre_battle_state was not a location state }
             look_around();
        }

    } else {
        printf("You were defeated...\n");
        print_dialogue("game_over_loss");
        set_game_state(STATE_GAME_OVER);
    }

    for(int i=0; i < current_battle.player_party_size; ++i) {
        if(current_battle.player_party[i]) free(current_battle.player_party[i]);
        current_battle.player_party[i] = NULL;
    }
     for(int i=0; i < current_battle.opponent_party_size; ++i) {
        if(current_battle.opponent_party[i]) free(current_battle.opponent_party[i]);
        current_battle.opponent_party[i] = NULL;
    }

    current_battle.player_active_kg = NULL;
    current_battle.opponent_active_kg = NULL;
    game.active_battle = NULL;
}
