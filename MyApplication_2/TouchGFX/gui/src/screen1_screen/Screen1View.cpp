#include <gui/screen1_screen/Screen1View.hpp>
#include <touchgfx/Unicode.hpp>
#include "string.h"
#include "math.h"
#include <climits>
#include "cmsis_os2.h" // Để sử dụng RTOS message queue

extern osMessageQueueId_t myQueue1Handle;


Screen1View::Screen1View()
: moveRight(true), zigzagStep(2), dropStep(10), tickCount(0), speedMultiplier(1),
isDimming(false), dimTickCount(0),
starActive(false), starX(0), starY(0), starSpeed(STAR_SPEED), shootCooldown(0),
stoneActive(false), stoneX(0), stoneY(0), stoneSpeed(STONE_SPEED), tulongShootCooldown(0),
tulongHealth(MAX_HEALTH), tulongAlive(true), tulongX(0)
{
    tickCount = 0;
    isDimming = false;
    dimTickCount = 0;

    // Khởi tạo hệ thống bắn của enemies
    starActive = false;
    starX = 0;
    starY = 0;
    shootCooldown = 0;

    // Khởi tạo hệ thống bắn của tulong
    stoneActive = false;
    stoneX = 0;
    stoneY = 0;
    tulongShootCooldown = 0;

    // Khởi tạo tulong movement
    tulongX = 0;

    // Khởi tạo health system - mỗi straw có 3 mạng riêng biệt
    for (int i = 0; i < 3; i++) {
        strawHealth[i] = MAX_HEALTH; // Straw 1, 2, 3 mỗi cái có 3 mạng riêng
        strawAlive[i] = true;
    }
    tulongHealth = MAX_HEALTH;
    tulongAlive = true;

    // Khởi tạo Hardware RNG
    initHardwareRNG();
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    // Gán enemy xen kẽ sb và c7
    enemies[0] = &sb1; enemies[1] = &c71; enemies[2] = &sb2;
    enemies[3] = &c72; enemies[4] = &sb3; enemies[5] = &c73;
    enemies[6] = &sb4; enemies[7] = &c74; enemies[8] = &sb5;
    enemies[9] = &c75; enemies[10] = &sb6; enemies[11] = &c76;
    enemies[12] = &sb7; enemies[13] = &c77; enemies[14] = &sb8;
    enemies[15] = &c78; enemies[16] = &sb9; enemies[17] = &c79;

    // Vị trí bắt đầu và khoảng cách
    int startX = 20;
    int startY = 20;
    int spacingX = 35;
    int spacingY = 40;

    // Khởi tạo vị trí ban đầu và trạng thái sống cho từng enemy
    for (int row = 0; row < NUM_ROWS; ++row)
    {
        for (int col = 0; col < NUM_COLS; ++col)
        {
            int idx = row * NUM_COLS + col;
            enemyX[idx] = startX + col * spacingX;
            enemyY[idx] = startY + row * spacingY;
            enemies[idx]->setXY(enemyX[idx], enemyY[idx]);
            enemies[idx]->setVisible(true);
            enemyAlive[idx] = true;
        }
    }

    // Khởi tạo đạn star (enemies)
    star.setVisible(false); // Ẩn đạn ban đầu
    starActive = false;

    // Khởi tạo đạn stone (tulong)
    stone.setVisible(false); // Ẩn đạn ban đầu
    stoneActive = false;

    // Khởi tạo tulong position
    tulongX = tulong.getX(); // Store initial position

    // Khởi tạo health system - mỗi straw có mạng riêng
    for (int i = 0; i < 3; i++) {
        strawHealth[i] = MAX_HEALTH; // Mỗi straw có 3 mạng riêng
        strawAlive[i] = true;
    }
    tulongHealth = MAX_HEALTH;
    tulongAlive = true;

    // Hiển thị các barrier và tulong
    straw1.setVisible(strawAlive[0]);
    straw2.setVisible(strawAlive[1]);
    straw3.setVisible(strawAlive[2]);
    tulong.setVisible(tulongAlive);

    updateHealthDisplay();
    invalidate(); // Cập nhật giao diện
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleTickEvent()
{
    tickCount++;

    // Handle tulong movement from queue messages EVERY tick for responsiveness
    handleTulongMovement();

    // Chỉ xử lý game logic mỗi 10 ticks
    if (tickCount % 10 != 0) return;

    // Nếu đang trong quá trình dimming -> nhấp nháy
    if (isDimming)
    {
        dimTickCount++;

        // Toggle hiển thị enemy để nhấp nháy
        for (int i = 0; i < NUM_ENEMIES; ++i)
        {
            if (enemyAlive[i])
            {
                bool visible = enemies[i]->isVisible();
                enemies[i]->setVisible(!visible);
            }
        }

        if (dimTickCount >= MAX_DIM_TICKS)
        {
            // Kết thúc nhấp nháy, ẩn tất cả enemy
            for (int j = 0; j < NUM_ENEMIES; ++j)
            {
                enemyAlive[j] = false;
                enemies[j]->setVisible(false);
            }

            isDimming = false;
            // Ẩn đạn khi game kết thúc
            star.setVisible(false);
            starActive = false;
            stone.setVisible(false);
            stoneActive = false;
        }

        invalidate();
        return;
    }

    bool hitEdge = false;

    // Di chuyển enemies
    for (int i = 0; i < NUM_ENEMIES; ++i)
    {
        if (enemyAlive[i])
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
    }

    if (hitEdge)
    {
        moveRight = !moveRight;
        for (int i = 0; i < NUM_ENEMIES; ++i)
        {
            if (enemyAlive[i])
            {
                enemyY[i] += dropStep;
                enemies[i]->setY(enemyY[i]);
            }
        }
    }

    // Xử lý hệ thống bắn của enemies
    handleEnemyShooting();

    // Xử lý hệ thống bắn của tulong
    handleTulongShooting();

    // Cập nhật chuyển động đạn
    updateStarMovement();
    updateStoneMovement();

    // Xử lý collision detection
    handleCollisions();
    handleStoneCollisions();

    // Kiểm tra va chạm với line
    for (int i = 0; i < NUM_ENEMIES; ++i)
    {
        if (enemyAlive[i])
        {
            int enemyBottom = enemyY[i] + enemies[i]->getHeight();
            int lineY = line.getY();

            if (enemyBottom >= lineY)
            {
                // Enemy chạm vào line -> bắt đầu hiệu ứng dimming
                isDimming = true;
                dimTickCount = 0;
                return;
            }
        }
    }

    invalidate(); // Cập nhật lại giao diện
}

void Screen1View::handleTulongMovement()
{
    uint8_t cmd;

    // Try to receive message from queue (non-blocking)
    osStatus_t status = osMessageQueueGet(myQueue1Handle, &cmd, NULL, 0);

    if (status == osOK)
    {
        if (!tulongAlive) return; // Don't move if tulong is dead

        int currentX = tulong.getX();
        int newX = currentX;

        switch(cmd)
        {
            case 1: // LEFT button pressed
                newX = currentX - 10; // 10px như bạn yêu cầu
                // Check left boundary
                if (newX < TULONG_MIN_X)
                {
                    newX = TULONG_MIN_X;
                }
                break;

            case 2: // RIGHT button pressed
                newX = currentX + 10; // 10px như bạn yêu cầu
                // Check right boundary
                if (newX > TULONG_MAX_X)
                {
                    newX = TULONG_MAX_X;
                }
                break;

            default:
                // Invalid command, do nothing
                return;
        }

        // Update tulong position
        if (newX != currentX)
        {
            tulong.setX(newX);
            tulongX = newX; // Update stored position
            invalidate(); // Force redraw immediately
        }
    }
    else if (status == osErrorTimeout)
    {
        // No message available, this is normal
        return;
    }
    else
    {
        // Other error occurred
        // Handle error if needed
        return;
    }
}

void Screen1View::handleEnemyShooting()
{
    // Giảm cooldown
    if (shootCooldown > 0)
    {
        shootCooldown--;
    }

    // Nếu không có đạn đang bay và đã hết cooldown
    if (!starActive && shootCooldown <= 0)
    {
        int shooterIndex = getRandomAliveEnemy();

        if (shooterIndex != -1)
        {
            // Tạo đạn mới từ vị trí enemy
            starX = enemyX[shooterIndex] + enemies[shooterIndex]->getWidth() / 2;
            starY = enemyY[shooterIndex] + enemies[shooterIndex]->getHeight();

            star.setXY(starX, starY);
            star.setVisible(true);
            starActive = true;

            // Reset cooldown
            shootCooldown = SHOOT_INTERVAL;
        }
    }
}

void Screen1View::handleTulongShooting()
{
    // Giảm cooldown
    if (tulongShootCooldown > 0)
    {
        tulongShootCooldown--;
    }

    // Nếu tulong còn sống, không có đạn stone đang bay và đã hết cooldown
    if (tulongAlive && !stoneActive && tulongShootCooldown <= 0)
    {
        int targetIndex = getClosestEnemy();

        if (targetIndex != -1)
        {
            // Tạo đạn stone mới từ vị trí tulong
            stoneX = tulong.getX() + tulong.getWidth() / 2;
            stoneY = tulong.getY(); // Bắn từ đỉnh tulong

            stone.setXY(stoneX, stoneY);
            stone.setVisible(true);
            stoneActive = true;

            // Reset cooldown
            tulongShootCooldown = TULONG_SHOOT_INTERVAL;
        }
    }
}

void Screen1View::updateStarMovement()
{
    if (starActive)
    {
        // Di chuyển đạn xuống dưới
        starY += starSpeed;
        star.setY(starY);

        // Kiểm tra nếu đạn ra khỏi màn hình
        if (starY > 320) // Giả sử chiều cao màn hình là 320
        {
            star.setVisible(false);
            starActive = false;
        }
    }
}

void Screen1View::updateStoneMovement()
{
    if (stoneActive)
    {
        // Di chuyển đạn lên trên (stoneSpeed là âm)
        stoneY += stoneSpeed;
        stone.setY(stoneY);

        // Kiểm tra nếu đạn ra khỏi màn hình
        if (stoneY < 0)
        {
            stone.setVisible(false);
            stoneActive = false;
        }
    }
}

//Collision here

void Screen1View::damageStraw(int strawIndex)
{
    if (strawIndex < 0 || strawIndex >= 3 || !strawAlive[strawIndex]) return;

    strawHealth[strawIndex]--;

    if (strawHealth[strawIndex] <= 0)
    {
        // Straw bị phá hủy
        strawAlive[strawIndex] = false;

        // Ẩn straw tương ứng
        switch (strawIndex)
        {
            case 0:
                straw1.setVisible(false);
                break;
            case 1:
                straw2.setVisible(false);
                break;
            case 2:
                straw3.setVisible(false);
                break;
        }
    }

    updateHealthDisplay();
}

void Screen1View::damageTulong()
{
    if (!tulongAlive) return;

    tulongHealth--;

    if (tulongHealth <= 0)
    {
        // Tulong bị phá hủy
        tulongAlive = false;
        tulong.setVisible(false);

        // Ẩn đạn stone nếu có
        if (stoneActive)
        {
            stone.setVisible(false);
            stoneActive = false;
        }

        // TODO: Thêm logic game over ở đây nếu cần
    }

    updateHealthDisplay();
}

void Screen1View::destroyEnemy(int enemyIndex)
{
    if (enemyIndex < 0 || enemyIndex >= NUM_ENEMIES || !enemyAlive[enemyIndex]) return;

    // Enemy chỉ có 1 mạng duy nhất, trúng đạn sẽ bị phá hủy ngay
    enemyAlive[enemyIndex] = false;
    enemies[enemyIndex]->setVisible(false);

    // TODO: Có thể thêm hiệu ứng explosion hoặc animation ở đây
    // TODO: Thêm điểm số khi tiêu diệt enemy
}

void Screen1View::updateHealthDisplay()
{
    // Cập nhật hiển thị mạng sống
    // Hiển thị mạng sống tulong
    hp1.setVisible(tulongHealth >= 1);
    hp2.setVisible(tulongHealth >= 2);
    hp3.setVisible(tulongHealth >= 3);
}

int Screen1View::getRandomAliveEnemy()
{
    // Tạo danh sách các enemy còn sống
    int aliveEnemies[NUM_ENEMIES];
    int aliveCount = 0;

    for (int i = 0; i < NUM_ENEMIES; ++i)
    {
        if (enemyAlive[i])
        {
            aliveEnemies[aliveCount] = i;
            aliveCount++;
        }
    }

    // Nếu không có enemy nào sống
    if (aliveCount == 0)
        return -1;

    // Sử dụng Hardware Random Generator để chọn enemy
    uint32_t randomValue = getHardwareRandom();
    int randomIndex = randomValue % aliveCount;
    return aliveEnemies[randomIndex];
}

int Screen1View::getClosestEnemy()
{
    // Tìm enemy gần nhất với tulong để bắn
    int closestIndex = -1;
    int minDistance = INT_MAX;
    int tulongCenterX = tulong.getX() + tulong.getWidth() / 2;

    for (int i = 0; i < NUM_ENEMIES; ++i)
    {
        if (enemyAlive[i])
        {
            int enemyCenterX = enemyX[i] + enemies[i]->getWidth() / 2;
            int distance = abs(enemyCenterX - tulongCenterX);

            if (distance < minDistance)
            {
                minDistance = distance;
                closestIndex = i;
            }
        }
    }

    return closestIndex;
}

// Optional helper methods for tulong
int Screen1View::getTulongX()
{
    return tulong.getX();
}

bool Screen1View::isTulongAtLeftBoundary()
{
    return tulong.getX() <= TULONG_MIN_X;
}

bool Screen1View::isTulongAtRightBoundary()
{
    return tulong.getX() >= TULONG_MAX_X;
}

void Screen1View::initHardwareRNG()
{
    // Khởi tạo RNG peripheral
    hrng.Instance = RNG;

    // Enable RNG clock
    __HAL_RCC_RNG_CLK_ENABLE();

    // Initialize RNG
    if (HAL_RNG_Init(&hrng) != HAL_OK)
    {
        // Handle initialization error
        // Có thể log error hoặc fallback về random method khác
    }
}

uint32_t Screen1View::getHardwareRandom()
{
    uint32_t randomValue = 0;

    // Generate random number using hardware RNG
    if (HAL_RNG_GenerateRandomNumber(&hrng, &randomValue) == HAL_OK)
    {
        return randomValue;
    }
    else
    {
        // Fallback nếu hardware RNG fail
        // Sử dụng method cũ hoặc một giá trị mặc định
//        return (tickCount * 7 + 13); // Simple fallback
    }
}
