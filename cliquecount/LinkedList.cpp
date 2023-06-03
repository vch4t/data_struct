
#include"LinkedList.h"
#include"misc.h"
#include<assert.h>
#include<stdio.h>
#include<stdlib.h>



//�ж��Ƿ�Ϊͷ�ڵ�
int isHead(Link* list)
{
    return (list->prev == NULL);//û��ǰ�ýڵ����ͷ�ڵ�
}

//�ж��Ƿ�Ϊβ�ڵ�
int isTail(Link* list)
{
    return (list->next == NULL);//û�к��ýڵ����β�ڵ�
}

//�ڵ�ǰ�ڵ��ĩβ��ӽ��
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

//�ڵ�ǰ�ڵ�ǰ����ӽڵ�
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

//�Ƴ��ڵ㲢���أ��������ڴ�
Link* removeLink(Link* list)
{

    list->next->prev = list->prev;
    list->prev->next = list->next;

    list->next = NULL;
    list->prev = NULL;

    return list;
}
//ɾ���ڵ㲢�������ݣ������ڴ�
int deleteLink(Link* list)
{

    int data = list->data;

    Link* linkToFree = removeLink(list);

    free(linkToFree);

    return data;
}

//�Ѿ������˽ڵ�����
void addLinkBefore(Link* list, Link* newLink)
{

    newLink->next = list;
    newLink->prev = list->prev;

    newLink->next->prev = newLink;
    newLink->prev->next = list->prev;
}

//������ʼ˫������
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

//�������
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

//��ͷ����ӽڵ�
Link* addFirst(LinkedList* linkedList, int data)
{

    return addAfter(linkedList->head, data);
}

//��ĩβ��ӽڵ�
Link* addLast(LinkedList* linkedList, int data)
{

    return addBefore(linkedList->tail, data);
}

//�õ���һ���ڵ������
int getFirst(LinkedList* linkedList)
{

    return linkedList->head->next->data;
}

//�õ�������
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

//�ж��Ƿ�Ϊ�ա�
int isEmpty(LinkedList* linkedList)
{
    return isTail(linkedList->head->next);
}

