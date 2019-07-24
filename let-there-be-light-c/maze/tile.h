/**
 * @file
 * @brief タイルの定義 ＜ヘッダファイル＞
 */
#ifndef TILE_H
#define TILE_H

/**
 * @brief タイル
 */
typedef enum Tile {
  //! 壁
  TILE_WALL   = 1 << 0,
  //! 陰
  TILE_SHADOW = 1 << 1,

  //! 道
  TILE_PATH   = 1 << 2,
  //! コイン
  TILE_COIN   = 1 << 3,
  //! 火の玉
  TILE_KERNEL = 1 << 4,

  //! 凍った地面
  TILE_SPOILED = 1 << 5,

  //! 閉じているタイル
  TILE_CLOSED = TILE_WALL | TILE_SHADOW,
  //! 開いているタイル
  TILE_OPEN   = TILE_PATH | TILE_COIN | TILE_KERNEL | TILE_SPOILED,

  //! アイテムが置かれるタイル
  TILE_ITEM   = TILE_COIN | TILE_KERNEL
} Tile;

#endif
