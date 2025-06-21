#ifndef __SOUND_H
#define __SOUND_H

#include "main.h"

typedef enum {
	SOUND_INTRO,
    SOUND_HIT_ENEMY,
    SOUND_HIT_BARRIER,
    SOUND_ENEMY_HIT_BARRIER,
    SOUND_ENEMY_HIT_PLAYER,
    SOUND_WIN,
    SOUND_LOSE,
    SOUND_UI_CLICK

} SoundEvent;

void playSound(uint16_t freq, uint16_t duration_ms);
void playSoundEvent(SoundEvent event);

#endif // __SOUND_H
