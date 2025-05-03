#ifndef PUZZLE_H
#define PUZZLE_H

#include <stdbool.h>

// Function to start the Saber summoning puzzle
void start_saber_puzzle(void);

// Function to check the player's answer to the puzzle
// Returns true if correct, false otherwise
bool check_puzzle_answer(const char *answer);

#endif // PUZZLE_H
