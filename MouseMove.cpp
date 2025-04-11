#include <iostream>
#include <cmath>
#include <vector>
#include <SDL3/SDL.h>

#ifndef M_PI
#define M_PI  3.14159265358979323846  /* pi */
#endif

const int WIDTH    = 1000;
const int HEIGHT    = 600;

const int BUFFER_SIZE = WIDTH; // one pixel per sample (scrolling)

// Constants for FPS control
const Uint64 FPS = 60; // Desired FPS
const Uint64 FRAME_DELAY = 1000 / FPS; // Time per frame in milliseconds

const int NUM_SAMPLES = 800;

SDL_Window* window;
SDL_Renderer* renderer;

int CreateWindow(){
    SDL_Init(SDL_INIT_VIDEO);
    if(!SDL_CreateWindowAndRenderer("Oscillosope", WIDTH, HEIGHT, 0 , &window, &renderer)) // SDL_WINDOW_BORDERLESS
    {
        std::cerr << "Window Creation Failed: " << std::endl;
        SDL_Quit();
        return -1;
    }

    return 0;
}

void drawGridToCanvas(SDL_Renderer* renderer, int rows, int cols, int width, int height) {
    float xStep = (float)width / cols;
    float yStep = (float)height / rows;

    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); // Subtle grid color

    // Draw horizontal lines
    for (int i = 1; i < rows; ++i) {
        float y = i * yStep;
        SDL_RenderLine(renderer, 0.0f, y, (float)width, y);
    }

    // Draw vertical lines
    for (int i = 1; i < cols; ++i) {
        float x = i * xStep;
        SDL_RenderLine(renderer, x, 0.0f, x, (float)height);
    }
}

SDL_FPoint previousPoint = { -1, -1 }; // Use (-1,-1) as "no previous point yet"

void drawToCanvas(SDL_Renderer* renderer, SDL_Texture* canvas, SDL_FPoint newPoint) {
    if (previousPoint.x >= 0 && previousPoint.y >= 0) {
        // Set the render target to the canvas
        SDL_SetRenderTarget(renderer, canvas);

        // Draw a line from previousPoint to newPoint
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // green line
        SDL_RenderLine(renderer, previousPoint.x, previousPoint.y, newPoint.x, newPoint.y);

        // Restore render target back to screen
        SDL_SetRenderTarget(renderer, nullptr);
    }

    previousPoint = newPoint;
}

void clearCanvas(SDL_Renderer* renderer, SDL_Texture* canvas) {
    SDL_SetRenderTarget(renderer, canvas);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    // Draw grid
    drawGridToCanvas(renderer, 10, 10, WIDTH, HEIGHT); // 10x10 grid for example

    SDL_SetRenderTarget(renderer, nullptr);

    previousPoint = { -1, -1 }; // Reset previous point to avoid connecting to old drawings
}

void renderFrame(SDL_Renderer* renderer, SDL_Texture* canvas) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // clear screen
    SDL_RenderClear(renderer);

    SDL_RenderTexture(renderer, canvas, NULL, NULL); // draw canvas
    SDL_RenderPresent(renderer);
}

int main()
{
    CreateWindow();

    
    
    SDL_Texture* canvas = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        WIDTH, HEIGHT
    );
    
    // Optional: Clear it once initially
    SDL_SetRenderTarget(renderer, canvas);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black background
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);
    
    
    bool running = true;
    SDL_Event e;
    while(running){

        while(SDL_PollEvent(&e)){
            if(e.type == SDL_EVENT_QUIT) { running = false; }

            if (e.type == SDL_EVENT_MOUSE_MOTION ) {
                SDL_FPoint mousePos = { (float)e.motion.x, (float)e.motion.y };
                drawToCanvas(renderer, canvas, mousePos);

            }

            if(e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_RIGHT) {
                clearCanvas(renderer, canvas);
            }
        }
        renderFrame(renderer, canvas);
    }
    
}