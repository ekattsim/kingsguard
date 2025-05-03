#include "../inc/dialogue.h"
#include <stdio.h>
#include <string.h>
#include <strings.h> // For strcasecmp

// --- Dialogue Storage ---
// Use keys to identify dialogue blocks. Could be simple strings or more complex structure.
// For now, a simple hardcoded list.

typedef struct {
    const char *key;
    const char *text;
} DialogueBlock;

static const DialogueBlock dialogues[] = {
    // Story - Arrival
    {"arrival",
     "The last thing you remember is... something else.\n"
     "Now, the scent of aged stone and ancient magic fills your lungs.\n"
     "Robed figures bow before you, calling you King.\n"
     "You are the Outsider King, pulled from another world by prophecy to wear the crown of Dicathen.\n\n"
     "A bureaucrat steps forward, clearing his throat.\n"
     "\"Your Majesty, we are most relieved by your arrival. The Kingdom of Alacrya prepares for war over The Holy Grail.\"\n"
     "He presents a magnificent, but heavy, crown.\n"
     "\"Custom dictates this conflict will be settled not by legions, but by a duel between kings, fought by their chosen Kingsguard.\"\n"
     "\"You must lead your Kingsguard strategically. Forge them. Master the duel. Save your kingdom.\"\n"
     "\"Your past life is gone. Your new one is on the line.\""
    },
    // Story - Meeting Existing Kingsguard (Archer & Lancer) - Triggered in Throne Room
     {"meet_kingsguard",
      "Two figures step forward, kneeling before the throne.\n"
      "\"Your Majesty,\" the first says, rising smoothly. He wears green and silver, a longbow strapped to his back. \"I am Archer. My arrows are yours to command.\"\n"
      "The second, clad in blue and silver with a long spear, rises with a confident air. \"And I am Lancer. My spear is the swift hand of justice.\"\n"
      "\"The previous king summoned us,\" Archer explains. \"He sought Saber, the third Kingsguard, but... he passed before the ritual was complete.\"\n"
      "\"The heir has the artifact needed for Saber's summoning,\" Lancer adds, a hint of disdain in his voice. \"He guards it jealously.\"\n"
      "You learn about their techniques: Archer's precision shots (high Special Attack, maybe ranged effects), Lancer's swift, piercing attacks (high Speed, Physical Attack, maybe criticals). You understand the core of the duels: three knights, turn-based combat, strategic switching and move usage."
     },
     // Location Descriptions
     {"look_throne_room",
      "You are in the grand Throne Room of the castle. Light streams through high windows, illuminating ancient columns. The air smells of stone and history.\n"
      "The bureaucrats are still here, observing you. Archer and Lancer stand respectfully to the side."
     },
     {"look_courtyard",
      "You are in the Castle Courtyard. The open sky is above, and training dummies stand in a corner. It feels exposed here.\n"
      "The stone is worn from centuries of footsteps."
     },
     {"look_summoning_hall",
      "This hall feels charged with latent energy. Runes cover the walls and floor, converging on a central pedestal. This is where the ritual must be performed.\n"
      "Dust motes dance in the single shaft of light from a high window."
     },
     {"look_colosseum",
      "You stand at the entrance to the great Colosseum. The roar of an unseen crowd echoes from within. Sand fills the arena floor, marked by countless past duels.\n"
      "Across the field, you see the enemy king's retinue."
     },
    // Plot - Heir Ambush
     {"heir_ambush",
      "As you step fully into the Courtyard, a figure blocks your path!\n"
      "It's the previous heir, flanked by a formidable knight.\n"
      "\"So, the Outsider King dares to step into my domain?\" he sneers. \"You may sit on the throne, but you won't claim the Holy Grail!\"\n"
      "\"That summoning artifact is mine! You won't complete your Kingsguard.\"\n"
      "\"Prove your worth here, in the duel!\""
     },
    // Plot - Heir Battle Win (Triggers artifact acquisition)
     {"heir_battle_win",
      "The Heir's Kingsguard falls, defeated. The Heir stares in disbelief.\n"
      "\"Impossible... My champion defeated?\"\n"
      "He clutches a small, ornate box. With a frustrated growl, he throws it at your feet.\n"
      "\"Take it! A pawn like you won't know its true power anyway!\"\n"
      "He storms off, defeated but defiant.\n"
      "You pick up the box. It contains the Summoning Artifact."
     },
    // Plot - Saber Puzzle Intro
     {"saber_puzzle_intro",
      "You stand before the pedestal in the Summoning Hall. The artifact hums in your hand.\n"
      "As you place it on the pedestal, the runes on the floor glow brightly.\n"
      "A spectral figure begins to form, solidifying into the imposing presence of a knight.\n"
      "\"I am Saber,\" the knight's voice resonates, deep and unwavering. \"I seek a master of true worth.\"\n"
      "\"The artifact calls, but power alone is not enough. Answer this riddle, King. Prove your understanding of what I value above all else.\"\n\n"
      "Saber presents the riddle: \"The crown weighs heavy, battles lie ahead. But without my anchor, victory is dead. I bind the knight, I test the king's heart. What is the virtue that sets us apart?\""
     },
    // Plot - Saber Puzzle Wrong Answer
     {"saber_puzzle_wrong",
      "\"Incorrect,\" Saber states, his form flickering slightly. \"That is not the virtue I seek in a master. Consider carefully.\" "
     },
     // Plot - Saber Puzzle Correct Answer
     {"saber_puzzle_correct",
      "\"...Loyalty,\" Saber says, a flicker of approval in his gaze. \"Precisely. A knight without loyalty is merely a sellsword. A king without loyalty to his people is a tyrant.\"\n"
      "His form solidifies completely, kneeling before you.\n"
      "\"You understand the bond, King. I am Saber, the Blade of Loyalty. My sword and my service are yours.\"\n"
      "Saber stands and joins your Kingsguard. Your team is complete!"
     },
    // Plot - Final Battle Intro
     {"final_battle_intro",
      "The announcer's voice booms through the Colosseum, distorted by magic.\n"
      "\"Introducing! The Outsider King of Dicathen! And his Kingsguard!\"\n"
      "You step into the arena sand. Across from you stands the King of Alacrya, an arrogant smile on his face.\n"
      "He gestures to his three imposing champions.\n"
      "\"Behold my Kingsguard, Usurper,\" he laughs. \"Forged in Alacrya's fires, unlike your hastily assembled toys. You think a foreign 'prophecy' can defeat true power?\"\n"
      "\"The Grail is mine. As is your fate! Let the duel... BEGIN!\""
     },
    // Game Over
     {"game_over_loss",
      "\nYour final Kingsguard falls. The King of Alacrya laughs triumphantly.\n"
      "\"Pathetic! Your so-called Kingsguard are nothing before mine!\"\n"
      "The Holy Grail is lost. Your kingdom falls. Your new life... ends here.\n"
      "--- GAME OVER ---"
     },
    // Victory
     {"game_victory",
      "\nThe final Alacryan Kingsguard falls. Silence descends upon the Colosseum.\n"
      "Then, a roar erupts from the Dicathen crowd!\n"
      "The King of Alacrya stares, speechless, his face pale.\n"
      "You have won the duel. The Holy Grail is safe, and your kingdom is saved.\n"
      "You, the Outsider King, have proven your worth.\n"
      "--- VICTORY! ---"
     }

    // Add more dialogue blocks as the game grows
};

static int num_dialogues = sizeof(dialogues) / sizeof(dialogues[0]);

void print_dialogue(const char *key)
{
    for (int i = 0; i < num_dialogues; i++) {
        if (strcasecmp(dialogues[i].key, key) == 0) {
            printf("%s\n", dialogues[i].text);
            return;
        }
    }
    printf("[Dialogue key '%s' not found]\n", key); // Debug message if key is missing
}
