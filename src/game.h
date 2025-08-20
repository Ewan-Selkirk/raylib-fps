#include <list>
#include <vector>

#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "imgui.h"
#include "rlImGui.h"
#include "imgui_impl_raylib.h"

const int screenWidth = 1280;
const int screenHeight = 720;
const int targetFps = 90;

const float gravity = 32.f;
const float walkSpeed = 20.f;
const float crouchSpeed = 5.f;
const float sprintSpeed = 28.f;
const float jumpForce = 12.f;
const float maxAccel = 150.f;
const float friction = 0.86f;
const float airDrag = 0.98f;
const float control = 15.f;
const float crouchHeight = 0.25f;
const float standHeight = 1.f;
const float bottomHeight = 0.5f;

const int normalizeInput = 0;

typedef struct {
    Vector3 position;
    Vector3 velocity;
    Vector3 dir;
    bool bIsGrounded;
    bool bIsSprinting;
} Body;

typedef struct {
    Vector3 position;
    Color color;
    bool bIsCollected;
} Collectable;

typedef struct {
    Ray ray;
    float lifetime;
} RayTrace;

static Vector2 screenCenter = { screenWidth / 2, screenHeight / 2};
static Vector2 sensitivity = {0.001f, 0.001f};
static Body player = { 0 };
static Vector2 lean = { 0 };
static Vector2 lookRotation = { 0 };
static float headTimer = 0.f;
static float walkLerp = 0.f;
static float headLerp = standHeight;

static float cameraFov = 60.f;

static void DrawLevel(void);
static void UpdateCameraAngle(Camera *camera);
static void UpdateBody(Body *body, float rot, char side, char forward, bool bJumpPressed, bool bCrouchHeld);

void ShowDebugMenu(bool* show);
