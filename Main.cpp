#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#include <thread>
#include <chrono>

#include "Oscilloscope.h"

std::atomic<bool> m_bRunning{ true };
std::thread inputThreadHandle;
std::mutex threadMutex;


void inputThread(Osciloscope* oscilloscope,int* samplingTime)
{
    using namespace std::chrono;

    while (true) {

        if (!m_bRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        auto start = steady_clock::now();

        oscilloscope->GenerateSampleSignal();

        auto elapsed = steady_clock::now() - start;
        auto sleep_time = milliseconds(*samplingTime) - duration_cast<milliseconds>(elapsed);

        if (sleep_time.count() > 0)
            std::this_thread::sleep_for(sleep_time);
    }
 }

// Main code
int main(int, char**)
{

    int SAMPLING_RATE = 1000;
    int DATA_LENGTH = 1000;
    int SAMPLING_TIME = (int)(1000 / SAMPLING_RATE); // ms

    int YMAX = 6;
    int YMIN = -6;

    int COLS = 10;
    int ROWS = 10;

    Osciloscope oscilloscope;

    if (!oscilloscope.OsciloscopeInit()) {
        std::cerr << "Oscilloscope Init Failed";
        return 0;
    }

    oscilloscope.Scale(SAMPLING_RATE, DATA_LENGTH, ROWS, COLS, YMIN, YMAX);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(oscilloscope.GetWindow(), oscilloscope.GetRenderer());
    ImGui_ImplSDLRenderer3_Init(oscilloscope.GetRenderer());

    // Main loop
    bool done = false;

    inputThreadHandle = std::thread(inputThread,&oscilloscope, &SAMPLING_TIME);

    SDL_Event event;


    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
            {
                done = true;
                m_bRunning = false;
                inputThreadHandle.detach();
            }
        }
        if (SDL_GetWindowFlags(oscilloscope.GetWindow()) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // === ImGui UI Panel ===
        ImGui::Begin("Control Panel");

        if (ImGui::SliderInt("Data Length", &DATA_LENGTH, 1000, 10000)) {
            oscilloscope.Scale(SAMPLING_RATE, DATA_LENGTH, ROWS, COLS, YMIN, YMAX);
        }

        if (ImGui::SliderInt("Rows", &ROWS, 2, 10)) {
            oscilloscope.Scale(SAMPLING_RATE, DATA_LENGTH, ROWS, COLS, YMIN, YMAX);
        }

        if (ImGui::SliderInt("Cols", &COLS, 2, 10)) {
            oscilloscope.Scale(SAMPLING_RATE, DATA_LENGTH, ROWS, COLS, YMIN, YMAX);
        }

        if (ImGui::SliderInt("Y Min", &YMIN, -10, 0)) {
            oscilloscope.Scale(SAMPLING_RATE, DATA_LENGTH, ROWS, COLS, YMIN, YMAX);
        }

        if (ImGui::SliderInt("Y Max", &YMAX, 0, 10)) {
            oscilloscope.Scale(SAMPLING_RATE, DATA_LENGTH, ROWS, COLS, YMIN, YMAX);
        }

        if (ImGui::Button("Pause Signal")) {
            m_bRunning = false;
        }

        if (ImGui::Button("Resume Signal")) {
            m_bRunning = true;
        }


        ImGui::End();
        // === End UI Panel ===

        oscilloscope.Draw();

        // Rendering
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), oscilloscope.GetRenderer());
        SDL_RenderPresent(oscilloscope.GetRenderer());
        SDL_Delay(16);
    }


    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
