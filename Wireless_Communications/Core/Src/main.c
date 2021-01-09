/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "stdio.h"
#include "nrf24L01.h"
#include "string.h"
#include "time.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define STATE_UNCHECK_MODE 0
#define STATE_IDLE 1
#define STATE_REQ_CONN 2
#define STATE_WAIT_CONN 3
#define STATE_TRY_SEND 4
#define STATE_TRY_RECV 5

#define l 22
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// addr
extern uint8_t TX_ADDRESS[]; //send address
extern uint8_t RX_ADDRESS[]; //接收地址

extern uint8_t recv_ch;
extern uint8_t send_ch;
uint8_t init = 1;

// command
static uint8_t cmd_flag = 0;
static uint8_t cmd_buff[33] = "wuwu\0";
static int TLE_SCALE = 10;

static uint8_t friend_mode = 0;
static int send_to_friend = 2;

// LCD
int lastOutput = 0;
int fontColor[100];

char outputData[100][l + 1];
static uint8_t counter = 0;

static int state = 0;
static int conn_init_flag = 0;

// 串口数据
static int uLength = 0;
uint8_t rxBuffer[33];
static uint8_t inputData[60] = " \0";
static uint8_t send_flag = 0;
static uint8_t recv_flag = 0;
static uint8_t send_buff[60] = "send~~0";
static uint8_t recv_buff[60] = " \0";

static uint8_t tip[] =
		"Command Format <code>:<content>\n"
				" code         meaning                          detail in content\n"
				"  1   Modify message recv address      Ten digit hexadecimal number\n"
				"  2   Modify message send address      Ten digit hexadecimal number\n"
				"  3   Modify RF recv channel           Decimal number less than 255\n"
				"  4   Modify RF send receive channel   Decimal number less than 255\n"
				"  5   Modify TLE time                  Decimal number, int, unit is second\n"
				"  6   Modify friend mode               0 is unset friend mode, 1 is set\n"
				"  7   Modify send/recv mode            0: recv mode, 1: send recv mode, valid in friend mode\n"
				"  8   Print this help message          There can be no content, but don't forget the colon : \n";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void reset_flags() {
	send_flag = 0;
	recv_flag = 0;
	state = 0;
	conn_init_flag = 0;
	counter = 0;
	lastOutput = 0;
	cmd_flag = 0;

	init = 1;
	LCD_Rec();
//	NRF24L01_TX_Mode();
	NRF24L01_Init();
	HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	LCD_Fill(181, 21, 199, 29, YELLOW);
}
int substr(char dst[], char src[], int start, int len) {
	char *p = src + start;   //定义指针变量指向�??要提取的字符的地�??
	int n = strlen(p);       //求字符串长度
	int i = 0;
	if (n < len) {
		len = n;

	}
	while (len != 0) {
		dst[i] = src[i + start];
		len--;
		i++;
	}                        //复制字符串到dst�??
	dst[i] = '\0';
	return 0;
}

