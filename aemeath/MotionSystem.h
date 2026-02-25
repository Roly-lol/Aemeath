#pragma once
#include <windows.h>

enum MotionState
{
    MOTION_WANDER,
    MOTION_FOLLOW,
    MOTION_CURIOUS,
    MOTION_REST
};

struct MotionConfig
{
    double speedX = 3;
    double speedY = 2;
    double inertia = 0.95;
    double intent = 0.05;
    double jitter = 0.15;
    int followStartDist = 200;
    int followStopDist = 60;
    int followDistance = 80;
    int restDistance = 20;
};

class MotionSystem
{
public:
    MotionSystem(int screenW, int screenH, int w, int h);

    void Update(bool followMouseEnabled, POINT mousePos);
    void GetPosition(double& outX, double& outY) const { outX = x; outY = y; }
    void SetPosition(double nx, double ny) { x = nx; y = ny; }
    void SetSize(int w, int h) { petW = w; petH = h; }
    //void SetVelocity(double nvx, double nvy) { vx = nvx; vy = nvy; }
    MotionState GetState() const { return state; }
    bool IsMovingRight() const { return vx >= 0; }
    void ApplyExternalVelocity(double nvx, double nvy);
private:
    void ChooseRandomTarget(bool allowOutside);
    void HandleEdge();

private:
    MotionConfig cfg;
    MotionState state = MOTION_WANDER;
    bool externalVelocity = false;
    int screenW, screenH;
    int petW, petH;

    double x = 200, y = 200;
    double vx = 3, vy = 2;
    double targetX, targetY;
    int targetTimer = 0;
    int restTimer = 0;

    int moveTick = 0;
    double jitterX = 0, jitterY = 0;
};