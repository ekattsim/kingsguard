#include "../inc/puzzle.h"
#include "../inc/game_state.h"
#include "../inc/dialogue.h"
#include "../inc/kingsguard.h"
#include "../inc/locations.h"

#include <stdio.h>
#include <strings.h>

// --- Puzzle State ---
static bool puzzle_is_active = false;
static const char *correct_answer = "LOYALTY";

void start_saber_puzzle(void)
{
    if (game.plot_flags.saber_recruited) {
        printf("Saber has already joined your Kingsguard.\n");
        if (get_game_state() == STATE_PUZZLE) set_game_state(game.current_location);
        return;
    }
    if (!game.plot_flags.has_summoning_artifact) {
         printf("You don't have the Summoning Artifact needed to perform the ritual.\n");
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
         if (get_game_state() == STATE_PUZZLE) set_game_state(game.current_location);
        return false;
    }

    if (strcasecmp(answer, correct_answer) == 0) {
        print_dialogue("saber_puzzle_correct");

        game.player_kingsguard.has_saber = true;
        game.player_kingsguard.num_kingsguard++;
        game.plot_flags.saber_recruited = true;

        puzzle_is_active = false;
        set_game_state(LOCATION_SUMMONING_HALL);
        game.current_location = LOCATION_SUMMONING_HALL;

        printf("The path to the Colosseum is now open from the Throne Room.\n");

        return true;
    } else {
        print_dialogue("saber_puzzle_wrong");
        return false;
    }
}
