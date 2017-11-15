#pragma once


enum class Scenario
{
    Task2A,
    Task2B,
    Task2C,
    Task3
};

enum class EnemyAI
{
    Simple,
    Normal,
    Good
};

struct Parameters
{
    // Current scenario
    Scenario scenario = Scenario::Task2A;
    // Current enemy AI
    EnemyAI enemy = EnemyAI::Simple;

    // Points of Player::Left
    int scoreLeft = 0;
    // Points of Player::Right
    int scoreRight = 0;

    // Width of the game field
    int fieldWidth = 1000;
    // Height of the game field
    int fieldHeight = 1000;

    // Cooldown in seconds until the next multi ball can be spawned
    // CD is reset if:
    // - a new ball is spawned
    // - a point was scored
    float multiBallCooldown = 0.0f;

    // Maximum paddle acceleration
    float paddleMaxAcceleration = 10000.0f;
    // Paddle velocity is dampened after each collision
    float paddleCollisionDampening = 0.5f;
    // v.x acceleration of balls
    float ballAcceleration = 200.0f;
    // start speed of new balls +- 20%
    float ballStartSpeed = 700.0f;
    // ball velocity is within +- angle at spawn
    float ballStartAngle = 30.0f;
    // ball linear drag
    float ballDrag = 0.1f;
    // paddles per side
    int paddlesLeft = 3;
    int paddlesRight = 3;
    // time in seconds until the next ball is spawned
    float multiBallTime = 5.0f;
};
