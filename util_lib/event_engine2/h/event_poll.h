
#ifndef _EVENT_POLL_H_
#define _EVENT_POLL_H_

typedef struct {
	char *name;
	void *ctx;
} event_poll_t;

typedef int (*event_poll_callback_t)(int, short int, void *);
typedef int (*event_timer_callback_t)(void *);

#define E_POLLIN		0x001		/* There is data to read.  */
#define E_POLLPRI		0x002		/* There is urgent data to read.  */
#define E_POLLOUT		0x004		/* Writing now will not block.  */

#define E_POLLERR		0x008		/* Error condition.  */
#define E_POLLHUP		0x010		/* Hung up.  */
#define E_POLLNVAL	0x020		/* Invalid polling request.  */

event_poll_t *event_init(void);
int event_add_fd(event_poll_t *e, int fd, short int events, void * arg, event_poll_callback_t callback);
int event_add_timer(event_poll_t *e, unsigned int interval, void *arg, event_timer_callback_t callback);
int event_main_loop(event_poll_t *e);
int event_remove_fd(event_poll_t *e, int fd);
int event_remove_timer(event_poll_t *e, int fd);

#endif /* _EVENT_POLL_H_ */
