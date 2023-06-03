

#pragma once

//�ڽ������ʵ��,��������ϵ���Ŀ

struct Link;


//˫������
struct LinkedList
{
    struct Link* head; //��ͷ
    struct Link* tail; //��β
};

struct Link
{
    int data; //�ڵ㱣�������
    struct Link* next; 
    struct Link* prev; 
};
//���¹��ܼ�cpp�ļ�
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

