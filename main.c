#include "webservice.h"
#include "packages/coro/coro.h"

DUDA_REGISTER("Coro Fibonacci Examples", "coro fibonacci demo");

struct bundle {
    channel_t *chan;
    duda_request_t *dr;
};

void consumer(void *data)
{
    struct bundle *bdl = data;
    channel_t *chan = bdl->chan;
    duda_request_t *dr = bdl->dr;
    response->http_status(dr, 200);
    while (!coro->chan_done(chan)) {
        int *n = coro->chan_recv(chan);
        response->printf(dr, "%d\n", *n);
        monkey->mem_free(n);
    }
    response->end(dr, NULL);
}

void productor(void *data)
{
    channel_t *chan = data;
    int num1 = 1;
    int num2 = 1;
    int i;
    for (i = 0; i < 20; ++i) {
        int *n = monkey->mem_alloc(sizeof(int));
        if (i == 0) {
            *n = num1;
        } else if (i == 1) {
            *n = num2;
        } else {
            *n = num1 + num2;
            num1 = num2;
            num2 = *n;
        }
        coro->chan_send(chan, n);
    }
    coro->chan_end(chan);
}

void cb_fibonacci(duda_request_t *dr)
{
    // unbuffered channel
    channel_t *chan = coro->chan_create(0, monkey->mem_free);
    struct bundle *bdl = monkey->mem_alloc(sizeof(*bdl));
    bdl->chan = chan;
    bdl->dr = dr;
    int cid = coro->create(consumer, bdl);
    int pid = coro->create(productor, chan);
    coro->chan_set_sender(chan, pid);
    coro->chan_set_receiver(chan, cid);
    coro->resume(cid);
    coro->chan_free(chan);
    monkey->mem_free(bdl);
}

int duda_main()
{
    duda_load_package(coro, "coro");
    map->static_add("/", "cb_fibonacci");
    return 0;
}
