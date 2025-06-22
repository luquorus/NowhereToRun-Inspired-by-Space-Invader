#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <touchgfx/Unicode.hpp>
#include "stm32f4xx_hal.h" // Để sử dụng HAL library cho RNG
#include "cmsis_os2.h" // Để sử dụng RTOS message queue

extern osMessageQueueId_t myQueue1Handle;

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}

    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    void triggerGameOver();
    void triggerVictory();
    void addScore(int amount);


protected:
protected:
    // Enemy Constants
    static const int NUM_ROWS = 3;
    static const int NUM_COLS = 6;
    static const int NUM_ENEMIES = NUM_ROWS * NUM_COLS;

    // Scoring
    int currentScore;
    int highScore;
    Unicode::UnicodeChar scoreBuffer[10];
    Unicode::UnicodeChar highScoreBuffer[10];

    int currentLevel;           // Level hiện tại
    int enemyShootCount;        // Số đạn enemy bắn mỗi lần


    // Enemy Image pointers & positions
    touchgfx::Image* enemies[NUM_ENEMIES];
    int enemyX[NUM_ENEMIES];
    int enemyY[NUM_ENEMIES];
    bool enemyAlive[NUM_ENEMIES];

    // Zigzag Movement
    bool moveRight;
    int zigzagStep;
    int dropStep;

    // Dimmed effect
    bool isDimming;
    int dimTickCount; // Đếm số lần tick nhấp nháy
    static const int MAX_DIM_TICKS = 5; // Số lần nhấp nháy

    // Enemy Shooting System
    bool starActive; // Trạng thái đạn có đang bay không
    int starX, starY; // Vị trí đạn
    int starSpeed; // Tốc độ đạn (pixel/tick)
    int shootCooldown; // Cooldown giữa các lần bắn
    static const int SHOOT_INTERVAL = 5; // Bắn mỗi 5 ticks
    static const int STAR_SPEED = 10; // Tốc độ đạn

    // Tulong Shooting System
    bool stoneActive; // Trạng thái đạn stone có đang bay không
    int stoneX, stoneY; // Vị trí đạn stone
    int stoneSpeed; // Tốc độ đạn stone (pixel/tick)
    int tulongShootCooldown; // Cooldown giữa các lần bắn của tulong
    static const int TULONG_SHOOT_INTERVAL = 2;
    static const int STONE_SPEED = -10; // Tốc độ đạn stone (âm vì bay lên trên)

    // Tulong Movement System
    int tulongX; // Vị trí X của tulong
    static const int TULONG_MOVE_STEP = 20; // Bước di chuyển của tulong (20px)
    static const int TULONG_MIN_X = 0; // Giới hạn trái
    static const int TULONG_MAX_X = 210; // Giới hạn phải (240 - width tulong)

    // Health System
    static const int MAX_HEALTH = 3; // Mạng sống tối đa
    int strawHealth[3]; // Mạng sống của straw1, straw2, straw3
    int tulongHealth; // Mạng sống của tulong
    bool strawAlive[3]; // Trạng thái sống của các straw
    bool tulongAlive; // Trạng thái sống của tulong

    // Collision Detection
    struct CollisionBox {
        int x, y, width, height;
    };

    // Helper methods for enemy shooting
    void handleEnemyShooting();
    void updateStarMovement();
    int getRandomAliveEnemy();
    uint32_t getHardwareRandom(); // Hardware RNG function
    void initHardwareRNG(); // Khởi tạo RNG

    // Helper methods for tulong shooting
    void handleTulongShooting();
    void updateStoneMovement();
    int getClosestEnemy(); // Tìm enemy gần nhất để bắn

    // Helper methods for tulong movement
    void handleTulongMovement(); // Xử lý di chuyển tulong từ queue

    // Optional helper methods for tulong
    int getTulongX(); // Get current tulong position
    bool isTulongAtLeftBoundary(); // Check if at left boundary
    bool isTulongAtRightBoundary(); // Check if at right boundary

    // Collision Detection Methods
    bool checkCollision(const CollisionBox& box1, const CollisionBox& box2);
    void handleCollisions();
    void handleStoneCollisions(); // Va chạm đạn stone của tulong
    CollisionBox getImageCollisionBox(touchgfx::Image* image);
    void damageStraw(int strawIndex);
    void damageTulong();
    void destroyEnemy(int enemyIndex); // Phá hủy enemy khi trúng đạn tulong
    void updateHealthDisplay();

    // Hardware RNG handle
    RNG_HandleTypeDef hrng;

    // Tick timing
    uint16_t tickCount;
    uint8_t speedMultiplier;
};

#endif // SCREEN1VIEW_HPP
