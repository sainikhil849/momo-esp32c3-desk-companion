// =====================================================================
//  games.h  -  games you play with Momo (best scores are saved) and the
//              Pong game Momo plays by itself when bored
//  Jump    : tap to jump over the blocks
//  Flappy  : tap to flap through the gaps
//  Catch   : tilt Momo left/right to catch the stars (taps work too)
//  Reaction: tap as soon as Momo's eyes pop open
//  Simon   : Momo shows tap / tilt left / tilt right moves, you repeat them
//  Copy me : do the move Momo calls (tap, tilt, shake) before time runs out
//  Rhythm  : Momo beeps a rhythm, you tap it back
//  Rock paper scissors: tilt left = rock, tap = paper, tilt right = scissors
// =====================================================================

enum GameId : uint8_t { G_JUMP, G_FLAPPY, G_CATCH, G_REACT, G_SIMON, G_COPY, G_RHYTHM, G_RPS, G_COUNT };
enum GState : uint8_t { GS_READY, GS_PLAY, GS_OVER };
const char *GAME_NAMES[G_COUNT] = {"Jump", "Flappy", "Catch", "Reaction", "Simon", "Copy me", "Rhythm", "Rock paper"};

uint8_t gameId = G_JUMP;
GState gState = GS_READY;
uint16_t gScore = 0;
bool gNewBest = false;
unsigned long gStateAt = 0, gLastStep = 0;
const unsigned long GAME_STEP = 30;   // physics step (ms)
const float FLAP_GAP = 14;            // Flappy: half the height of the gap

// shared objects
struct GObj { float x, y, w, h, v; bool on; };
GObj gObs[4];
float gPy = 0, gPv = 0, gSpeed = 0;
uint8_t gLives = 3;
float gPaddle = 64, gPaddleDir = 1;
// reaction game
enum ReactState : uint8_t { RG_WAIT, RG_GO, RG_EARLY };
uint8_t reactState = RG_WAIT;
unsigned long reactAt = 0;

// Simon, Copy me, Rhythm, Rock paper scissors
enum Move : uint8_t { MV_TAP, MV_LEFT, MV_RIGHT, MV_SHAKE };
const uint16_t MOVE_TONE[4] = {1047, 784, 1319, 523};
uint8_t gSeq[40];
uint8_t gSeqLen = 0, gSeqPos = 0, gPhase = 0;
unsigned long gPhaseAt = 0, gDeadline = 0;
int8_t gFlash = -1;
unsigned long gFlashUntil = 0;
uint8_t gCmd = 0;
uint16_t gWindow = 2600;
uint8_t gRound = 0, gMomoFace = 255;
unsigned long gMomoFaceUntil = 0;
uint16_t rGap[8];
uint8_t rBeats = 0, rTaps = 0, rStars = 0, rAcc = 0;
unsigned long rTap[8];
uint8_t rpsYou = 255, rpsMomo = 255, rpsMomoWins = 0;
int8_t rpsResult = 0;     // 1 you win, 0 draw, -1 Momo wins

// Pong that Momo plays by itself
float ballX = 64, ballY = 32, ballVX = 1.6f, ballVY = 1.1f, padL = 24, padR = 24;

float frand(float lo, float hi) { return lo + (hi - lo) * (random(0, 10000) / 10000.0f); }

void gameReset() {
  for (int i = 0; i < 4; i++) gObs[i].on = false;
  gScore = 0;
  gNewBest = false;
  gLives = 3;
  gPv = 0;
  gSeqLen = gSeqPos = gPhase = gRound = 0;
  gFlash = -1;
  gMomoFace = 255;
  rTaps = rStars = rAcc = 0;
  rpsMomoWins = 0;
  rpsYou = rpsMomo = 255;
  switch (gameId) {
    case G_JUMP:   gPy = 0; gSpeed = 2.2f; break;
    case G_FLAPPY: gPy = 30; gSpeed = 1.5f; break;
    case G_CATCH:  gPaddle = 64; gSpeed = 0.9f; break;
    default: break;
  }
}

// ---------- helpers for the move games ----------
void momoMood(uint8_t face, unsigned long ms, unsigned long now) {   // Momo's little face in the corner + its sound
  gMomoFace = face;
  gMomoFaceUntil = now + ms;
  playFaceSound(face);
}

void flashMove(uint8_t mv, unsigned long ms, unsigned long now) {
  gFlash = (int8_t)mv;
  gFlashUntil = now + ms;
  playGameTone(MOVE_TONE[mv & 3], (uint16_t)min(260UL, ms));
}