void LCD_Rec() {
	if (init == 1) {
		LCD_Clear(GRAY);

		LCD_Fill(10, 10, 220, 90, WHITE);
		LCD_Fill(5, 5, 10, 95, YELLOW);
		LCD_Fill(10, 5, 50, 10, YELLOW);
		LCD_Fill(230, 100, 240, 310, YELLOW);
		LCD_Fill(100, 310, 240, 320, YELLOW);
		POINT_COLOR = BLACK;
		init = 0;

	}
}
void LCD_Head() {
	uint8_t temp[32];
	POINT_COLOR = RED;
	LCD_ShowString(15, 15, 200, 24, 24, (uint8_t*) "Group: 30");
	LCD_ShowNum(155, 20, TLE_SCALE, 3, 12);
	char LocalMac[] = { };
	POINT_COLOR = BLUE;
	sprintf(temp, "S: %02x.%02x.%02x.%02x.%02x / %d", TX_ADDRESS[0],
			TX_ADDRESS[1], TX_ADDRESS[2], TX_ADDRESS[3], TX_ADDRESS[4],
			send_ch);
	LCD_ShowString(55, 50, 200, 24, 12, (uint8_t*) temp);

	sprintf(temp, "R: %02x.%02x.%02x.%02x.%02x / %d", RX_ADDRESS[0],
			RX_ADDRESS[1], RX_ADDRESS[2], RX_ADDRESS[3], RX_ADDRESS[4],
			recv_ch);
	LCD_ShowString(55, 65, 200, 24, 12, (uint8_t*) temp);

	/*LCD_ShowString(10, 45, 200, 24, 12, (uint8_t*) TX_ADDRESS); //local mac
	 LCD_ShowString(10, 60, 200, 24, 12, (uint8_t*) RX_ADDRESS); //pair mac
	 LCD_ShowString(70, 45, 200, 24, 12, (uint8_t*) recv_ch); //local ch
	 LCD_ShowString(70, 60, 200, 24, 12, (uint8_t*) send_ch); //pair ch*/

	if (NRF24L01_Check() == 1) {
//		POINT_COLOR = RED;
//		LCD_DrawRectangle(180, 20, 200, 30);
		LCD_Fill(181, 21, 199, 29, RED);
//		LCD_ShowString(100, 10, 200, 24, 16, (uint8_t*) "NRF24L01 is ERROR");
	} else {
//		POINT_COLOR = GREEN;
//		LCD_ShowString(100, 10, 200, 24, 16, (uint8_t*) "NRF24L01 is OK");
//		LCD_DrawRectangle(180, 20, 200, 30);
		if (state == STATE_TRY_SEND || state == STATE_TRY_RECV) {
			LCD_Fill(181, 21, 199, 29, GREEN);
		} else {
			LCD_Fill(181, 21, 199, 29, YELLOW);
		}

	}
}

void LCD_Unconn() {
	//LCD_Head();
	LCD_Rec();
//	LCD_ShowString(10, 75, 200, 24, 12, (uint8_t*) "KEY_0: disconnect");
//	LCD_ShowString(10, 90, 200, 24, 12,
//			(uint8_t*) "Use \"\" to set new pair MAC");
//	LCD_ShowString(10, 105, 200, 24, 12,
//			(uint8_t*) "Use \"\" to set new channel");

}

void LCD_Conn() {
	if (send_flag == 0 && recv_flag == 0) {
		if (conn_init_flag == 0) {
			return;
		} else {
			conn_init_flag = 0;
		}

	}
	char *white_space = "                    ";
	char inputData[1024] = { 0 };
	//LCD_Head();

	POINT_COLOR = YELLOW;
	LCD_Color_Fill(10, 110, 220, 300, WHITE);

	//inputData = {};

	if (send_flag == 1 && state == STATE_TRY_SEND) {
		int size_s = strlen(send_buff);
		int start_s = 0;

		while (size_s > 0) {
			if (size_s < l) {
				strcpy(outputData[lastOutput], white_space);
				int white_space_size = l - size_s;
				substr(outputData[lastOutput] + white_space_size, send_buff,
						start_s * l, size_s);
			} else {
				substr(outputData[lastOutput], send_buff, start_s * l, l);
			}
			fontColor[lastOutput] = 0;
			size_s -= l;
			++start_s;

			++lastOutput;

		}
	}                        //send
	if (recv_flag == 1 && state == STATE_TRY_RECV) {
		int size_r = strlen(recv_buff);
		int start_r = 0;
		while (size_r > 0) {
			substr(outputData[lastOutput], recv_buff, start_r * l, l);
			++start_r;
			size_r -= l;
			fontColor[lastOutput] = 1;
			++lastOutput;
		} //receive
		recv_flag = 0;
	}
	int outpoint = lastOutput > 8 ? lastOutput - 8 : 0;
	int y = 110;

	while (outpoint < lastOutput) {
		POINT_COLOR = fontColor[outpoint] == 0 ? BRRED : BLACK;
		LCD_ShowString(25, y, 200, 15, 16, (uint8_t*) outputData[outpoint]);
		y += 24;
		++outpoint;
	}
	//LCD_ShowString(10, 130, 200, 10, 16, (uint8_t*) "SPACE");

}

