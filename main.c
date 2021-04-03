#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#define SCALE   24

int running, paused;
Chip8 chip8;
SDL_Window* window;
SDL_Renderer* renderer;

/*
    Initializes SDL, creates a window and a renderer.
    The latter two are stored in the two pointers given as function arguments.
    Returns 1 if successfull, 0 if not.
*/
int initGraphics() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return 0;
    }

    window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * SCALE, HEIGHT * SCALE, 0);
    if (window == NULL) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        return 0;
    }

    SDL_RenderSetScale(renderer, SCALE, SCALE);

    return 1;
}

int scancodeToHexKey(SDL_Scancode sc) {
    switch (sc) {
        case SDL_SCANCODE_1: return 1;
        case SDL_SCANCODE_2: return 2;
        case SDL_SCANCODE_3: return 3;
        case SDL_SCANCODE_4: return 12;
        case SDL_SCANCODE_Q: return 4;
        case SDL_SCANCODE_W: return 5;
        case SDL_SCANCODE_E: return 6;
        case SDL_SCANCODE_R: return 13;
        case SDL_SCANCODE_A: return 7;
        case SDL_SCANCODE_S: return 8;
        case SDL_SCANCODE_D: return 9;
        case SDL_SCANCODE_F: return 14;
        case SDL_SCANCODE_Z: return 10;
        case SDL_SCANCODE_X: return 0;
        case SDL_SCANCODE_C: return 11;
        case SDL_SCANCODE_V: return 15;
        default: return -1;
    }
}

void pollEvents() {
    // Poll events
    SDL_Event event;
    int hexKey = -1; // for key handling
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = 0;
                return;
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_P) {
                    paused = !paused;
                } else {
                    hexKey = scancodeToHexKey(event.key.keysym.scancode);
                    if (hexKey < 0) break;
                    chip8.key[hexKey] = 1;
                }
                break;
            case SDL_KEYUP:
                hexKey = scancodeToHexKey(event.key.keysym.scancode);
                if (hexKey < 0) break;
                chip8.key[hexKey] = 0;
                break;
            default:
                return;
        }
    }
}

void render() {
    // clear the screen (SDL, not CHIP-8)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, NULL);

    // draw all the white pixels
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (chip8.gfx[y * 64 + x]) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }

    // Present the image
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Please provide a file/ROM.\n");
        return 1;
    }

    CHIP8_Initialize(&chip8);

    FILE *f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    int f_len = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *program = (uint8_t*) malloc(f_len * sizeof(uint8_t));
    if (program == NULL) {
        printf("Could not allocate memory for the program.\n");
        return 1;
    }
    int read_size = fread(program, 1, f_len, f);
    fclose(f);

    CHIP8_LoadProgram(&chip8, program, f_len);
    printf("Program was loaded into memory. Size: %d. Loaded: %d.\n", f_len, read_size);

    if (!initGraphics()) {
        return 1;
    }

    // for random number generation
    srand(time(NULL));

    running = 1;

    int updates = 0;
    int frames = 0;
    double delta = 0.0;
    Uint32 secTime = SDL_GetTicks();
    Uint32 quickTime = SDL_GetTicks();
    while (running) {
        if (paused) {
            pollEvents();

            char cmd = 0;
            scanf("%c", &cmd);
            while (getc(stdin) != '\n') {}
            if (cmd == 'r') { //rdump
                CHIP8_RegisterDump(&chip8);
            } else if (cmd == 'm') { // mdupm
                uint16_t start, length;
                printf("Start address (uint): ");
                scanf("%hu", &start);
                while (getc(stdin) != '\n') {}
                printf("Number of bytes (uint): ");
                scanf("%hu", &length);
                while (getc(stdin) != '\n') {}
                CHIP8_MemoryDump(&chip8, start, length);
            } else if (cmd == 'p') { // unpause
                paused = 0;
            }
        } else {
            Uint32 curTime = SDL_GetTicks();
            delta += (curTime - quickTime) / (1000.0 / 60.0); // if delta = 1.0, then 1/60 of a second has passed
            quickTime = curTime;
            if (delta >= 1.0) {
                pollEvents();
                CHIP8_EmulateCycle(&chip8);
                updates++;
                delta--;
            }

            render();
            frames++;

            // print ups and fps
            if (curTime - secTime > 1000) {
                secTime = curTime;
                printf("%d updates, %d fps\n", updates, frames);
                updates = 0;
                frames = 0;
            }
        }
    }

    SDL_Quit();
    return 1;
}