void copyNewCommand(unsigned long now) {
  uint8_t c = (uint8_t)random(0, 4);
  if (c == gCmd && random(0, 2)) c = (uint8_t)((c + 1 + random(0, 3)) % 4);   // fewer repeats
  gCmd = c;
  gWindow = (uint16_t)max(900, 2600 - (int)gScore * 80);
  gDeadline = now + gWindow;
  gPhase = 1;
  gPhaseAt = now;
  playGameTone(MOVE_TONE[c], 70);
}

void rhythmNewPattern(unsigned long now) {
  gRound++;
  rBeats = (uint8_t)min(8, 3 + (int)gRound);
  for (uint8_t i = 0; i + 1 < rBeats; i++) rGap[i] = (uint16_t)(250 * random(1, 4));   // 250, 500 or 750 ms
  rTaps = 0;
  gSeqPos = 0;
  gPhase = 0;          // listen
  gPhaseAt = now;
}

void rpsNewRound(unsigned long now) {
  gPhase = 0;          // "rock... paper... scissors..."
  gSeqPos = 0;
  gPhaseAt = now;
  rpsYou = rpsMomo = 255;
}

void gameOver(unsigned long now);

void moveGameStart(unsigned long now) {
  switch (gameId) {
    case G_SIMON:
      gSeqLen = 1; gSeq[0] = (uint8_t)random(0, 3);
      gSeqPos = 0; gPhase = 0; gPhaseAt = now;
      break;
    case G_COPY:   gPhase = 2; gPhaseAt = now; break;   // first command after a short pause
    case G_RHYTHM: gRound = 0; rhythmNewPattern(now); break;
    case G_RPS:    rpsNewRound(now); break;
    default: break;
  }
}

void rhythmCheck(unsigned long now) {
  if (rTaps < rBeats) { rStars = 0; rAcc = 0; momoMood(F_SAD, 1500, now); gameOver(now); return; }
  long sumE = 0, sumT = 0;
  for (uint8_t j = 1; j < rBeats; j++) {
    long u = (long)(rTap[j] - rTap[j - 1]), t = rGap[j - 1];
    sumE += labs(u - t);
    sumT += t;
  }
  int acc = sumT > 0 ? 100 - (int)(sumE * 100 / sumT) : 0;
  rAcc = (uint8_t)constrain(acc, 0, 100);
  rStars = rAcc >= 90 ? 3 : rAcc >= 80 ? 2 : rAcc >= 70 ? 1 : 0;
  if (rStars == 0) { momoMood(F_SAD, 1500, now); gameOver(now); return; }
  gScore += rStars;
  gPhase = 2;          // result
  gPhaseAt = now;
  momoMood(rStars == 3 ? F_EXCITED : F_HAPPY, 1500, now);
}

void rpsResolve(uint8_t you, unsigned long now) {
  rpsYou = you;
  rpsMomo = (uint8_t)random(0, 3);
  if (you > 2) rpsResult = -1;
  else {
    int d = ((int)you - (int)rpsMomo + 3) % 3;
    rpsResult = d == 1 ? 1 : d == 0 ? 0 : -1;
  }
  if (rpsResult > 0) { gScore++; momoMood(random(0, 2) ? F_DISAPPOINTED : F_ANNOYED, 1800, now); }
  else if (rpsResult < 0) { rpsMomoWins++; momoMood(random(0, 2) ? F_MISCHIEVOUS : F_LAUGH, 1800, now); }
  else momoMood(F_CONFUSED, 1800, now);
  gPhase = 2;
  gPhaseAt = now;
}

// A move from you: tap, tilt left/right or shake
void gameInput(uint8_t mv, unsigned long now) {
  if (gState != GS_PLAY) return;
  switch (gameId) {
    case G_SIMON:
      if (gPhase != 1 || mv == MV_SHAKE) return;
      flashMove(mv, 250, now);
      if (mv == gSeq[gSeqPos]) {
        gSeqPos++;
        gDeadline = now + 5000;
        if (gSeqPos >= gSeqLen) {
          gScore = gSeqLen;
          gPhase = 2;
          gPhaseAt = now;
          momoMood(gSeqLen % 5 == 0 ? F_EXCITED : F_HAPPY, 900, now);
        }
      } else {
        momoMood(F_SAD, 1500, now);
        gameOver(now);
      }
      break;
    case G_COPY:
      if (gPhase != 1) return;
      if (mv == gCmd) {
        gScore++;
        gFlash = (int8_t)mv;
        gFlashUntil = now + 300;
        playSound(S_POINT);
        gPhase = 2;
        gPhaseAt = now;
      } else {
        momoMood(F_LAUGH, 1500, now);
        gameOver(now);
      }
      break;
    case G_RHYTHM:
      if (mv != MV_TAP || gPhase != 1 || rTaps >= rBeats) return;
      rTap[rTaps++] = now;
      gFlash = MV_TAP;
      gFlashUntil = now + 120;
      playGameTone(1047, 60);
      if (rTaps >= rBeats) rhythmCheck(now);
      break;
    case G_RPS:
      if (gPhase != 1) return;
      if (mv == MV_LEFT) rpsResolve(0, now);
      else if (mv == MV_TAP) rpsResolve(1, now);
      else if (mv == MV_RIGHT) rpsResolve(2, now);
      break;
    default: break;
  }
}

