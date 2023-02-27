#include <SDL2/SDL.h>

#include <iostream>

int main (int argc, char** argv) {
    int error = SDL_Init(SDL_INIT_EVERYTHING);

    if (error) {
        std::cout << "SDL Init error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        640,                               // width, in pixels
        480,                               // height, in pixels
        0                  // flags - see below
    );

    std::cout << "SDL Initialized" << std::endl;

    bool keep_running = true;
    SDL_Event event;

    while (keep_running) {
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    keep_running = false;
                    break;

                case SDL_KEYDOWN:
                    std::cout << "keydown: " << event.key.keysym.sym << std::endl;
                    break;

                    case SDL_KEYUP:
                    std::cout << "keyup: " <<  event.key.keysym.sym << std::endl;
                    break;
            }

            SDL_Delay(1);
        }
    }

    SDL_Quit();

    return 0;
}