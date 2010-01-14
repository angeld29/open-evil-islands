#ifndef CE_TIMER_H
#define CE_TIMER_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct timer timer;

extern timer* timer_open(void);
extern void timer_close(timer* tmr);

extern void timer_advance(timer* tmr);
extern float timer_elapsed(timer* tmr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_TIMER_H */
