#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct ListNode {
    int data;
    struct ListNode* next;
    struct ListNode* prev;
} ListNode;

typedef struct {
    ListNode* head;
    ListNode* tail;
    int size;
} LinkedList;

LinkedList* list_create(void);
void list_free(LinkedList* l);
void list_push_back(LinkedList* l, int data);
void list_push_front(LinkedList* l, int data);
int list_pop_front(LinkedList* l);
int list_pop_back(LinkedList* l);
void list_insert_at(LinkedList* l, int index, int data);
int list_remove_at(LinkedList* l, int index);
int list_get(LinkedList* l, int index);
int list_find(LinkedList* l, int data);
void list_reverse(LinkedList* l);
void list_print(LinkedList* l);
int list_size(LinkedList* l);

#endif