uint8_t get_hex_num(uint8_t c) {
	if (c >= '0' && c <= '9') {
		return (uint8_t) (c - '0');
	} else if (c >= 'a' && c < 'z') {
		return (uint8_t) (c - 'a' + 10);
	}
	return (uint8_t) 1;
}

void LED_control() {
	if (counter % 20 == 0) {
		switch (state) {
		case STATE_UNCHECK_MODE:
			break;
		case STATE_IDLE:
			break;
		case STATE_REQ_CONN:
			HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
			break;
		case STATE_WAIT_CONN:
			HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
			break;
		case STATE_TRY_SEND:
			HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
			HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
			break;
		case STATE_TRY_RECV:
			HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
			HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
			break;
		default:
			break;

		}
	}

}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
//	char msg[40];
	/*uint8_t *plus = '+';
	 HAL_UART_Transmit(&huart1, (uint8_t*) plus, strlen(plus), 1000);*/

	if (huart->Instance == USART1) {
		if (rxBuffer[0] == '\n') {
			inputData[uLength] = '\0';
			printf("DEBUG: data input from UART --> ");
			printf(inputData);
			printf("\n");
			if (inputData[0] > '0' && inputData[0] < '9'
					&& inputData[1] == ':') {
				cmd_flag = inputData[0] - '0';
				strcpy(cmd_buff, inputData + 2);
				switch (cmd_flag) {
				case 1:
					for (int i = 0; i < 5; ++i) {
						RX_ADDRESS[i] = get_hex_num(cmd_buff[2 * i]) * 16
								+ get_hex_num(cmd_buff[2 * i + 1]);
					}
					printf("DEBUG: Change RX_ADDRESS to %02x%02x%02x%02x%02x\n",
							RX_ADDRESS[0], RX_ADDRESS[1], RX_ADDRESS[2],
							RX_ADDRESS[3], RX_ADDRESS[4]);
					break;
				case 2:
					for (int i = 0; i < 5; ++i) {
						TX_ADDRESS[i] = get_hex_num(cmd_buff[2 * i]) * 16
								+ get_hex_num(cmd_buff[2 * i + 1]);
					}
					printf("DEBUG: Change TX_ADDRESS to %02x%02x%02x%02x%02x\n",
							TX_ADDRESS[0], TX_ADDRESS[1], TX_ADDRESS[2],
							TX_ADDRESS[3], TX_ADDRESS[4]);
					break;
				case 3:
					recv_ch = atoi(cmd_buff);
					printf("DEBUG: Change recv_ch to %s\n", cmd_buff);
					break;
				case 4:
					send_ch = atoi(cmd_buff);
					printf("DEBUG: Change send_ch to %s\n", cmd_buff);
					break;
				case 5:
					TLE_SCALE = atoi(cmd_buff);
					printf("DEBUG: Change TLE scale to %d\n", TLE_SCALE);
					break;
				case 6:
					friend_mode = atoi(cmd_buff);
					printf("DEBUG: Change friend mode to %d\n", friend_mode);
					break;
				case 7:
					if (friend_mode == 1) {
						send_to_friend = atoi(cmd_buff);
						if (send_to_friend == 1 && state == STATE_TRY_RECV) {
							state = STATE_TRY_SEND;
							printf(
									"DEBUG: Change mode to send to friend.\n");
						} else if (send_to_friend
								== 0&& state == STATE_TRY_SEND) {
							state = STATE_TRY_RECV;
							printf("DEBUG: Change mode to recv from friend.\n");
						}
					}
					break;
				case 8:
					printf(tip);
				default:
					break;
				}
			} else {
				strcpy(send_buff, inputData);
				send_flag = 1;
				/*printf("DEBUG: set send_flag: %d, data recv from uart: %s\n",
				 send_flag, inputData);*/

			}
			uLength = 0;
		} else {
			inputData[uLength] = rxBuffer[0];
			++uLength;
		}
	}
}

