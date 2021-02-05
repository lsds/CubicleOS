#ifndef MEMREQ_H
#define MEMREQ_H

void *dl_nginx_malloc(int size);
void *dl_nginx_calloc(int num, int size);
void dl_nginx_free(void *ptr);
char* get_memory_ramfs(unsigned num_bytes);

#endif /*MEMREQ_H*/

