#include "lru.h"
#include "config.h"
#include <stdlib.h>

typedef struct FrameNumberNode {
    int frame_number;
    FrameNumberNode *prev;
    FrameNumberNode *next;
} FrameNumberNode;

static FrameNumberNode *head;
static FrameNumberNode *tail; 

static FrameNumberNode *lru_map[FRAME_STORE_SIZE/FRAME_SIZE];

int lru_map_init() {
    head = malloc(sizeof(FrameNumberNode));
    if (head == NULL) return 1;
    head->prev = NULL; 
    head->frame_number = 0;
    FrameNumberNode* prev = head;
    for (int i=1; i<FRAME_STORE_SIZE/FRAME_SIZE; i++) {
        FrameNumberNode *new = malloc(sizeof(FrameNumberNode));
        if (new == NULL) return 1;
        new->prev = prev;
        new->frame_number = i;
        lru_map[i] = new;
        lru_map[i-1] = prev;
        prev->next = new;
        prev = new;
    }
    tail = prev;
    tail->next = NULL;
    return 0;
}

void update_mru(int frame_number) { 
    FrameNumberNode* curr = lru_map[frame_number];
    if (curr == head) { 
        return;
    }

    FrameNumberNode* prev = curr->prev;
    FrameNumberNode* next = curr->next;
    if (curr == tail) {
        tail = prev;
        tail->next = NULL;      
    }
    else {
        next->prev = prev;
    }
    prev->next = next;
    curr->prev = NULL;
    curr->next = head;
    head->prev = curr;
    head = curr; 
}

int get_lru_and_reorder() {
    FrameNumberNode *lru = tail;
    FrameNumberNode *new_tail= tail->prev;
    new_tail->next = NULL;
    lru->next = head;
    lru->prev = NULL;
    tail = new_tail;
    head->prev = lru;
    head = lru;
    return lru->frame_number;
}
