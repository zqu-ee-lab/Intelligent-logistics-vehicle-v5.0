#ifndef SYSINFOTEST_H
#define SYSINFOTEST_H


extern TaskHandle_t SysInfoTestSent_Handle;       			//+任务2句柄
/* 在主函数中调用 */
extern void vSetupSysInfoTest(void);

extern void SysInfoTestSent(void *parameter);


#endif
