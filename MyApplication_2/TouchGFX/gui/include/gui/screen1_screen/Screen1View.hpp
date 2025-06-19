#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}

    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent(); // Để cập nhật chuyển động zigzag

protected:
    // Enemy Constants
    static const int NUM_ROWS = 3;
    static const int NUM_COLS = 6;
    static const int NUM_ENEMIES = NUM_ROWS * NUM_COLS;

    // Enemy Image pointers & positions
    touchgfx::Image* enemies[NUM_ENEMIES];
    int enemyX[NUM_ENEMIES];
    int enemyY[NUM_ENEMIES];

    // Zigzag Movement
    bool moveRight;
    int zigzagStep;
    int dropStep;

    // Tick timing
    uint16_t tickCount;
    uint8_t speedMultiplier;
};

#endif // SCREEN1VIEW_HPP
