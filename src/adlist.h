/* adlist.h - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */

typedef struct listNode {//双向链表的节点
    struct listNode *prev;
    struct listNode *next;
    void *value;
} listNode;

typedef struct listIter {//链表迭代器
    listNode *next;
    int direction;
} listIter;

typedef struct list {//链表对象
    listNode *head;
    listNode *tail;
    void *(*dup)(void *ptr);
    void (*free)(void *ptr);//函数指针
    int (*match)(void *ptr, void *key);
    unsigned long len;//长度
} list;

/* Functions implemented as macros */
#define listLength(l) ((l)->len)
#define listFirst(l) ((l)->head)
#define listLast(l) ((l)->tail)
#define listPrevNode(n) ((n)->prev)
#define listNextNode(n) ((n)->next)
#define listNodeValue(n) ((n)->value)

#define listSetDupMethod(l,m) ((l)->dup = (m))
#define listSetFreeMethod(l,m) ((l)->free = (m))
#define listSetMatchMethod(l,m) ((l)->match = (m))

#define listGetDupMethod(l) ((l)->dup)
#define listGetFreeMethod(l) ((l)->free)
#define listGetMatchMethod(l) ((l)->match)

/* Prototypes */
list *listCreate(void);//创建一个链表，并返回头结点
void listRelease(list *list);//释放链表
void listEmpty(list *list);//判断链表是否为空
list *listAddNodeHead(list *list, void *value);//头插法插入链表
list *listAddNodeTail(list *list, void *value);//尾插法插入链表
list *listInsertNode(list *list, listNode *old_node, void *value, int after);//指定位置插入节点
void listDelNode(list *list, listNode *node);//删除指定节点
listIter *listGetIterator(list *list, int direction);//创建链表迭代器
listNode *listNext(listIter *iter);//迭代方法
void listReleaseIterator(listIter *iter);//释放迭代器
list *listDup(list *orig);//拷贝链表
listNode *listSearchKey(list *list, void *key);//根据key搜索节点
listNode *listIndex(list *list, long index);//根据索引获取节点
void listRewind(list *list, listIter *li);//获取链表迭代器，从头部开始
void listRewindTail(list *list, listIter *li);//获取链表迭代器，从尾部开始
void listRotateTailToHead(list *list);//将链表尾部节点移动到头部
void listRotateHeadToTail(list *list);//将链表头部节点移动到尾部
void listJoin(list *l, list *o);//链表合并

/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1

#endif /* __ADLIST_H__ */
