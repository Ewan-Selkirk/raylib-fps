#include "game.h"

#ifndef DEBUG_FUCKING_EVERYTHING
#define DEBUG_FUCKING_EVERYTHING
#endif

int main(void) {
    bool showImGui = false;

    InitWindow(screenWidth, screenHeight, "First Person Controller");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    rlImGuiSetup(true);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    Camera camera = { 0 };
    
    camera.fovy = cameraFov;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = (Vector3){
        player.position.x,
        player.position.y + (bottomHeight + headLerp),
        player.position.z,
    };

    UpdateCameraAngle(&camera);

    DisableCursor();

    SetTargetFPS(targetFps);

    while (!WindowShouldClose()) {
        Vector2 mouseDelta = GetMouseDelta();
        
        lookRotation.x -= mouseDelta.x * sensitivity.x;
        lookRotation.y += mouseDelta.y * sensitivity.y;

        if (IsKeyPressed(KEY_MINUS)) cameraFov -= 2.5f;
        if (IsKeyPressed(KEY_EQUAL)) cameraFov += 2.5f;
        if (IsMouseButtonPressed(0)) DoLineTrace(&camera, &player, 50.f);
        if (IsKeyPressed(KEY_UP)) showImGui = !showImGui;

        char sideway = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
        char forward = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
        bool crouching = IsKeyDown(KEY_LEFT_CONTROL);
        UpdateBody(&player, lookRotation.x, sideway, forward, IsKeyPressed(KEY_SPACE), crouching);

        float delta = GetFrameTime();
        headLerp = Lerp(headLerp, (crouching ? crouchHeight : standHeight), 20.f * delta);

        camera.fovy = cameraFov;
        camera.position = (Vector3){
            player.position.x,
            player.position.y + (bottomHeight + headLerp),
            player.position.z,
        };

        if (player.bIsGrounded && ((forward != 0) || (sideway != 0))) {
            headTimer += delta * 3.f;
            walkLerp = Lerp(walkLerp, 1.f, 10.f * delta);
            camera.fovy = Lerp(camera.fovy, cameraFov - 5.f, 5.f * delta);
        } else {
            walkLerp = Lerp(walkLerp, 0.f, 10.f * delta);
            camera.fovy = Lerp(camera.fovy, cameraFov, 5.f * delta);
        }

        lean.x = Lerp(lean.x, sideway * 0.02f, 10.f * delta);
        lean.y = Lerp(lean.y, forward * 0.015f, 10.f * delta);

        UpdateCameraAngle(&camera);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                DrawLevel();
            EndMode3D();

            // DrawCircleV((Vector2){ (screenWidth / 2) - (lean.x * 300.f), (screenHeight / 2) - lean.y }, 2.5f, WHITE);
            // DrawCircleLinesV((Vector2){ (screenWidth / 2) - (lean.x * 300.f), (screenHeight / 2) - lean.y }, 2.5f, BLACK);

            DrawCircleV(Vector2Subtract(screenCenter, Vector2Scale(lean, 80.f)), 2.5f, WHITE);
            DrawCircleLinesV(Vector2Subtract(screenCenter, Vector2Scale(lean, 80.f)), 2.5f, BLACK);

            DrawRectangle(5, 5, 330, 75, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(5, 5, 330, 75, BLUE);

            DrawFPS(15, 10);

            DrawText(TextFormat("Velocity: (%06.3f)", Vector2Length((Vector2){ player.velocity.x, player.velocity.z})), 15, 35, 10, BLACK);
            DrawText(TextFormat("Camera FOV: (%01.1f)", cameraFov), 15, 50, 10, BLACK);

            rlImGuiBegin();

            // ImGui::ShowDemoWindow(&show);
            ShowDebugMenu(&showImGui);

            rlImGuiEnd();
        EndDrawing();
    }

    rlImGuiShutdown();

    CloseWindow();

    return 0;
}

void UpdateBody(Body* body, float rot, char side, char forward, bool bJumpPressed, bool bCrouchHeld) {
    Vector2 input = (Vector2){ (float)side, (float)-forward };

    if (normalizeInput) {
        if ((side != 0) && (forward != 0)) input = Vector2Normalize(input);
    }

    float delta = GetFrameTime();

    if (!body->bIsGrounded) body->velocity.y -= gravity * delta;

    if (body->bIsGrounded && bJumpPressed) {
        body->velocity.y = jumpForce;
        body->bIsGrounded = false;
    }

    Vector3 front = (Vector3){ sin(rot), 0.f, cos(rot) };
    Vector3 right = (Vector3){ cos(-rot), 0.f, sin(-rot) };

    Vector3 desiredDir = (Vector3){ input.x * right.x + input.y * front.x, 0.f, input.x * right.z + input.y * front.z };
    body->dir = Vector3Lerp(body->dir, desiredDir, control * delta);

    float decel = (body->bIsGrounded ? friction : airDrag);
    Vector3 hvel = (Vector3){ body->velocity.x * decel, 0.f, body->velocity.z * decel };

    float hvelLength = Vector3Length(hvel);
    if (hvelLength < (walkSpeed * 0.01f)) hvel = (Vector3){ 0 };

    float speed = Vector3DotProduct(hvel, body->dir);

    float maxSpeed = (bCrouchHeld ? crouchSpeed : walkSpeed);
    float accel = Clamp(maxSpeed - speed, 0.f, maxAccel * delta);
    hvel.x += body->dir.x * accel;
    hvel.z += body->dir.z * accel;

    body->velocity.x = hvel.x;
    body->velocity.z = hvel.z;

    body->position.x += body->velocity.x * delta;
    body->position.y += body->velocity.y * delta;
    body->position.z += body->velocity.z * delta;

    if (body->position.y <= 0.f) {
        body->position.y = 0.f;
        body->velocity.y = 0.f;
        body->bIsGrounded = true;
    }
}

static void UpdateCameraAngle(Camera *camera) {
    const Vector3 up = (Vector3){ 0.f, 1.f, 0.f };
    const Vector3 targetOffset = (Vector3){ 0.f, 0.f, -1.f };

    Vector3 yaw = Vector3RotateByAxisAngle(targetOffset, up, lookRotation.x);

    float maxAngleUp = Vector3Angle(up, yaw);
    maxAngleUp -= 0.001f;
    if ( -(lookRotation.y) > maxAngleUp) { lookRotation.y = -maxAngleUp; }

    float maxAngleDown = Vector3Angle(Vector3Negate(up), yaw);
    maxAngleDown *= -1.f;
    maxAngleDown += 0.001f;
    if (-(lookRotation.y) < maxAngleDown) { lookRotation.y = -maxAngleDown; }

    Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, up));
    Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, -lookRotation.y - lean.y);

    float headSin = sin(headTimer * PI);
    float headCos = cos(headTimer * PI);
    const float stepRotation = 0.01f;
    camera->up = Vector3RotateByAxisAngle(up, pitch, headSin * stepRotation + lean.x);

    const float bobSide = 0.1f;
    const float bobUp = 0.15f;
    Vector3 bobbing = Vector3Scale(right, headSin * bobSide);
    bobbing.y = fabsf(headCos * bobUp);

    camera->position = Vector3Add(camera->position, Vector3Scale(bobbing, walkLerp));
    camera->target = Vector3Add(camera->position, pitch);
}

