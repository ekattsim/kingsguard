#include "../inc/kingsguard.h"
#include <stdio.h>
#include <string.h>
#include <strings.h> // For strcasecmp

// --- Define Moves (as const) ---
// Added max_pp
const Move move_slash = {"Slash", MOVE_TYPE_PHYSICAL, 40, 100, 30, 30};
const Move move_pierce = {"Pierce", MOVE_TYPE_PHYSICAL, 60, 90, 20, 20};
const Move move_fire_arrow = {"Fire_Arrow", MOVE_TYPE_SPECIAL, 70, 85, 15, 15};
const Move move_healing_light = {"Healing_Light", MOVE_TYPE_STATUS, -30, 100, 10, 10};
const Move move_defend = {"Defend", MOVE_TYPE_STATUS, 0, 100, 20, 20};
const Move move_heroic_strike = {"Heroic_Strike", MOVE_TYPE_PHYSICAL, 80, 75, 10, 10};
const Move move_arrow_volly = {"Arrow_Volly", MOVE_TYPE_SPECIAL, 40, 100, 15, 15};

// Placeholder empty move
const Move move_empty = {"", MOVE_TYPE_STATUS, 0, 0, 0, 0};


// --- Define Kingsguard Instances (as static) ---
// Initialize moves array directly with const move structs
Kingsguard kingsguard_saber = {
    "Saber", 120, 120, 100, 80, 50, 70, 90,
    {move_slash, move_heroic_strike, move_defend, move_empty}, 3
};

Kingsguard kingsguard_archer = {
    "Archer", 80, 80, 60, 50, 110, 60, 100,
    {move_slash, move_fire_arrow, move_arrow_volly, move_healing_light}, 4
};

Kingsguard kingsguard_lancer = {
    "Lancer", 100, 100, 90, 70, 70, 50, 110,
    {move_pierce, move_slash, move_defend, move_empty}, 3
};

// Example Opponent Kingsguard Instances
Kingsguard opponent_heir_guard = {
    "Heir_Guard", 70, 70, 70, 60, 60, 50, 70,
    {move_slash, move_pierce, move_empty, move_empty}, 2
};

Kingsguard opponent_enemy_guard_1 = {
    "Alacrya_Guard_1", 110, 110, 90, 70, 60, 60, 80,
    {move_slash, move_pierce, move_defend, move_empty}, 3
};

Kingsguard opponent_enemy_guard_2 = {
    "Alacrya_Guard_2", 90, 90, 70, 80, 90, 80, 70,
    {move_fire_arrow, move_arrow_volly, move_healing_light, move_empty}, 3
};

Kingsguard opponent_enemy_guard_3 = {
    "Alacrya_Guard_3", 130, 130, 80, 100, 50, 90, 60,
    {move_slash, move_pierce, move_defend, move_healing_light}, 4
};


// Array of pointers to all Kingsguard templates for lookup
// Initialized with addresses of the static Kingsguard instances defined above
static Kingsguard* all_kingsguard_templates[] = {
    &kingsguard_saber,
    &kingsguard_archer,
    &kingsguard_lancer,
    &opponent_heir_guard,
    &opponent_enemy_guard_1,
    &opponent_enemy_guard_2,
    &opponent_enemy_guard_3
    // Add more templates here as needed
};
static int num_all_kingsguard_templates = sizeof(all_kingsguard_templates) / sizeof(all_kingsguard_templates[0]);

// Array of pointers to all Move templates for lookup
// Initialized with addresses of the const static Move instances defined above
static const Move* all_move_templates[] = {
    &move_slash,
    &move_pierce,
    &move_fire_arrow,
    &move_healing_light,
    &move_defend,
    &move_heroic_strike,
    &move_arrow_volly,
    &move_empty
    // Add more templates here
};
static int num_all_move_templates = sizeof(all_move_templates) / sizeof(all_move_templates[0]);


// --- Function Implementations ---

// Get a pointer to Kingsguard template data by name
Kingsguard* get_kingsguard_by_name(const char *name)
{
    for (int i = 0; i < num_all_kingsguard_templates; i++) {
        if (strcasecmp(all_kingsguard_templates[i]->name, name) == 0) {
            return all_kingsguard_templates[i]; // Return pointer to static template
        }
    }
    return NULL; // Not found
}

// Get a pointer to Move template data by name
const Move* get_move_by_name(const char *name)
{
    for (int i = 0; i < num_all_move_templates; i++) {
        if (strcasecmp(all_move_templates[i]->name, name) == 0) {
            return all_move_templates[i]; // Return pointer to static const template
        }
    }
    return NULL; // Not found
}


void print_kingsguard_stats(const Kingsguard *kg)
{
    if (kg == NULL) return;
    printf("%s: HP: %d/%d | Att: %d | Def: %d | SpAtt: %d | SpDef: %d | Speed: %d\n",
           kg->name, kg->current_hp, kg->max_hp, kg->attack, kg->defense, kg->sp_attack, kg->sp_defense, kg->speed);
    printf("  Moves: ");
    for (int i = 0; i < kg->num_moves; i++) {
        printf("%s (PP: %d/%d) ", kg->moves[i].name, kg->moves[i].pp, kg->moves[i].max_pp); // Print current/max PP
    }
    printf("\n");
}

bool is_conscious(const Kingsguard *kg)
{
    if (kg == NULL) return false;
    return kg->current_hp > 0;
}
