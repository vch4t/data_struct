

#pragma once

//邻接链表的实现,借鉴了网上的项目

struct Link;


//双向链表
struct LinkedList
{
    struct Link* head; //表头
    struct Link* tail; //表尾
};

struct Link
{
    int data; //节点保存的数据
    struct Link* next; 
    struct Link* prev; 
};
//以下功能见cpp文件
int isHead(Link* list);

int isTail(Link* list);

int deleteLink(Link* list);

Link* addAfter(Link* list, int data);

Link* addBefore(Link* list, int data);

void addLinkBefore(Link* list, Link* newLink);

Link* removeLink(Link* list);

LinkedList* createLinkedList(void);

void destroyLinkedList(LinkedList* linkedList);

Link* addFirst(LinkedList* linkedList, int data);

Link* addLast(LinkedList* linkedList, int data);

int getFirst(LinkedList* linkedList);

int length(LinkedList* linkedList);

int isEmpty(LinkedList* linkedList);

