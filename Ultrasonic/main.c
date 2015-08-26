//*****************************************************************************
//
//Ultrasonic example.
//
// Copyright (c) 2011-2012 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 9453 of the EK-LM4F232H5QD Firmware Package.
//
//*****************************************************************************
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/pin_map.h"
#include "inc/lm4f232h5qd.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/gpio.h"
#include "grlib/grlib.h"
#include "driverlib/pwm.h"
#include "drivers/cfal96x64x16.h"
#include "driverlib/adc.h"
#include <stdio.h>

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Timer (timers)</h1>
//!
//! This example application demonstrates the use of the timers to generate
//! periodic interrupts.  One timer is set up to interrupt once per second and
//! the other to interrupt twice per second; each interrupt handler will toggle
//! its own indicator on the display.
//
//*****************************************************************************
//*****************************************************************************
//
// Flags that contain the current value of the interrupt indicator as displayed
// on the CSTN display.
//
//*****************************************************************************
unsigned long g_ulFlags;
int led = 0;
int show = 0;
unsigned time = 0;
unsigned long temp_time = 0;
unsigned long distance_time = 0;
float distance = 0;
int sound = 0;
int sound_turn = 0;
int connect = 0;
int start_cap = 0;
char distance_char[5];

int flag = 0;
unsigned long start = 0;
unsigned long end = 0;

//*****************************************************************************
//
// Graphics context used to show text on the CSTN display.
//
//*****************************************************************************
tContext g_sContext;
tRectangle sRect;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

int distance_int = 0;

//float 转 char
void FloatToChar(void)
{
	distance_int = (int) (distance * 10);
	distance_char[0] = distance_int / 1000 + '0';
	distance_int %= 1000;
	distance_char[1] = distance_int / 100 + '0';
	distance_int %= 100;
	distance_char[2] = distance_int / 10 + '0';
	distance_int %= 10;
	distance_char[3] = '.';
	distance_char[4] = distance_int + '0';
}

//*****************************************************************************
//
// The interrupt handler for the first timer interrupt.
//
//*****************************************************************************
void
Timer0IntHandler(void)
{
    // Clear the timer interrupt.
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Toggle the flag for the first timer.
    HWREGBITW(&g_ulFlags, 0) ^= 1;

    // 产生脉冲
    ROM_IntMasterDisable();
    if(led < 1500)
    {
    	led = led++;
    }
    else
    	led = 0;

    //每秒显示一次数据
    if(show < 1000)
    {
    	show = show++;
    }
    else
    {
    	show = 0;
    	FloatToChar();
    	// 屏幕的上面24行填充蓝色覆盖上次的显示结果
    	sRect.sXMin = 0;
    	sRect.sYMin = 22;
    	sRect.sXMax = GrContextDpyWidthGet(&g_sContext) - 1;
    	sRect.sYMax = 63;
    	GrContextForegroundSet(&g_sContext, ClrBlack);
    	GrRectFill(&g_sContext, &sRect);
    	// 更新显示
    	GrContextForegroundSet(&g_sContext, ClrWhite);
    	GrContextFontSet(&g_sContext, g_pFontCm14);
        GrContextFontSet(&g_sContext, g_pFontFixed6x8);
        GrStringDrawCentered(&g_sContext, "Ultrasonic", -1,
                             GrContextDpyWidthGet(&g_sContext) / 2, 4, 0);
        GrContextFontSet(&g_sContext, g_pFontFixed6x8);
        GrStringDraw(&g_sContext, "Distance:", -1, 16, 26, 0);
    	GrStringDraw(&g_sContext, distance_char, -1, 16, 36, 0);
    	GrFlush(&g_sContext);
    }
    ROM_IntMasterEnable();
}

//*****************************************************************************
//
// The interrupt handler for the second timer interrupt.
//
//*****************************************************************************
void
Timer1IntHandler(void)
{
	temp_time = ROM_TimerValueGet(TIMER1_BASE,TIMER_A);
	TimerIntClear(TIMER1_BASE,TIMER_CAPA_EVENT);
	SysCtlDelay(3);
	flag++;
	if(flag ==1)
	{
		if(start_cap == 1)
		{
			start = temp_time;
			start_cap = 0;
		}
		else
			flag--;
	}

	if(flag ==2)
	{
		end = temp_time;
		flag = 0;
	}
	distance_time = start - end;
	if(end > start)
		distance_time = distance_time + 65536;
	distance = distance_time / 82.5;
	TimerIntClear(TIMER1_BASE,TIMER_CAPA_EVENT);
}

//*****************************************************************************
//
// This example application demonstrates the use of the timers to generate
// periodic interrupts.
//
//*****************************************************************************