void gameTilt(int8_t dir, unsigned long now) {
  if (gState == GS_PLAY) gameInput(dir > 0 ? MV_RIGHT : MV_LEFT, now);
}

void gameShake(unsigned long now) {
  if (gState == GS_PLAY) gameInput(MV_SHAKE, now);
}

// Timing of the move games (called every 30 ms while playing)
void moveGameStep(unsigned long now) {
  switch (gameId) {
    case G_SIMON: {
      unsigned long per = (unsigned long)max(380, 760 - 25 * (int)gSeqLen);
      if (gPhase == 0) {                                   // Momo shows the moves
        if (gSeqPos < gSeqLen && (long)(now - (gPhaseAt + 500 + gSeqPos * per)) >= 0) {
          flashMove(gSeq[gSeqPos], per * 6 / 10, now);
          gSeqPos++;
        } else if (gSeqPos >= gSeqLen && (long)(now - (gPhaseAt + 500 + gSeqLen * per)) >= 0) {
          gPhase = 1; gSeqPos = 0; gDeadline = now + 5000;
        }
      } else if (gPhase == 1) {                            // your turn
        if ((long)(now - gDeadline) > 0) { momoMood(F_SLEEPY, 1500, now); gameOver(now); }
      } else if ((long)(now - gPhaseAt) > 900) {                   // next round, one move longer
        if (gSeqLen < sizeof(gSeq)) gSeq[gSeqLen++] = (uint8_t)random(0, 3);
        gSeqPos = 0; gPhase = 0; gPhaseAt = now;
      }
      break;
    }
    case G_COPY:
      if (gPhase == 1 && (long)(now - gDeadline) > 0) { momoMood(F_LAUGH, 1500, now); gameOver(now); }
      else if (gPhase == 2 && (long)(now - gPhaseAt) > 450) copyNewCommand(now);
      break;
    case G_RHYTHM:
      if (gPhase == 0) {                                   // Momo plays the rhythm
        unsigned long at = gPhaseAt + 700;
        for (uint8_t j = 0; j < gSeqPos && j + 1 < rBeats; j++) at += rGap[j];
        if (gSeqPos < rBeats && (long)(now - at) >= 0) {
          gFlash = MV_TAP; gFlashUntil = now + 120;
          playGameTone(880, 60);
          gSeqPos++;
          if (gSeqPos >= rBeats) { gPhase = 3; gPhaseAt = now; }
        }
      } else if (gPhase == 3) {                            // short pause, then your turn
        if ((long)(now - gPhaseAt) > 700) { gPhase = 1; gPhaseAt = now; rTaps = 0; }
      } else if (gPhase == 1) {
        if ((rTaps == 0 && (long)(now - gPhaseAt) > 4000) || (rTaps > 0 && (long)(now - rTap[rTaps - 1]) > 2500)) rhythmCheck(now);
      } else if (gPhase == 2 && (long)(now - gPhaseAt) > 1600) {
        rhythmNewPattern(now);
      }
      break;
    case G_RPS:
      if (gPhase == 0) {                                   // rock... paper... scissors...
        if (gSeqPos < 3 && (long)(now - (gPhaseAt + 300 + gSeqPos * 450UL)) >= 0) {
          playGameTone((uint16_t)(784 + 200 * gSeqPos), 90);
          gSeqPos++;
        } else if (gSeqPos >= 3 && (long)(now - (gPhaseAt + 300 + 3 * 450UL)) >= 0) {
          gPhase = 1; gPhaseAt = now; gDeadline = now + 2500;
          playGameTone(1568, 120);
        }
      } else if (gPhase == 1) {
        if ((long)(now - gDeadline) > 0) rpsResolve(255, now);   // too slow: Momo wins the round
      } else if ((long)(now - gPhaseAt) > 2000) {
        if (rpsMomoWins >= 3) gameOver(now);
        else rpsNewRound(now);
      }
      break;
    default: break;
  }
}

void gameStart(uint8_t id, unsigned long now) {
  gameId = id;
  gState = GS_READY;
  gStateAt = now;
  gameReset();
}

