
#include"LinkedList.h"
#include"misc.h"
#include<assert.h>
#include<stdio.h>
#include<stdlib.h>



//判断是否为头节点
int isHead(Link* list)
{
    return (list->prev == NULL);//没有前置节点就是头节点
}

//判断是否为尾节点
int isTail(Link* list)
{
    return (list->next == NULL);//没有后置节点就是尾节点
}

//在当前节点的末尾添加结点
Link* addAfter(Link* list, int data)
{

    Link* newLink = (Link*) malloc(sizeof(Link));

    newLink->data = data;

    newLink->next = list->next;
    newLink->prev = list;

    list->next->prev = newLink;
    list->next = newLink;

    return newLink;
}

//在当前节点前面添加节点
Link* addBefore(Link* list, int data)
{

    Link* newLink = (Link*)malloc(sizeof(Link));

    newLink->data = data;

    newLink->next = list;
    newLink->prev = list->prev;

    list->prev->next = newLink;
    list->prev = newLink;

    return newLink;
}

//移除节点并返回，不回收内存
Link* removeLink(Link* list)
{

    list->next->prev = list->prev;
    list->prev->next = list->next;

    list->next = NULL;
    list->prev = NULL;

    return list;
}
//删除节点并返回数据，回收内存
int deleteLink(Link* list)
{

    int data = list->data;

    Link* linkToFree = removeLink(list);

    free(linkToFree);

    return data;
}

//已经创建了节点的添加
void addLinkBefore(Link* list, Link* newLink)
{

    newLink->next = list;
    newLink->prev = list->prev;

    newLink->next->prev = newLink;
    newLink->prev->next = list->prev;
}

//创建初始双向链表
LinkedList* createLinkedList(void)
{
    LinkedList* linkedList = (LinkedList*) malloc(sizeof(LinkedList));

    linkedList->head = (Link *) malloc(sizeof(Link));
    linkedList->tail = (Link *) malloc(sizeof(Link));

    linkedList->head->prev = NULL;
    linkedList->head->next = linkedList->tail;
    linkedList->head->data = (int) 0xDEAD0000;

    linkedList->tail->prev = linkedList->head;
    linkedList->tail->next = NULL;
    linkedList->tail->data = (int) 0xDEADFFFF;

    return linkedList;
}

//清空链表
void destroyLinkedList(LinkedList* linkedList)
{
    Link* curr = linkedList->head;
    while(curr != NULL)
    {
        Link* currNext = curr->next;
        free(curr);
        curr = currNext;
    }
    free(linkedList);
}

//在头部添加节点
Link* addFirst(LinkedList* linkedList, int data)
{

    return addAfter(linkedList->head, data);
}

//在末尾添加节点
Link* addLast(LinkedList* linkedList, int data)
{

    return addBefore(linkedList->tail, data);
}

//得到第一个节点的数据
int getFirst(LinkedList* linkedList)
{

    return linkedList->head->next->data;
}

//得到链表长度
int length(LinkedList* linkedList)
{
    int length = 0;
    Link* curr = linkedList->head->next;

    while(!isTail(curr))
    {
        length++;
        curr = curr->next;
    }

    return length;
}

//判断是否为空。
int isEmpty(LinkedList* linkedList)
{
    return isTail(linkedList->head->next);
}

