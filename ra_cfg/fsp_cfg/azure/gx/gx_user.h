/* generated configuration header file - do not edit */
#ifndef GX_USER_H_
#define GX_USER_H_
#ifdef __cplusplus
extern "C" {
#endif

#define GX_USE_RENESAS_JPEG (0)
#define GX_RENESAS_DAVE2D_DRAW  (1)

#define GX_RENESAS_DAVE2D_COMMAND_COUNT_TO_REFRESH 30
#define GX_RENESAS_DAVE2D_FONT_SUPPORT

#define GX_THREAD_STACK_SIZE       (4096)
#define GX_SYSTEM_THREAD_PRIORITY  (30)
#define GX_SYSTEM_THREAD_TIMESLICE (10)

#define GX_SYSTEM_TIMER_MS    (20)
#define GX_TICKS_SECOND       (1000 / GX_SYSTEM_TIMER_MS)

#if (0)
  #define GUIX_DISABLE_MULTITHREAD_SUPPORT
#endif
#if (0)
  #define GX_DISABLE_UTF8_SUPPORT
#endif

#define GX_MAX_QUEUE_EVENTS (48)

#if (0)
  #define GX_WIDGET_USER_DATA
#endif

#ifdef __cplusplus
}
#endif
#endif /* GX_USER_H_ */
