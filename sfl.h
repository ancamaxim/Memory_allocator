#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct sfl_node{
    long addr;
    int size;
    char *data; // void *data
    struct sfl_node *next;
    struct sfl_node *prev;
}sfl_node;

typedef struct sfl_v{
    sfl_node **vector;
    int len, size, capacity;
    int malloc_nr;
    int free_nr;
    int frag_nr;
    int total_size;
}sfl_v;

sfl_node *init_list(long addr, int size, int max_size) {
    int i;
    sfl_node *head  = malloc(sizeof(sfl_node));
    sfl_node *prev;
    head->data = malloc(size);
    head->prev = NULL;
    head->size = size;
    prev = head;
    for (i = 0; i < max_size / size; i++) {
        sfl_node *node = malloc(sizeof(sfl_node));
        node->addr = size * i + addr;
        node->size = size;
        node->data = malloc(size);
        node->prev = prev;
        prev->next = node;
        node->next = NULL;
        prev = node;
    }
    return head;
}

sfl_node *delete_node(void *node) {
    if(node == NULL)
        return NULL;
    sfl_node *tmp = node;
    if(((sfl_node *)node)->next)
        ((sfl_node *)node)->next->prev = ((sfl_node *)node)->prev;
    if(((sfl_node *)node)->prev)
        ((sfl_node *)node)->prev->next = ((sfl_node *)node)->next;
    free(tmp->data);
    free(tmp);
    return NULL;
}

sfl_node *add_node(sfl_node *head, int size, long addr) {
    if (head == NULL)
        return init_list(addr, size, size);
    sfl_node *new, *tmp;
    new = malloc(sizeof(sfl_node));
    new->data = malloc(size);
    new->size = size;
    new->addr = addr;
    new->next = NULL;
    new->prev = NULL;
    if (head->next == NULL) {
        head->next = new;
        new->prev = head;
        return head;
    }
    tmp = head->next;
    while(tmp->next && tmp->addr < addr)
        tmp = tmp->next;
    if (tmp->next == NULL && tmp->addr < addr) {
        tmp->next = new;
        new->prev = tmp;
        return head;
    }
    new->next = tmp;
    new->prev = tmp->prev;
    if (tmp->prev)
        tmp->prev->next = new;
    tmp->prev = new;
    return head;
}

void add_list_entry(sfl_v *v, int size, long addr) {
    int i = 0, j;
    while(i < v->len && v->vector[i]->size < size) {
        i++;
    }
    if (i < v->len) {
        if (v->vector[i]->size == size) {
            v->vector[i] = add_node(v->vector[i], size, addr);
            return;
        }
        if (v->len == v->capacity) {
            v->capacity *= 2;
            v->vector = realloc(v->vector, v->capacity * sizeof(sfl_node *));
        }
        for (j = v->len; j > i; j--)
            v->vector[j] = v->vector[j - 1];
    }
    v->vector[i] = init_list(addr, size, size);
    v->len++;
}

void add_to_heap(void *free_v, void *node) {
    int i;
    for (i = 0; i < ((sfl_v *)free_v)->len; i++) 
        if (((sfl_v *)free_v)->vector[i]->size == ((sfl_node *)node)->size) {
            ((sfl_v *)free_v)->vector[i] = add_node(((sfl_v *)free_v)->vector[i], 
            ((sfl_node *)node)->size, ((sfl_node *)node)->addr);
            return;
        }
    add_list_entry(free_v, ((sfl_node *)node)->size, ((sfl_node *)node)->addr);
}

sfl_node *free_list(sfl_node *head) {
    if (head == NULL)
        return head;
    sfl_node *tmp, *iter;
    iter = head;
    while(iter) {
        tmp = iter;
        free(tmp->data);
        iter = iter->next;
        free(tmp);
    }
    return NULL;
}