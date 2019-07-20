/**
 * @file
 * @brief 双方向連結リスト ＜ヘッダファイル＞
 */
#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

/**
 * @brief ノード（節点）オブジェクト
 */
typedef struct Node {
  //! ノードに格納されるデータ
  void* data;
  //! 前のノード
  struct Node* prev;
  //! 次のノード
  struct Node* next;
} Node;

/**
 * @brief リストオブジェクト
 */
typedef struct List {
  //! リストのサイズ（ノードの数）
  size_t count;
  //! 冒頭のノード
  Node* head;
  //! 末尾のノード
  Node* tail;
} List;

/**
 * @brief リストの最後に一個のノードを追加する
 *
 * @param list 目標のリスト
 * @param node 追加したいノード
 */
void listAppend(List* list, Node* node);

/**
 * @brief リストの先頭に一個のノードを前付ける
 *
 * @param list 目標のリスト
 * @param node 前付けたいノード
 */
void listPrepend(List* list, Node* node);

/**
 * @brief 特定のノードの後ろに一個のノードを追加する
 *
 * @param list 目標のリスト
 * @param prev 目標のノード
 * @param node 追加したいノード
 */
void listInsertAfter(List* list, Node* prev, Node* node);

/**
 * @brief リストから特定のノードを削除する
 *
 * @param list 目標のリスト
 * @param node 削除したいノード
 */
void listDelete(List* list, Node* node);

/**
 * @brief 既存のリストを新しいリストにコピーする
 *
 * @param[out] dest     新しいリスト
 * @param[in] src       コピー元のリスト
 * @param dataSize      リストのノードが持つデータのサイズ ＝ `sizeof(node.data)`
 */
void listCopy(List* dest, List* src, size_t dataSize);

/**
 * @brief 既存のリストをクローンする
 *
 * @param[in] src      既存のリスト
 * @param dataSize     リストのノードが持つデータのサイズ ＝ `sizeof(node.data)`
 *
 * @return List*   リストのクローン
 */
List* listClone(List* src, size_t dataSize);

/**
 * @brief 特定のデータを持つノードをリストから削除する
 *
 * @param list 目標のリスト
 * @param data 特定のデータ
 *
 * @return true  削除が成功
 * @return false 削除が失敗＝ノードが見つからない
 */
bool listFindDelete(List* list, void* data);

/**
 * @brief ノードのメモリ領域を解放する
 *
 * @param node 目標のノード
 */
void listFreeNode(Node* node);

/**
 * @brief リストのノードを全部削除する
 *
 * @param list 目標のリスト
 */
void listClear(List* list);

/**
 * @brief リストを廃棄する
 *
 * @details この関数は下記の操作に等しい
 * ```c
 * listClear(list);
 * free(list);
 * ```
 *
 * @param list 目標のリスト
 */
void listDestory(List* list);

/**
 * @brief 新しいリストを生成する
 *
 * @return List* 新しいリスト
 */
List* createList(void);

/**
 * @brief 新しいノードを生成する
 *
 * @return Node* 新しいノード
 */
Node* createNode(void);

///////////// LIST ITERATOR /////////////

/**
 * @brief リストの繰り返し処理を抽象化する，リストイテレータオブジェクト
 */
typedef struct ListIterator {
  /**
   * @private
   * @brief 目標のリスト
   */
  List* list;

  /**
   * @private
   * @brief イテレータが指している次のノード
   */
  Node* nextNode;

  /**
   * @private
   * @brief このイテレータが作られたときのリストのサイズ
   */
  size_t initialLength;
  /**
   * @private
   * @brief 次のノードのインデックス
   */
  size_t nextIndex;

  /**
   * @brief イテレーションが終わったのか
   */
  bool done;

  /**
   * @brief イテレータを次のノードに移動させるメソッド
   * @details このメソッドは予め定義されている
   *
   * @see iterateNext
   */
  Node* (*next)(struct ListIterator*);
} ListIterator;

/**
 * @brief 特定のリストに対して，イテレータを生成する
 *
 * @details リストイテレータを使うことで，安全なリストの繰り返し処理ができる．使用例：
 * ```c
 * ListIterator it = createListIterator(list);
 *
 * while (!it.done) {
 *   Node* node = it.next(list);
 *   // play with node...
 *
 *   // ノードをリストから削除しても，正しい繰り返し処理ができる
 *   listDelete(list, node);
 * }
 * ```
 *
 * @param list 目標のリスト
 *
 * @return ListIterator リストイテレータ
 */
ListIterator createListIterator(List* list);


#endif