void try_conn() {
	uint8_t tmp_buff[33] = "-\0";
	uint8_t nrf_state = 0;

	if (state == STATE_REQ_CONN) {
		NRF24L01_TX_Mode();

		printf("DEBUG: Try conn other.\n");

		for (int i = 0; i < 180 * TLE_SCALE; ++i) {
			nrf_state = NRF24L01_TxPacket(tmp_buff);
			HAL_Delay(5);

			++counter;
			LED_control();

			if (nrf_state == TX_OK) {
				printf("DEBUG: Active request connection successful.\n");
				if(friend_mode == 1){
					printf("Recv mode, input command 7:1 change to Send Mode\n");
				}
				state = STATE_TRY_RECV;
				conn_init_flag = 1;
				HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
				return;
			}

			if (state != STATE_REQ_CONN) {
				printf("DEBUG: Exit try request conn\n");
				break;
			}
		}
	}

	if (state == STATE_WAIT_CONN) {
		NRF24L01_RX_Mode();

		printf("DEBUG: Active connection fail, wait other conn me\n");

		for (int j = 0; j < 200 * TLE_SCALE; ++j) {
			nrf_state = NRF24L01_RxPacket(tmp_buff);
			HAL_Delay(5);
			++counter;
			LED_control();

			if (nrf_state == 0) {
				state = STATE_TRY_SEND;
				printf("DEBUG: Accept(passive) connection successful\n");
				if(friend_mode == 1){
					printf("Send mode, input command 7:0 change to Recv Mode or chat msg to send\n");
				}
				conn_init_flag = 1;
				HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
				return;
			}
			if (state != STATE_WAIT_CONN) {
				printf("DEBUG: Exit try waiting conn\n");
				return;
			}

		}
		printf("DEBUG: Try conn time out, exit try conn\n");
//		state = STATE_IDLE;
		reset_flags();
	}

}

void try_recv() {
	uint8_t tmp_buff[60] = "+\0";
	uint8_t nrf_state = 0;

	NRF24L01_RX_Mode();

	for (int i = 0; i < 200 * TLE_SCALE; ++i) {
		if (state != STATE_TRY_RECV) {
			break;
		}
		++counter;
		LED_control();

		nrf_state = NRF24L01_RxPacket(tmp_buff);
		HAL_Delay(5);

		if (nrf_state == 0) {
			state = STATE_TRY_SEND;
			LCD_Fill(181, 21, 199, 29, GREEN);
			if (tmp_buff[0] == '@') {
				/*HAL_UART_Transmit(&huart1, (uint8_t*) recv_buff + 1,
				 strlen(recv_buff) - 1, 500);*/
				recv_flag = 1;
				strcpy(recv_buff, tmp_buff + 1);
				printf("DEBUG: recv --> ");
				printf(recv_buff);
				printf("\n");
			}
			return;
		}
		if (i > 140 * TLE_SCALE) {
			LCD_Fill(181, 21, 199, 29, YELLOW);
		}
	}

	if(friend_mode == 0 || state == STATE_IDLE ){
		printf("DEBUG: try recv ping time out, disconnect.\n");
			/*state = STATE_IDLE;
			 init = 1;*/
			reset_flags();
	}
}

