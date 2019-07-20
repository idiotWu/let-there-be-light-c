/**
 * @file
 * @brief ゲームの状態 ＜ヘッダファイル＞
 */
#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#include "config.h"

#include "scene/game/player.h"
#include "scene/game/enemy.h"

#include "scene/scene.h"
#include "util/util.h"
#include "util/list.h"
#include "maze/tile.h"

/**
 * @brief 出力画面における矩形空間
 */
typedef struct ClientRect {
  //! 矩形区間の上辺
  double top;
  //! 矩形区間の右辺
  double right;
  //! 矩形区間の下辺
  double bottom;
  //! 矩形区間の左辺
  double left;
  //! 矩形区間の長さ
  double width;
  //! 矩形区間の高さ
  double height;
} ClientRect;

/**
 * @brief ゲームの状態
 */
typedef struct State {
  //! 現在のシーン
  Scene* scene;

  //! 現在のレベル
  struct {
    //! ワールド
    int major;
    //! ステージ
    int minor;
  } level;

  //! 残りのアイテム数
  int remainItem;

  //! 現在の迷路の構成
  Tile maze[MAZE_SIZE][MAZE_SIZE];

  //! 各タイルにおけるプレイヤーまでの最小歩数
  int stepsFromPlayer[MAZE_SIZE][MAZE_SIZE];

  //! 歩けるタイル
  vec2i* openTiles;

  //! 径路の総計の長さ ＝ 開いているタイルの個数
  int pathLength;

  //! ゲームを一時中止する
  bool paused;

  //! 見える範囲（エネルギー）
  double visibleRadius;

  //! 1 つ前のステージのエネルギー残量
  double lastVisibleRadius;

  //! 制限を解除する
  bool unlockLimits;

  //! プレイヤーインスタンス
  Player player;

  //! 敵のリスト
  List* enemies;

  //! 現在の座標系
  ClientRect ortho;

  //! 現在の表示領域（ビューポート）
  ClientRect viewport;
} State;

/**
 * @brief ゲーム全体の状態を表すグローバル変数
 */
extern State GameState;

/**
 * @brief ゲームの状態を初期化する
 */
void initGameState(void);

#endif
