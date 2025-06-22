# Nowhere To Run - Inspired by Space Invaders

> Graduation Capstone Project - Embedded Systems  
> Hanoi University of Science and Technology  
> School of Information and Communication Technology  

## Overview

**"Nowhere To Run"** is an embedded shooting game built on the STM32F429ZIT6 microcontroller, inspired by the classic **Space Invaders** arcade game. Players control a character named **"Phú ông"** (Rich Old Man) who must eliminate two enemies ("Cậu cả" and "Cậu út" – First and Second Son) by shooting "rocks" before they reach the bottom boundary or deplete the player’s health.

The game is developed using **FreeRTOS** for multitasking and **TouchGFX** for smooth GUI rendering. The system interacts with physical buttons, uses a buzzer for audio feedback, and integrates the STM32 hardware random number generator (RNG) for unpredictability.

## Features

### ✅ Functional Requirements

- **User Input**: Move character using two physical buttons connected to PG2 (LEFT) and PG3 (RIGHT).
- **Game Screens**:  
  - Home  
  - Intro  
  - Game Screen  
  - Game Over  
  - Victory
- **Enemy shooting**: Enemies move in a zigzag pattern and fire bullets randomly using hardware RNG.
- **Collision Handling**:  
  - Rock hits enemy → destroy enemy and score points  
  - Bullet hits barrier → barrier takes damage  
  - Enemy hits player → player loses health  
  - Enemy reaches bottom line → game over
- **Audio Feedback**: Buzzer (on PA5) plays sound on collisions.
- **Scoring System**:  
  - Score and high score tracking  
  - Level increases every 5 points  
  - Difficulty increases with each level
- **Multitasking with FreeRTOS**:  
  - `GUI_Task`: Game logic and rendering  
  - `DefaultTask`: Poll button input and send commands via RTOS message queue
- **Win/Lose Condition**: Game transitions to the Victory or Game Over screen accordingly.

## Key Game Logic Functions

- `handleTickEvent()`: Game loop called every 10 ticks
- `handleTulongMovement()`: Character movement based on queue messages
- `handleEnemyShooting()`: Random enemy shooting logic
- `handleCollisions()`, `handleStoneCollisions()`: AABB-based collision detection
- `triggerVictory()`, `triggerGameOver()`: End-game transitions
- `playSoundEffect()`: Buzzer control for sound feedback
- v.v

## Setup and Build

1. Open the project in **STM32CubeIDE**
2. Ensure **TouchGFX** is correctly installed
3. Connect STM32F429I-DISC1 board via USB
4. Build and flash the firmware to the board

## Repository

🔗 GitHub: [NowhereToRun-Inspired-by-Space-Invader](https://github.com/luquorus/NowhereToRun-Inspired-by-Space-Invader)

## Authors

- **Nguyễn Lan Nhi** – 20225991  
- **Bùi Hà My** – 20225987  

Instructor: **Ph.D. Ngô Lam Trung**

---

> 🛠️ Capstone Project for the Embedded System course, June 2025.
