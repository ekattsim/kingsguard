#ifndef LOCATIONS_H
#define LOCATIONS_H

// Enum for game locations
typedef enum {
    LOCATION_THRONE_ROOM,
    LOCATION_COURTYARD,
    LOCATION_SUMMONING_HALL,
    LOCATION_COLOSSEUM,
    LOCATION_UNKNOWN // Default/Error state
} Location;

// External declaration for the location names array
extern const char *location_names[];

// Function to print the description of the current location
void look_around(void);

// Function to attempt to go to a specified location
void go_to_location(const char *destination_name);

// Function to get location ID by name
int get_location_id(const char *name);

#endif // LOCATIONS_H
