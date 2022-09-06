/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#define CCM_RAM __attribute__((section(".ccmram")))
#define Treceiver 100
// ----------------------------------------------------------------------------
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------


void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}

void vApplicationTickHook(void) {
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

//------------------------------------------------------------------------------

TaskHandle_t myTask1sHandle=NULL;
TaskHandle_t myTask2sHandle=NULL;
TaskHandle_t myTask3rHandle=NULL;
QueueHandle_t myQueue=NULL;
SemaphoreHandle_t our_sender_Semaphore=NULL;
SemaphoreHandle_t our_reciever_Semaphore=NULL;
TimerHandle_t myTimer_1=NULL;
TimerHandle_t myTimer_2=NULL;
TimerHandle_t myTimer_3=NULL;
// ----------------------------------------------------------------------------
int Range_Low_Array[6]={50, 80, 110, 140, 170, 200};
int Range_High_Array[6]={150, 200, 250, 300, 350, 400};
int Tsender= 0;
int BlockedMessagesCounter=0;
int TransmittedMessagesCounter=0;
int RecievedMessagesCounter=0;
int Current_period=0;
char myTxBuffer[200];
char myRxBuffer[200];
// ----------------------------------------------------------------------------
void vTimerCallbackSender(TimerHandle_t xTimer1)
{
	xSemaphoreGive(our_sender_Semaphore);
}
void vTimerCallbackReceiver(TimerHandle_t xTimer2)
{
	if(RecievedMessagesCounter==500)
	{
		Reset_After_500MS();
	}
	xSemaphoreGive(our_reciever_Semaphore);
}
void myTask1sender(void *p1)
{
	while(1)
		{  xSemaphoreTake(our_sender_Semaphore,0xffffffff);


		     if(xQueueSend(myQueue,& myTxBuffer,(TickType_t) 0))
		     {  TransmittedMessagesCounter++;
		    	 printf(myTxBuffer,"Time is ",xTaskGetTickCount);

		     }
		    else
		    {
			  BlockedMessagesCounter++;
		    }
			}}
	void myTask2sender(void *p2)
	{
		while(1)
			{  xSemaphoreTake(our_sender_Semaphore,0xffffffff);


			     if(xQueueSend(myQueue,& myTxBuffer,(TickType_t) 0))
			     {  TransmittedMessagesCounter++;
			    	 printf(myTxBuffer,"Time is ",xTaskGetTickCount);

			     }
			    else
			    {
				  BlockedMessagesCounter++;
			    }
				}}
void myTask3reciever(void *p3)
{
	while(1) {
		xSemaphoreTake(our_reciever_Semaphore,0xffffffff);
		if (myQueue != 0)
		{
				if (xQueueReceive(myQueue,& myRxBuffer,0)==pdPASS)
		    {
					printf(myRxBuffer);
					RecievedMessagesCounter++;
		    }
		}
		}
    }

int uniform_distribution(int rangeLow, int rangeHigh)
{
    int myRand = (int)rand();
    int range = rangeHigh - rangeLow + 1; //+1 makes it [rangeLow, rangeHigh], inclusive.
    int myRand_scaled = (myRand % range) + rangeLow;
    return myRand_scaled;
}
void Reset_After_500MS()
{
	printf("The total number of successfully sent messages=, The total number of blocked messages= %c\n", TransmittedMessagesCounter , BlockedMessagesCounter );
	TransmittedMessagesCounter=0;
	BlockedMessagesCounter=0;
	RecievedMessagesCounter=0;
	Current_period++;
	xQueueReset(myQueue);
	xTimerChangePeriod( myTimer_1 ,pdMS_TO_TICKS(Tsender) , 1) ;
	xTimerChangePeriod( myTimer_2 ,pdMS_TO_TICKS(Tsender) , 1) ;
	xTimerReset( myTimer_1 , 0 );
	xTimerReset( myTimer_2 , 0 );
	xTimerReset( myTimer_3 , 0 );
	if (Current_period==6)
			{
			     printf("game over!");
				xTimerDelete( myTimer_1 , 0);
				xTimerDelete( myTimer_2 , 0);
				xTimerDelete( myTimer_3 , 0);
				vTaskEndScheduler();
			}

}

int main(int argc, char *argv[])
{
	myQueue= xQueueCreate(2,200*sizeof(char));
	xTaskCreate(myTask1sender,"task1Sender",10000 ,(void*) 0,1,& myTask1sHandle );
	xTaskCreate(myTask2sender,"task2Sender",10000 ,(void*) 0,1,& myTask2sHandle );
	xTaskCreate(myTask3reciever,"task3Reciver",10000,(void*) 0,2,& myTask3rHandle);
    our_sender_Semaphore= xSemaphoreCreateBinary();
	our_reciever_Semaphore=xSemaphoreCreateBinary();
    Tsender= uniform_distribution(Range_Low_Array[Current_period],Range_High_Array[Current_period]);
	myTimer_1=xTimerCreate("TimerTask 1",pdMS_TO_TICKS(Tsender),pdTRUE,(void*)0,vTimerCallbackSender);
	myTimer_2=xTimerCreate("TimerTask 2", pdMS_TO_TICKS(Tsender),pdTRUE,( void *) 0 ,vTimerCallbackSender);
	myTimer_3=xTimerCreate("TimerTask 3", pdMS_TO_TICKS(Treceiver),pdTRUE,( void *) 0 ,vTimerCallbackReceiver);
	if ((myTimer_1 != NULL) && (myTimer_2 != NULL) && (myTimer_3 != NULL))
	{
		xTimerStart(myTimer_1,0);
		xTimerStart(myTimer_2,0);
		xTimerStart(myTimer_3,0);
	}
	if ((xTimerStart(myTimer_1,0) == pdPASS) && (xTimerStart(myTimer_2,0)== pdPASS) && (xTimerStart(myTimer_3,0) == pdPASS))
		{
			        vTaskStartScheduler();
		}

	return 0;}




