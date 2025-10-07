/*
 * sys.c
 *
 *  Created on: 2025年9月1日
 *      Author: bitstream
 */
#include "hal_data.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

int _close(int file);
off_t _lseek(int file, off_t ptr, int dir);
int _read(int file, char *ptr, int len);
int _write(int file, const char *ptr, int len);
int _fstat(int file, struct stat *st);
int _isatty(int file);

fsp_err_t err = FSP_SUCCESS;
volatile bool uart_send_complete_flag = false;
void user_uart_callback (uart_callback_args_t * p_args)
{
    if(p_args->event == UART_EVENT_TX_COMPLETE)
    {
        uart_send_complete_flag = true;
    }
}
#ifdef __GNUC__                                 //串口重定向
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE;

PUTCHAR_PROTOTYPE
{
        err = R_SCI_UART_Write(&g_uart9_ctrl, (uint8_t *)&ch, 1);
        if(FSP_SUCCESS != err) __BKPT();
        while(uart_send_complete_flag == false){}
        uart_send_complete_flag = false;
        return ch;
}

int _close(int file) {
    (void)file;
    return -1;  // Not implemented
}

off_t _lseek(int file, off_t ptr, int dir) {
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;  // Not implemented
}

int _read(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    return 0;  // Not implemented
}

int _write(int file, const char *ptr, int len) {
    (void)file;
    for(int i=0;i<len;i++)
    {
        __io_putchar(*ptr++);
    }
    return (int)len;  // Simply return the number of bytes written
}

int _fstat(int file, struct stat *st) {
    (void)file;
    (void)st;
    return -1;  // 返回一个失败状态
}

int _isatty(int file) {
    (void)file;
    return 0;  // 返回0表示不是一个终端设备
}


typedef int pid_t;

void _exit(int status);
int _kill(pid_t pid, int sig);
int _getpid(void);

void _exit(int status) {
    (void)status;
    while (1);  // 进入死循环，通常在嵌入式系统中这样处理退出
}

int _kill(pid_t pid, int sig) {
    (void)pid;
    (void)sig;
    return 0;  // 终止信号，不进行任何操作
}

int _getpid(void) {
    return 1;  // 在没有操作系统的情况下，返回一个假设的进程ID
}
