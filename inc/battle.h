#ifndef BATTLE_H
#define BATTLE_H

#include <stdbool.h>
#include "../inc/kingsguard.h" // Need Kingsguard struct

// Enum for battle actions
typedef enum {
    BATTLE_ACTION_ATTACK,
    BATTLE_ACTION_SWITCH,
    BATTLE_ACTION_RUN // Maybe not allowed in duels?
} BattleAction;

// Structure to hold the state of an active battle
typedef struct BattleState {
    // Player's side
    Kingsguard *player_party[3]; // Pointers to Kingsguard instances in player's party
    int player_party_size;
    Kingsguard *player_active_kg; // Pointer to the currently active Kingsguard
    int player_active_index; // Index in player_party

    // Opponent's side
    Kingsguard *opponent_party[3]; // Pointers to Kingsguard instances in opponent's party
    int opponent_party_size;
    Kingsguard *opponent_active_kg; // Pointer to the currently active Kingsguard
    int opponent_active_index; // Index in opponent_party

    // Current turn data
    BattleAction player_action;
    const char *player_action_param; // Move name or KG name
    BattleAction opponent_action;
    const char *opponent_action_param; // Move name or KG name (AI chooses this)

    // Battle status
    bool is_active;
    bool player_won; // Result of the battle
    // Add turn counter, status effects, etc. later
} BattleState;


// Function to start a new battle
void start_battle(Kingsguard *opponent_kingsguard[], int num_opponents);

// Function to handle player's command during battle
// Returns true if a valid action was chosen, false otherwise (e.g., bad command, fainted KG)
bool battle_handle_player_action(BattleAction action, const char *param);

// Function to process a full turn of the battle (player + opponent)
// Called automatically after player_handle_action returns true
void battle_process_turn(void);

// Function to check if the battle is over
bool is_battle_over(void);

// Function to clean up battle state
void end_battle(void);

#endif // BATTLE_H
