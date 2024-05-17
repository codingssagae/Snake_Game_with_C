#include "SDL.h" // SDL 헤더 포함
#include "SDL_ttf.h"
#include <iostream>
#include <list>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;

struct Point {
    int x;
    int y;
};

class SnakeGame {
private:
    int width, height;
    int score; // 점수 변수
    int speed; // 속도 변수
    int level; // 레벨 변수
    Point food;
    Point blueFood; // 파란색 아이템 변수
    std::vector<Point> obstacles; // 장애물 변수
    std::list<Point> snake;
    bool gameOver;
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    bool showReady;

    void GenerateFood() {
        bool validPosition;
        do {
            validPosition = true;
            food.x = rand() % width;
            food.y = rand() % height;

            // Check for collision with blueFood
            if (food.x == blueFood.x && food.y == blueFood.y) {
                validPosition = false;
            }

            // Check for collision with obstacles
            for (const auto& obstacle : obstacles) {
                if (food.x == obstacle.x && food.y == obstacle.y) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition || food.x == 0 || food.x == width - 1 || food.y == 0 || food.y == height - 1);
    }

    void GenerateBlueFood() {
        bool validPosition;
        do {
            validPosition = true;
            blueFood.x = rand() % width;
            blueFood.y = rand() % height;

            // Check for collision with red food
            if (blueFood.x == food.x && blueFood.y == food.y) {
                validPosition = false;
            }

            // Check for collision with obstacles
            for (const auto& obstacle : obstacles) {
                if (blueFood.x == obstacle.x && blueFood.y == obstacle.y) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition || blueFood.x == 0 || blueFood.x == width - 1 || blueFood.y == 0 || blueFood.y == height - 1);
    }

    void GenerateObstacles(int numObstacles) {
        for (int i = 0; i < numObstacles; ++i) {
            Point obstacle;
            bool validPosition;
            do {
                validPosition = true;
                obstacle.x = rand() % width;
                obstacle.y = rand() % height;

                // Check for collision with food
                if (obstacle.x == food.x && obstacle.y == food.y) {
                    validPosition = false;
                }

                // Check for collision with blueFood
                if (obstacle.x == blueFood.x && obstacle.y == blueFood.y) {
                    validPosition = false;
                }

                // Check for collision with other obstacles
                for (const auto& existingObstacle : obstacles) {
                    if (obstacle.x == existingObstacle.x && obstacle.y == existingObstacle.y) {
                        validPosition = false;
                        break;
                    }
                }
            } while (!validPosition || obstacle.x == 0 || obstacle.x == width - 1 || obstacle.y == 0 || obstacle.y == height - 1);
            obstacles.push_back(obstacle);
        }
    }


    void DrawText(const char* text, int x, int y) {
        SDL_Color color = { 255, 255, 255, 255 };
        SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect dstRect = { x, y, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture, NULL, &dstRect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    void Draw() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw borders
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect border = { 0, 0, width * 20, height * 20 };
        SDL_RenderDrawRect(renderer, &border);

        // Draw food
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect foodRect = { food.x * 20, food.y * 20, 20, 20 };
        SDL_RenderFillRect(renderer, &foodRect);

        // Draw blue food
        static int frame = 0;
        frame++;
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_Rect blueFoodRect = { blueFood.x * 20, blueFood.y * 20, 20, 20 };
        SDL_RenderFillRect(renderer, &blueFoodRect);

        // Draw obstacles
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        for (const auto& obstacle : obstacles) {
            SDL_Rect obstacleRect = { obstacle.x * 20, obstacle.y * 20, 20, 20 };
            SDL_RenderFillRect(renderer, &obstacleRect);
        }

        // Draw snake
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (auto s : snake) {
            SDL_Rect snakeRect = { s.x * 20, s.y * 20, 20, 20 };
            SDL_RenderFillRect(renderer, &snakeRect);
        }

        if (showReady) {
            DrawText("Ready", (width * 20) / 2 - 50, (height * 20) / 2 - 10);
        }

        if (gameOver) {
            DrawText("Game Over", (width * 20) / 2 - 60, (height * 20) / 2 - 10);
        }

        // Draw score and level border
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect infoBorder = { 0, height * 20, width * 20, 40 };
        SDL_RenderDrawRect(renderer, &infoBorder);

        // Draw score and level
        std::string scoreText = "Score: " + std::to_string(score);
        std::string levelText = "Level: " + std::to_string(level);
        DrawText(scoreText.c_str(), 10, height * 20 + 10);
        DrawText(levelText.c_str(), width * 10, height * 20 + 10);

        // Blinking effect
        static bool blink = false;
        static int blinkCounter = 0;
        if (blink) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 128); // Semi-transparent white
            SDL_Rect blinkRect = { 0, 0, width * 20, height * 20 };
            SDL_RenderFillRect(renderer, &blinkRect);
            blinkCounter++;
            if (blinkCounter > 10) { // Blink for a short period
                blink = false;
                blinkCounter = 0;
            }
        }

        SDL_RenderPresent(renderer);
    }


    void Input() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameOver = true;
                showReady = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                showReady = false;
                switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    dir = LEFT;
                    break;
                case SDLK_RIGHT:
                    dir = RIGHT;
                    break;
                case SDLK_UP:
                    dir = UP;
                    break;
                case SDLK_DOWN:
                    dir = DOWN;
                    break;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    ResetGame();
                }
            }
        }
    }

    void Logic() {
        if (gameOver || showReady) return;

        Point prev = snake.front();
        Point next = prev;

        switch (dir) {
        case LEFT:
            next.x--;
            break;
        case RIGHT:
            next.x++;
            break;
        case UP:
            next.y--;
            break;
        case DOWN:
            next.y++;
            break;
        default:
            break;
        }

        if (next.x < 0 || next.x >= width || next.y < 0 || next.y >= height) {
            gameOver = true;
            return;
        }

        for (const auto& obstacle : obstacles) {
            if (next.x == obstacle.x && next.y == obstacle.y) {
                gameOver = true;
                return;
            }
        }

        for (auto it = ++snake.begin(); it != snake.end(); ++it) {
            if (it->x == next.x && it->y == next.y) {
                gameOver = true;
                return;
            }
        }

        snake.push_front(next);
        if (next.x == food.x && next.y == food.y) {
            score += 10; // 점수 증가
            GenerateFood();
        }
        else if (next.x == blueFood.x && next.y == blueFood.y) {
            score = std::max(0, score - 10); // 점수가 0보다 작아지지 않도록 함
            if (snake.size() > 1) {
                snake.erase(std::prev(snake.end())); // 지렁이 길이 감소
            }
            GenerateBlueFood();
            if (speed > 10) { // 최소 속도 제한 (너무 빨라지지 않도록)
                speed -= 10; // 속도 증가 (딜레이 감소)
            }
            static bool blink = true;
        }
        else {
            snake.pop_back();
        }

        // 레벨 및 장애물 조정
        int newLevel = score / 50;
        if (newLevel > level) {
            level = newLevel;
            GenerateObstacles(3); // 레벨당 3개의 장애물 추가
            if (speed > 20) {
                speed -= 10; // 속도 증가 (딜레이 감소)
            }
        }
    }

    void ResetGame() {
        snake.clear();
        snake.push_back({ width / 2, height / 2 });
        dir = STOP;
        gameOver = false;
        showReady = true;
        score = 0; // 점수 초기화
        level = 0; // 레벨 초기화
        speed = 100; // 초기 속도 설정
        GenerateFood();
        GenerateBlueFood(); // 파란색 아이템 초기화
        obstacles.clear(); // 장애물 초기화
    }

public:
    SnakeGame(int w, int h) : width(w), height(h), score(0), level(0), speed(100), gameOver(false), window(nullptr), renderer(nullptr), font(nullptr), showReady(true) {
        SDL_Init(SDL_INIT_VIDEO);
        TTF_Init();
        window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width * 20, height * 20 + 40, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        // 절대 경로 사용 예제
        const char* fontPath = "../../Resources/YEONGJUSeonbi.ttf";

        font = TTF_OpenFont(fontPath, 24);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            exit(1);
        }
        ResetGame();
        srand(time(0));
    }

    ~SnakeGame() {
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
    }

    void Run() {
        while (true) {
            Input();
            if (!showReady && !gameOver) {
                Logic();
            }
            Draw();
            SDL_Delay(speed);
        }
    }
};

int main(int argc, char* argv[]) {
    SnakeGame game(40, 20);
    game.Run();
    return 0;
}
