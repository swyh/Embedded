/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"
#include <time.h>

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_RND_TASKS                     4       /* Number of identical tasks                          */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        RndTaskStk[N_RND_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        DecTaskStk[TASK_STK_SIZE];
OS_STK        TaskStartStk[TASK_STK_SIZE];
char          TaskData[N_RND_TASKS];                      /* Parameters to pass to each task               */
OS_EVENT     *RandomSem;
OS_EVENT	 *myMailbox[8];	// 8개의 mailbox를 생성하기 위한 event control block

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  DecisionTask(void *data);                       /* Function prototypes of tasks                  */
void  RandomTask(void *data);
void  TaskStart(void *data);                  /* Function prototypes of Startup task           */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main(void)
{
	INT8U i;

	PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

	OSInit();                                              /* Initialize uC/OS-II                      */

	PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
	PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

														   // Initiate sem
	RandomSem = OSSemCreate(1);

	OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
	OSStart();                                             /* Start multitasking                       */
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart(void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
	OS_CPU_SR  cpu_sr;
#endif
	char       s[100];
	INT16S     key;


	pdata = pdata;                                         /* Prevent compiler warning                 */

	TaskStartDispInit();                                   /* Initialize the display                   */

	OS_ENTER_CRITICAL();
	PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
	PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
	OS_EXIT_CRITICAL();

	OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

	TaskStartCreateTasks();                                /* Create all the application tasks         */

	for (;;) {
		TaskStartDisp();                                  /* Update the display                       */


		if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
			if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
				PC_DOSReturn();                            /* Return to DOS                            */
			}
		}

		OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
		OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
	}
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                        INITIALIZE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDispInit(void)
{
	/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
	/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
	PC_DispStr(0, 0, "                         uC/OS-II, The Real-Time Kernel                         ", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
	PC_DispStr(0, 1, "                                Embedded S/W 003                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 2, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 3, "                                      Week 5                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 5, "   Task1:   [ ]  Task2:   [ ]  Task3:   [ ]  Task4:   [ ]                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 6, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 8, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 9, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 10, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 11, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 12, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 13, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 14, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 15, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 16, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 17, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 18, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 19, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 20, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 21, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 22, "#Tasks          :        CPU Usage:     %                                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 23, "#Task switch/sec:                                                               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 24, "                            <-PRESS 'ESC' TO QUIT->                             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY + DISP_BLINK);
	/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
	/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           UPDATE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDisp(void)
{
	char   s[80];


	sprintf(s, "%5d", OSTaskCtr);                                  /* Display #tasks running               */
	PC_DispStr(18, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

#if OS_TASK_STAT_EN > 0
	sprintf(s, "%3d", OSCPUUsage);                                 /* Display CPU usage in %               */
	PC_DispStr(36, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);
#endif

	sprintf(s, "%5d", OSCtxSwCtr);                                 /* Display #context switches per second */
	PC_DispStr(18, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

	sprintf(s, "V%1d.%02d", OSVersion() / 100, OSVersion() % 100); /* Display uC/OS-II's version number    */
	PC_DispStr(75, 24, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

	switch (_8087) {                                               /* Display whether FPU present          */
	case 0:
		PC_DispStr(71, 22, " NO  FPU ", DISP_FGND_YELLOW + DISP_BGND_BLUE);
		break;

	case 1:
		PC_DispStr(71, 22, " 8087 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
		break;

	case 2:
		PC_DispStr(71, 22, "80287 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
		break;

	case 3:
		PC_DispStr(71, 22, "80387 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
		break;
	}
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks(void)
{
	INT8U i;

	// 8개의 Mailbox를 생성
	for (i = 0; i < 8; i++)
		myMailbox[i] = OSMboxCreate((void*)0);

	//DecisionTask 생성
	OSTaskCreate(DecisionTask, (void *)5, &DecTaskStk[TASK_STK_SIZE - 1], 5);

	// 4개의 RandomTask 생성
	for (i = 0; i < N_RND_TASKS; i++) {
		TaskData[i] = i;
		OSTaskCreate(RandomTask, (void *)&TaskData[i], &RndTaskStk[i][TASK_STK_SIZE - 1], 1 + i);	//2번째 파라미터 : 파라미터로 넘길 값
		}   


}


// Define maximum random number and tables
INT8U  const  myMapTbl[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
INT8U  const  myUnMapTbl[] = {
	0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x00 to 0x0F                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x10 to 0x1F                             */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x20 to 0x2F                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x30 to 0x3F                             */
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x40 to 0x4F                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x50 to 0x5F                             */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x60 to 0x6F                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x70 to 0x7F                             */
	7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x80 to 0x8F                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x90 to 0x9F                             */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xA0 to 0xAF                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xB0 to 0xBF                             */
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xC0 to 0xCF                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xD0 to 0xDF                             */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xE0 to 0xEF                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0        /* 0xF0 to 0xFF                             */
};



/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void  DecisionTask(void *pdata)	// 4개의 RandomTask로부터 random한 숫자를 받아, 가장 작은 숫자를 준 task에게 'W' 전달. 나머지 task에는 'L' 전달
{

	// Variable declaration section
	INT8U  myRdyTbl[] = { 0,0,0,0,0,0,0,0 };
	INT8U task_value[] = { 0,0,0,0 };
	INT8U  myRdyGrp;
	INT8U err;
	INT8U value;
	INT8U value_x, value_y;
	INT8U min;
	INT8U i;
	INT8U alphabet;
	// For compiler warning
	pdata = pdata;

	for (;;) {
		for (i = 0; i < 4; i++) {
			value = *(char*)OSMboxPend(myMailbox[i], 0, &err);	//mailbox 0 ~ 4번 순서대로 기다림
			task_value[i] = value;	// 값을 받은 경우 task_value 배열 i번째에 값 저장

			// 값 대입
			value_y = (value >> 3);   // 앞 3비트
			value_x = (value & 0x07);   // 뒤 3비트

			myRdyGrp |= OSMapTbl[value_y];   // myRdyGrp의 해당되는 비트를 1로 만듬
			myRdyTbl[value_y] |= OSMapTbl[value_x];   // myRdy의 해당되는 인덱스에서 비트를 1로 만듬
		}

		// find min value
		value_y = myUnMapTbl[myRdyGrp];   // 앞 3비트
		value_x = myUnMapTbl[myRdyTbl[value_y]];   // 뒤 3비트
		min = (value_y << 3) | value_x;   // 번호 최소값


		for (i = 0; i < 4; i++) {
			if (min == task_value[i]) {	// 최소값일 경우 W
				task_value[i] = 'W';
				min = -1;	// 여러 task가 같은 값의 최소값일 경우를 대비 
			}
			else
				task_value[i] = 'L';	// 최소값이 아닐 경우 L
			OSMboxPost(myMailbox[i + 4], &task_value[i]);	// 각 task에게 문자 전달 위해 mailbox로 보냄
		}

		for (i = 0; i < 8; i++)	// 초기화
			myRdyTbl[i] = 0;
		myRdyGrp = 0;

		OSTimeDly(1);
	}

}


void RandomTask(void *pdata) {	// random한 값을 생성하여 출력하고, DecisionTask에게 보냄. 
								// DecisionTask에게 다시 문자를 받아 W인 task의 색깔로 화면을 칠한다.
	// Variable declaration sectionx
	INT8U   rnd;
	INT8U   err;
	INT8U   alphabet;
	INT8U   i, j;
	INT8U   color;
	// For compiler warning
	pdata = pdata;
	// Input random seed
	srand(time(NULL));

	for (;;) {
		// Create a random number
		OSSemPend(RandomSem, 0, &err);
		rnd = random(64);	//0 ~ 63의 random 값 생성
		OSSemPost(RandomSem);

		//각 task 번호에 맞는 화면 위치에 출력
		if (*(char*)pdata == 0) {
			PC_DispChar(10, 5, '0' + rnd / 10, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			PC_DispChar(11, 5, '0' + rnd % 10, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		}
		else if (*(char*)pdata == 1) {
			PC_DispChar(24, 5, '0' + rnd / 10, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			PC_DispChar(25, 5, '0' + rnd % 10, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		}
		else if (*(char*)pdata == 2) {
			PC_DispChar(38, 5, '0' + rnd / 10, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			PC_DispChar(39, 5, '0' + rnd % 10, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		}
		else if (*(char*)pdata == 3) {
			PC_DispChar(52, 5, '0' + rnd / 10, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			PC_DispChar(53, 5, '0' + rnd % 10, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		}
		OSMboxPost(myMailbox[*(char*)pdata], &rnd);	// random 값을 각 mailbox로 보냄


		alphabet = *(char*)OSMboxPend(myMailbox[*(char*)pdata + 4], 0, &err);	// mailbox로부터 값을 기다림

		//mailbox로부터 받은 값을 각 task 화면 위치에 출력
		if (*(char*)pdata == 0) {
			PC_DispChar(13, 5, alphabet, DISP_FGND_WHITE + DISP_BGND_RED);
		}
		else if (*(char*)pdata == 1) {
			PC_DispChar(27, 5, alphabet, DISP_FGND_WHITE + DISP_BGND_CYAN);
		}
		else if (*(char*)pdata == 2) {
			PC_DispChar(41, 5, alphabet, DISP_FGND_WHITE + DISP_BGND_BLUE);
		}
		else if (*(char*)pdata == 3) {
			PC_DispChar(55, 5, alphabet, DISP_FGND_WHITE + DISP_BGND_GREEN);
		}

		//받은 값이 W일 경우 최소값이므로 화면을 각 색깔에 맞게 출력
		if (alphabet == 'W') {
			if (*(char*)pdata == 0)
				color = DISP_BGND_RED;
			else if (*(char*)pdata == 1)
				color = DISP_BGND_CYAN;
			else if (*(char*)pdata == 2)
				color = DISP_BGND_BLUE;
			else if (*(char*)pdata == 3)
				color = DISP_BGND_GREEN;

			for (i = 6; i <= 20; i++) {
				for (j = 0; j < 80; j++) {
					PC_DispChar(j, i, ' ', DISP_FGND_WHITE + color);
				}
			}
		}

		OSTimeDly(500);
	}
}