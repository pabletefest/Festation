#include <stdio.h>

#include <SDL2/SDL.h>

#include "psx_system.hpp"

/* Sets constants */
#define WIDTH 800
#define HEIGHT 600
#define DELAY 3000

namespace festation
{
    PSXSystem psxSystem;

    static constexpr const char* EMU_TITLE = "Festation (PSX Emulator)";
};

int main(int, char**)
{
    printf("Hello, from Festation!\n");

    /* Initialises data */
    SDL_Window *window = NULL;
    
    /*
    * Initialises the SDL video subsystem (as well as the events subsystem).
    * Returns 0 on success or a negative error code on failure using SDL_GetError().
    */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
        return 1;
    }

    /* Creates a SDL window */
    window = SDL_CreateWindow(festation::EMU_TITLE, /* Title of the SDL window */
                    SDL_WINDOWPOS_UNDEFINED, /* Position x of the window */
                    SDL_WINDOWPOS_UNDEFINED, /* Position y of the window */
                    WIDTH, /* Width of the window in pixels */
                    HEIGHT, /* Height of the window in pixels */
                    0); /* Additional flag(s) */

    /* Checks if window has been created; if not, exits program */
    if (window == NULL) {
        fprintf(stderr, "SDL window failed to initialise: %s\n", SDL_GetError());
        return 1;
    }

    /* Pauses all SDL subsystems for a variable amount of milliseconds */
    SDL_Delay(DELAY);

    /* Frees memory */
    SDL_DestroyWindow(window);
    
    /* Shuts down all SDL subsystems */
    SDL_Quit(); 
    
    return 0;
}
