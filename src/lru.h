#ifndef LRU_H
#define LRU_H
typedef struct FrameNumberNode FrameNumberNode;
int lru_map_init();  
int get_lru_and_reorder();
void update_mru(int frame_number);
#endif