void gamePlay(unsigned long now) {
  gameReset();
  gState = GS_PLAY;
  gStateAt = now;
  gLastStep = now;
  if (gameId == G_REACT) {
    reactState = RG_WAIT;
    reactAt = now + (unsigned long)random(1500, 4000);
  }
  playSound(S_GAME_START);
  moveGameStart(now + 600);   // the move games begin after the start sound
}

void gameOver(unsigned long now) {
  gState = GS_OVER;
  gStateAt = now;
  bool better;
  if (gameId == G_REACT) better = gScore > 0 && (st.best[3] == 0 || gScore < st.best[3]);
  else better = gScore > bestScore(gameId);
  if (better) {
    bestScore(gameId) = gScore;
    gNewBest = true;
    markStats(now);
    playSound(S_NEW_BEST);
  } else {
    playSound(S_GAME_OVER);
  }
  Serial.printf("Game %s over: score %u%s\n", GAME_NAMES[gameId], (unsigned)gScore, gNewBest ? " (new best!)" : "");
}

// Called the moment the touch sensor is pressed (fast, for jumping/flapping)
void gamePress(unsigned long now) {
  if (gState == GS_READY) { gamePlay(now); return; }
  if (gState == GS_OVER) {
    if (now - gStateAt > 700) gamePlay(now);   // short pause so a late tap does not restart by accident
    return;
  }
  switch (gameId) {
    case G_JUMP:
      if (gPy <= 0.01f) { gPv = 6.4f; playSound(S_CLICK); }
      break;
    case G_FLAPPY:
      gPv = -2.6f;
      playSound(S_CLICK);
      break;
    case G_CATCH:
      gPaddleDir = -gPaddleDir;                      // taps steer if you do not tilt
      break;
    case G_SIMON: case G_COPY: case G_RHYTHM: case G_RPS:
      gameInput(MV_TAP, now);
      break;
    case G_REACT:
      if (reactState == RG_WAIT) {
        reactState = RG_EARLY;
        gScore = 0;
        playSound(S_SAD);
        gState = GS_OVER;
        gStateAt = now;
      } else if (reactState == RG_GO) {
        gScore = (uint16_t)min(9999UL, now - reactAt);
        gameOver(now);
      }
      break;
    default: break;
  }
}

bool hit(float ax0, float ay0, float aw, float ah, float bx0, float by0, float bw, float bh) {
  return ax0 < bx0 + bw && ax0 + aw > bx0 && ay0 < by0 + bh && ay0 + ah > by0;
}

