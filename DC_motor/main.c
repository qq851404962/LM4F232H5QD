//******************** DC_motor*****************************
//该实验通过滑动变阻器控制电机转速，同时利用编码器在液晶屏幕上进行显示速度
//********************************************************

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
#include "driverlib/timer.h"

int p1 = 4200;
int p2 = 0;
int temp_p;
int turn = 0;
int test = 0;
int ival = 0;
float speed = 0;
int speed_int = 0;
char speed_char[9];

//ADC data
float vr = 0;
int last_button = 0;
int button = 0;
unsigned long ulADC0_Value;
int num = 0;
tContext sContext;
tRectangle sRect;
int show = 0;

//float 转 char
void FloatToChar(void)
{
	speed_int = (int) (speed * 10 * 60);
	speed_char[0] = speed_int / 10000 + '0';
	speed_int %= 10000;
	speed_char[1] = speed_int / 1000 + '0';
	speed_int %= 1000;
	speed_char[2] = speed_int / 100 + '0';
	speed_int %= 100;
	speed_char[3] = speed_int / 10 + '0';
	speed_int %= 10;
	speed_char[4] = '.';
	speed_char[5] = speed_int + '0';
	speed_char[6] = 'r';
	speed_char[7] = 'p';
	speed_char[8] = 'm';
}

void PortCIntHandler(void)
{
	ROM_GPIOPinIntClear(GPIO_PORTC_BASE, GPIO_PIN_7);
	ROM_GPIOPinIntClear(GPIO_PORTC_BASE, GPIO_PIN_7);
	ROM_GPIOPinIntClear(GPIO_PORTC_BASE, GPIO_PIN_7);
	num++;
	ROM_GPIOPinIntClear(GPIO_PORTC_BASE, GPIO_PIN_7);
}

void
Timer0IntHandler(void)
{
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    ROM_IntMasterDisable();
    speed = (num / 334.0) * 10;
    num = 0;
    if(show < 5)
    {
    	show++;
    }
    else
    {
    	FloatToChar();
    	show = 0;
    	display();
    }
    ROM_IntMasterEnable();
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void display(void)
{
	// 屏幕的上面24行填充蓝色覆盖上次的显示结果
	sRect.sXMin = 0;
	sRect.sYMin = 22;
	sRect.sXMax = GrContextDpyWidthGet(&sContext) - 1;
	sRect.sYMax = 63;
	GrContextForegroundSet(&sContext, ClrBlack);
	GrRectFill(&sContext, &sRect);
	// 更新显示
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextFontSet(&sContext, g_pFontCm14);
    GrContextFontSet(&sContext, g_pFontFixed6x8);
    GrStringDrawCentered(&sContext, "DC_motor", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 4, 0);
    GrContextFontSet(&sContext, g_pFontFixed6x8);
    GrStringDraw(&sContext, "speed:", -1, 16, 20, 0);
	GrStringDraw(&sContext, speed_char, -1, 16, 40, 0);
	GrFlush(&sContext);
}


int main (void)
{
    // 使能FPU
    FPUEnable();
	FPULazyStackingEnable();

	// 设置系统时钟为50MHz
	SysCtlClockSet(SYSCTL_SYSDIV_4 |SYSCTL_USE_PLL
	|SYSCTL_OSC_MAIN |SYSCTL_XTAL_16MHZ);

	CFAL96x64x16Init();
	GrContextInit(&sContext, &g_sCFAL96x64x16);

	// 使能PWM模块 */
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);
	// 使能PWM2和PWM3输出所在GPIO
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);

	// 将PH0/PH1选择为P WM功能引脚
	ROM_GPIOPinConfigure(GPIO_PH0_M0PWM0);
	ROM_GPIOPinConfigure(GPIO_PH1_M0PWM1);

	// 对PH0/PH1引脚作PWM功能的配置
	ROM_GPIOPinTypePWM(GPIO_PORTH_BASE, GPIO_PIN_0);
	ROM_GPIOPinTypePWM(GPIO_PORTH_BASE, GPIO_PIN_1);

	// PWM时钟配置：不分频
	ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
	// 配置PWM发生器1：加减计数
	ROM_PWMGenConfigure(PWM_BASE, PWM_GEN_0,PWM_GEN_MODE_UP_DOWN
	| PWM_GEN_MODE_NO_SYNC);

	 // 设置PWM发生器1的周期
	ROM_PWMGenPeriodSet(PWM_BASE, PWM_GEN_0, 6000);

	// 设置PWM2/PWM3输出的脉冲宽度
	ROM_PWMPulseWidthSet(PWM_BASE, PWM_OUT_0, 4200);
	ROM_PWMPulseWidthSet(PWM_BASE, PWM_OUT_1, 0);
	// 使能PWM2和PWM3的输出
	ROM_PWMOutputState(PWM_BASE, (PWM_OUT_0_BIT | PWM_OUT_1_BIT), true);
	// 使能PWM发生器1
	ROM_PWMGenEnable(PWM0_BASE, PWM_GEN_0);


    // 初始化ADC0/PD5
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_5);
    // 设置ADC参考电压为外部3V
    ROM_ADCReferenceSet(ADC0_BASE, ADC_REF_EXT_3V);
    // 配置ADC采集序列
    ROM_ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0 | ADC_CTL_END | ADC_CTL_IE);
    // 使能ADC采集序列
    ROM_ADCSequenceEnable(ADC0_BASE, 0);
    ROM_ADCIntClear(ADC0_BASE, 0);
    ROM_ADCIntEnable(ADC0_BASE, 0);

//    num++;
    //配置外设portf
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    ROM_GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_7);
    ROM_GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    ROM_GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_FALLING_EDGE);
    ROM_GPIOPinIntEnable(GPIO_PORTC_BASE, GPIO_PIN_7);
    ROM_IntEnable(INT_GPIOC);
    ROM_IntPrioritySet(INT_GPIOC, 0x00);

    //配置定时器
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Enable processor interrupts.
    ROM_IntMasterEnable();

    // Configure the two 32-bit periodic timers.
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet() / 10);

    // Setup the interrupts for the timer timeouts.
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Enable the timers.
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);

    ROM_IntMasterEnable();
    IntMasterEnable();

	while(1)
	{
    	// 触发采集
    	ADCProcessorTrigger(ADC0_BASE, 0);
    	// 等待采集结束
    	while(!ADCIntStatus(ADC0_BASE, 0, false)) ;
    	// 获取采集结果
    	ADCSequenceDataGet(ADC0_BASE, 0, &ulADC0_Value);
        // 将采集结果从32位无符号数转化为char

    	vr = ((float)ulADC0_Value) / 182;

        p1 = (vr / 3.3) * 2400 + 3600;
        p2 = 0;

    	// push the button
    	if(!(ROM_GPIOPinRead(GPIO_PORTM_BASE,GPIO_PIN_4)&&GPIO_PIN_4))
    	{
    		button = 1;
    	}
    	else
    		button = 0;

    	if(button == 1 && last_button == 0)
    	{
    		turn = 1 - turn;
    	}

    	last_button = button;

    	if(turn)
    	{
    		temp_p = p1;
    		p1 = p2;
    		p2 = temp_p;
    	}

		ROM_PWMPulseWidthSet(PWM_BASE, PWM_OUT_0, p1);
		ROM_PWMPulseWidthSet(PWM_BASE, PWM_OUT_1, p2);
	}
}
