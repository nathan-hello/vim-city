#include <raylib.h>
#include <rcamera.h>

#define RAYGUI_IMPLEMENTATION
#include "../external/raygui-4.0/src/raygui.h"
#include "../external/raygui-4.0/styles/dark/style_dark.h"

#define LOG_LEVEL LOG_DEBUG
#define MAX_COLUMNS 20

typedef struct state_t {
    int screenWidth;
    int screenHeight;
    int cameraMode;
    
    // TODO: seperate into flag struct
    int menuFlag;
    int exitWindow;
    int showMessageBox;
    int makeSure;
    int tmp1;
    int tmp2;
} State; 

void drawHelpMenu(Camera *camera, State *state)
{
    DrawRectangle(5, 5, 330, 100, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(5, 5, 330, 100, BLUE);
    
    DrawText("Camera controls:", 15, 15, 10, BLACK);
    DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
    DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
    DrawText("- Camera mode keys: 1, 2, 3, 4", 15, 60, 10, BLACK);
    DrawText("- Zoom keys: num-plus, num-minus or mouse scroll", 15, 75, 10, BLACK);
    DrawText("- Camera projection key: P", 15, 90, 10, BLACK);
    
    DrawRectangle(state->screenWidth - 200, 5, 195, 100, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(state->screenWidth - 200, 5, 195, 100, BLUE);
    
    DrawText("Camera status:", state->screenWidth - 190, 15, 10, BLACK);
    DrawText(TextFormat("- Mode: %s", (state->cameraMode == CAMERA_FREE) ? "FREE" :
                                      (state->cameraMode == CAMERA_FIRST_PERSON) ? "FIRST_PERSON" :
                                      (state->cameraMode == CAMERA_THIRD_PERSON) ? "THIRD_PERSON" :
                                      (state->cameraMode == CAMERA_ORBITAL) ? "ORBITAL" : "CUSTOM"), state->screenWidth - 190, 30, 10, BLACK);
    DrawText(TextFormat("- Projection: %s", (camera->projection == CAMERA_PERSPECTIVE) ? "PERSPECTIVE" :
                                            (camera->projection == CAMERA_ORTHOGRAPHIC) ? "ORTHOGRAPHIC" : "CUSTOM"), state->screenWidth - 190, 45, 10, BLACK);
    DrawText(TextFormat("- Position: (%06.3f, %06.3f, %06.3f)", camera->position.x, camera->position.y, camera->position.z), state->screenWidth - 190, 60, 10, BLACK);
    DrawText(TextFormat("- Target: (%06.3f, %06.3f, %06.3f)", camera->target.x, camera->target.y, camera->target.z), state->screenWidth - 190, 75, 10, BLACK);
    DrawText(TextFormat("- Up: (%06.3f, %06.3f, %06.3f)", camera->up.x, camera->up.y, camera->up.z), state->screenWidth - 190, 90, 10, BLACK);
}

void handleKeyboard(Camera *camera, State *state)
{
    int key_pressed = GetKeyPressed();
    if (key_pressed)
    {
        TraceLog(LOG_DEBUG, "Pressed key: %d", key_pressed);
        if (IsKeyPressed(KEY_ESCAPE)) state->showMessageBox = !state->showMessageBox;

        if (IsKeyPressed(KEY_ONE))
        {
            state->cameraMode = CAMERA_FREE;
            camera->up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_TWO))
        {
            state->cameraMode = CAMERA_FIRST_PERSON;
            camera->up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_THREE))
        {
            state->cameraMode = CAMERA_THIRD_PERSON;
            camera->up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_FOUR))
        {
            state->cameraMode = CAMERA_ORBITAL;
            camera->up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_M)) state->menuFlag = !state->menuFlag;
        
        // Switch camera projection
        if (IsKeyPressed(KEY_P))
        {
            if (camera->projection == CAMERA_PERSPECTIVE)
            {
                // Create isometric view
                state->cameraMode = CAMERA_THIRD_PERSON;
                // Note: The target distance is related to the render distance in the orthographic projection
                camera->position = (Vector3){ 0.0f, 2.0f, -100.0f };
                camera->target = (Vector3){ 0.0f, 2.0f, 0.0f };
                camera->up = (Vector3){ 0.0f, 1.0f, 0.0f };
                camera->projection = CAMERA_ORTHOGRAPHIC;
                camera->fovy = 20.0f; // near plane width in CAMERA_ORTHOGRAPHIC
                CameraYaw(camera, -135 * DEG2RAD, true);
                CameraPitch(camera, -45 * DEG2RAD, true, true, false);
            }
            else if (camera->projection == CAMERA_ORTHOGRAPHIC)
            {
                // Reset to default view
                state->cameraMode = CAMERA_THIRD_PERSON;
                camera->position = (Vector3){ 0.0f, 2.0f, 10.0f };
                camera->target = (Vector3){ 0.0f, 2.0f, 0.0f };
                camera->up = (Vector3){ 0.0f, 1.0f, 0.0f };
                camera->projection = CAMERA_PERSPECTIVE;
                camera->fovy = 60.0f;
            }
        }
    }
}

