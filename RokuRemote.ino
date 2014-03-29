
// Using OC1 = PB1 = Pin 9
const int ledPin =  9;      // the number of the LED pin
#define PULSE_ON     (TCCR1A |= _BV(COM1A1))
#define PULSE_OFF    (TCCR1A &= ~(_BV(COM1A1)))

int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated
unsigned long interval = 1000;           // interval at which to blink (milliseconds)

#define INIT_MARK   9000
#define INIT_SPACE  4500
#define BIT_MARK    562
#define ZERO_SPACE  562
#define ONE_SPACE   1687

#define REP_MARK    9000
#define REP_SPACE   2250

#define END_MARK    562

#define SEND_ZERO PULSE_ON;delayMicroseconds(BIT_MARK);PULSE_OFF;delayMicroseconds(ZERO_SPACE)
#define SEND_ONE  PULSE_ON;delayMicroseconds(BIT_MARK);PULSE_OFF;delayMicroseconds(ONE_SPACE)

void txNEC(unsigned char dev, unsigned char subdev, unsigned char command) {
    // Initial Framing?
    PULSE_ON; delayMicroseconds(INIT_MARK);
    PULSE_OFF; delayMicroseconds(INIT_SPACE);
    
    //Logical '0' – a 562.5µs pulse burst followed by a 562.5µs space, with a total transmit time of 1.125ms
    //Logical '1' – a 562.5µs pulse burst followed by a 1.6875ms space, with a total transmit time of 2.25ms
    //Note Data transferred Least Sig BIT first
    //8-bit address for receiving device
    for (int i = 0; i < 8; i++ ){
        if((dev >> i) & 0x1) {
            SEND_ONE;
        } else {
            SEND_ZERO;
        }
    }
    //the 8-bit logical inverse of the address -- Actually the sub address?
    for (int i = 0; i < 8; i++ ){
        if((subdev >> i) & 0x1) {
            SEND_ONE;
        } else {
            SEND_ZERO;
        }
    }
    
    //the 8-bit command
    for (int i = 0; i < 8; i++ ){
        if((command >> i) & 0x1) {
            SEND_ONE;
        } else {
            SEND_ZERO;
        }
    }
    
    //the 8-bit logical inverse of the command
    for (int i = 0; i < 8; i++ ){
        if(((~command) >> i) & 0x1) {
            SEND_ONE;
        } else {
            SEND_ZERO;
        }
    }
    
    //a final 562.5µs pulse burst to signify the end of message transmission.
    PULSE_ON; delayMicroseconds(END_MARK);
    PULSE_OFF;delay(40);
}

void txRepNEC1(unsigned char dev, unsigned char subdev, unsigned char command, unsigned int delta) {
    txNEC(dev,subdev,command); //-78 delay = 40ms embedded in tx
    
    unsigned long goal = millis() + delta;
    //unsigned long current = millis();
    
    while (millis() < goal) {
        PULSE_ON; delayMicroseconds(REP_MARK);
        PULSE_OFF;delayMicroseconds(REP_SPACE);
        PULSE_ON; delayMicroseconds(END_MARK);
        PULSE_OFF;delay(97); // Approx 110ms - duration of above (11.8215ms)
        //current = millis();
    }    
}

unsigned char cnt;
void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  init38kPulse();
  cnt = 0;
}



void loop()
{
  /*txRepNEC1(234, 194, 0xBF, 3000); //0xBF - power, 0xBB menu/home
  delay(1000);
  txRepNEC1(234, 194, 0xC9, 3000); //0xBF - power, 0xBB menu/home
  delay(1000);
  txRepNEC1(234, 194, 0xBB, 3000); //0xBF - power, 0xBB menu/home
  delay(2000);*/
  txNEC(234,194,cnt++);
}

void init38kPulse(void) {
    cli();
    const uint16_t ticks = 16000000 / 38222 / 2; // Should be about 210
    TCCR1A = 0x00;         // Clear timer1's control registers
    TCCR1B = 0x00;
    TCCR1C = 0x00;
    TCNT1 = 0;              // Pre-load the timer to 0. Not necessary for this mode
    
    ICR1 = ticks;           // See Below
    OCR1A = ticks / 3;      // See Below
    
    TCCR1A |= _BV(WGM11);  // Turn on PWM phase correct mode (mode 10),
                           // i.e. TOP is ICR1, Update of OCR1A at TOP,  TOV1 flag set on BOTTOM
    TCCR1B |= _BV(WGM13) | _BV(CS10)|_BV(CS12);  // Set prescaler to 1,
    sei();
}

// 38khz = 26.3us
// Although probably want  up down beat up down beat and not up beat down beat
// Where beat is the 38 khz metronome
/*void initTimer1(void) {
    cli();
    const uint16_t ticks = 16000000/ 38000 / 2;
    TCCR1A = 0x00;         // Clear timer1's control registers
    TCCR1B = 0x00;
    TCCR1C = 0x00;
    
    TCNT1 = 0;             // Pre-load the timer to 0
    OCR1A = ticks;           // Set output compare register to 210. for 38khz
                           // Was originally 421, but divide by 2, because i think I want
                           // Up and then down, to happen each beat of the frequency
                           // Do i need to subtract 1 as well?
                  
    TCCR1B |= _BV(WGM12);  // Turn on CTC mode
    TCCR1B |= _BV(CS10);  // Set prescaler to 1
    TIMSK1 |= _BV(OCIE1A); // Enable timer compare interrupt 
    sei();
}

ISR(TIMER1_COMPA_vect)
{
    // Is there proper PWM thing on timer i should be using
    if (ledState == HIGH) {
        PORTB ^= (1<<1);
    }
    // yeah, letus use OC1 == PB1 == Pin 9 instead
}*/