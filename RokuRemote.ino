
// Using OC1 = PB1 = Pin 9
const int homeButton    = 3;
const int okButton      = 4;
const int rightButton   = 5;
const int leftButton    = 6;
const int upButton      = 7;
const int downButton    = 8;

#define IR_LED   9
#define REG_LED 10

#define PULSE_ON_PWM     (TCCR1A |= _BV(COM1A1))
#define PULSE_OFF_PWD    (TCCR1A &= ~(_BV(COM1A1)))
#define PULSE_ON         ledState=HIGH
#define PULSE_OFF        ledState=LOW

int ledState = LOW;             // ledState used to set the LED

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
    txNEC(dev,subdev,command); //(-78 delay = 40ms) embedded in tx
    
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
bool incmode;

void setup() {
  pinMode(homeButton, INPUT_PULLUP);
  pinMode(okButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(IR_LED, OUTPUT);
  pinMode(REG_LED, OUTPUT);
  digitalWrite(IR_LED, LOW);
  digitalWrite(REG_LED, LOW);
  init38kPulse();
  cnt = 0;
  Serial.begin(115200);
  incmode = false;
}

char x[10];

void loop()
{
    if (!digitalRead(homeButton)) {
        Serial.write("Home was pressed\n");
        txNEC(234, 194, 3);
    }
    if (!digitalRead(okButton)) {
        Serial.write("OK was pressed\n");
        txNEC(234, 194, 42);
    }
    if (!digitalRead(rightButton)) {
        Serial.write("Right was pressed\n");
        txNEC(234, 194, 173);
    }
    if (!digitalRead(leftButton)) {
        Serial.write("Left was pressed\n");
        txNEC(234, 194, 30);
    }
    if (!digitalRead(upButton)) {
        Serial.write("Up was pressed\n");
        txNEC(234, 194, 25);
    }
    if (!digitalRead(downButton)) {
        Serial.write("Down was pressed\n");
        txNEC(234, 194, 51);
    }
    if (Serial.available() > 0) {
        int i = Serial.parseInt();
        if (i == -1) {
            incmode = !incmode;
            Serial.write("Flipped mode.\n");
            return;
        }
        sprintf(x, "Read: %d\n", i);
        Serial.write(x);

        if (incmode) {
            cnt = i;
        } else {
            txRepNEC1(234, 194, i, 3000);
        }
    }

    if (incmode) {
        /*
        sprintf(x, "%d\n", cnt);
        Serial.write(x);
        txNEC(234, 194, 3);
        delay(200);
        txNEC(234, 194, 173);
        delay(500);
        txNEC(234, 194, 173);
        delay(1000);
        txNEC(234, 194, cnt++);
        delay(800);
        */
        switch (cnt) {
            case 3:
            case 25:
            case 51:
            case 173:
            case 42:
            case 180:
            case 179:
            case 203:
            case 75:
            case 97:
            case 225:
            case 144:
            case 16:
            case 210:
            case 82:
            case 160:
            case 153:
            case 230:
            case 102:
            case 131:
            case 248:
            case 8:
            case 120:
            case 127:
            case 132:
            case 136:
            case 143:
                break;

            default:
                sprintf(x, "%d\n", cnt);
                Serial.write(x);
                txNEC(234, 194, cnt);
                delay(800);
                break;
        }

        cnt++;
    }
}

// 38khz = 26.3us
// Although probably want  up down beat up down beat and not up beat down beat
// Where beat is the 38 khz metronome
void init38kPulse(void) {
    cli();
    const uint16_t ticks = 16000000/ 38000 / 2;
    TCCR1A = 0x00;         // Clear timer1's control registers
    TCCR1B = 0x00;
    TCCR1C = 0x00;
    
    TCNT1 = 0;             // Pre-load the timer to 0
    OCR1A = ticks-1;           // Set output compare register to 210. for 38khz
                           // Was originally 421, but divide by 2, because i think I want
                           // Up and then down, to happen each beat of the frequency
                           // Do i want/need to subtract 1 as well?
                  
    TCCR1B |= _BV(WGM12);  // Turn on CTC mode
    TCCR1B |= _BV(CS10);  // Set prescaler to 1
    TIMSK1 |= _BV(OCIE1A); // Enable timer compare interrupt 
    sei();
}

ISR(TIMER1_COMPA_vect)
{
    // Is there proper PWM thing on timer i should be using
    if (ledState == HIGH) {
        PORTB ^= (1 << 1) | (1 << 2);
    } else {
        PORTB &= ~((1 << 1) | (1 << 2));
    }
    // yeah, letus use OC1 == PB1 == Pin 9 instead
}

/* Experimentally Determined function codes so far
3           home
25          up
51          down
173         right
42          ok
180         <<
179         some form of down-ish
203/75      netflix
97/225      asterisk/options
144/16      amazon
210/82      pandora
160         play/pause ?
153         some uppish, maybe rewind?
230         back/actual back
102         Another form of back?
131         back3/home?
248         ?
8           vudu
120         ? 
127         crackle
132         ?
136         play?
143         movies
*/
    
/*void init38kPulse(void) {
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
    TCCR1B |= _BV(WGM13) | _BV(CS10);  // Set prescaler to 1,
    sei();
}*/
