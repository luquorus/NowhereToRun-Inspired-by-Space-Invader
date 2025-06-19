#include <gui/screen1_screen/Screen1View.hpp>

Screen1View::Screen1View()
    : moveRight(true), zigzagStep(2), dropStep(10), tickCount(0), speedMultiplier(1)
{
	tickCount = 0;
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    // Gán enemy xen kẽ sb và c7
    enemies[0] = &sb1;  enemies[1] = &c71;  enemies[2] = &sb2;
    enemies[3] = &c72;  enemies[4] = &sb3;  enemies[5] = &c73;
    enemies[6] = &sb4;  enemies[7] = &c74;  enemies[8] = &sb5;
    enemies[9] = &c75;  enemies[10] = &sb6; enemies[11] = &c76;
    enemies[12] = &sb7; enemies[13] = &c77; enemies[14] = &sb8;
    enemies[15] = &c78; enemies[16] = &sb9; enemies[17] = &c79;

    // Vị trí bắt đầu và khoảng cách
    int startX = 20;
    int startY = 20;
    int spacingX = 35;
    int spacingY = 40;

    // Khởi tạo vị trí ban đầu cho từng enemy
    for (int row = 0; row < NUM_ROWS; ++row)
    {
        for (int col = 0; col < NUM_COLS; ++col)
        {
            int idx = row * NUM_COLS + col;
            enemyX[idx] = startX + col * spacingX;
            enemyY[idx] = startY + row * spacingY;
            enemies[idx]->setXY(enemyX[idx], enemyY[idx]);
            enemies[idx]->setVisible(true);
        }
    }

    invalidate(); // Cập nhật giao diện
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleTickEvent()
{

	tickCount++;
    // Chỉ di chuyển enemy mỗi 6 tick
    if (tickCount % 20 != 0) return;

    bool hitEdge = false;

    for (int i = 0; i < NUM_ENEMIES; ++i)
    {
        // Di chuyển theo hướng zigzag
        enemyX[i] += (moveRight ? zigzagStep : -zigzagStep);
        enemies[i]->setX(enemyX[i]);

        // Kiểm tra nếu chạm biên
        if (enemyX[i] <= 0 || enemyX[i] + enemies[i]->getWidth() >= 240)
        {
            hitEdge = true;
        }
    }

    if (hitEdge)
    {
        moveRight = !moveRight;
        for (int i = 0; i < NUM_ENEMIES; ++i)
        {
            enemyY[i] += dropStep;
            enemies[i]->setY(enemyY[i]);
        }
    }

    invalidate();
}
