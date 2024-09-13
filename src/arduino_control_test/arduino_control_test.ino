#include <OneWire.h> 
#include <DallasTemperature.h> 
#include <avr/interrupt.h>  // For setting up the timer interrupt
#define DQ_Pin 2  

OneWire oneWire(DQ_Pin);
DallasTemperature sensors(&oneWire);
volatile int interruptCount = 0; //volatile for read in ISR (read value directly from memory)
volatile float TempC = 0; //set volatile as global variable for temperature and will be modified in ISR

// Timer setup for periodic interrupt (5 second)
void setupTimer() {
  // Set timer1 to call an interrupt every 5 second (1 Hz)
  noInterrupts();           // Disable all interrupts
  TCCR1A = 0;               // Set entire TCCR1A register to 0
  TCCR1B = 0;               // Same for TCCR1B
  TCNT1  = 0;               // Initialize counter value to 0
  // Set compare match register for 1 Hz increments
  OCR1A = 15624;            // = (16*10^6) / (1024*1) - 1 (must be <65536) (16000000/1024)-1
  TCCR1B |= (1 << WGM12);   // Turn on CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10);  // Prescaler 1024
  TIMSK1 |= (1 << OCIE1A);  // Enable timer compare interrupt
  interrupts();             // Enable all interrupts
}


void setup(void)
{

  Serial.begin(9600);

  pinMode(7,OUTPUT); //Tec-12708
  pinMode(4,OUTPUT); //Cpu Tower Fan
  sensors.begin(); //initialize ds18b20(TO-92 package) sensor
  digitalWrite(7,HIGH);
  digitalWrite(4,HIGH);
  setupTimer();     // Initialize the timer interrupt
}

void loop(void)
{
	if(TempC > 18){
    		digitalWrite(4,HIGH);
    		digitalWrite(7,HIGH);
    		if(TempC > 24){
      			//alert and noitfication
    		}
	}
  	}else if(TempC < 14){
	    	digitalWrite(7,LOW);
	    	digitalWrite(4,LOW);
		if(TempC < 7){
			//alert and noitfication
		}
	}else{
		//do nothing(Temperature is in the range of 14-18) ==> Keep the status of the devices
	}
}

ISR(TIMER1_COMPA_vect) {
  interruptCount++;

  if(interruptCount >= 5) {
    sensors.requestTemperatures();
    TempC = sensors.getTempCByIndex(0);
    Serial.print("Temperatures --> ");
    Serial.print(TempC);
    Serial.println("°C");
  // Store the temperature from sensor index 0
    interruptCount = 0;
  }
}

