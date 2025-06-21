#include "sound.h"
#include "tim.h"
#include "main.h"

void playSound(uint16_t freq, uint16_t duration_ms) {
    if (freq == 0) return;
    TIM2->ARR = (1000000UL / freq) - 1;
    TIM2->CCR1 = TIM2->ARR >> 1;  // 50% duty
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_Delay(duration_ms);
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
}

void playIntroMusic() {
	playSound(523, 200);  // C5
	    HAL_Delay(50);
	    playSound(659, 200);  // E5
	    HAL_Delay(50);
	    playSound(783, 200);  // G5
	    HAL_Delay(50);
	    playSound(1046, 250); // C6
	    HAL_Delay(100);

	    playSound(987, 200);  // B5
	    HAL_Delay(50);
	    playSound(880, 250);  // A5
	    HAL_Delay(150);
}

void soundPlayerHitTarget() {
    playSound(1200, 60);
    HAL_Delay(20);
    playSound(1400, 60);
}

void soundPlayerHitBarrier() {
    playSound(700, 50);
    HAL_Delay(30);
    playSound(650, 50);
}

void soundEnemyHitBarrier() {
	playSound(1050, 70);
	HAL_Delay(30);
	playSound(970, 70);
}

void soundEnemyHitPlayer() {
    playSound(400, 150);
    HAL_Delay(30);
    playSound(380, 150);
}

void soundVictory() {
	playSound(880, 150);
	HAL_Delay(50);
	playSound(988, 150);
	HAL_Delay(50);
	playSound(1046, 150);
	HAL_Delay(50);
	playSound(1175, 150);  // nốt sáng hơn
}

void soundGameOver() {
	playSound(800, 250);
	HAL_Delay(70);
	playSound(600, 250);
	HAL_Delay(70);
	playSound(400, 300);
}

void soundClick() {
    playSound(1200, 50);
}

void playSoundEvent(SoundEvent event) {
    switch (event) {
    	case SOUND_INTRO:             playIntroMusic(); break;
        case SOUND_HIT_ENEMY:         soundPlayerHitTarget(); break;
        case SOUND_HIT_BARRIER:       soundPlayerHitBarrier(); break;
        case SOUND_ENEMY_HIT_BARRIER: soundEnemyHitBarrier(); break;
        case SOUND_ENEMY_HIT_PLAYER:  soundEnemyHitPlayer(); break;
        case SOUND_WIN:               soundVictory(); break;
        case SOUND_LOSE:              soundGameOver(); break;
        case SOUND_UI_CLICK:          soundClick(); break;
    }
}
