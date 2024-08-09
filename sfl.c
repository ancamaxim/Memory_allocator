#include "sfl.h"

void init_heap(void *free_v, int start, int len, int size) {
    int byte = 8, i;
    ((sfl_v *)free_v)->len = len;
    ((sfl_v *)free_v)->capacity = len;
    ((sfl_v *)free_v)->size = size;
    ((sfl_v *)free_v)->malloc_nr = 0;
    ((sfl_v *)free_v)->free_nr = 0;
    ((sfl_v *)free_v)->frag_nr = 0;
    ((sfl_v *)free_v)->vector = malloc(2 * len * sizeof(sfl_node *));
    for (i = 0; i < len; i++, byte <<= 1) {
        ((sfl_v *)free_v)->vector[i] = init_list(start, byte, size);
        start += size;
    }
    ((sfl_v *)free_v)->total_size = ((sfl_v *)free_v)->len * ((sfl_v *)free_v)->size;
}

int sfl_malloc(void *free_v, sfl_node **occ_v, int nr_bytes) {
    int i, ok = 0;
    sfl_node *tmp;
    for (i = 0; i < ((sfl_v *)free_v)->len; i++) {
        tmp = ((sfl_v *)free_v)->vector[i]->next;
        if(tmp && tmp->size == nr_bytes) {
            *occ_v = add_node(*occ_v, nr_bytes, tmp->addr);
            delete_node(tmp);
            ((sfl_v *)free_v)->malloc_nr++;
            ok = 1;
            break;
        }
        if(tmp && tmp->size > nr_bytes) {
            ((sfl_v *)free_v)->frag_nr++;
            add_list_entry(free_v, tmp->size - nr_bytes, tmp->addr + nr_bytes);
            *occ_v = add_node(*occ_v, nr_bytes, tmp->addr);
            delete_node(tmp);
            ((sfl_v *)free_v)->malloc_nr++;
            ok = 1;
            break;
        }
    }
    if (ok == 0) {
        puts("Out of memory");
        return -1;
    }
    return 0;
}

int sfl_free(void *free_v, sfl_node **occ_v, long addr, int type) {
    if (addr == 0)
        return 0;
    int ok = 0;
    sfl_node *tmp = (*occ_v)->next;
    while(tmp) {
        if (tmp->addr == addr) {
            if (type == 0) {
                add_to_heap(free_v, tmp);
            }
            if (tmp->prev == NULL)
                *occ_v = (*occ_v)->next;
            delete_node(tmp);
            ((sfl_v *)free_v)->free_nr++;
            ok = 1;
            return 0;
        }
        tmp = tmp->next;
    }
    if (ok == 0) {
        puts("Invalid free");
        return -1;
    }
    return 0;
}

void dump_memory(void *free_v, sfl_node *occ_v) {
    puts("+++++DUMP+++++");
    int total_mem = ((sfl_v *)free_v)->total_size;
    printf("Total memory: %d bytes\n", total_mem);
    int total_alloc = 0, alloc_blocks = 0;
    sfl_node *tmp;
    if (occ_v) {
        tmp = occ_v->next;
        while(tmp) {
            total_alloc += tmp->size;
            alloc_blocks++;
            tmp = tmp->next;
        }
    }
    int i, *free_blocks, total_blocks = 0;
    free_blocks = (int *)malloc(((sfl_v *)free_v)->len * sizeof(int));
    for (i = 0; i < ((sfl_v *)free_v)->len; i++) {
        tmp = ((sfl_v *)free_v)->vector[i]->next;
        int nr_blocks = 0;
        while(tmp) {
            total_blocks++;
            nr_blocks++;
            tmp = tmp->next;
        }
        free_blocks[i] = nr_blocks;
    }
    printf("Total allocated memory: %d bytes\n", total_alloc);
    printf("Total free memory: %d bytes\n", total_mem - total_alloc);
    printf("Free blocks: %d\n", total_blocks);
    printf("Number of allocated blocks: %d\n", alloc_blocks);
    printf("Number of malloc calls: %d\n", ((sfl_v *)free_v)->malloc_nr);
    printf("Number of fragmentations: %d\n", ((sfl_v *)free_v)->frag_nr);
    printf("Number of free calls: %d\n", ((sfl_v *)free_v)->free_nr);
    for (i = 0; i < ((sfl_v *)free_v)->len; i++) {
        tmp = ((sfl_v *)free_v)->vector[i]->next;
        if (tmp) {
            printf("Blocks with %d bytes - %d free block(s) : ",
            tmp->size, free_blocks[i]);
            while(tmp) {
                printf("0x%lx ", tmp->addr);
                tmp = tmp->next;
            }
            printf("\n");
        }
    }
    printf("Allocated blocks : ");
    if (occ_v) {
        tmp = occ_v->next;
        while(tmp) {
            printf("(0x%lx - %d) ", tmp->addr, tmp->size);
            tmp = tmp->next;
        } 
    }
    printf("\n");
    puts("-----DUMP-----");
    free(free_blocks);
}

