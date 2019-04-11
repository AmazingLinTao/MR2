
//*****************************************************************************//

#include "usart.h"
#include "gpio.h"
#include "dma.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"

#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"      //os ʹ��	  
#endif

uint8_t ch;
uint8_t ch_r;
//��д�������,�ض���printf���������ڣ���˼����˵printfֱ����������ڣ���Ĭ�����������̨��
/*fputc*/
int fputc(int c, FILE * f)
{
    ch=c;
    HAL_UART_Transmit(&huart2,&ch,1,1000);//���ʹ���
    //HAL_UART_Transmit_DMA(&huart2, (uint8_t *) &ch,sizeof(ch));//���ʹ���
    return c;
}
//�ض���scanf���������� ��˼����˵���ܴ��ڷ����������ݣ���Ĭ���ǽ��ܿ���̨������
/*fgetc*/
int fgetc(FILE * F)
{
    //HAL_UART_Receive_DMA (&huart2,&ch_r,1);//����
    HAL_UART_Receive (&huart2,&ch_r,1,0xffff);//����
    //HAL_UART_Receive_DMA(&huart2, USART2RxBuf, USART2RXBUFSIZE);
    return ch_r;
}
#if EN_USART_RX   //���ʹ���˽���
uint16_t USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
uint16_t aRxBuffer[RXBUFFERSIZE];//HAL��ʹ�õĴ��ڽ��ջ���
uint16_t USART_RX_STA=0;       //����״̬���
#endif

#if EN_USART2_RX   //���ʹ���˽���
uint16_t USART2_RX_STA=0;       //����״̬���
uint8_t USART2TxBuf[USART2TXBUFSIZE];
uint8_t USART2RxBuf[USART2RXBUFSIZE];
#endif

#if EN_USART6_RX   //���ʹ���˽���
uint16_t USART6_RX_STA=0;       //����״̬���
uint8_t USART6RxBuf[USART6RXBUFSIZE];
#endif

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart6_rx;

/* USART2 init function */
void MX_USART2_UART_Init(void)
{

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&huart2);					    //HAL_UART_Init()��ʹ��UART2

    // HAL_UART_Receive_DMA(&huart2, (uint8_t *)aRxBuffer, RXBUFFERSIZE);
    HAL_UART_Receive_IT(&huart2, (uint8_t *)aRxBuffer, RXBUFFERSIZE);//�ú����Ὺ�������жϣ���־λUART_IT_RXNE���������ý��ջ����Լ����ջ���������������

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USART6 init function */

void MX_USART6_UART_Init(void)
{

    huart6.Instance = USART6;
    huart6.Init.BaudRate = 115200;
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
    huart6.Init.StopBits = UART_STOPBITS_1;
    huart6.Init.Parity = UART_PARITY_NONE;
    huart6.Init.Mode = UART_MODE_TX_RX;
    huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart6.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart6) != HAL_OK)
    {
        Error_Handler();
    }

}

//UART�ײ��ʼ����ʱ��ʹ�ܣ��������ã��ж�����
//�˺����ᱻHAL_UART_Init()����
//huart:���ھ��
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct;

    if(uartHandle->Instance==USART2)
    {
        /* USER CODE BEGIN USART2_MspInit 0 */

        /* USER CODE END USART2_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        /**USART2 GPIO Configuration
        PD6     ------> USART2_RX
        PD5     ------> USART2_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* Peripheral DMA init*/

        hdma_usart2_rx.Instance = DMA1_Stream5;
        hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
        hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart2_rx.Init.Mode = DMA_NORMAL;
        hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

        /* Peripheral interrupt init */
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspInit 1 */
//HAL_DMA_Init(&hdma_usart2_rx);
        /* USER CODE END USART2_MspInit 1 */
    }

    else if(uartHandle->Instance==USART6)
    {
        /* USER CODE BEGIN USART6_MspInit 0 */

        /* USER CODE END USART6_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_USART6_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        /**USART6 GPIO Configuration
        PG14     ------> USART6_TX
        PG9     ------> USART6_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

        /* Peripheral DMA init*/

        hdma_usart6_rx.Instance = DMA2_Stream1;
        hdma_usart6_rx.Init.Channel = DMA_CHANNEL_5;
        hdma_usart6_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart6_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart6_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart6_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart6_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart6_rx.Init.Mode = DMA_CIRCULAR;
        hdma_usart6_rx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_usart6_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_usart6_rx) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart6_rx);

        /* Peripheral interrupt init */
        HAL_NVIC_SetPriority(USART6_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART6_IRQn);
        /* USER CODE BEGIN USART6_MspInit 1 */

        /* USER CODE END USART6_MspInit 1 */
    }
}

