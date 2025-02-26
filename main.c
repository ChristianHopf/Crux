#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <SDL2/SDL.h>

int main(int argc, char *argv[]){
  // SDL Init
  if (SDL_Init(SDL_INIT_VIDEO) < 0){
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }

  // Create a window
  SDL_Window *window = SDL_CreateWindow(
    "Engine",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    640,
    480,
    SDL_WINDOW_SHOWN
  );
  if (!window){
    SDL_Log("Failed to create window: %s", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  // Create a renderer
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer){
    SDL_Log("Failed to create renderer: %s", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  int running = 1;
  while(running){
    SDL_Event event;
    while (SDL_PollEvent(&event)){
      switch (event.type){
        case SDL_QUIT:
          running = 0;
          break;
        case SDL_KEYDOWN:
          if (event.key.keysym.sym == SDLK_ESCAPE){
            running = 0;
          }
          break;
      }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
