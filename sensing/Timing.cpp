#include <stm32f10x_conf.h>
#include <stdint.h>

/**
 * 5 channel 1 input capture timer configuration
 */
 void TIM5_Cap_Init(u16 arr,u16 psc)
{
	  //structurs identification
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM5_ICInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	
    //setting clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);    /*Enable TIM5 clock*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); /*Enable GPIOA clock*/
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; /**/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; /*PA0 input*/
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    GPIO_ResetBits(GPIOA,GPIO_Pin_0); /*PA0 drop-down*/
    
    /*Initialize the timer 5 TIM5*/
    TIM_TimeBaseStructure.TIM_Period = arr; /*Set the counter and automatic reload value */
    TIM_TimeBaseStructure.TIM_Prescaler = psc; /*The prescaler */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; /*Set the clock division: TDTS = Tck_tim*/
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; /*TIM up counting mode*/
    TIM_TimeBaseInit(TIM5,&TIM_TimeBaseStructure); /*According to the TIM_TimeBaseInitStruct specified in the TIMx parameter initialization time base unit*/
            
    /* Initialize the TIM5 input capture parameters */
    TIM5_ICInitStructure.TIM_Channel = TIM_Channel_1; /*CC1S=01     Select the input IC1 is mapped to TI1*/    
    TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising; /*Rising along the capture*/
    TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; /*Mapping to the TI1*/
    TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1; /*Configure input frequency, regardless of the frequency*/
    TIM5_ICInitStructure.TIM_ICFilter = 0; /*The IC1F=0000 configuration input filter without filter*/
    TIM_ICInit(TIM5,&TIM5_ICInitStructure); 
    
    /*Interrupt packet initialization*/
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;    /*TIM5 interrupt*/
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; /*Preemptive priority level 2*/
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /*From the priority level 0*/
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; /*The IRQ channel is enabled*/
    NVIC_Init(&NVIC_InitStructure);
    
    TIM_ITConfig(TIM5,TIM_IT_Update|TIM_IT_CC1,ENABLE);/*Allow updates to interrupt, allows the CC1IE to capture interrupt*/
    TIM_Cmd(TIM5,ENABLE); /*Enable the timer 5*/    
}

u8 TIM5CH1_CAPTURE_STA=0;    //Input capture state                         
u16    TIM5CH1_CAPTURE_VAL=0;    //Input capture value

/**
 * Timer 5 interrupt service routine     
 */
void TIM5_IRQHandler(void)
{ 
    if((TIM5CH1_CAPTURE_STA&0X80)==0)//Not successfully capture    
    {
        if(TIM_GetITStatus(TIM5,TIM_IT_Update) != RESET)
        {
            if(TIM5CH1_CAPTURE_STA&0X40)//Has captured to a high level
            {
                if((TIM5CH1_CAPTURE_STA&0X3F)==0X3F)//The high level is too long
                {
                    TIM5CH1_CAPTURE_STA|=0X80;//Marker successfully captured a
                    TIM5CH1_CAPTURE_VAL=0XFFFF;
                }else TIM5CH1_CAPTURE_STA++;
            }
        }
        
        if(TIM_GetITStatus(TIM5,TIM_IT_CC1) !=RESET)
        {
            if(TIM5CH1_CAPTURE_STA & 0x40)
            {
                TIM5CH1_CAPTURE_STA|=0X80;        //Marker successfully captures a rising edge
                TIM5CH1_CAPTURE_VAL = TIM_GetCounter(TIM5);
                TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Rising); //CC1P=0 is set to rise along the capture
            }
            else
            {
                TIM5CH1_CAPTURE_STA=0;            //Empty
                TIM5CH1_CAPTURE_VAL=0;
                
                TIM_SetCounter(TIM5,0);
                TIM5CH1_CAPTURE_STA|=0X40;        //Mark captured the rising edge
                TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Falling);        //CC1P=1 is set to decrease along the capture
            }
        }
        
    }
    TIM_ClearITPendingBit(TIM5,TIM_IT_CC1|TIM_IT_Update); /*Clear interrupt flag*/
}

extern u8 TIM5CH1_CAPTURE_STA;        //Input capture state                         
extern u16    TIM5CH1_CAPTURE_VAL;    //Input capture value    
 int main(void)
 {        
     u32 temp=0; 
 
     TIM5_Cap_Init(0XFFFF,72-1);    //The frequency count 1Mhz 
       while(1)
    {
         if(TIM5CH1_CAPTURE_STA&0X80)//The successful capture to a rising edge
        {
            temp=TIM5CH1_CAPTURE_STA&0X3F;
            temp*=65536;//Overflow time sum
            temp+=TIM5CH1_CAPTURE_VAL;//High level of total time
            //printf("HIGH:%d us\r\n",temp);//Print the total high level time
            TIM5CH1_CAPTURE_STA=0;//Open the next acquisition
        }
    }
 }