#include "matrixkey.h"


static void delay(uint32_t ms)
{
    volatile uint32_t count, i;
    
    while(ms--) {
        // 内层循环产生约1ms延时
        for(count = 42000; count > 0; count--);
    }
}

void MatrixKey_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    /*开启时钟*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOG,ENABLE);
 
    /*GPIO初始化，将列设置为输入上拉模式*/
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init( GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init( GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;  
    GPIO_Init( GPIOG, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;  
    GPIO_Init( GPIOC, &GPIO_InitStruct);
    
    /*GPIO初始化，将行设置为推挽输出模式*/
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;  
    GPIO_Init( GPIOC, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;  
    GPIO_Init( GPIOB, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;  
    GPIO_Init( GPIOE, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;  
    GPIO_Init( GPIOA, &GPIO_InitStruct);
    
    R1(1);
    R2(1);
    R3(1);
    R4(1);
}
/*
逐行扫描的方法，Row记录当前的行号，函数的结尾Row＋＋以便快速切换行扫描，Switch就是用来切换当前某行输出为低电平。
当某行的某列按下时，延时一小会，取消抖动，再对所有列的具体键值逐一判断。
*/
u8 MatrixKey_Scan(void)
{
    static u8 Row = 1;
    
    /*不断切换行值*/
    switch(Row)
    {
        case 1:R1(0); R2(1);R3(1);R4(1);break;
        case 2:R2(0); R1(1);R3(1);R4(1);break;
        case 3:R3(0); R1(1);R2(1);R4(1);break;
        case 4:R4(0); R1(1);R2(1);R3(1);break;
        default:break;
    }
    /*确定当前行是否有按键按下*/
    if( C1 == 0 || C2 == 0 || C3 == 0 || C4 == 0)
    {
        /*延时一小会，防止抖动*/
        delay(15);
        
        /*当前行的第一列被按下*/
        if(C1 == 0)
        {
            /*松手检测*/
            while(C1 == 0);
            /*返回当前按键的索引，例如按下S5，则当前Row为2，带入公式返回5*/
            return 1+(Row-1)*4;
        }
        else if(C2 == 0)
        {
            while(C2 == 0);
            return 2+(Row-1)*4;
        }            
        else if(C3 == 0)
        {
            while(C3 == 0);
            return 3+(Row-1)*4;
        }
        else if(C4 == 0)
        {
            while(C4 == 0);
            return 4+(Row-1)*4;
        }
    }
    else
        ;
    /*当行值扫描到第四行，就从头开始扫描*/
    Row >=4? Row = 1:Row++;
    return 0;

}

void MatrixKey_Demo()
{
    while (1) {
        u8 key = MatrixKey_Scan();
        switch(key)
        {
            case MATRXKEY_S1:
                    GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
                    break;
            case MATRXKEY_S2:
                    GPIO_ToggleBits(GPIOF, GPIO_Pin_10);
                    break;            
            case MATRXKEY_S3:
                    GPIO_ToggleBits(GPIOE, GPIO_Pin_13);
                    break;
            case MATRXKEY_S4:
                    GPIO_ToggleBits(GPIOE, GPIO_Pin_14);
                    break;
            case MATRXKEY_S5:
                    break;
            case MATRXKEY_S6:
                    break;
            case MATRXKEY_S7:
                    break;
            case MATRXKEY_S8:
                    break;
            
            case MATRXKEY_S9:
                    break;
            case MATRXKEY_S10:
                    break;
            case MATRXKEY_S11:
                    break;
            case MATRXKEY_S12:
                    break;    
            case MATRXKEY_S13:
                    break;
            case MATRXKEY_S14:
                    break;
            case MATRXKEY_S15:
                    break;
            case MATRXKEY_S16:
                    break;                
        }
    }

}
