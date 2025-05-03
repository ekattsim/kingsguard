#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include <stdlib.h> // For NULL

// Enum for different game states
typedef enum {
    STATE_START_SCREEN, // Not really used in the loop, but good to define
    STATE_THRONE_ROOM,
    STATE_COURTYARD,
    STATE_SUMMONING_HALL,
    STATE_COLOSSEUM,
    STATE_BATTLE,       // In a battle
    STATE_PUZZLE,       // Solving a puzzle (specifically Saber's)
    STATE_GAME_OVER,    // Game has ended (loss)
    STATE_VICTORY       // Game has ended (win)
} GameState;

// Structure to hold player's Kingsguard status
typedef struct {
    // We'll add actual Kingsguard pointers/indices here later
    int num_kingsguard; // Number of KG player has (max 3)
    // Add flags for acquired Kingsguard (Saber, Archer, Lancer)
    bool has_archer;
    bool has_lancer;
    bool has_saber;
    // Add pointers to the actual Kingsguard data if needed
} PlayerKingsguard;

// Structure to hold plot progression flags
typedef struct {
    bool met_initial_kingsguard;
    bool heir_battle_fought;
    bool has_summoning_artifact; // Moved from main GameState_t for organization
    bool saber_recruited; // Flag for completing the puzzle and recruiting Saber
    bool final_battle_intro_shown;
} PlotFlags;


// Structure to hold overall game state
typedef struct {
    GameState current_state;
    int current_location; // Use an enum or int to represent location

    PlayerKingsguard player_kingsguard; // Player's KG ownership

    PlotFlags plot_flags; // Plot progression flags

    // Add battle state pointer if a battle is active
    void *active_battle; // Pointer to a struct BattleState (defined in battle.h)
} GameState_t;

// Global game state instance
extern GameState_t game;

// Function to initialize the game state
void initialize_game_state(void);

// Function to set the current game state
void set_game_state(GameState new_state);

// Function to get the current game state
GameState get_game_state(void);

// Function to get the current location ID
int get_current_location(void);

// Function to attempt to move the player to a new location
void player_go_to_location(int destination_id);


#endif // GAME_STATE_H