int
main(void)
{
	FPUEnable();
	FPULazyStackingEnable();
    tRectangle sRect;

    ROM_FPULazyStackingEnable();

    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    //
    // Initialize the display driver.
    //
    CFAL96x64x16Init();

    //
    // Initialize the graphics context and find the middle X coordinate.
    //
    GrContextInit(&g_sContext, &g_sCFAL96x64x16);

    //
    // Fill the top part of the screen with blue to create the banner.
    //
    sRect.sXMin = 0;
    sRect.sYMin = 0;
    sRect.sXMax = GrContextDpyWidthGet(&g_sContext) - 1;
    sRect.sYMax = 9;
    GrContextForegroundSet(&g_sContext, ClrDarkBlue);
    GrRectFill(&g_sContext, &sRect);

    //
    // Change foreground for white text.
    //
    GrContextForegroundSet(&g_sContext, ClrWhite);

    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&g_sContext, g_pFontFixed6x8);
    GrStringDrawCentered(&g_sContext, "Ultrasonic", -1,
                         GrContextDpyWidthGet(&g_sContext) / 2, 4, 0);

    //
    // Initialize timer status display.
    //
    GrContextFontSet(&g_sContext, g_pFontFixed6x8);
    GrStringDraw(&g_sContext, "Distance:", -1, 16, 26, 0);

    // Enable the timer
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

	// Enable the PWM
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);

	//Enable the GPIO
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //配置完系统寄存器后要延时一段时间
    SysCtlDelay(200);

    //Setup the Timer to cap the time
    ROM_GPIOPinConfigure(GPIO_PF2_T1CCP0);
    ROM_GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_2);
//    ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
//    ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_2);

    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE,GPIO_PIN_4);

    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    //TIMER_CFG_SPLIT_PAIR 将两个16位定时器分开；
    ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_CAP_TIME);
    ROM_TimerControlEvent(TIMER1_BASE,TIMER_A,TIMER_EVENT_BOTH_EDGES);

    //装载条件，如触发时间或者递减的最大值
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet() / 1000);
    ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, 0xffff);

    //使能定时器及中断
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerIntEnable(TIMER1_BASE, TIMER_CAPA_EVENT);

    ROM_IntEnable(INT_TIMER0A);
    ROM_IntEnable(INT_TIMER1A);

    //使能处理器中断
    ROM_IntMasterEnable();

    //使能定时器
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
    ROM_TimerEnable(TIMER1_BASE, TIMER_A);

	// 将PM5选择为PWM功能引脚，M0PWM5是指module1， 输出5，
    //对应的datasheet手册中可看出发生器为2
	ROM_GPIOPinConfigure(GPIO_PM7_M0PWM5);

	// 对PM5引脚作PWM功能的配置
	ROM_GPIOPinTypePWM(GPIO_PORTM_BASE, GPIO_PIN_7);

	// PWM时钟配置：不分频
	ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

	// 配置PWM发生器2：加减计数
	ROM_PWMGenConfigure(PWM_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN
	| PWM_GEN_MODE_NO_SYNC);

	 // 设置PWM发生器2的周期
	ROM_PWMGenPeriodSet(PWM_BASE, PWM_GEN_2, 6000);

	//设置PWM输出的占空比
	ROM_PWMPulseWidthSet(PWM_BASE, PWM_OUT_5, sound);

	// 使能PWM5的输出
	ROM_PWMOutputState(PWM_BASE, PWM_OUT_5_BIT, true);

	// 使能PWM发生器12
	ROM_PWMGenEnable(PWM_BASE, PWM_GEN_2);

    while(1)
    {
    	//距离小于20cm，蜂鸣器报警
    	if(distance < 200)
    	{
    		sound_turn = 0;
    		if(distance < 50)
    		{
    			sound = 6000 - 1;
    		}
    		else
    		{
    			sound = (200 - distance) / 150 * 6000 - 1;
    		}
    	}

    	//避免蜂鸣器因为扰动而不报警
    	else
    	{
    		if(sound_turn == 10)
    		{
    			sound = 1;
    			sound_turn = 0;
    		}
    		else
    		{
    			sound_turn++;
    		}

    	}
    	ROM_PWMPulseWidthSet(PWM_BASE, PWM_OUT_5, sound);

    	//产生触发的pwm脉冲
    	if(led == 0)
        {
        	GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_4, GPIO_PIN_4);
        	connect = 1;
        }
        else
        {
        	GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_4, 0);
        	//使得第二个定时器不胡乱触发
        	if(connect == 1)
        	{
        		connect = 0;
        		start_cap = 1;
        	}
        }
    }

}