int main(void)
{
    State state = {
        .screenWidth = 1200,
        .screenHeight = 600,
        .cameraMode = CAMERA_FIRST_PERSON,
        .menuFlag = 0,
        .showMessageBox = 0,
        .exitWindow = 0,
        .makeSure = 0,
        .tmp1 = 0,
        .tmp2 = 0,
    };

    InitWindow(state.screenWidth, state.screenHeight, "vimcity");
    SetExitKey(0);

    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 4.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type


    // Generates some random columns
    float heights[MAX_COLUMNS] = { 0 };
    Vector3 positions[MAX_COLUMNS] = { 0 };
    Color colors[MAX_COLUMNS] = { 0 };

    for (int i = 0; i < MAX_COLUMNS; i++) {
        heights[i] = (float)GetRandomValue(1, 12);
        positions[i] = (Vector3){ (float)GetRandomValue(-15, 15), heights[i]/2.0f, (float)GetRandomValue(-15, 15) };
        colors[i] = (Color){ GetRandomValue(20, 255), GetRandomValue(10, 55), 30, 255 };
    }

    DisableCursor();                    // Limit cursor to relative movement inside the window
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    GuiLoadStyleDark();

    while (!state.exitWindow)        // Detect window close button or ESC key
    {
        state.exitWindow = WindowShouldClose();
        // Update window size each loop
        state.screenWidth = GetScreenWidth();
        state.screenHeight = GetScreenHeight();

        handleKeyboard(&camera, &state);
        // Update camera computes movement internally depending on the camera mode
        // Some default standard keyboard/mouse inputs are hardcoded to simplify use
        // For advanced camera controls, it's recommended to compute camera movement manually
        UpdateCamera(&camera, state.cameraMode);                  // Update camera

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 32.0f, 32.0f }, LIGHTGRAY); // Draw ground
                DrawCube((Vector3){ -16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, BLUE);     // Draw a blue wall
                DrawCube((Vector3){ 16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, LIME);      // Draw a green wall
                DrawCube((Vector3){ 0.0f, 2.5f, 16.0f }, 32.0f, 5.0f, 1.0f, GOLD);      // Draw a yellow wall

                // Draw some cubes around
                for (int i = 0; i < MAX_COLUMNS; i++)
                {
                    DrawCube(positions[i], 2.0f, heights[i], 2.0f, colors[i]);
                    DrawCubeWires(positions[i], 2.0f, heights[i], 2.0f, MAROON);
                }

                // Draw player cube
                if (state.cameraMode == CAMERA_THIRD_PERSON)
                {
                    DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
                    DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
                }

            EndMode3D();

            if (state.menuFlag)
            {
                drawHelpMenu(&camera, &state);
            }
            
            if (state.showMessageBox)
            {
                int my = (float)state.screenHeight/2;
                int mx = (float)state.screenWidth/2;
                GuiGroupBox((Rectangle){ mx - 60, my - 75, 120, 150 }, "Menu");
                if (GuiDropdownBox((Rectangle){ mx - 60, my + 45, 120, 30 },
                                   "(S) Settings;TWO;THREE",
                                   &state.tmp1, state.tmp2))
                                   state.tmp2 = !state.tmp2;
                if (IsKeyPressed(KEY_S)) state.tmp1 = 1;

                if (GuiButton((Rectangle){ mx - 60, my + 75 , 120, 30 }, "(Q) Quit")) state.makeSure = 1;
                if (IsKeyPressed(KEY_Q)) state.makeSure = 1;

                if(state.makeSure) {
                    DrawRectangle(0, 0, state.screenWidth, state.screenHeight, Fade(RAYWHITE, 0.8f));
                    int result = GuiMessageBox(
                    (Rectangle){ mx - 125, my - 50, 250, 100 },
                    GuiIconText(ICON_EXIT, "Close Window"),
                    "Do you really want to exit?", "(Y) Yes;(N) No");
                    
                    if (IsKeyPressed(KEY_Y)) result = 1;
                    else if (IsKeyPressed(KEY_N)) result = 0;
 
                    if ((result == 0) || (result == 2)) state.makeSure = 0;
                    else if (result == 1) state.exitWindow = 1;
                }
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