//�����жϻص�����
//һ�ν�����ִ��һ��
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance==USART2)//����Ǵ���2
    {
        if((USART_RX_STA&0x8000)==0)//����δ���
        {
            if(USART_RX_STA&0x4000)//���յ���0x0d
            {
                if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
                else USART_RX_STA|=0x8000;	//���������
            }
            else //��û�յ�0X0D
            {
                if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
                    USART_RX_STA++;
                    if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����
                }
            }
        }

    }
    else if(huart == &huart6)
    {

    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &huart2)
    {

    }
    else if(huart == &huart6)
    {

    }
}


//����2�жϷ������
//ԭ��������stm32f4_it.c���� ��ע�� ��������д
void USART2_IRQHandler(void)
{
    uint32_t timeout=0;
    uint32_t maxDelay=0x1FFFF;

    HAL_UART_IRQHandler(&huart2);	//����HAL���жϴ������ú���

    timeout=0;
    while (HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY)//�ȴ�����
    {
        timeout++;////��ʱ����
        if(timeout>maxDelay) break;
    }

    timeout=0;
    while(HAL_UART_Receive_IT(&huart2, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)//һ�δ������֮�����¿����жϲ�����RxXferCountΪ1
    {
        timeout++; //��ʱ����
        if(timeout>maxDelay) break;
    }
}



//void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
//{

//  if(uartHandle->Instance==USART1)
//  {
//  /* USER CODE BEGIN USART1_MspDeInit 0 */

//  /* USER CODE END USART1_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_USART1_CLK_DISABLE();
//
//    /**USART1 GPIO Configuration
//    PB7     ------> USART1_RX
//    PB6     ------> USART1_TX
//    */
//    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7|GPIO_PIN_6);

//    /* Peripheral DMA DeInit*/
//    HAL_DMA_DeInit(uartHandle->hdmarx);

//    /* Peripheral interrupt Deinit*/
//    HAL_NVIC_DisableIRQ(USART1_IRQn);

//  /* USER CODE BEGIN USART1_MspDeInit 1 */

//  /* USER CODE END USART1_MspDeInit 1 */
//  }
//  else if(uartHandle->Instance==USART2)
//  {
//  /* USER CODE BEGIN USART2_MspDeInit 0 */

//  /* USER CODE END USART2_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_USART2_CLK_DISABLE();
//
//    /**USART2 GPIO Configuration
//    PD6     ------> USART2_RX
//    PD5     ------> USART2_TX
//    */
//    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_6|GPIO_PIN_5);

//    /* Peripheral DMA DeInit*/
//    HAL_DMA_DeInit(uartHandle->hdmarx);

//    /* Peripheral interrupt Deinit*/
//    HAL_NVIC_DisableIRQ(USART2_IRQn);

//  /* USER CODE BEGIN USART2_MspDeInit 1 */

//  /* USER CODE END USART2_MspDeInit 1 */
//  }
//  else if(uartHandle->Instance==USART3)
//  {
//  /* USER CODE BEGIN USART3_MspDeInit 0 */

//  /* USER CODE END USART3_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_USART3_CLK_DISABLE();
//
//    /**USART3 GPIO Configuration
//    PD9     ------> USART3_RX
//    PD8     ------> USART3_TX
//    */
//    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_9|GPIO_PIN_8);

//    /* Peripheral DMA DeInit*/
//    HAL_DMA_DeInit(uartHandle->hdmarx);

//    /* Peripheral interrupt Deinit*/
//    HAL_NVIC_DisableIRQ(USART3_IRQn);

//  /* USER CODE BEGIN USART3_MspDeInit 1 */

//  /* USER CODE END USART3_MspDeInit 1 */
//  }
//  else if(uartHandle->Instance==USART6)
//  {
//  /* USER CODE BEGIN USART6_MspDeInit 0 */

//  /* USER CODE END USART6_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_USART6_CLK_DISABLE();
//
//    /**USART6 GPIO Configuration
//    PG14     ------> USART6_TX
//    PG9     ------> USART6_RX
//    */
//    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_14|GPIO_PIN_9);

//    /* Peripheral DMA DeInit*/
//    HAL_DMA_DeInit(uartHandle->hdmarx);

//    /* Peripheral interrupt Deinit*/
//    HAL_NVIC_DisableIRQ(USART6_IRQn);

//  /* USER CODE BEGIN USART6_MspDeInit 1 */

//  /* USER CODE END USART6_MspDeInit 1 */
//  }
//}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/