void gameStep(unsigned long now, float tilt) {
  switch (gameId) {
    case G_JUMP: {
      // player: x 14..24, bottom at 56 - gPy
      gPy += gPv;
      gPv -= 0.55f;
      if (gPy <= 0) { gPy = 0; gPv = 0; }
      float rightmost = -100;
      for (int i = 0; i < 4; i++) {
        if (!gObs[i].on) continue;
        gObs[i].x -= gSpeed;
        if (gObs[i].x + gObs[i].w < 14 && gObs[i].v == 0) { gObs[i].v = 1; gScore++; playSound(S_POINT); gSpeed += 0.06f; }
        if (gObs[i].x < -12) gObs[i].on = false;
        rightmost = max(rightmost, gObs[i].x);
        if (hit(15, 46 - gPy, 8, 9, gObs[i].x, 56 - gObs[i].h, gObs[i].w, gObs[i].h)) { gameOver(now); return; }
      }
      if (rightmost < 128 - frand(55, 95)) {
        for (int i = 0; i < 4; i++) if (!gObs[i].on) {
          gObs[i].on = true; gObs[i].x = 130; gObs[i].w = frand(5, 10); gObs[i].h = frand(7, 17); gObs[i].v = 0;
          break;
        }
      }
      break;
    }
    case G_FLAPPY: {
      gPv += 0.25f;
      gPy += gPv;
      if (gPy < 0 || gPy > 58) { gameOver(now); return; }
      float rightmost = -100;
      for (int i = 0; i < 4; i++) {
        if (!gObs[i].on) continue;
        gObs[i].x -= gSpeed;
        if (gObs[i].x + 10 < 26 && gObs[i].v == 0) { gObs[i].v = 1; gScore++; playSound(S_POINT); gSpeed += 0.04f; }
        if (gObs[i].x < -12) gObs[i].on = false;
        rightmost = max(rightmost, gObs[i].x);
        float gapTop = gObs[i].y - FLAP_GAP, gapBot = gObs[i].y + FLAP_GAP;
        if (hit(27, gPy - 3, 7, 6, gObs[i].x, 0, 10, gapTop) || hit(27, gPy - 3, 7, 6, gObs[i].x, gapBot, 10, 64 - gapBot)) {
          gameOver(now);
          return;
        }
      }
      if (rightmost < 128 - 58) {
        for (int i = 0; i < 4; i++) if (!gObs[i].on) {
          gObs[i].on = true; gObs[i].x = 130; gObs[i].y = frand(19, 45); gObs[i].v = 0;
          break;
        }
      }
      break;
    }
    case G_CATCH: {
      if (mpuOk && fabsf(tilt) > 0.08f) gPaddle += tilt * 9.0f;     // tilt to move
      else if (!mpuOk || fabsf(tilt) <= 0.08f) gPaddle += gPaddleDir * 1.2f; // otherwise glide, taps turn
      if (gPaddle < 11) { gPaddle = 11; gPaddleDir = 1; }
      if (gPaddle > 117) { gPaddle = 117; gPaddleDir = -1; }
      int active = 0;
      for (int i = 0; i < 4; i++) {
        if (!gObs[i].on) continue;
        active++;
        gObs[i].y += gObs[i].v;
        if (gObs[i].y >= 54 && gObs[i].y <= 60 && fabsf(gObs[i].x - gPaddle) < 14) {
          gObs[i].on = false; gScore++; playSound(S_POINT); gSpeed = min(2.6f, gSpeed + 0.05f);
        } else if (gObs[i].y > 64) {
          gObs[i].on = false;
          if (gLives > 0) gLives--;
          playSound(S_SAD);
          if (gLives == 0) { gameOver(now); return; }
        }
      }
      if (active < 1 + (gScore >= 8 ? 1 : 0)) {
        for (int i = 0; i < 4; i++) if (!gObs[i].on) {
          gObs[i].on = true; gObs[i].x = frand(8, 120); gObs[i].y = 8; gObs[i].v = gSpeed * frand(0.8f, 1.3f);
          break;
        }
      }
      break;
    }
    case G_SIMON: case G_COPY: case G_RHYTHM: case G_RPS:
      moveGameStep(now);
      break;
    case G_REACT:
      if (reactState == RG_WAIT && (long)(now - reactAt) >= 0) {
        reactState = RG_GO;
        reactAt = now;
        playSound(S_CLICK);
      } else if (reactState == RG_GO && now - reactAt > 3000) {
        gScore = 0;
        gState = GS_OVER;
        gStateAt = now;
        playSound(S_SAD);
      }
      break;
    default: break;
  }
}

void updateGame(unsigned long now, float tilt) {
  if (gState != GS_PLAY) return;
  int steps = 0;
  while (now - gLastStep >= GAME_STEP && steps < 4 && gState == GS_PLAY) {
    gLastStep += GAME_STEP;
    gameStep(now, tilt);
    steps++;
  }
  if (now - gLastStep > 200) gLastStep = now;   // never fall far behind
}

void drawMiniMomo(int x, int y) {   // 10x10 Momo for the games
  display.fillRoundRect(x, y, 10, 10, 3, INK);
  display.drawFastVLine(x + 3, y + 3, 3, ERASE);
  display.drawFastVLine(x + 6, y + 3, 3, ERASE);
}

// Pictures for the move games
void drawMoveIcon(uint8_t mv, int cx, int cy, bool lit, unsigned long now) {
  uint16_t c = INK;
  switch (mv) {
    case MV_TAP:
      if (lit) display.fillCircle(cx, cy, 10, c); else display.drawCircle(cx, cy, 10, c);
      display.fillCircle(cx, cy, 3, lit ? ERASE : INK);
      break;
    case MV_LEFT:
    case MV_RIGHT: {
      int d = mv == MV_LEFT ? -1 : 1;
      if (lit) {
        display.fillTriangle(cx + d * 12, cy, cx, cy - 10, cx, cy + 10, c);
        display.fillRect(d > 0 ? cx - 11 : cx, cy - 4, 11, 9, c);
      } else {
        display.drawTriangle(cx + d * 12, cy, cx, cy - 10, cx, cy + 10, c);
        display.drawRect(d > 0 ? cx - 11 : cx, cy - 4, 12, 9, c);
      }
      break;
    }
    default: {   // shake: a little box with wiggle lines
      int w = lit ? (int)((now / 60) % 3) - 1 : 0;
      display.drawRoundRect(cx - 6 + w, cy - 8, 12, 16, 3, c);
      if (lit) display.fillRoundRect(cx - 4 + w, cy - 6, 8, 12, 2, c);
      for (int k = 0; k < 3; k++) {
        display.drawPixel(cx - 10 + w, cy - 6 + k * 6, c); display.drawPixel(cx - 12 + w, cy - 3 + k * 6, c);
        display.drawPixel(cx + 10 + w, cy - 6 + k * 6, c); display.drawPixel(cx + 12 + w, cy - 3 + k * 6, c);
      }
      break;
    }
  }
}

