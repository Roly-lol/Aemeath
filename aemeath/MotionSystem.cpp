#include "MotionSystem.h"
#include <cmath>
#include <cstdlib>
// 生成 a 到 b 之间的随机整数，包含 a 和 b
static int RandInt(int a, int b) { return a + rand() % (b - a + 1); }
// 生成 a 到 b 之间的随机 double，包含 a 但不包含 b
static double RandDouble(double a, double b)
{
    return a + (double)rand() / RAND_MAX * (b - a);
}
// 初始化屏幕和宠物尺寸，选择初始目标位置，并设置目标计时器
MotionSystem::MotionSystem(int sw, int sh, int w, int h)
    : screenW(sw), screenH(sh), petW(w), petH(h)
{
    ChooseRandomTarget(false);
    targetTimer = RandInt(200, 500);
}
// 选择一个新的随机目标位置，允许在屏幕外生成目标以增加逃逸行为的多样性
void MotionSystem::ChooseRandomTarget(bool allowOutside)
{
    if (allowOutside && RandDouble(0, 1) < 0.4)
    {
        int side = RandInt(0, 3);
        int margin = 50 + 50;
        if (side == 0)
        { // left
            targetX = -margin;
            targetY = RandInt(0, screenH - petH);
        }
        else if (side == 1)
        { // right
            targetX = screenW + margin;
            targetY = RandInt(0, screenH - petH);
        }
        else if (side == 2)
        { // top
            targetX = RandInt(0, screenW - petW);
            targetY = -margin;
        }
        else
		{ // bottom
            targetX = RandInt(0, screenW - petW);
            targetY = screenH + margin;
        }
    }
    else
    {
        targetX = RandInt(0, screenW - petW);
        targetY = RandInt(0, screenH - petH);
    }
}
// 处理边界碰撞和逃逸逻辑
void MotionSystem::HandleEdge()
{
    bool escaped = false;
    if (x < -petW || x > screenW) escaped = true;
    if (y < -petH || y > screenH) escaped = true;

    if (escaped)
    {
        if (RandDouble(0, 1) < 0.3)
        {
            int side = RandInt(0, 3);
            int margin = 50;
            if (side == 0) { x = -margin; y = RandInt(0, screenH - petH); }
            else if (side == 1) { x = screenW + margin; y = RandInt(0, screenH - petH); }
            else if (side == 2) { y = -margin; x = RandInt(0, screenW - petW); }
            else { y = screenH + margin; x = RandInt(0, screenW - petW); }
            vx = (RandInt(0, 1) == 0 ? -3 : 3);
            vy = RandInt(-2, 2);
        }
        else
        {
            vx = -vx;
            vy = -vy;
            if (x < 0) x = 0;
            if (x > screenW - petW) x = screenW - petW;
            if (y < 0) y = 0;
            if (y > screenH - petH) y = screenH - petH;
        }
    }
    else
    {
        if (x <= 0) { x = 0; vx = std::abs(vx); }
        if (x + petW >= screenW) { x = screenW - petW; vx = -std::abs(vx); }
        if (y <= 0) { y = 0; vy = std::abs(vy); }
        if (y + petH >= screenH) { y = screenH - petH; vy = -std::abs(vy); }
    }
}
// 应用外部速度（如拖动结束时的惯性），并设置标志以启用惯性滑动
void MotionSystem::ApplyExternalVelocity(double nvx, double nvy)
{
    vx = nvx;
    vy = nvy;
    externalVelocity = true;
}
// 更新位置和状态，处理跟随鼠标、漫游、休息等逻辑，并应用惯性和抖动
void MotionSystem::Update(bool followMouseEnabled, POINT mousePos)
{
    // 外部惯性滑动
    if (externalVelocity)
    {
        // 位置更新
        x += vx;
        y += vy;

        // 阻尼衰减
        vx *= 0.90;
        vy *= 0.90;

        // 速度很小就停止惯性
        if (fabs(vx) < 0.1 && fabs(vy) < 0.1)
        {
            externalVelocity = false;
            vx = vy = 0;
        }

        HandleEdge();  // 保留边缘处理
        return;        
    }
    // 休息状态
    if (state == MOTION_REST)
    {
        restTimer -= 30;
        if (restTimer <= 0)
        {
            state = MOTION_WANDER;
            ChooseRandomTarget(true);
            targetTimer = RandInt(200, 500);
        }
        return;
    }

    double dx = targetX - x;
    double dy = targetY - y;
    double dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 1) dist = 1;

    // 跟随逻辑
    if (!followMouseEnabled && (state == MOTION_FOLLOW || state == MOTION_CURIOUS))
    {
        state = MOTION_WANDER;
    }
	// 鼠标跟随逻辑
    if (followMouseEnabled)
    {
        double mx = mousePos.x;
        double my = mousePos.y;
        double mdx = mx - x;
        double mdy = my - y;
        double mdist = std::sqrt(mdx * mdx + mdy * mdy);

        if (mdist > cfg.followStartDist) state = MOTION_FOLLOW;
        else if (mdist < cfg.followStopDist) state = MOTION_CURIOUS;

        if (state == MOTION_FOLLOW || state == MOTION_CURIOUS)
        {
            int offset = (state == MOTION_FOLLOW) ? cfg.followDistance : cfg.followStopDist;
            targetX = mx + RandInt(-offset, offset);
            targetY = my + RandInt(-offset, offset);
            dx = targetX - x;
            dy = targetY - y;
            dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 1) dist = 1;
        }
    }
    else if (state == MOTION_WANDER && dist < cfg.restDistance)
    {
        if (RandDouble(0, 1) < 0.6)
        {
            state = MOTION_REST;
            restTimer = RandInt(1000, 3000);
            return;
        }
        else
        {
            ChooseRandomTarget(true);
            targetTimer = RandInt(200, 500);
        }
    }
	// 漫游状态定时更换目标
    if (state == MOTION_WANDER)
    {
        targetTimer--;
        if (targetTimer <= 0)
        {
            ChooseRandomTarget(true);
            targetTimer = RandInt(200, 500);
        }
    }

    double speedMul = 1.0;
    if (state == MOTION_WANDER) speedMul = 0.8;
    else if (state == MOTION_FOLLOW) speedMul = 1.2;
    else if (state == MOTION_CURIOUS) speedMul = 0.5;

    double desiredVx = dx / dist * cfg.speedX * speedMul;
    double desiredVy = dy / dist * cfg.speedY * speedMul;

    vx = vx * cfg.inertia + desiredVx * cfg.intent;
    vy = vy * cfg.inertia + desiredVy * cfg.intent;

    moveTick++;
    if (moveTick % 5 == 0)
    {
        jitterX = RandDouble(-cfg.jitter, cfg.jitter);
        jitterY = RandDouble(-cfg.jitter, cfg.jitter);
    }
    vx += jitterX;
    vy += jitterY;

    x += vx;
    y += vy;

    HandleEdge();
}
