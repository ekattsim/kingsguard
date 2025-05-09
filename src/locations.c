#include "../inc/locations.h"
#include "../inc/game_state.h"
#include "../inc/dialogue.h"
#include "../inc/battle.h"
#include "../inc/kingsguard.h"
#include "../inc/puzzle.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>

// Array of location names (must match enum order)
const char *location_names[] = {
    "Throne_Room",
    "Courtyard",
    "Summoning_Hall",
    "Colosseum",
    "Unknown_Location" // Corresponds to LOCATION_UNKNOWN
};

// Simple function to get location ID by case-insensitive name
int get_location_id(const char *name) {
    for (int i = 0; i <= LOCATION_UNKNOWN; ++i) {
        if (strcasecmp(location_names[i], name) == 0) {
            return i;
        }
    }
    return LOCATION_UNKNOWN;
}


void look_around(void)
{
    int current_loc_id = get_current_location();

    switch (current_loc_id) {
        case LOCATION_THRONE_ROOM:
            print_dialogue("look_throne_room");
            // Optional: Trigger meeting KG here if not met? Or rely on 'talk'?
            // Let's keep the 'talk' trigger in parse_exec for meet_kingsguard.
            break;
        case LOCATION_COURTYARD:
            print_dialogue("look_courtyard");
            // Check if heir battle needs to be triggered (only once upon first arrival after meeting KG)
            // Trigger this immediately upon entering the courtyard for the first time IF meet_kingsguard is true
            if (!game.plot_flags.heir_battle_fought && game.plot_flags.met_initial_kingsguard) {
                 print_dialogue("heir_ambush");
                 Kingsguard *opponents[] = {get_kingsguard_by_name("Heir_Guard")};
                 start_battle(opponents, 1);
                 // Note: start_battle changes game state to STATE_BATTLE
                 game.plot_flags.heir_battle_fought = true;
            }
            break;
        case LOCATION_SUMMONING_HALL:
             print_dialogue("look_summoning_hall");
             // Check if Saber puzzle needs to be triggered (only once if Saber not recruited)
             if (!game.plot_flags.saber_recruited && game.plot_flags.has_summoning_artifact) {
                  print_dialogue("saber_puzzle_intro");
                  start_saber_puzzle();
             }
            break;
        case LOCATION_COLOSSEUM:
            print_dialogue("look_colosseum");
             // Check if final battle needs to be triggered (only once upon first arrival)
             if (!game.plot_flags.final_battle_intro_shown && game.plot_flags.saber_recruited) {
                 print_dialogue("final_battle_intro");
                 Kingsguard *opponents[] = {
                    get_kingsguard_by_name("Alacrya_Guard_1"),
                    get_kingsguard_by_name("Alacrya_Guard_2"),
                    get_kingsguard_by_name("Alacrya_Guard_3")
                 };
                 start_battle(opponents, 3);
                 game.plot_flags.final_battle_intro_shown = true;
                 // Note: start_battle changes game state to STATE_BATTLE
             }
            break;
        default:
            printf("You are in an unknown location.\n");
            break;
    }
}
