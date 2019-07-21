/**
 * @file main.c
 * @brief メイン関数など
 */

/**
 * @mainpage
 *
 * @section intro 概要
 *
 * 本ゲームは Pac-Man を模倣したゲームです．Pac-Man との相違点は，
 *
 *    1. 迷路がランダムに生成され，
 *    2. プレイヤーの見える範囲が限られていることです．
 *
 * また，Ghost の代わりに，自爆するスライムが敵になっています．
 *
 * @section rules 基本ルール
 *
 * 1. 無数個のワールドが存在し，それぞれ5つのステージがあります．
 * 2. 迷路の中に散らばっているアイテム（コインと火の玉）をすべて拾ったらステージがクリアできます．
 * 3. 時間が経つとプレイヤーの見える範囲（エネルギー）がどんどん縮まり，完全に見えなくなったらゲームオーバー．
 * 4. エネルギーの継承ルールは `e = max(0.5, base + last / 2)` で，各ステージの始まりに最大半分までチャージされます．基準値はゲームの進行に従って減少していきます．
 * 5. 敵（スライム）は一定の間隔で生まれますが，プレイヤーが近づくまでは動きません．
 * 6. 動き始めた敵はプレイヤーを追いながらどんどん縮まり，最後に自爆して地面を凍らせます．
 * 7. プレイヤーが敵に当たってしまうと敵はすぐに爆発し，より広い地面を凍らせます．（爆発範囲は敵の大きさに比例します）
 * 8. 凍った地面を走るとダメージを受けます．
 * 9. 火の玉を拾うとエネルギーが増え，凍った地面が消えます（敵は消えません）．
 * 10. ステージが上がると，アイテムの密度が低くなります．
 * 11. ワールドが上がるたびに火の玉の数は1個増えますが，敵の生成スピードも早くなります．
 *
 * @section howto 操作方法
 *
 * 基本的には方向キーを使ってプレイヤーを動かすだけです．
 *
 * ゲームオーバーになってしまったら，R を押すと今のステージをやり直し，T を押すとタイトルに戻ることができます．
 *
 * @section docs 関数の仕様
 *
 * [ファイル一覧](files.html)を参考してください．
 */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "glut.h"

#include "config.h"
#include "state.h"

#include "util/util.h"
#include "maze/tile.h"
#include "maze/maze.h"
#include "render/engine.h"
#include "render/texture.h"
#include "maze/direction.h"
#include "scene/title/title.h"

//! ゲームのタイトル
#define GAME_TITLE "Let There Be Light"
//! Window の長さ
#define WIN_WIDTH  800
//! Window の高さ
#define WIN_HEIGHT 800
//! Window をスクリーンの中央に表示させる
#define CENTERING_WINDOW

//! 最後に FPS を更新してから経過したフレーム数
static int frameCount = 0;

/**
 * @brief 初期化する
 */
void init(void) {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  loadTextures();
  initGameState();
  switchScene(titleScene);
}

/**
 * @brief 描画する
 */
void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  GameState.scene->render();

  engineRender();

  glutSwapBuffers();
}

/**
 * @brief Window のタイトルを更新する
 *
 * @param fps 現在のフレームレート
 */
void updateTitle(int fps) {
  char tmp[128];

  sprintf(tmp, "%s: %2d FPS @ %d x %d",
          GAME_TITLE,
          fps,
          glutGet(GLUT_WINDOW_WIDTH),
          glutGet(GLUT_WINDOW_HEIGHT));

  glutSetWindowTitle(tmp);
}

/**
 * @brief ゲームの状態を更新する
 *
 * @param timestamp 最後に FPS を更新した時刻
 */
void update(int timestamp) {
  // count FPS
  frameCount++;

  int now = glutGet(GLUT_ELAPSED_TIME);

  if (now - timestamp >= 1000) {
    updateTitle(frameCount);
    timestamp = now;
    frameCount = 0;
  }

  glutTimerFunc(ANIMATION_REFRESH_INTERVAL, update, timestamp);

  GameState.scene->update();
  engineNextFrame();

#ifdef __APPLE__
  // `glutPostRedisplay()` blocks the next timer on mac
  display();
#else
  glutPostRedisplay();
#endif
}

/**
 * @brief 出力画面を調整する
 *
 * @details 表示領域を Window の中央に調整する
 *
 * @param w Window の長さ
 * @param h Window の高さ
 */
void reshape(int w, int h) {
  ClientRect* vp = &GameState.viewport;
  ClientRect* ortho = &GameState.ortho;

  double ratio = (double)MAZE_SIZE / (MAZE_SIZE + HUD_HEIGHT);
  double size = clamp(min(/* w */ h * ratio, /* h */ w / ratio),
                      0, min(w, h));

  vp->width = size;
  vp->height = size / ratio;
  vp->left = (w - vp->width) / 2;
  vp->bottom = (h - vp->height) / 2;
  vp->top = vp->bottom + vp->height;
  vp->right = vp->left + vp->width;

  glViewport(vp->left, vp->bottom, vp->width, vp->height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  ortho->top = MAZE_SIZE + HUD_HEIGHT;
  ortho->right = MAZE_SIZE;
  ortho->bottom = 0;
  ortho->left = 0;
  ortho->width = ortho->right - ortho->left;
  ortho->height = ortho->top - ortho->bottom;

  gluOrtho2D(ortho->left, ortho->right, ortho->bottom, ortho->top);

  glMatrixMode(GL_MODELVIEW);
}

/**
 * @brief メイン関数
 */
int main(int argc, char* argv[]) {
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);

  glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);

#ifdef CENTERING_WINDOW
  int winX = (glutGet(GLUT_SCREEN_WIDTH) - WIN_WIDTH) / 2;
  int winY = (glutGet(GLUT_SCREEN_HEIGHT) - WIN_HEIGHT) / 2;
  glutInitWindowPosition(winX, winY);
#endif

  glutCreateWindow(GAME_TITLE);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);

  reshape(WIN_WIDTH, WIN_HEIGHT);
  init();
  update(glutGet(GLUT_ELAPSED_TIME));
  updateTitle(1000 / ANIMATION_REFRESH_INTERVAL);

  glutMainLoop();

  return EXIT_SUCCESS;
}
