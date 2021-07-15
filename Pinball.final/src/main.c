#include "stm32l476xx.h"
#include "helper_functions.h"
#include "adc.h"
#include "led_button.h"
#include "7seg.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define pins for 7_seg
#define SEG_gpio GPIOC
#define DIN_pin 3
#define CS_pin 4
#define CLK_pin 5

// Define pins for 8 leds
#define LED_gpio GPIOA
#define LED1_pin 0
#define LED2_pin 1
#define LED3_pin 5
#define LED4_pin 6
#define LED5_pin 7
#define LED6_pin 8
#define LED7_pin 9
#define LED8_pin 10

// pinball track voltage
#define ball_num 15
#define trackA 2.5
#define trackB 2.3
#define trackC 1.8
#define trackD 1.6
#define trackE 1.3
#define trackF 0.95
#define trackG 0.6
#define trackH 0.4

// Define Counter timer
#define COUNTER_timer TIM2

int button = 1;
int goal_track, track_kind;

void GPIO_init(){
	// Setup FPU
	SCB->CPACR |= (0xF << 20);
	__DSB();
	__ISB();
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;   // PA5 LED
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;   // PC13 Button
	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;     // ADC
	// Setup Button
	GPIOC->MODER &= ~GPIO_MODER_MODE13_Msk;
	GPIOC->MODER |= (0x0 << GPIO_MODER_MODE13_Pos);
	// Setup PA5 LED
	GPIOA->MODER &= ~GPIO_MODER_MODE5_Msk;
	GPIOA->MODER |= (0x1 << GPIO_MODER_MODE5_Pos);
	// PC0 ADC
	GPIOC->MODER &= ~GPIO_MODER_MODE0_Msk;
	GPIOC->MODER |= (0x3 << GPIO_MODER_MODE0_Pos);
	GPIOC->ASCR |= GPIO_ASCR_ASC0;
}

void EXTI_Setup(){
	// Enable SYSCFG CLK
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	// Select output bits
	SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13_Msk;
	SYSCFG->EXTICR[3] |= (2 << SYSCFG_EXTICR4_EXTI13_Pos);
	// Enable interrupt
	EXTI->IMR1 |= EXTI_IMR1_IM13;
	// Enable Falling Edge
	EXTI->RTSR1 |= EXTI_RTSR1_RT13;
	// Enable NVIC
	NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void ADCInit(){
	ADCResolution(ADC1, 0);              // 12 bits
	ADCContinuousConversion(ADC1, 0);    // enable continuous conversion
	ADCDataAlign(ADC1, 0);               // set right align
	ADCCommonDualMode(0);                // independent mode
	ADCCommonClockMode(1);               // hclk / 1
	ADCCommonPrescaler(0);               // div 1
	ADCCommonDMAMode(0);                 // disable dma
	ADCCommonDelayTwoSampling(0b0100);   // 5 adc clk cycle
	ADCChannel(ADC1, 1, 1, 2);           // channel 1, rank 1, 12.5 adc clock cycle
	ADCWakeup(ADC1);
	ADCInterrupt(ADC1, ADC_IER_EOCIE, 1);
	NVIC_EnableIRQ(ADC1_2_IRQn);
	ADCEnable(ADC1);
}

void EXTI15_10_IRQHandler(){
	if(EXTI->PR1 & EXTI_PR1_PIF13_Msk){
		button = 1-button;
		EXTI->PR1 = EXTI_PR1_PIF13_Msk;
	}
}

void SysTick_Handler() {
	if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){
		ADCStartConversion(ADC1);
	}
}

int ADC1_2_IRQHandler(){
	double voltage;
	int track;
	voltage = ADCGetValue(ADC1);
	voltage = (3.3 / 4095) * voltage;
	if(voltage>trackA && voltage<3)
		track = 1;
	else if(voltage>trackB)
		track = 2;
	else if(voltage>trackC)
		track = 3;
	else if(voltage>trackD)
		track = 4;
	else if(voltage>trackE)
		track = 5;
	else if(voltage>trackF)
		track = 6;
	else if(voltage>trackG)
		track = 7;
	else if(voltage>trackH)
		track = 8;
	else
		track = 9;
	return track;
}

int get_score(int track, int ball){
	if(ball == 20)
		return 0;
	else if(track_kind == 1){
		for(int i=goal_track; i<=8; i+=2){
			if(track==i){
				return 10;
			}

		}
		return 0;
	}
	else if(track_kind == 2){
		for(int i=goal_track; i<=8; i+=3){
			if(track==i)
				return 20;
		}
		return 0;
	}
	else if(track_kind == 3){
		if(track==goal_track || track==((goal_track+4)%8))
			return 30;
		return 0;
	}
	else{
		if(track==goal_track)
			return 40;
		return 0;
	}
}

