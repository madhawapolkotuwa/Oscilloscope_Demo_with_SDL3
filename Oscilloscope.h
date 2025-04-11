#ifndef __OSCILLOSCOPE__
#define __OSCILLOSCOPE__

#pragma once

#include <array>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <random>

#include <mutex>
#include <atomic>

#ifndef M_PI
#define M_PI  3.14159265358979323846  /* pi */
#endif

#define WINDOW_WIDTH        1280
#define WINDOW_HEIGHT        720
#define MAX_CHANNELS           8

#define BG_COLOR      0.04f   , 0.161f , 0.243f , 1.0f
#define GRID_COLOR    0.165f , 0.8f   , 0.016f , 1.0f

const float CHANNEL_COLORS[][4] = {{1.0f  , 0.0f , 0.0f  , 1.0f },
                                   {1.0f  , 1.0f , 0.0f  , 1.0f },
                                   {0.0f  , 1.0f , 0.0f  , 1.0f },
                                   {0.0f  , 1.0f , 0.918f, 1.0f },
                                   {0.349f,0.349f, 0.969f, 1.0f },
                                   {0.941f,0.06f , 0.967f, 1.0f },
                                   {0.988f,0.733f, 0.094f, 1.0f },
                                   {0.396f,0.941f, 0.773f, 1.0f }};

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dist(0.0f, 1.0f);

class Osciloscope 
{
    std::mutex mtx;

public:
    Osciloscope(){
        m_pWindow = nullptr;
        m_pRenderer = nullptr;
        m_pCanvas = nullptr;

        memset((void*)&m_fphase, 0, sizeof(float) * MAX_CHANNELS);
        m_usWriteIndex = 0;
        m_usTempNewValueCount = 0;
        m_bCanvasClear = false;

        m_fHalfHight  =  (float)WINDOW_HEIGHT / 2.0f;


        for(int i=0; i<MAX_CHANNELS; i++){
            m_PreviousPoint[i] = {-1, -1};
        }
        for (int i = 0; i < MAX_CHANNELS; i++) {
            m_pNewPoint[i] = new SDL_FPoint[1000];
        }
    }

    ~Osciloscope(){
        SDL_DestroyRenderer(m_pRenderer);
        SDL_DestroyWindow(m_pWindow);
        SDL_Quit();
    }

    SDL_Window* GetWindow() const {
        return m_pWindow;
    }

    SDL_Renderer* GetRenderer() const {
        return m_pRenderer;
    }

    bool OsciloscopeInit(){
        if(!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "Error: SDL_Init(): " << SDL_GetError() << "\n";
            return false;
        }

        if(!SDL_CreateWindowAndRenderer("Oscillosope", WINDOW_WIDTH, WINDOW_HEIGHT, 0 , &m_pWindow, &m_pRenderer)) // SDL_WINDOW_BORDERLESS
        {
            std::cerr << "Error: SDL_CreateWindowAndRenderer(): " << SDL_GetError() << "\n";
            SDL_Quit();
            return false;
        }
        return true;
    }

    void Scale(int sampleRate, int dataLength, int rows, int cols, int ymin, int ymax){

        std::lock_guard<std::mutex> lock(mtx); // Lock during signal generation + canvas draw

        m_nSamplingRate = sampleRate; 
        m_nDataLength = dataLength;
        m_fTimeSteps = 1.0f / m_nSamplingRate;

        m_fSamplesPerPixel = (float)WINDOW_WIDTH / (float)dataLength ;
        m_nRows = rows;
        m_nCols = cols;

        m_fYStep = (float)WINDOW_HEIGHT / rows;
        m_fXStep = (float)WINDOW_WIDTH  / cols;

        m_fYmax = ymax;
        m_fYmin = ymin;

        m_usWriteIndex = 0;
        m_usTempNewValueCount = 0;

        m_pCanvas = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);
        clearCanvas();

    }

    void Draw()
    { 
        {
            std::lock_guard<std::mutex> lock(mtx);

            for (int i = 0; i < m_usTempNewValueCount; i++)
                    drawToCanvas(m_pNewPoint, i);

            if (m_bCanvasClear) {
                m_bCanvasClear = false;
                clearCanvas();
            }

            renderFrame();
            m_usTempNewValueCount = 0;
        }
    }

