#ifndef CE_TIMER_H
#define CE_TIMER_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern void timer_start(void);
extern void timer_advance(void);
extern float timer_elapsed(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_TIMER_H */