void random_track(int ball){
	int seed,kind;
	//seed = COUNTER_timer->CNT;
	seed = ADCGetValue(ADC1);
	int leds[8] = {LED1_pin, LED2_pin, LED3_pin,
	LED4_pin,LED5_pin, LED6_pin, LED7_pin, LED8_pin};
	for(int i=0; i<8; ++i){
		reset_gpio(LED_gpio, leds[i]);
	}
	kind = seed%100;
	if(kind<50){
		if((seed%2) == 1){
			for(int i=0; i<8; i+=2){
				set_gpio(LED_gpio, leds[i]);
			}
			goal_track = 1;
			track_kind = 1;
		}
		else{
			for(int i=1; i<8; i+=2){
				set_gpio(LED_gpio, leds[i]);
			}
			goal_track = 2;
			track_kind = 1;
		}
	}
	else if(kind<75){
		if((seed%2) == 1){
			for(int i=0; i<8; i+=3){
				set_gpio(LED_gpio, leds[i]);
			}
			goal_track = 1;
			track_kind = 2;
		}
		else{
			for(int i=1; i<8; i+=3){
				set_gpio(LED_gpio, leds[i]);
			}
			goal_track = 2;
			track_kind = 2;
		}
	}
	else if(kind<90){
		seed = seed%8;
		set_gpio(LED_gpio, leds[seed]);
		set_gpio(LED_gpio, leds[(seed+4)%8]);
		goal_track = seed+1;
		track_kind = 3;
	}
	else{
		seed = seed%8;
		set_gpio(LED_gpio, leds[seed]);
		goal_track = seed+1;
		track_kind = 4;
	}
}

void startADC() {
	while (!(ADC1->ISR & ADC_ISR_ADRDY)) ADC1->CR |= ADC_CR_ADEN; // TURN ON
	delay_us(5000);
	ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR; // Clear flags
	ADC1->CR |= ADC_CR_ADSTART; // START CONV
}

int keypad(double ball,int score){
	int display;
	display = ball*10000 + score;
	return display;
}


int main(){
	// GPIO initialize
	GPIO_init();
	// 7_seg initialize
	if(init_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin) != 0){
		return -1;
	}
	// LED initialize
	if(init_led(LED_gpio, LED1_pin) != 0 || init_led(LED_gpio, LED2_pin) != 0 ||
			init_led(LED_gpio, LED3_pin) != 0 || init_led(LED_gpio, LED4_pin) != 0 ||
			init_led(LED_gpio, LED5_pin) != 0 || init_led(LED_gpio, LED6_pin) != 0 ||
			init_led(LED_gpio, LED7_pin) != 0 || init_led(LED_gpio, LED8_pin) != 0)
		{
			// Fail to init LED
			return -1;
		}
	// 7_seg setting
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DECODE_MODE, 0xFF);
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SCAN_LIMIT, 0x07);
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SHUTDOWN, 0x01);
	send_7seg(SEG_gpio,DIN_pin,CS_pin,CLK_pin,SEG_ADDRESS_ITENSITY,0x0F);


	// timer initialize

	/*timer_enable(COUNTER_timer);
	timer_init(COUNTER_timer, 40000, 100);
	timer_start(COUNTER_timer);*/

	EXTI_Setup();
	ADCInit();
	delay_ms(100);
	SystemClock_Config_Interrupt(4, 1000000);
	delay_ms(500);
	int ball=ball_num, add;
	int track, debounce_track=-1;
	int score=0,final_score;
	int debounce = 0;
	int a=1;
	while(1){
		if(button==0){
			display_number(SEG_gpio, DIN_pin, CS_pin,
			CLK_pin, final_score,num_digits(final_score));
		}
		else{
			if(a==1){
				random_track(ball);
				a=0;
			}
			track = ADC1_2_IRQHandler();
			if(track != 9){
				if(track==debounce_track){
					++debounce;
					if(debounce==2){
						ball--;
						++a;
						add = get_score(track,ball);
						score += add;
					}
				}
				else{
					debounce = 0;
					debounce_track=0;
				}
				debounce_track=track;
			}
			else{
				debounce = 0;
				debounce_track=0;
			}
			EXTI15_10_IRQHandler();
			delay_ms(100);
			//display_number(SEG_gpio, DIN_pin, CS_pin, CLK_pin, COUNTER_timer->CNT,8);
			display_number(SEG_gpio, DIN_pin, CS_pin, CLK_pin, keypad(ball, score),8);
			if(ball==0){
				button = 0;
				final_score = score;
				score = 0;
				ball = ball_num;
				a=1;
			}
		}
	}
	while(1){}
	return 0;
}
