#ifndef KINGSGUARD_H
#define KINGSGUARD_H

#include <stdbool.h>

// Forward declaration
struct Kingsguard;
struct BattleState; // Need this for move effects potentially

// --- Move Data ---
typedef enum {
    MOVE_TYPE_PHYSICAL,
    MOVE_TYPE_SPECIAL, // Energy/Magic attacks
    MOVE_TYPE_STATUS   // Buffs, Debuffs, Heals
} MoveType;

typedef struct {
    const char *name;
    MoveType type;
    int power;    // Damage/Heal amount (0 for status, negative for healing)
    int accuracy; // 0-100
    int pp;       // Power points (how many times it can be used)
    int max_pp;   // Store max PP separately
    // void (*effect)(struct Kingsguard *user, struct Kingsguard *target, struct BattleState *battle); // Pointer to a function for complex effects (optional for v1)
} Move;

// --- Kingsguard Data ---
typedef struct Kingsguard {
    const char *name;
    int max_hp;
    int current_hp;
    int attack;  // Physical attack stat
    int defense; // Physical defense stat
    int sp_attack; // Special attack stat
    int sp_defense; // Special defense stat
    int speed;   // Determines turn order

    // Moves (copies of Move structs)
    Move moves[4];
    int num_moves;

    // Status effects (optional for v1)
    // bool is_poisoned;
    // bool is_stunned;
} Kingsguard;

// --- Move Definitions (Extern declarations) ---
// Declare the const instances defined in kingsguard.c
extern const Move move_slash;
extern const Move move_pierce;
extern const Move move_fire_arrow;
extern const Move move_healing_light;
extern const Move move_defend;
extern const Move move_heroic_strike;
extern const Move move_arrow_volly;
extern const Move move_empty;


// --- Kingsguard Definitions (Extern declarations) ---
// Declare the instances defined in kingsguard.c
extern Kingsguard kingsguard_saber;
extern Kingsguard kingsguard_archer;
extern Kingsguard kingsguard_lancer;

// Example Opponent Kingsguard
extern Kingsguard opponent_heir_guard;
extern Kingsguard opponent_enemy_guard_1;
extern Kingsguard opponent_enemy_guard_2;
extern Kingsguard opponent_enemy_guard_3;


// --- Functions ---
// Function to get Kingsguard data by name (returns pointer to template)
Kingsguard* get_kingsguard_by_name(const char *name); // Note: Returns pointer to static data

// Function to get Move data by name (returns pointer to template)
const Move* get_move_by_name(const char *name); // Note: Returns pointer to static data

// Function to print Kingsguard stats
void print_kingsguard_stats(const Kingsguard *kg);

// Function to check if a Kingsguard is conscious
bool is_conscious(const Kingsguard *kg);


#endif // KINGSGUARD_H
