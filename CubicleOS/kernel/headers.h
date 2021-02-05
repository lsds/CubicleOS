void cmp_lock(int mid);
void cmp_unlock(int mid);
void cmp_attach(int mid, void *start, size_t size);

void heap_attach(int mid, long start, size_t size);
void stack_attach(int mid, void *start, size_t size);

void add_ipc_tab_heap(int mid, void *ipc_tab, void *nr);
void add_ipc_tab_stack(int mid, void *ipc_tab, void *nr);
void add_ipc_tab_globals(int mid, void *ipc_tab, void *nr);

int get_memory_permission(void *address);


struct metadata_ifpages {
	unsigned long	num_pages;
	void		*base;
};

struct ukplat_memregion_desc {
	void *base;
	size_t len;
	int flags;
};

void setup_heap(int a, struct ukplat_memregion_desc *desc);

struct uk_thread {
	const char *name;
	void *stack;
	void *tls;
	void *ctx;
//other fields are removed
};
