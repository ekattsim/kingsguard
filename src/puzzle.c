#include "../inc/puzzle.h"
#include "../inc/game_state.h" // Need game state and plot flags
#include "../inc/dialogue.h"
#include "../inc/kingsguard.h" // To add Saber to the party
#include "../inc/locations.h" // Need LOCATION_SUMMONING_HALL enum

#include <stdio.h>
#include <strings.h> // For strcasecmp

// --- Puzzle State ---
// A simple state to track if the puzzle is active
static bool puzzle_is_active = false;
// The correct answer (case-insensitive)
static const char *correct_answer = "LOYALTY";

void start_saber_puzzle(void)
{
    // Trigger puzzle only if not already recruited and has artifact
    if (game.plot_flags.saber_recruited) {
        printf("Saber has already joined your Kingsguard.\n");
        // Return to exploration state if somehow re-triggered while in puzzle state
        if (get_game_state() == STATE_PUZZLE) set_game_state(game.current_location);
        return;
    }
    if (!game.plot_flags.has_summoning_artifact) {
         printf("You don't have the Summoning Artifact needed to perform the ritual.\n");
         // Return to exploration state
         if (get_game_state() == STATE_PUZZLE) set_game_state(game.current_location);
         return;
    }

    puzzle_is_active = true;
    set_game_state(STATE_PUZZLE);
    // The intro dialogue ("saber_puzzle_intro") is printed by locations.c look_around when arriving
    // and the puzzle is triggered there.
}

bool check_puzzle_answer(const char *answer)
{
    if (!puzzle_is_active) {
        printf("You are not currently working on a puzzle.\n");
        // Return to exploration state if somehow re-triggered
         if (get_game_state() == STATE_PUZZLE) set_game_state(game.current_location);
        return false;
    }

    if (strcasecmp(answer, correct_answer) == 0) {
        print_dialogue("saber_puzzle_correct");

        // Add Saber to player's Kingsguard state flags
        game.player_kingsguard.has_saber = true;
        game.player_kingsguard.num_kingsguard++;
        game.plot_flags.saber_recruited = true; // Set the flag!

        puzzle_is_active = false;
        // Set game state back to the location state where the puzzle happened
        set_game_state(LOCATION_SUMMONING_HALL);
        game.current_location = LOCATION_SUMMONING_HALL; // Ensure location is set

        // Announce the new path opening
        printf("The path to the Colosseum is now open from the Throne Room.\n");

        return true;
    } else {
        print_dialogue("saber_puzzle_wrong");
        return false;
    }
}