void drawHand(uint8_t h, int cx, int cy) {   // 0 rock, 1 paper, 2 scissors
  if (h == 0) {
    display.fillCircle(cx, cy, 9, INK);
    for (int k = 0; k < 3; k++) display.drawFastHLine(cx - 5, cy - 4 + k * 4, 9, ERASE);
  } else if (h == 1) {
    display.fillRoundRect(cx - 8, cy - 11, 16, 22, 2, INK);
    for (int k = 0; k < 3; k++) display.drawFastHLine(cx - 5, cy - 6 + k * 5, 10, ERASE);
  } else if (h == 2) {
    display.drawLine(cx - 9, cy - 10, cx + 5, cy + 6, INK); display.drawLine(cx - 8, cy - 10, cx + 6, cy + 6, INK);
    display.drawLine(cx + 9, cy - 10, cx - 5, cy + 6, INK); display.drawLine(cx + 8, cy - 10, cx - 6, cy + 6, INK);
    display.drawCircle(cx - 6, cy + 8, 3, INK);
    display.drawCircle(cx + 6, cy + 8, 3, INK);
  } else {
    display.setTextSize(2); display.setCursor(cx - 5, cy - 7); display.print("?"); display.setTextSize(1);
  }
}

void drawMoveGame(unsigned long now) {
  char buf[24];
  bool flash = gFlash >= 0 && (long)(now - gFlashUntil) < 0;
  if (gMomoFace != 255 && (long)(now - gMomoFaceUntil) < 0) drawMiniEyes(112, 8, gMomoFace, now);
  display.setTextSize(1);
  switch (gameId) {
    case G_SIMON:
      snprintf(buf, sizeof(buf), "Round %u", (unsigned)gSeqLen);
      display.setCursor(0, 0); display.print(buf);
      for (uint8_t mv = 0; mv < 3; mv++) {
        int cx = mv == MV_LEFT ? 22 : mv == MV_TAP ? 64 : 106;
        drawMoveIcon(mv, cx, 32, flash && gFlash == (int8_t)mv, now);
      }
      display.setCursor(gPhase == 1 ? 37 : 49, 54);
      display.print(gPhase == 0 ? "watch" : gPhase == 1 ? "your turn" : "good!");
      if (gPhase == 1)   // dots: how far you are in the pattern
        for (uint8_t i = 0; i < gSeqLen && i < 20; i++) {
          int x = 64 - (int)min(20, (int)gSeqLen) * 3 + i * 6;
          if (i < gSeqPos) display.fillCircle(x, 47, 2, INK); else display.drawCircle(x, 47, 2, INK);
        }
      break;
    case G_COPY: {
      snprintf(buf, sizeof(buf), "%u", (unsigned)gScore);
      display.setCursor(0, 0); display.print(buf);
      if (gPhase == 1) {
        drawMoveIcon(gCmd, 64, 18, true, now);
        static const char *WORDS[4] = {"TAP!", "LEFT!", "RIGHT!", "SHAKE!"};
        display.setTextSize(2);
        display.setCursor(64 - (int)strlen(WORDS[gCmd]) * 6, 34);
        display.print(WORDS[gCmd]);
        display.setTextSize(1);
        long left = (long)(gDeadline - now);
        int w = (int)constrain(left * 120 / (long)max((uint16_t)1, gWindow), 0L, 120L);
        display.drawRect(3, 57, 122, 6, INK);
        display.fillRect(4, 58, w, 4, INK);
      } else if (flash) {
        drawMoveIcon((uint8_t)gFlash, 64, 26, true, now);
        sparkleShape(40, 16, 4, INK); sparkleShape(90, 34, 4, INK);
      }
      break;
    }
    case G_RHYTHM: {
      snprintf(buf, sizeof(buf), "%u", (unsigned)gScore);
      display.setCursor(0, 0); display.print(buf);
      // the rhythm as dots on a line, your taps as ticks under it
      long total = 0;
      for (uint8_t j = 0; j + 1 < rBeats; j++) total += rGap[j];
      if (total <= 0) total = 1;
      display.drawFastHLine(8, 34, 112, INK);
      long acc = 0;
      for (uint8_t j = 0; j < rBeats; j++) {
        int x = 8 + (int)(acc * 112 / total);
        bool shown = gPhase != 0 || j < gSeqPos;
        if (shown) display.fillCircle(x, 28, 3, INK); else display.drawPixel(x, 28, INK);
        if (j + 1 < rBeats) acc += rGap[j];
      }
      for (uint8_t j = 0; j < rTaps; j++) {
        long d = (long)(rTap[j] - rTap[0]);
        int x = 8 + (int)constrain(d * 112 / total, 0L, 119L);
        display.drawFastVLine(x, 37, 7, INK);
      }
      if (flash) display.fillCircle(64, 14, 6 + (int)((gFlashUntil - now) / 30), INK);
      if (gMomoFace == 255 || (long)(now - gMomoFaceUntil) >= 0) drawMiniEyes(112, flash ? 6 : 9, F_MUSIC, now);   // Momo bobs to the beat
      display.setCursor(gPhase == 1 ? 37 : 46, 54);
      if (gPhase == 0 || gPhase == 3) display.print("listen");
      else if (gPhase == 1) display.print("your turn");
      else {
        for (uint8_t k = 0; k < 3; k++) {
          if (k < rStars) starShape(44 + k * 20, 56, 6, INK);
          else display.drawCircle(44 + k * 20, 56, 2, INK);
        }
      }
      break;
    }
    case G_RPS: {
      snprintf(buf, sizeof(buf), "%u", (unsigned)gScore);
      display.setCursor(0, 0); display.print(buf);
      for (uint8_t l = 0; l < 3; l++) if (l >= rpsMomoWins) heartShape(80 - l * 9, 3, 2, INK);
      if (gPhase == 0) {
        static const char *CALL[3] = {"Rock", "Paper", "Scissors"};
        uint8_t k = gSeqPos ? gSeqPos - 1 : 0;
        if (gSeqPos > 0) {
          drawHand(k, 64, 26);
          display.setTextSize(gSeqPos == 3 ? 1 : 2);
          int cw = gSeqPos == 3 ? 6 : 12;
          display.setCursor(64 - (int)strlen(CALL[k]) * cw / 2, 46);
          display.print(CALL[k]);
          display.setTextSize(1);
        }
      } else if (gPhase == 1) {
        display.setTextSize(2); display.setCursor(46, 12); display.print("GO!"); display.setTextSize(1);
        drawHand(0, 18, 40); drawHand(1, 64, 40); drawHand(2, 110, 40);
        display.setCursor(0, 56); display.print("<tilt");
        display.setCursor(55, 56); display.print("tap");
        display.setCursor(98, 56); display.print("tilt>");
      } else {
        display.setCursor(11, 16); display.print("You");
        display.setCursor(88, 16); display.print("Momo");
        drawHand(rpsYou, 20, 37);
        drawHand(rpsMomo, 100, 37);
        display.setCursor(58, 33); display.print("vs");
        const char *r = rpsResult > 0 ? "You win!" : rpsResult < 0 ? (rpsYou > 2 ? "Too slow!" : "Momo wins") : "Draw";
        display.setCursor(64 - (int)strlen(r) * 3, 54);
        display.print(r);
      }
      break;
    }
    default: break;
  }
}