private:
    SDL_Window*                     m_pWindow;
    SDL_Renderer*                   m_pRenderer;

    int                             m_nSamplingRate;
    float                           m_fTimeSteps;
    int                             m_nDataLength;
    float                           m_fXStep;
    float                           m_fYStep;
    int                             m_nRows;
    int                             m_nCols;
    float                           m_fphase[MAX_CHANNELS]; 
    uint16_t                        m_usWriteIndex;


    float                           m_fSamplesPerPixel;
    float                           m_fHalfHight;

    float                           m_fYmax;
    float                           m_fYmin;

    uint16_t                        m_usTempNewValueCount;
    bool                            m_bCanvasClear;

    #pragma region CANVAS

    ////////////////////////////////////////////////////////
    /////////////// CANVAS SETTINGS ////////////////////////
    ////////////////////////////////////////////////////////

    SDL_Texture*             m_pCanvas;
    SDL_FPoint               m_PreviousPoint[MAX_CHANNELS];  
    SDL_FPoint*              m_pNewPoint[MAX_CHANNELS];

    void clearCanvas() {
        SDL_SetRenderTarget(m_pRenderer, m_pCanvas);
        SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(m_pRenderer);
    
        // Draw grid
        drawGridToCanvas(); // 10x10 grid for example
    
        SDL_SetRenderTarget(m_pRenderer, nullptr);
    
        for(int iCh=0; iCh < MAX_CHANNELS; iCh++)
            m_PreviousPoint[iCh] = { -1, -1 }; // Reset previous point to avoid connecting to old drawings

        memset((void*)&m_fphase, 0, sizeof(float) * MAX_CHANNELS);
    }

    void drawGridToCanvas()
    {

        SDL_SetRenderDrawColor(m_pRenderer, 40, 40, 40, 255); // Subtle grid color

        // Draw horizontal lines
        for (int i = 1; i < m_nRows; ++i) {
            float y = i * m_fYStep;
            SDL_RenderLine(m_pRenderer, 0.0f, y, (float)WINDOW_WIDTH, y);
        }

        // Draw vertical lines
        for (int i = 1; i < m_nCols; ++i) {
            float x = i * m_fXStep;
            SDL_RenderLine(m_pRenderer, x, 0.0f, x, (float)WINDOW_HEIGHT);
        }
    }

    void drawToCanvas(SDL_FPoint* newPoint[], int index) {

        SDL_SetRenderTarget(m_pRenderer, m_pCanvas);

        for(int iCh=0; iCh < MAX_CHANNELS; iCh++){
            if (m_PreviousPoint[iCh].x >= 0 && m_PreviousPoint[iCh].y >= 0) {
                // Set the render target to the canvas
        
                // Draw a line from previousPoint to newPoint
                SDL_SetRenderDrawColorFloat(m_pRenderer, CHANNEL_COLORS[iCh][0],CHANNEL_COLORS[iCh][1],CHANNEL_COLORS[iCh][2],CHANNEL_COLORS[iCh][3]);
                SDL_RenderLine(m_pRenderer, m_PreviousPoint[iCh].x, m_PreviousPoint[iCh].y, newPoint[iCh][index].x, newPoint[iCh][index].y);
            }
        
            m_PreviousPoint[iCh] = newPoint[iCh][index];
        }

        // Restore render target back to screen
        SDL_SetRenderTarget(m_pRenderer, nullptr);
    }

    void renderFrame() {
        SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255); // clear screen
        SDL_RenderClear(m_pRenderer);
    
        SDL_RenderTexture(m_pRenderer, m_pCanvas, NULL, NULL); // draw canvas
    }

    #pragma endregion

    #pragma region SIGANL_GENERATION

public:
    void GenerateSampleSignal(){

        std::lock_guard<std::mutex> lock(mtx); // Lock during signal generation + canvas draw

        float signal;

        m_fphase[0] += 2.0f * M_PI * 1.0f * m_fTimeSteps;
        m_fphase[1] += 2.0f * M_PI * 1.5f * m_fTimeSteps;
        m_fphase[2] += 2.0f * M_PI * 2.0f * m_fTimeSteps;
        m_fphase[3] += 2.0f * M_PI * 2.5f * m_fTimeSteps;
        m_fphase[4] += 2.0f * M_PI * 3.0f * m_fTimeSteps;
        m_fphase[5] += 2.0f * M_PI * 3.5f * m_fTimeSteps;
        m_fphase[6] += 2.0f * M_PI * 4.0f * m_fTimeSteps;
        m_fphase[7] += 2.0f * M_PI * 4.5f * m_fTimeSteps;
        
        for(int i=0; i < MAX_CHANNELS; i++){
           
            signal = std::sin(m_fphase[i])* (i + 1) / 2.0f;

            // Normalize the signal based on the current Y-min/Y-max
            float normalized = (signal - m_fYmin) / (m_fYmax - m_fYmin);
            normalized = std::clamp(normalized, 0.0f, 1.0f); // avoid overshoot
            m_pNewPoint[i][m_usTempNewValueCount].y = (1.0f - normalized) * WINDOW_HEIGHT;

            m_pNewPoint[i][m_usTempNewValueCount].x = m_fSamplesPerPixel * m_usWriteIndex;
        }


        if (m_usWriteIndex >= m_nDataLength) {
            m_usWriteIndex = 0;
            m_bCanvasClear = true;
        }

        m_usWriteIndex++;
        m_usTempNewValueCount++;
    }

    #pragma endregion
};

#endif
