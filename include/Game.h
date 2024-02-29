#ifndef GAMEOFLIFE_GAME_H
#define GAMEOFLIFE_GAME_H

#include <vector>
#include <ctime>

#include <SDL2/SDL.h>
#include <imgui/imgui.h>

#include "Cell.h"

class Game {
private:
    SDL_Window* mWindow = nullptr;
    SDL_Renderer* mRenderer = nullptr;
    ImGuiIO* mImGuiIO = nullptr;

    int mWindowWidth = 0;
    int mWindowHeight = 0;

    int mGridCenterX = 0;
    int mGridCenterY = 0;

    std::vector<std::vector<Cell>> mCells;
    int mCellWidth = 16;
    int mCellHeight = 16;
    int mCellCount = 35;

    SDL_Event mEvent;
    float mDeltaTime = 0;
    float mTimePassed = 0;
    float mSimulationUpdateRate = 0.15f;
    bool mRunning = true;
    bool mSimulationRunning = true;

    void HandleEvent();
    void Update();
    void Render();

    void SetNewCellCount();
    void ClearBoard();

public:
    ~Game();
    bool Initialize(int windowWidth, int windowHeight);
    void Start();
};

#endif