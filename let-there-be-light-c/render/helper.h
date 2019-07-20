/**
 * @file
 * @brief 実用的なレンダリング関数 ＜ヘッダファイル＞
 */
#ifndef HELPER_H
#define HELPER_H

/**
 * @brief 出力画面から矩形のクリッピング領域を作る
 *
 * @details 指定される矩形領域の内側の部分は表示され，外側の部分は非表示になる
 *
 * @param x      矩形の x 座標
 * @param y      矩形の y 座標
 * @param width  矩形の長さ
 * @param height 矩形の高さ
 */
void clipRect(double x, double y, double width, double height);

#endif