void drawGame(unsigned long now) {
  display.setTextColor(INK);
  char buf[32];
  if (gState == GS_READY) {
    display.setTextSize(2);
    int w = (int)strlen(GAME_NAMES[gameId]) * 12;
    display.setCursor((128 - w) / 2, 4);
    display.print(GAME_NAMES[gameId]);
    display.setTextSize(1);
    const char *how = gameId == G_JUMP ? "Tap = jump" : gameId == G_FLAPPY ? "Tap = flap" :
                      gameId == G_CATCH ? "Tilt me to move" : gameId == G_SIMON ? "Repeat Momo's moves" :
                      gameId == G_COPY ? "Do what Momo says" : gameId == G_RHYTHM ? "Tap the beat back" :
                      gameId == G_RPS ? "Tilt or tap to pick" : "Tap when eyes pop!";
    display.setCursor((128 - (int)strlen(how) * 6) / 2, 26);
    display.print(how);
    if (gameId == G_REACT) {
      if (st.best[3] > 0) snprintf(buf, sizeof(buf), "Best: %u ms", (unsigned)st.best[3]);
      else snprintf(buf, sizeof(buf), "No best yet");
    } else snprintf(buf, sizeof(buf), "Best: %u", (unsigned)bestScore(gameId));
    display.setCursor((128 - (int)strlen(buf) * 6) / 2, 38);
    display.print(buf);
    if ((now / 500) % 2) {
      display.setCursor(10, 54);
      display.print("Tap:start  2x:back");
    }
    return;
  }
  if (gState == GS_OVER) {
    display.setTextSize(1);
    if (gameId == G_REACT && gScore == 0) {
      display.setCursor(22, 4);
      display.print(reactState == RG_EARLY ? "Too early! :)" : "Too slow!");
    } else {
      display.setCursor(37, 4);
      display.print("Game over");
    }
    display.setTextSize(2);
    if (gameId == G_REACT) snprintf(buf, sizeof(buf), "%u ms", (unsigned)gScore);
    else snprintf(buf, sizeof(buf), "%u", (unsigned)gScore);
    display.setCursor((128 - (int)strlen(buf) * 12) / 2, 16);
    display.print(buf);
    display.setTextSize(1);
    if (gNewBest) {
      if ((now / 300) % 2) { display.setCursor(37, 36); display.print("NEW BEST!"); }
      drawOverlay(O_CONFETTI, now, 0);
    } else {
      if (gameId == G_REACT) snprintf(buf, sizeof(buf), "Best %u ms", (unsigned)st.best[3]);
      else snprintf(buf, sizeof(buf), "Best %u", (unsigned)bestScore(gameId));
      display.setCursor((128 - (int)strlen(buf) * 6) / 2, 36);
      display.print(buf);
    }
    display.setCursor(10, 54);
    display.print("Tap:again  2x:back");
    return;
  }
  // playing
  display.setTextSize(1);
  if (gameId >= G_SIMON) { drawMoveGame(now); return; }
  switch (gameId) {
    case G_JUMP:
      display.drawFastHLine(0, 56, 128, INK);
      for (int k = 0; k < 6; k++) display.drawPixel((int)((k * 23 + 200 - (now / 20) % 128)) % 128, 59 + k % 3, INK);
      drawMiniMomo(14, (int)(46 - gPy));
      for (int i = 0; i < 4; i++)
        if (gObs[i].on) display.fillRect((int)gObs[i].x, (int)(56 - gObs[i].h), (int)gObs[i].w, (int)gObs[i].h, INK);
      break;
    case G_FLAPPY:
      for (int i = 0; i < 4; i++) {
        if (!gObs[i].on) continue;
        int x = (int)gObs[i].x, top = (int)(gObs[i].y - FLAP_GAP), bot = (int)(gObs[i].y + FLAP_GAP);
        display.fillRect(x, 0, 10, top, INK);
        display.fillRect(x - 1, top - 3, 12, 3, INK);
        display.fillRect(x, bot, 10, 64 - bot, INK);
        display.fillRect(x - 1, bot, 12, 3, INK);
      }
      drawMiniMomo(26, (int)gPy - 5);
      display.drawPixel(35, (int)gPy - 1 + ((now / 100) % 2), INK);   // little wing
      break;
    case G_CATCH:
      for (int i = 0; i < 4; i++) if (gObs[i].on) sparkleShape((int)gObs[i].x, (int)gObs[i].y, 4, INK);
      display.fillRoundRect((int)gPaddle - 11, 58, 22, 5, 2, INK);
      for (int l = 0; l < gLives; l++) heartShape(116 - l * 9, 3, 2, INK);
      break;
    case G_REACT:
      if (reactState == RG_WAIT) {
        display.fillRect(30, 30, 30, 4, INK);
        display.fillRect(68, 30, 30, 4, INK);
        display.setCursor(40, 52);
        display.print("wait for it");
      } else {
        display.fillRoundRect(26, 10, 32, 40, 12, INK);
        display.fillRoundRect(70, 10, 32, 40, 12, INK);
        display.setCursor(52, 54);
        display.print("TAP!");
      }
      return;
    default: break;
  }
  snprintf(buf, sizeof(buf), "%u", (unsigned)gScore);
  display.setCursor(gameId == G_CATCH ? 2 : 128 - (int)strlen(buf) * 6 - 2, 1);
  display.print(buf);
}

// ---------- Pong, played by Momo when bored ----------
void pongReset() { ballX = 64; ballY = 32; ballVX = 1.6f; ballVY = 1.1f; padL = padR = 24; }

void drawPong() {
  ballX += ballVX;
  ballY += ballVY;
  if (ballY < 1 || ballY > 62) ballVY = -ballVY;
  float target = ballY - 8;
  padL += constrain(target - padL, -1.4f, 1.4f);
  padR += constrain(target - padR, -1.3f, 1.3f);
  padL = constrain(padL, 0.0f, 48.0f);
  padR = constrain(padR, 0.0f, 48.0f);
  if (ballX < 6 && ballVX < 0) ballVX = -ballVX;
  if (ballX > 121 && ballVX > 0) ballVX = -ballVX;
  for (int y = 0; y < 64; y += 6) display.drawFastVLine(64, y, 3, INK);
  display.fillRect(2, (int)padL, 3, 16, INK);
  display.fillRect(123, (int)padR, 3, 16, INK);
  display.fillRect((int)ballX - 1, (int)ballY - 1, 3, 3, INK);
}
