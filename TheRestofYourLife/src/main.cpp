#include "raylib.h"
#include <vector>
#include <random>
#include <string>

struct Point {
    float x, y;
    bool inside;
};

double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

int main() {
    const int screenWidth = 800;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Book 3: Monte Carlo Pi Estimation");

    std::vector<Point> points;
    int inside_circle = 0;
    int total_runs = 0;

    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    BeginTextureMode(target);
    ClearBackground(BLACK);
    EndTextureMode();

    SetTargetFPS(120); 

    while (!WindowShouldClose()) {
        for(int i = 0; i < 100; i++) {
            float x = random_double() * 2.0 - 1.0; 
            float y = random_double() * 2.0 - 1.0; 
            
            bool inside = (x*x + y*y) < 1.0;
            if (inside) inside_circle++;
            total_runs++;

            int sx = (x + 1.0) * 0.5 * screenWidth;
            int sy = (y + 1.0) * 0.5 * screenHeight;

            BeginTextureMode(target);
                DrawPixel(sx, sy, inside ? GREEN : RED);
            EndTextureMode();
        }

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTextureRec(target.texture, 
                           (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height}, 
                           (Vector2){0, 0}, WHITE);

            DrawCircleLines(screenWidth/2, screenHeight/2, screenWidth/2, WHITE);
            DrawRectangleLines(0, 0, screenWidth, screenHeight, WHITE);

            double pi_estimate = 4.0 * double(inside_circle) / double(total_runs);
            
            DrawRectangle(10, 10, 300, 90, Fade(BLACK, 0.7f));
            DrawText(TextFormat("Total Points: %d", total_runs), 20, 20, 20, WHITE);
            DrawText(TextFormat("Inside: %d", inside_circle), 20, 45, 20, GREEN);
            DrawText(TextFormat("Pi Estimate: %.5f", pi_estimate), 20, 70, 20, YELLOW);
            DrawText("(Real Pi: 3.14159...)", 160, 72, 10, GRAY);

        EndDrawing();
    }

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}