static void DrawLevel(void) {
    const int floorExtent = 25;
    const float tileSize = 5.f;
    const Color tileColor1 = (Color){ 150, 200, 200, 255 };

    for (int y = -floorExtent; y < floorExtent; y++) {
        for (int x = -floorExtent; x < floorExtent; x++) {
            if ((y & 1) && (x & 1)) {
                DrawPlane((Vector3){ x * tileSize, 0.f, y * tileSize }, (Vector2){ tileSize, tileSize }, tileColor1);
            } else if (!(y & 1) && !(x & 1)) {
                DrawPlane((Vector3){ x * tileSize, 0.f, y * tileSize }, (Vector2){ tileSize, tileSize }, LIGHTGRAY);
            }
        }
    }

    const Vector3 towerSize = (Vector3){ 16.f, 32.f, 16.f };
    const Color towerColor = (Color){ 150, 200, 200, 255 };

    Vector3 towerPos = (Vector3){ 16.f, 16.f, 16.f };
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    towerPos.x *= -1;
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    towerPos.z *= -1;
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    towerPos.x *= -1;
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    DrawSphere((Vector3){ 300.f, 300.f, 0.f }, 100.f, MAROON);

    for (int i = 0; i < 10; i++) {
        DrawCircle3D((Vector3){ 0.f, 0.f, 5.f * i }, 1.f, (Vector3){ 0.f, 1.f, 0.f }, GetTime() * 100.f, VIOLET);
        // DrawMesh(GenMeshCylinder(1.f, 1.f, 8), LoadMaterialDefault(), (Matrix){ 0 });
    }
}

void DoLineTrace(Camera *camera, Body *body, float length) {
    BeginDrawing();
        BeginMode3D(*camera);
            // DrawLine3D(camera->position, camera->position + body->dir * length, RED);
            DrawLine3D(camera->position, Vector3Add(camera->position, body->dir), RED);
        EndMode3D();
    EndDrawing();
}

void ShowDebugMenu(bool* show) {
    if (*show != true) return;

    ImGui::SetNextWindowSizeConstraints(ImVec2(GetWindowScaleDPI().x * 400.f, GetWindowScaleDPI().x * 400.f), ImVec2(float(GetScreenWidth()), float(GetScreenHeight())));

    if (ImGui::Begin("Debug Menu", show, ImGuiWindowFlags_NoScrollbar)) {
        Rectangle contentRect = { ImGui::GetWindowPos().x + ImGui::GetCursorScreenPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorScreenPos().y, ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };

        if (ImGui::BeginChild("Variables", ImGui::GetContentRegionAvail())) {
            ImGui::SetCursorPosX(2);
            ImGui::SetCursorPosY(3);

            ImGui::Text("Camera FOV: %f", cameraFov);
            ImGui::SameLine();

            if (ImGui::Button("Reset FOV")) {
                cameraFov = 60.f;
            }

            ImGui::SliderFloat("Value", &cameraFov, 60.f, 120.f);

            ImGui::EndChild();
        }

        ImGui::End();
    }
}