int sfl_read(void *free_v, sfl_node *occ_v, long addr, int nr_bytes) {
    int ok = 0;
    long curr_addr;
    sfl_node *tmp;
    if (!occ_v) {
        puts("Segmentation fault (core dumped)");
        dump_memory(free_v, occ_v);
        return -1;
    }
    tmp = occ_v->next;
    while(tmp) {
        if (tmp->addr <= addr && addr < tmp->addr + tmp->size) {
            ok = 1;
            curr_addr = addr;
            int bytes = 0;
            while(tmp && tmp->addr < addr + nr_bytes) {
                bytes += (tmp->addr + tmp->size - curr_addr);
                tmp = tmp->next;
                if (tmp)
                    curr_addr = tmp->addr;
            }
            if (bytes < nr_bytes)
                ok = 0;
            break;
        }
        tmp = tmp->next;
    }
    if (ok == 0) {
        puts("Segmentation fault (core dumped)");
        dump_memory(free_v, occ_v);
        return -1;
    }
    else {
        tmp = occ_v->next;
        while(tmp) {
            if (tmp->addr <= addr && addr < tmp->addr + tmp->size) {
                int idx = addr - tmp->addr, i = 0;
                for (; idx < tmp->size && i < nr_bytes; idx++, i++)
                    printf("%c", tmp->data[idx]);
                tmp = tmp->next;
                while(tmp && i < nr_bytes) {
                    idx = 0;
                    while(i < nr_bytes && idx < tmp->size) {
                        printf("%c", tmp->data[idx]);
                        i++;
                        idx++;
                    }
                    tmp = tmp->next;
                }
                break;
            }
            tmp = tmp->next;
        }
        printf("\n");
        return 0;
    }
}

int sfl_write(void *free_v, sfl_node *occ_v, void *text, long addr, int nr_bytes) {
    if ((int)strlen((char *)text) < nr_bytes)
        nr_bytes = strlen((char *)text);
    int ok = 0;
    long curr_addr, prev_addr;
    if (!occ_v) {
        puts("Segmentation fault (core dumped)");
        dump_memory(free_v, occ_v);
        return -1;
    }
    sfl_node *tmp = occ_v->next;
    while(tmp) {
        if (tmp->addr <= addr && addr < tmp->addr + tmp->size) {
            ok = 1;
            curr_addr = addr;
            int bytes = 0;
            while(tmp && tmp->addr < addr + nr_bytes) {
                bytes += (tmp->addr + tmp->size - curr_addr);
                prev_addr = tmp->addr + tmp->size;
                tmp = tmp->next;
                if (tmp) {
                    curr_addr = tmp->addr;
                    if (prev_addr != curr_addr) {
                        ok = 0;
                        break;
                    }
                }
            }
            if (bytes < nr_bytes)
                ok = 0;
            break;
        }
        tmp = tmp->next;
    }
    if (ok == 0) {
        puts("Segmentation fault (core dumped)");
        dump_memory(free_v, occ_v);
        return -1;
    }
    else {
        tmp = occ_v->next;
        while(tmp) {
            if (tmp->addr <= addr && addr < tmp->addr + tmp->size) {
                int idx = addr - tmp->addr, i = 0;
                for (; idx < tmp->size && i < nr_bytes; idx++, i++)
                    tmp->data[idx] = ((char *)text)[i];
                tmp = tmp->next;
                while(tmp && i < nr_bytes) {
                    idx = 0;
                    while(i < nr_bytes && idx < tmp->size)
                        tmp->data[idx++] = ((char *)text)[i++];
                    tmp = tmp->next;
                }
                break;
            }
            tmp = tmp->next;
        }
        return 0;
    }
}
void destroy_heap(void *free_v, sfl_node **occ_v) {
    sfl_v sfl = *(sfl_v *)free_v;
    sfl_node *occ = *occ_v;
    int i;
    for (i = 0; i < sfl.len; i++)
        sfl.vector[i] = free_list(sfl.vector[i]);
    free(sfl.vector);
    occ = free_list(occ);
}
int main() {
    char s[602];
    int params[4];
    int len;
    sfl_v free_v;
    sfl_node *occ_v = NULL;
    do {
        fgets(s, 600, stdin);
        len = strlen(s) - 1;
        s[len] = '\0';
        if(strstr(s, "INIT_HEAP")) {
            char *p = strtok(s + 10, " ");
            int i = 1;
            params[0] = strtol(p + 2, NULL, 16);
            p = strtok(NULL, " ");
            while(p) {
                params[i++] = strtol(p, NULL, 10);
                p = strtok(NULL, " ");
            }
            init_heap(&free_v, params[0], params[1], params[2]);
        }
        else if(strstr(s, "MALLOC")) {
            int nr_bytes = strtol(s + 7, NULL, 10);
            sfl_malloc(&free_v, &occ_v, nr_bytes);
        }
        else if(strstr(s, "FREE")) {
            long addr = strtol(s + 7, NULL, 16);
            sfl_free(&free_v, &occ_v, addr, params[3]);
        }
        else if(strstr(s, "READ")) {
            long addr = strtol(s + 7, NULL, 16);
            char *p = strtok(s + 7, " ");
            p = strtok(NULL, " ");
            int nr_bytes = strtol(p, NULL, 10);
            int exit_code = sfl_read(&free_v, occ_v, addr, nr_bytes);
            if (exit_code) {
                destroy_heap(&free_v, &occ_v);
                return 0;
            }
        }
        else if(strstr(s, "WRITE")) {
            char *p = strtok(s + 8, " ");
            long addr = strtol(p, NULL, 16);
            p = strtok(NULL, "\"");
            char text[600];
            strcpy(text, p);
            p = strtok(NULL, " ");
            int nr_bytes = strtol(p, NULL, 10);
            text[p - text] = '\0';
            int exit_code = sfl_write(&free_v, occ_v, text, addr, nr_bytes);
            if (exit_code) {
                destroy_heap(&free_v, &occ_v);
                return 0;
            }
        }
        else if(strstr(s, "DUMP_MEMORY")) {
            dump_memory(&free_v, occ_v);
        }
        else if(strstr(s, "DESTROY HEAP")) {
            destroy_heap(&free_v, &occ_v);
        }

    } while(len > 0);
    destroy_heap(&free_v, &occ_v);
    return 0;
}