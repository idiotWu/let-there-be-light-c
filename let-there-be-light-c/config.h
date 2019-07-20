/**
 * @file
 * @brief ゲームの設定
 */
#ifndef CONFIG_H
#define CONFIG_H

//! 迷路のサイズ
#define MAZE_SIZE         23
//! HUD の高さ
#define HUD_HEIGHT        2
//! 迷路における径路の総計長さの最小値
#define MIN_PATH_LENGTH   (MAZE_SIZE * MAZE_SIZE * 0.1)

//! ステージの間隔
#define LEVEL_INTERVAL      5

//! 迷路生成器の最小個数
#define MIN_SPAWNER_COUNT   4

//! キャラクタの移動時間 ＝ 移動スピード
#define CHARACTER_MOVE_ANIMATION_DURATION   150
//! キャラクタのスプライトの状態（フレーム）を更新するアニメーションの持続時間
#define CHARACTER_STATE_ANIMATION_DURATION  300

//! 見える範囲の基準値
#define BASE_VISIBLE_RADIUS                 3.0
//! 基準値の最小値
#define BASE_VISIBLE_RADIUS_MIN             1.0
//! ワールドが上がるたびに基準値の減少量
#define BASE_VISIBLE_RADIUS_REDUCING_RATE   0.5

//! 最大見える範囲
#define MAX_VISIBLE_RADIUS    (MAZE_SIZE / 2.0)
//! 見える範囲の減らすスピード
#define RADIUS_REDUCING_RATE  0.001

//! 敵の最大走行距離
#define ENEMY_MIN_STEPS   20
//! 敵の最短走行距離
#define ENEMY_MAX_STEPS   30
//! 敵の減速ファクター（プレイヤーの移動スピード / 敵の移動スピード）
#define ENEMY_SLOW_DOWN   1.1

//! 敵の生成間隔
#define ENEMY_SPAWN_INTERVEL                 3000
//! 敵の最小生成間隔
#define ENEMY_SPAWN_INTERVEL_MIN             100
//! 生成間隔の減らすスピード（大きいほど早く生成させる）
#define ENEMY_SPAWN_INTERVEL_REDUCING_RATE   500

//! 敵の爆発範囲
#define ENEMY_EXPLODE_RADIUS  8

//! 凍った地面を走るときのダメージ倍数
#define SPOILED_DAMAGE  10.0

//! ステージが切り替えるときのアニメーションの持続時間
#define LEVEL_TRANSITION_DURATION   300
//! ステージのタイトルを表示する時間
#define LEVEL_TRANSITION_REST       1000

#endif
