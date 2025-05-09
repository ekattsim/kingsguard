#include "../inc/kingsguard.h"
#include <stdio.h>
#include <string.h>
#include <strings.h> // For strcasecmp

// --- Define Moves (as const) ---
// These remain as templates for lookups or other potential uses
const Move move_slash = {"Slash", MOVE_TYPE_PHYSICAL, 40, 100, 30, 30};
const Move move_pierce = {"Pierce", MOVE_TYPE_PHYSICAL, 60, 90, 20, 20};
const Move move_fire_arrow = {"Fire_Arrow", MOVE_TYPE_SPECIAL, 70, 85, 15, 15};
const Move move_healing_light = {"Healing_Light", MOVE_TYPE_STATUS, -10, 100, 10, 10};
const Move move_defend = {"Defend", MOVE_TYPE_STATUS, 0, 100, 20, 20};
const Move move_heroic_strike = {"Heroic_Strike", MOVE_TYPE_PHYSICAL, 80, 75, 10, 10};
const Move move_arrow_volly = {"Arrow_Volly", MOVE_TYPE_SPECIAL, 40, 100, 15, 15};
const Move move_empty = {"", MOVE_TYPE_STATUS, 0, 0, 0, 0};


// --- Define Kingsguard Instances (as static) ---
Kingsguard kingsguard_saber = {
    "Saber", 120, 120, 100, 80, 50, 70, 90,
    { // moves
        {"Slash", MOVE_TYPE_PHYSICAL, 40, 100, 30, 30},
        {"Heroic_Strike", MOVE_TYPE_PHYSICAL, 80, 75, 10, 10},
        {"Defend", MOVE_TYPE_STATUS, 0, 100, 20, 20},
        {"", MOVE_TYPE_STATUS, 0, 0, 0, 0} // empty
    }, 3
};

Kingsguard kingsguard_archer = {
    "Archer", 80, 80, 60, 50, 110, 60, 100,
    { // moves
        {"Slash", MOVE_TYPE_PHYSICAL, 40, 100, 30, 30},
        {"Fire_Arrow", MOVE_TYPE_SPECIAL, 70, 85, 15, 15},
        {"Arrow_Volly", MOVE_TYPE_SPECIAL, 40, 100, 15, 15},
        {"Healing_Light", MOVE_TYPE_STATUS, -10, 100, 10, 10}
    }, 4
};

Kingsguard kingsguard_lancer = {
    "Lancer", 100, 100, 90, 70, 70, 50, 110,
    { // moves
        {"Pierce", MOVE_TYPE_PHYSICAL, 60, 90, 20, 20},
        {"Slash", MOVE_TYPE_PHYSICAL, 40, 100, 30, 30},
        {"Defend", MOVE_TYPE_STATUS, 0, 100, 20, 20},
        {"", MOVE_TYPE_STATUS, 0, 0, 0, 0} // empty
    }, 3
};

Kingsguard opponent_heir_guard = {
    "Heir_Guard", 70, 70, 70, 60, 60, 50, 70,
    { // moves
        {"Slash", MOVE_TYPE_PHYSICAL, 40, 100, 30, 30},
        {"Pierce", MOVE_TYPE_PHYSICAL, 60, 90, 20, 20},
        {"", MOVE_TYPE_STATUS, 0, 0, 0, 0}, // empty
        {"", MOVE_TYPE_STATUS, 0, 0, 0, 0}  // empty
    }, 2
};

Kingsguard opponent_enemy_guard_1 = {
    "Alacrya_Guard_1", 110, 110, 90, 70, 60, 60, 80,
    { // moves
        {"Slash", MOVE_TYPE_PHYSICAL, 40, 100, 30, 30},
        {"Pierce", MOVE_TYPE_PHYSICAL, 60, 90, 20, 20},
        {"Defend", MOVE_TYPE_STATUS, 0, 100, 20, 20},
        {"", MOVE_TYPE_STATUS, 0, 0, 0, 0} // empty
    }, 3
};

Kingsguard opponent_enemy_guard_2 = {
    "Alacrya_Guard_2", 90, 90, 70, 80, 90, 80, 70,
    { // moves
        {"Fire_Arrow", MOVE_TYPE_SPECIAL, 70, 85, 15, 15},
        {"Arrow_Volly", MOVE_TYPE_SPECIAL, 40, 100, 15, 15},
        {"Healing_Light", MOVE_TYPE_STATUS, -10, 100, 10, 10},
        {"", MOVE_TYPE_STATUS, 0, 0, 0, 0} // empty
    }, 3
};

Kingsguard opponent_enemy_guard_3 = {
    "Alacrya_Guard_3", 130, 130, 80, 100, 50, 90, 60,
    { // moves
        {"Slash", MOVE_TYPE_PHYSICAL, 40, 100, 30, 30},
        {"Pierce", MOVE_TYPE_PHYSICAL, 60, 90, 20, 20},
        {"Defend", MOVE_TYPE_STATUS, 0, 100, 20, 20},
        {"Healing_Light", MOVE_TYPE_STATUS, -10, 100, 10, 10}
    }, 4
};


// Array of pointers to all Kingsguard templates for lookup
static Kingsguard* all_kingsguard_templates[] = {
    &kingsguard_saber,
    &kingsguard_archer,
    &kingsguard_lancer,
    &opponent_heir_guard,
    &opponent_enemy_guard_1,
    &opponent_enemy_guard_2,
    &opponent_enemy_guard_3
};
static int num_all_kingsguard_templates = sizeof(all_kingsguard_templates) / sizeof(all_kingsguard_templates[0]);

// Array of pointers to all Move templates for lookup
static const Move* all_move_templates[] = {
    &move_slash,
    &move_pierce,
    &move_fire_arrow,
    &move_healing_light,
    &move_defend,
    &move_heroic_strike,
    &move_arrow_volly,
    &move_empty
};
static int num_all_move_templates = sizeof(all_move_templates) / sizeof(all_move_templates[0]);


// --- Function Implementations ---

Kingsguard* get_kingsguard_by_name(const char *name)
{
    for (int i = 0; i < num_all_kingsguard_templates; i++) {
        if (strcasecmp(all_kingsguard_templates[i]->name, name) == 0) {
            return all_kingsguard_templates[i];
        }
    }
    return NULL;
}

const Move* get_move_by_name(const char *name)
{
    for (int i = 0; i < num_all_move_templates; i++) {
        // Ensure we don't try to compare a NULL name from an incompletely defined move
        if (all_move_templates[i]->name && strcasecmp(all_move_templates[i]->name, name) == 0) {
            return all_move_templates[i];
        }
    }
    return NULL;
}


void print_kingsguard_stats(const Kingsguard *kg)
{
    if (kg == NULL) return;
    printf("%s: HP: %d/%d | Att: %d | Def: %d | SpAtt: %d | SpDef: %d | Speed: %d\n",
           kg->name, kg->current_hp, kg->max_hp, kg->attack, kg->defense, kg->sp_attack, kg->sp_defense, kg->speed);
    printf("  Moves: ");
    for (int i = 0; i < kg->num_moves; i++) {
        // Check if move name is not NULL or empty before printing, for safety with "empty" moves
        if (kg->moves[i].name && strlen(kg->moves[i].name) > 0) {
            printf("%s (PP: %d/%d) ", kg->moves[i].name, kg->moves[i].pp, kg->moves[i].max_pp);
        }
    }
    printf("\n");
}

bool is_conscious(const Kingsguard *kg)
{
    if (kg == NULL) return false;
    return kg->current_hp > 0;
}
