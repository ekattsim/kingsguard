#include "../inc/game_state.h"
#include "../inc/locations.h" // Need location definitions and look_around
#include <stdlib.h> // For NULL
#include <stdio.h> // For printf

// Define the global game state instance
GameState_t game;

void initialize_game_state(void)
{
    game.current_state = STATE_THRONE_ROOM; // Start in the throne room
    game.current_location = LOCATION_THRONE_ROOM; // Set starting location

    // Initialize Player Kingsguard ownership
    game.player_kingsguard.num_kingsguard = 0;
    game.player_kingsguard.has_archer = false;
    game.player_kingsguard.has_lancer = false;
    game.player_kingsguard.has_saber = false;

    // Initialize Plot Flags
    game.plot_flags.met_initial_kingsguard = false; // Meet them *after* arrival dialogue
    game.plot_flags.heir_battle_fought = false;
    game.plot_flags.has_summoning_artifact = false;
    game.plot_flags.saber_recruited = false;
    game.plot_flags.final_battle_intro_shown = false;


    game.active_battle = NULL; // No active battle initially

    // Story initialization: Player starts with Archer and Lancer *owned*
    // The 'meet' happens shortly after arrival.
    game.player_kingsguard.has_archer = true;
    game.player_kingsguard.has_lancer = true;
    game.player_kingsguard.num_kingsguard = 2; // Archer and Lancer start with you

    // The actual instances used in battle are copied from templates.
    // The flags just indicate *which* templates are available to the player.
}

void set_game_state(GameState new_state)
{
    // Optional: Print a message or trigger actions based on state change
    // printf("--- Game State changed from %d to %d ---\n", game.current_state, new_state); // Debugging
    game.current_state = new_state;
}

GameState get_game_state(void)
{
    return game.current_state;
}

int get_current_location(void)
{
    return game.current_location;
}

// New function to handle player movement between locations
void player_go_to_location(int destination_id)
{
    if (destination_id == LOCATION_UNKNOWN) {
        printf("You don't know how to get there.\n"); // Should be caught by caller
        return;
    }

    // Check if currently in a state that prevents movement
    GameState current_state = get_game_state();
    if (current_state == STATE_BATTLE || current_state == STATE_PUZZLE ||
        current_state == STATE_GAME_OVER || current_state == STATE_VICTORY)
    {
        printf("You can't go anywhere right now.\n");
        return;
    }

    int current_loc_id = get_current_location();
    bool can_go = false;

    // --- Define valid transitions based on plot flags ---
    switch (current_loc_id) {
        case LOCATION_THRONE_ROOM:
            if (destination_id == LOCATION_COURTYARD) can_go = true;
            // Can go to Summoning Hall after getting the artifact (after Heir battle)
            if (destination_id == LOCATION_SUMMONING_HALL && game.plot_flags.has_summoning_artifact) can_go = true;
            // Can go to Colosseum after recruiting Saber
            if (destination_id == LOCATION_COLOSSEUM && game.plot_flags.saber_recruited) can_go = true;
            break;
        case LOCATION_COURTYARD:
            if (destination_id == LOCATION_THRONE_ROOM) can_go = true;
            break;
        case LOCATION_SUMMONING_HALL:
            if (destination_id == LOCATION_THRONE_ROOM) can_go = true;
            break;
        case LOCATION_COLOSSEUM:
            // Can't leave Colosseum once arrived.
            printf("You are at the colosseum, preparing for the final duel. There is no turning back.\n");
            return; // Prevent movement
        default:
            break; // Should not happen
    }

    if (can_go) {
        game.current_location = destination_id;
        // We stay in the exploration state (e.g. STATE_THRONE_ROOM -> STATE_COURTYARD is a location change within the same state type)
        // state doesn't necessarily change just by moving UNLESS the new location triggers an event.
        printf("You go to the %s.\n", location_names[destination_id]);
        look_around(); // Print description and potentially trigger events upon arrival
    } else {
        printf("You cannot go to the %s from here.\n", location_names[destination_id]);
    }
}
