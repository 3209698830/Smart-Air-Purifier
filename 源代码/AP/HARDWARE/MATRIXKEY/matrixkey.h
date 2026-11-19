#ifndef __MATRXKEY_H
#define __MATRXKEY_H

#include "stm32f4xx.h"

typedef enum {
    MATRXKEY_NONE=0,
        
    MATRXKEY_S1,
    MATRXKEY_S2,
    MATRXKEY_S3,
    MATRXKEY_S4,    
             
    MATRXKEY_S5,
    MATRXKEY_S6,
    MATRXKEY_S7,
    MATRXKEY_S8,
             
    MATRXKEY_S9,
    MATRXKEY_S10,
    MATRXKEY_S11,
    MATRXKEY_S12,
             
    MATRXKEY_S13,
    MATRXKEY_S14,
    MATRXKEY_S15,
    MATRXKEY_S16,    
}MATRXKEY_ID;


//PC9
#define R1(x)    do{\
                if(x==1)\
                    GPIO_WriteBit(GPIOC, GPIO_Pin_9, 1);\
                else\
                    GPIO_WriteBit(GPIOC, GPIO_Pin_9, 0);\
                }while(0);
//PB6
#define R2(x)    do{\
                if(x==1)\
                    GPIO_WriteBit(GPIOB, GPIO_Pin_6, 1);\
                else\
                    GPIO_WriteBit(GPIOB, GPIO_Pin_6, 0);\
                }while(0);    
//PE6
#define R3(x)    do{\
                if(x==1)\
                    GPIO_WriteBit(GPIOE, GPIO_Pin_6, 1);\
                else\
                    GPIO_WriteBit(GPIOE, GPIO_Pin_6, 0);\
                }while(0);
//PA8
#define R4(x)    do{\
                if(x==1)\
                    GPIO_WriteBit(GPIOA, GPIO_Pin_8, 1);\
                else\
                    GPIO_WriteBit(GPIOA, GPIO_Pin_8, 0);\
                }while(0);            

//PC7
#define C1    GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7)                
//PG15
#define C2    GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_15)    
//PA4
#define C3    GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)    
//PB7
#define C4    GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7)                    
    

extern void MatrixKey_Init(void);
extern u8 MatrixKey_Scan(void);
extern void MatrixKey_Demo();
             
#endif