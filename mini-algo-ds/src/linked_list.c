#include "linked_list.h"
#include <stdlib.h>
#include <stdio.h>

LinkedList* list_create(void) {
    LinkedList* l = (LinkedList*)malloc(sizeof(LinkedList));
    l->head = NULL;
    l->tail = NULL;
    l->size = 0;
    return l;
}

void list_free(LinkedList* l) {
    ListNode* cur = l->head;
    while (cur != NULL) {
        ListNode* next = cur->next;
        free(cur);
        cur = next;
    }
    free(l);
}

void list_push_back(LinkedList* l, int data) {
    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    node->data = data;
    node->next = NULL;
    node->prev = l->tail;
    if (l->tail != NULL) {
        l->tail->next = node;
    } else {
        l->head = node;
    }
    l->tail = node;
    l->size++;
}

void list_push_front(LinkedList* l, int data) {
    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    node->data = data;
    node->prev = NULL;
    node->next = l->head;
    if (l->head != NULL) {
        l->head->prev = node;
    } else {
        l->tail = node;
    }
    l->head = node;
    l->size++;
}

int list_pop_front(LinkedList* l) {
    ListNode* node = l->head;
    int data = node->data;
    l->head = node->next;
    if (l->head != NULL) {
        l->head->prev = NULL;
    } else {
        l->tail = NULL;
    }
    free(node);
    l->size--;
    return data;
}

int list_pop_back(LinkedList* l) {
    ListNode* node = l->tail;
    int data = node->data;
    l->tail = node->prev;
    if (l->tail != NULL) {
        l->tail->next = NULL;
    } else {
        l->head = NULL;
    }
    free(node);
    l->size--;
    return data;
}

void list_insert_at(LinkedList* l, int index, int data) {
    if (index <= 0) {
        list_push_front(l, data);
        return;
    }
    if (index >= l->size) {
        list_push_back(l, data);
        return;
    }
    ListNode* cur = l->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    node->data = data;
    node->next = cur;
    node->prev = cur->prev;
    cur->prev->next = node;
    cur->prev = node;
    l->size++;
}

int list_remove_at(LinkedList* l, int index) {
    if (index <= 0) {
        return list_pop_front(l);
    }
    if (index >= l->size - 1) {
        return list_pop_back(l);
    }
    ListNode* cur = l->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    int data = cur->data;
    cur->prev->next = cur->next;
    cur->next->prev = cur->prev;
    free(cur);
    l->size--;
    return data;
}

int list_get(LinkedList* l, int index) {
    ListNode* cur = l->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    return cur->data;
}

int list_find(LinkedList* l, int data) {
    ListNode* cur = l->head;
    int idx = 0;
    while (cur != NULL) {
        if (cur->data == data) {
            return idx;
        }
        cur = cur->next;
        idx++;
    }
    return -1;
}

void list_reverse(LinkedList* l) {
    ListNode* cur = l->head;
    ListNode* temp = NULL;
    while (cur != NULL) {
        temp = cur->prev;
        cur->prev = cur->next;
        cur->next = temp;
        cur = cur->prev;
    }
    temp = l->head;
    l->head = l->tail;
    l->tail = temp;
}

void list_print(LinkedList* l) {
    ListNode* cur = l->head;
    while (cur != NULL) {
        printf("%d ", cur->data);
        cur = cur->next;
    }
    printf("\n");
}

int list_size(LinkedList* l) {
    return l->size;
}
