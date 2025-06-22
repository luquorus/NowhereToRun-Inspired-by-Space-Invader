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

    stoneActive = false;
    stoneX = 0;
    stoneY = 0;
    tulongShootCooldown = 0;

    tulongX = 0;

    for (int i = 0; i < 3; i++) {
        strawHealth[i] = 6;
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

    //Scoring
    currentScore = 0;

    score.setWildcard(scoreBuffer);
    Unicode::snprintf(scoreBuffer, 10, "%d", currentScore);
    score.invalidate();

    textArea1.setWildcard(highScoreBuffer); // <- highscore
    Unicode::snprintf(highScoreBuffer, 10, "%d", highScore);
    textArea1.invalidate();

    currentLevel = 1;
    enemyShootCount = 1; // level 1 chỉ bắn 1 viên
    speedMultiplier = 1;

    // Hiển thị text level
    Unicode::snprintf(levelBuffer, 10, "%d", currentLevel);
    level.setWildcard(levelBuffer);
    level.invalidate();


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

    // Khởi tạo đạn
    star.setVisible(false); // Ẩn đạn ban đầu
    starActive = false;

    stone.setVisible(false);
    stoneActive = false;

    tulongX = tulong.getX(); // Store initial position

    // Khởi tạo health system - mỗi straw có mạng riêng
    for (int i = 0; i < 3; i++) {
        strawHealth[i] = 6; // Mỗi straw có 6 mạng riêng
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
    invalidate();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleTickEvent()
{
    tickCount++;

    if (tickCount % 3 != 0) handleTulongMovement();

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

            triggerGameOver();

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

    osStatus_t status = osMessageQueueGet(myQueue1Handle, &cmd, NULL, 0);

    if (status == osOK)
    {
        if (!tulongAlive) return; // Don't move if tulong is dead

        int currentX = tulong.getX();
        int newX = currentX;

        switch(cmd)
        {
            case 1: // LEFT button pressed
                newX = currentX - 10;
                // Check left boundary
                if (newX < TULONG_MIN_X)
                {
                    newX = TULONG_MIN_X;
                }
                break;

            case 2: // RIGHT button pressed
                newX = currentX + 10;
                // Check right boundary
                if (newX > TULONG_MAX_X)
                {
                    newX = TULONG_MAX_X;
                }
                break;

            default:
                return;
        }

        // Update tulong position
        if (newX != currentX)
        {
            tulong.setX(newX);
            tulongX = newX; // Update stored position
            invalidate();
        }
    }
    else if (status == osErrorTimeout)
    {
        return;
    }
    else
    {
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
        for (int i = 0; i < enemyShootCount; i++)
        {
            int shooterIndex = getRandomAliveEnemy();
            if (shooterIndex != -1)
            {
                // Tính toán vị trí mới
                starX = enemyX[shooterIndex] + enemies[shooterIndex]->getWidth() / 2;
                starY = enemyY[shooterIndex] + enemies[shooterIndex]->getHeight();
                star.setXY(starX, starY);
                star.setVisible(true);
                starActive = true;
            }
        }

        shootCooldown = SHOOT_INTERVAL / speedMultiplier;
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
            addScore(1);
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

bool Screen1View::checkCollision(const CollisionBox& box1, const CollisionBox& box2)
{
    return !(box1.x + box1.width < box2.x ||
             box2.x + box2.width < box1.x ||
             box1.y + box1.height < box2.y ||
             box2.y + box2.height < box1.y);
}

Screen1View::CollisionBox Screen1View::getImageCollisionBox(touchgfx::Image* image)
{
    CollisionBox box;
    box.x = image->getX();
    box.y = image->getY();
    box.width = image->getWidth();
    box.height = image->getHeight();
    return box;
}

void Screen1View::handleCollisions()
{
    if (!starActive) return;

    // Tạo collision box cho star
    CollisionBox starBox;
    starBox.x = starX;
    starBox.y = starY;
    starBox.width = star.getWidth();
    starBox.height = star.getHeight();

    // Kiểm tra va chạm với straw barriers
    touchgfx::Image* straws[3] = {&straw1, &straw2, &straw3};

    for (int i = 0; i < 3; i++)
    {
        if (strawAlive[i] && straws[i]->isVisible())
        {
            CollisionBox strawBox = getImageCollisionBox(straws[i]);

            if (checkCollision(starBox, strawBox))
            {
                // Va chạm với straw
                damageStraw(i);

                // Ẩn đạn
                star.setVisible(false);
                starActive = false;
                return;
            }
        }
    }

    // Kiểm tra va chạm với tulong
    if (tulongAlive && tulong.isVisible())
    {
        CollisionBox tulongBox = getImageCollisionBox(&tulong);

        if (checkCollision(starBox, tulongBox))
        {
            // Va chạm với tulong
            damageTulong();

            // Ẩn đạn
            star.setVisible(false);
            starActive = false;
            return;
        }
    }
}

void Screen1View::handleStoneCollisions()
{
    if (!stoneActive) return;

    // Tạo collision box cho stone
    CollisionBox stoneBox;
    stoneBox.x = stoneX;
    stoneBox.y = stoneY;
    stoneBox.width = stone.getWidth();
    stoneBox.height = stone.getHeight();

    // Kiểm tra va chạm với enemies
    for (int i = 0; i < NUM_ENEMIES; ++i)
    {
        if (enemyAlive[i] && enemies[i]->isVisible())
        {
            CollisionBox enemyBox;
            enemyBox.x = enemyX[i];
            enemyBox.y = enemyY[i];
            enemyBox.width = enemies[i]->getWidth();
            enemyBox.height = enemies[i]->getHeight();

            if (checkCollision(stoneBox, enemyBox))
            {
                // Va chạm với enemy -> phá hủy enemy
                destroyEnemy(i);

                // Ẩn đạn stone
                stone.setVisible(false);
                stoneActive = false;
                return;
            }
        }
    }

    // Kiểm tra va chạm với straw barriers (tulong có thể bắn nhầm)
    touchgfx::Image* straws[3] = {&straw1, &straw2, &straw3};

    for (int i = 0; i < 3; i++)
    {
    	if (strawAlive[i] && straws[i]->isVisible())
    	{
			CollisionBox strawBox = getImageCollisionBox(straws[i]);

			if (checkCollision(stoneBox, strawBox))
			{
				// Va chạm với straw (friendly fire)
				damageStraw(i);

				// Ẩn đạn stone
				stone.setVisible(false);
				stoneActive = false;
				return;
			}
    	}
    }
}

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

        triggerGameOver();
    }

    updateHealthDisplay();
}

void Screen1View::destroyEnemy(int enemyIndex)
{
    if (enemyIndex < 0 || enemyIndex >= NUM_ENEMIES || !enemyAlive[enemyIndex]) return;

    // Enemy chỉ có 1 mạng duy nhất, trúng đạn sẽ bị phá hủy ngay
    enemyAlive[enemyIndex] = false;
    enemies[enemyIndex]->setVisible(false);

    addScore(1);

    // Kiểm tra nếu tất cả enemy đã chết và tulong còn sống
        bool allDead = true;
        for (int i = 0; i < NUM_ENEMIES; ++i)
        {
            if (enemyAlive[i])
            {
                allDead = false;
                break;
            }
        }

        if (allDead && tulongAlive)
        {
            triggerVictory();
        }
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

    }
}

uint32_t Screen1View::getHardwareRandom()
{
    uint32_t randomValue = 0;

    if (HAL_RNG_GenerateRandomNumber(&hrng, &randomValue) == HAL_OK)
    {
        return randomValue;
    }
    else
    {
        return (tickCount * 7 + 13); // Simple fallback
    }
}

void Screen1View::addScore(int amount)
{
    currentScore += amount;
    Unicode::snprintf(scoreBuffer, 10, "%d", currentScore);
    score.invalidate();

    if (currentScore > highScore) {
        highScore = currentScore;
        Unicode::snprintf(highScoreBuffer, 10, "%d", highScore);
        textArea1.invalidate();
    }

    // Tăng level mỗi 5 điểm
    int newLevel = currentScore / 5 + 1;
    if (newLevel > currentLevel && newLevel <= 5) {
        currentLevel = newLevel;

        // Tăng tốc độ và số đạn
        speedMultiplier++;
        enemyShootCount++;

        zigzagStep = 1 + speedMultiplier;
        dropStep = 1 + speedMultiplier;
        Unicode::snprintf(levelBuffer, 10, "%d", currentLevel);
        level.invalidate();
    }
}
void Screen1View::triggerGameOver()
{
    star.setVisible(false);
    stone.setVisible(false);
    starActive = false;
    stoneActive = false;

    application().gotoGameOverScreenNoTransition();
}

void Screen1View::triggerVictory()
{
    star.setVisible(false);
    stone.setVisible(false);
    starActive = false;
    stoneActive = false;

    application().gotoVictoryScreenNoTransition();
}