void try_send() {
	uint8_t tmp_buff[60] = " \0";
	uint8_t nrf_state = 0;

	if (send_flag == 1) {
		tmp_buff[0] = '@';
		strcpy(tmp_buff + 1, send_buff);
	}

	NRF24L01_TX_Mode();

	for (int i = 0; i < 80 * TLE_SCALE; ++i) {
		if (state != STATE_TRY_SEND) {
			break;
		}

		++counter;
		LED_control();

		nrf_state = NRF24L01_TxPacket(tmp_buff);
//		HAL_Delay(5);

		if (nrf_state == TX_OK) {
			state = STATE_TRY_RECV;

			LCD_Fill(181, 21, 199, 29, GREEN);
			if (send_flag == 1) {
				send_flag = 0;
				printf("DEBUG: send --> ");
				printf(send_buff);
				printf("\n");
				/*HAL_UART_Transmit(&huart1, (uint8_t*) send_buff,
				 strlen(send_buff),
				 500);*/
			}
			return;
		}

		if (i > 56 * TLE_SCALE) {
			LCD_Fill(181, 21, 199, 29, YELLOW);
		}

	}

	if(friend_mode==0 || state == STATE_IDLE){
		printf("DEBUG: try send ping time out, disconnect.\n");
			/*state = STATE_IDLE;
			 init = 1;*/
			reset_flags();
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//	HAL_Delay(100);
	switch (GPIO_Pin) {

	case KEY_UP_Pin:
		if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == GPIO_PIN_RESET) {
			//HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
			HAL_Delay(100);
			if (state == STATE_REQ_CONN) {
				state = STATE_WAIT_CONN;
				printf("DEBUG: Try waiting active conn\n");
			} else if (state == STATE_WAIT_CONN) {
				state = STATE_IDLE;
				reset_flags();
				printf("DEBUG: Active cancel connection\n");
			} else {
				/*state = STATE_IDLE;
				 init = 1;*/
				reset_flags();
				printf("DEBUG: Active disconnection\n");
			}
		}
		break;
	case KEY_DOWN_Pin:
		if (HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port, KEY_DOWN_Pin)
				== GPIO_PIN_RESET) {
//			HAL_Delay(100);
//	 HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
			if (state == STATE_IDLE) {
				printf("DEBUG: Try active conn\n");
				state = STATE_REQ_CONN;
			} /*else if (state == STATE_REQ_CONN) {
			 state = STATE_WAIT_CONN;
			 printf("DEBUG: Try waiting active conn\n");
			 }*/
		}
		break;
	case KEY_WK_Pin:
		/*if (HAL_GPIO_ReadPin(KEY_WK_GPIO_Port, KEY_WK_Pin) == GPIO_PIN_SET) {
		 HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
		 HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
		 }*/
		break;
	default:
		break;
	}
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */
	LCD_Init();
	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_SPI1_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */

	NRF24L01_Init();
	init = 1;
	LCD_Rec();
	state = STATE_UNCHECK_MODE;
	HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	HAL_Delay(5);
	HAL_UART_Receive_IT(&huart1, (uint8_t*) rxBuffer, 1);

	for (int j = 0; j < 100; ++j) {
		outputData[j][l] = '\0';
	}
	printf(tip);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	while (1) {
		++counter;
		//printf("%d", state);
		HAL_Delay(20);
		if (counter % 25 == 0 && NRF24L01_Check() == 1) {
			state = STATE_UNCHECK_MODE;
			printf("DEBUG: Module exception.\n");
		}
		// USER CODE END WHILE
		LCD_Head();
		switch (state) {
		case STATE_UNCHECK_MODE:
			LCD_Unconn();
			if (counter % 50 == 0) {
				printf("DEBUG: Checking module\n");
			}
			for (int i = 0; i < 300; ++i) {
				if (NRF24L01_Check() == 0) {
					printf("DEBUG: Module pass check\n");
					state = STATE_IDLE;
					break;
				}
			}

			break;

		case STATE_REQ_CONN:
			LCD_Unconn();
			HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			try_conn();
			break;
		case STATE_TRY_RECV:
			LCD_Conn();
			try_recv();
			if (friend_mode == 1 && send_to_friend == 0) {
				state = STATE_TRY_RECV;
			}
			break;
		case STATE_TRY_SEND:
			LCD_Conn();
			try_send();
			if (friend_mode == 1 && send_to_friend == 1) {
				state = STATE_TRY_SEND;
			}
			break;
		default:
			LCD_Unconn();
			break;
		}
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
