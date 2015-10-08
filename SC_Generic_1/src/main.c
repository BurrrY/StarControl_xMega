
#include <asf.h>
#include <adc.h>
#include <stdio.h>
#include <string.h>


void vbus_event(bool b_high);
void usb_init(void);

void my_button_unpress_event(void);
void my_button_press_event(void);

#define LED_RED    0x01
#define LED_GREEN    0x02

#define INSTRUCTION_SET_1 0x04
#define INSTRUCTION_SET_2 0x08
#define INSTRUCTION_SET_3 0x10
#define INSTRUCTION_SET_4 0x20
#define INSTRUCTION_SET_5 0x40
#define INSTRUCTION_SET_6 0x80

static void adc_init(void);
unsigned int ADCA_Conversion(ADC_CH_t *Channel, char Pin);

uint8_t oldState[6];

#define SAMPLE_COUNT 100
/*! Sample storage (all four channels).*/
int16_t adcSamples[4][SAMPLE_COUNT];
uint16_t interrupt_count = 0;
int8_t offset;

struct {
	int8_t x;
	uint8_t y;
	uint8_t buttons_A;
	uint8_t buttons_B;
	uint8_t buttons_C;
	uint8_t buttons_D;
} *report;

struct adc_config           adc_conf;
struct adc_channel_config       adc_ch_conf_ch0;


static void adc_init(void)
{
	ADCA.CTRLB = ADC_RESOLUTION_8BIT_gc;
	ADCA.REFCTRL = ADC_REFSEL_INT1V_gc | 0x02;
	ADCA.PRESCALER = ADC_PRESCALER_DIV256_gc;
	ADCA.CTRLA = ADC_ENABLE_bm;
}


int main (void)
{

	PORTD.DIR = 0x03;
	PORTCFG.MPCMASK = 0xFC;
	PORTD.PIN0CTRL = PORT_OPC_PULLDOWN_gc;
	PORTD.OUTCLR = LED_RED;
	PORTD.OUT = 0x03; //LED OUT
	
	
	PORTC.DIR = 0x00;
	PORTCFG.MPCMASK = 0xFF;
	PORTC.PIN3CTRL = PORT_OPC_PULLDOWN_gc;
	
	
	PORTA.DIR = 0x00;
/*

	PORTCFG.MPCMASK = 0xFC;
	PORTA.PIN3CTRL = PORT_OPC_PULLDOWN_gc;
	PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
	

	*/
	sysclk_init();
	irq_initialize_vectors();
	cpu_irq_enable();
	board_init();
	
	adc_init();
	
	udc_start();
	vbus_event(true);
	
	
	uint8_t oldStateC = 0;
	uint8_t currentStateC = 0;
	
	uint8_t oldStateA = 0;
	uint8_t currentStateA = 0;
	uint8_t sendReport = 0;
	
	PORTD.OUTSET = LED_RED;
	PORTD.OUTCLR = LED_GREEN;
	
	while(42) {
		//Buttons 1-5

			currentStateC = (PORTC.IN & 0x3F);
			
				if(PORTD.IN & INSTRUCTION_SET_1) {
					if(oldState[0] != currentStateC) {
						oldState[0] = currentStateC;
						sendReport = 1;
						report->buttons_A = currentStateC;	
					}
				}
				
				
				if(PORTD.IN & INSTRUCTION_SET_2) {
					if(oldState[1] != currentStateC) {
						oldState[1] = currentStateC;
						sendReport = 1;
						report->buttons_A = currentStateC<<5;
						report->buttons_B = currentStateC>>3;
					}
				}
				
				if(PORTD.IN & INSTRUCTION_SET_3) {
					if(oldState[2] != currentStateC) {
						oldState[2] = currentStateC;
						sendReport = 1;
						report->buttons_B = currentStateC<<2;
					}
				}
				
				
				if(PORTD.IN & INSTRUCTION_SET_4) {
					if(oldState[3] != currentStateC) {
						oldState[3] = currentStateC;
						sendReport = 1;
						report->buttons_B = currentStateC<<7;						
						report->buttons_C = currentStateC>>1;
					}	
				}
				
				if(PORTC.IN & INSTRUCTION_SET_5) {
					if(oldState[4] != currentStateC) {
						oldState[4] = currentStateC;
						sendReport = 1;
						report->buttons_C = currentStateC<<4;
						report->buttons_D = currentStateC>>4;
					}
				}
				
				
				if(PORTC.IN & INSTRUCTION_SET_6) {
					if(oldState[5] != currentStateC) {
						oldState[5] = currentStateC;
						sendReport = 1;
						report->buttons_D = currentStateC<<1;
					}
				}
			
			
/*
			currentStateA = PORTA.IN ;
			if(currentStateA != oldStateA) {
				report->buttons_D = currentStateA;
				oldStateA = currentStateA;
				sendReport = 1;
			}*/
			
			if(sendReport==1) {
				PORTD.OUTCLR = LED_RED;
				
				report->y = ADCA_Conversion(&(ADCA.CH0), 1);
				udi_hid_generic_send_report_in(report);
				delay_ms(10);
				sendReport = 0;
				report->buttons_A = 0;
				report->buttons_B = 0;
				report->buttons_C = 0;
				report->buttons_D = 0;
				PORTD.OUTSET = LED_RED;
			}
		delay_ms(100);
	}
	// Insert application code here, after the board has been initialized.
}


unsigned int ADCA_Conversion(ADC_CH_t *Channel, char Pin)
{
	Channel->INTFLAGS = 0x01;
	Channel->CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	Channel->MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;
	Channel->CTRL |= ADC_CH_START_bm;
	
	while(!Channel->INTFLAGS);
	return Channel->RES;
}


void vbus_event(bool b_high)
{
	if (b_high) {
		// Attach USB Device
		udc_attach();
		} else {
		// VBUS not present
		udc_detach();
	}
}

void usb_init(void)
{
	udc_start();
}

static bool my_flag_autorize_generic_events = false;
bool my_callback_generic_enable(void)
{
	my_flag_autorize_generic_events = true;
	return true;
}
void my_callback_generic_disable(void)
{
	my_flag_autorize_generic_events = false;
}

void my_callback_generic_report_out(uint8_t *report)
{
	/*if ((report[0] == MY_VALUE_0)
	(report[1] == MY_VALUE_1)) {
		// The report is correct
	}*/
}
void my_callback_generic_set_feature(uint8_t *report_feature)
{
/*
	if ((report_feature[0] == MY_VALUE_0)
	(report_feature[1] == MY_VALUE_1)) {
		// The report feature is correct
	}*/
}