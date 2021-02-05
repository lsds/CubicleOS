#include <stdint.h>
#include <ucontext.h>

int _Ux86_64_getcontext(ucontext_t *ucp);
int _Ux86_64_setcontext(const ucontext_t *ucp);

typedef struct coro_t_		coro_t;
typedef struct thread_t_	thread_t;
typedef enum {
    CORO_NEW,
    CORO_RUNNING,
    CORO_FINISHED
} coro_state_t;

#ifdef __x86_64__
union ptr_splitter {
    void *ptr;
    uint32_t part[sizeof(void *) / sizeof(uint32_t)];
};
#endif

struct thread_t_ {
    struct {
        ucontext_t callee, caller;
    } coro;
};

struct coro_t_ {
    coro_state_t state;
    void *function;
    thread_t *thread;

    ucontext_t context;
    char *stack;
    uint64_t yield_value;
};

#define STACK_SIZE	(1 << 22)
static const int default_stack_size = (1 << 22);

void coro_yield(coro_t *coro, uint64_t value);

int swapcontext2(ucontext_t *oucp, ucontext_t *ucp) {
    volatile char swapped = 0;
    int result = _Ux86_64_getcontext(oucp);
    if (result == 0 && !swapped) {
        swapped = 1;
        result = _Ux86_64_setcontext(ucp);
    }
    return result;
}

uint64_t
coro_resume(coro_t *coro)
{
    if (coro->state == CORO_NEW)
        coro->state = CORO_RUNNING;
    else if (coro->state == CORO_FINISHED)
        return 0;

    ucontext_t old_context = coro->thread->coro.caller;
    swapcontext2(&coro->thread->coro.caller, &coro->context);
    coro->context = coro->thread->coro.callee;
    coro->thread->coro.caller = old_context;

    return coro->yield_value;
}

void
coro_yield(coro_t *coro, uint64_t value)
{
    coro->yield_value = value;
    swapcontext2(&coro->thread->coro.callee, &coro->thread->coro.caller);
}

void
coro_free(coro_t *coro)
{
//    free(coro->stack);
    free(coro);
}
