#include "Game.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>

Game::~Game() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

bool Game::Initialize(int windowWidth, int windowHeight) {
    mWindowWidth = windowWidth;
    mWindowHeight = windowHeight;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow(
        "Game of Life",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight,
        SDL_WINDOW_RESIZABLE
    );
    if (mWindow == nullptr) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (mRenderer == nullptr) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    mImGuiIO = &ImGui::GetIO();
    mImGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(mWindow, mRenderer);
    ImGui_ImplSDLRenderer2_Init(mRenderer);

    mCells.resize(mCellCount);
    for (int i = 0; i < mCells.size(); ++i) {
        mCells[i].resize(mCellCount);
    }

    return true;
}

void Game::Start() {
    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 last = 0;

    while (mRunning) {
        while (SDL_PollEvent(&mEvent)) {
            HandleEvent();
        }

        last = now;
        now = SDL_GetPerformanceCounter();

        mDeltaTime = static_cast<float>(((now - last) * 1000) / static_cast<float>(SDL_GetPerformanceFrequency())) * 0.001f;
        mTimePassed += mDeltaTime;

        Update();
        Render();
    }
}

void Game::HandleEvent() {
    ImGui_ImplSDL2_ProcessEvent(&mEvent);

    switch (mEvent.type) {
    case SDL_QUIT:
        mRunning = false;
        break;
    case SDL_MOUSEBUTTONUP:
        if (mEvent.button.button == 1) {
            int x = (mEvent.button.x - mGridCenterX) / mCellWidth;
            int y = (mEvent.button.y - mGridCenterY) / mCellHeight;

            if (x >= mCells.size() || y >= mCells.size()) break;
            mCells[y][x].state = !mCells[y][x].state;
        }

        break;
    case SDL_WINDOWEVENT:
        if (mEvent.window.event == SDL_WINDOWEVENT_RESIZED) {
            mWindowWidth = mEvent.window.data1;
            mWindowHeight = mEvent.window.data2;
        }

        break;
    }
}

void Game::Update() {
    mGridCenterX = (mWindowWidth / 2) - ((mCells[0].size() * mCellWidth) / 2);
    mGridCenterY = (mWindowHeight / 2) - ((mCells.size() * mCellHeight) / 2);

    if (mTimePassed < mSimulationUpdateRate) return;
    mTimePassed = 0;

    if (!mSimulationRunning) return;

    for (int y = 0; y < mCells.size(); ++y) {
        for (int x = 0; x < mCells[y].size(); ++x) {
            int aliveNeighbours = 0;

            for (int ny = y - 1; ny <= (y + 1); ++ny) {
                for (int nx = x - 1; nx <= (x + 1); ++nx) {
                    if (nx < 0 || ny < 0 || nx >= mCells[y].size() || ny >= mCells.size()) continue;
                    if (nx == x && ny == y) continue;
                    if (mCells[ny][nx].state == 0) continue;

                    ++aliveNeighbours;
                    if (aliveNeighbours > 3) break;
                }
            }

            if (mCells[y][x].state == 0) {
                mCells[y][x].nextState = (aliveNeighbours == 3);
            }
            else if (mCells[y][x].state == 1) {
                mCells[y][x].nextState = (aliveNeighbours == 2 || aliveNeighbours == 3);
            }
        }
    }

    for (int y = 0; y < mCells.size(); ++y) {
        for (int x = 0; x < mCells.size(); ++x) {
            mCells[y][x].state = mCells[y][x].nextState;
        }
    }
}

void Game::Render() {
    SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
    SDL_RenderClear(mRenderer);

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImVec2 pos(10, 10);
    ImGui::SetNextWindowPos(pos);
    ImGui::Begin("Simulation settings");

    ImGui::Text("FPS: %i", static_cast<int>(1 / mDeltaTime));
    ImGui::Text("Delta time: %f", mDeltaTime);

    ImGui::SliderFloat("Simulation update rate", &mSimulationUpdateRate, 0.1f, 1.0f);

    ImGui::NewLine();

    ImGui::InputInt("Cell count", &mCellCount);
    if (ImGui::Button("Apply"))
        SetNewCellCount();

    ImGui::NewLine();

    ImGui::PushStyleColor(ImGuiCol_Text, mSimulationRunning ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));
    ImGui::Text(mSimulationRunning ? "Simulation is running" : "Simulation is not running");
    ImGui::PopStyleColor();
    if (ImGui::Button("Start"))
        mSimulationRunning = true;
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
        mSimulationRunning = false;
    ImGui::SameLine();
    if (ImGui::Button("Clear board"))
        ClearBoard();
    ImGui::End();

    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 0);

    for (int y = 0; y < mCells.size(); ++y) {
        for (int x = 0; x < mCells[y].size(); ++x) {
            SDL_Rect rect{ mGridCenterX + x * mCellWidth , mGridCenterY + y * mCellHeight, mCellWidth, mCellHeight };

            if (mCells[y][x].state == 0) {
                SDL_RenderDrawRect(mRenderer, &rect);
            }
            else {
                SDL_RenderFillRect(mRenderer, &rect);
            }
        }
    }

    ImGui::Render();
    SDL_RenderSetScale(mRenderer, mImGuiIO->DisplayFramebufferScale.x, mImGuiIO->DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(mRenderer);
}

void Game::SetNewCellCount() {
    if (mCellCount < 1) return;

    mCells.resize(mCellCount);

    for (int i = 0; i < mCellCount; ++i) {
        mCells[i].resize(mCellCount);
    }
}

void Game::ClearBoard() {
    for (std::vector<Cell>& row : mCells) {
        for (Cell& cell : row) {
            cell.state = 0;
            cell.nextState = 0;
        }
    }
}