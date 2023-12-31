/*3-phase oscillator
   This circuit generate a triangle shaped PWM to digital output 3 6 and 11 with 120° phase delay.
   analog input 0 is frequency control (0.1hz to 20hz)
   Analog input 1 is Depth control(0% to 100%)
   Analog input 2 is OffSet control (min = -100 %, middle = 0% max = 100%)
By Tek465B
*/

//This set the input and output pin number and the analog input poll time
#define Out1   3
#define Out2   6
#define Out3   11
#define FPot   0
#define DPot   1
#define OPot   2
//#define interval2 50

//This set the start position and the direction
byte ch1 = 0;
int ch1O = 0;
byte ch2 = 170;
int ch2O = 0;
byte ch3 = 170;
int ch3O = 0;
int d1 = 1;
int d2 = 1;
int d3 = -1;
//This set the Depth at minimum and offset at middle.
volatile byte PWM = 0;
volatile int Offsetv = 0;

//this is used for the state machine
//unsigned long currentMillis;
//unsigned long previousMillis = 0;

//Variables for storing the analog input reading.
int Freq = 0;
int Freq1 = 0;
int FreqDebounce = 0;
int Depth = 0;
int Offset = 0;

//This store the value for the speed control
long interval;

void setup() {
  // put your setup code here, to run once:
  //This set the PWM frequency for pin 3 and 11
  noInterrupts();
  TCCR2B = TCCR2B & B11111000 | _BV(CS20);
  TCCR0B = TCCR0B & B11110000 | _BV(CS00);
  // this set Phase correct PWM for timer0
  TCCR0A = TCCR0A & B11111101 | _BV(WGM00);
  //This disable the millis function(overflow interrupt)
  TIMSK0 &= ~_BV(TOIE0);
  //cbi(TIMSK0, TOIE0);
  //This set the timer1 for timer interrupts
  
  TCCR1A = 0;
  TCCR1B = _BV(WGM12) | _BV(CS11);
  TCNT1 = 0;
  OCR1A = 20000;
  //TCCR1B |= (1 << WGM12);
  //TCCR1B |= (1 << CS11);
  TIMSK1 |= (1 << OCIE1A);
  interrupts();

}




    //This Set the Digital output PWM
    ISR(TIMER1_COMPA_vect)
{
    analogWrite(Out1, ch1O);
    analogWrite(Out2, ch2O);
    analogWrite(Out3, ch3O);
    //Counter for the trianglewave
    if ((ch1 == 255) && (d1 == 1)) {
      d1 = -1;
    }
    if ((ch1 == 0) && (d1 == -1)) {
      d1 = 1;
    }
    if ((ch2 == 255) && (d2 == 1)) {
      d2 = -1;
    }
    if ((ch2 == 0) && (d2 == -1)) {
      d2 = 1;
    }
    if ((ch3 == 255) && (d3 == 1)) {
      d3 = -1;
    }
    if ((ch3 == 0) && (d3 == -1)) {
      d3 = 1;
    }
    ch1 += d1;
    //This set the PWM according to the Depth and offset control
    ch1O = FMap(ch1, PWM, 5, PWM);
    ch1O += Offsetv;
    ch1O = constrain(ch1O, 0, 255);
    ch2 += d2;
    ch2O = FMap(ch2, PWM, 5, PWM);
    ch2O += Offsetv;
    ch2O = constrain(ch2O, 0, 255);
    ch3 += d3;
    ch3O = FMap(ch3, PWM, 5, PWM);
    ch3O += Offsetv;
    ch3O = constrain(ch3O, 0, 255);
}
void loop() {
     // put your main code here, to run repeatedly:
   //currentMillis = millis();
//Polling of the analog input
    //if (currentMillis - previousMillis > interval2) {
      //previousMillis = currentMillis;
      //This read the speed control and store its value into Freq variable.
      Freq1 = analogRead(FPot) + 1;
      if(Freq1 < (FreqDebounce - 1) || Freq1 > (FreqDebounce + 1)) {
      FreqDebounce = Freq1;
      //this use a logarithmic to make it a bit more human useable/linear.(output a value from 0 to 300)
      Freq = 100 * log10(Freq1);
      //This is to invert the value so clockwise = faster(300=0, 0=300).
      Freq = 301 - Freq;
      //mapping our value to the timer interrupt value.
      interval = map(Freq, 0, 300, 196, 39216);
      //When a change in speed is detected we temporarly disable interrupts and we set the counter 1 back to 0 and write the new interrupt(speed) value.
      if(interval != OCR1A) {
      noInterrupts();
      TCNT1 = 0;
      OCR1A = interval;
      interrupts(); 
      }
      }
      //This read and set the Depth control value
      Depth = analogRead(DPot);
      PWM = FMap(Depth, 255, 7, 255);
      //This read and set the offset control value
      Offset = analogRead(OPot);
      Offsetv = FMap(Offset, 255, 7, 510);
    //}

  

}

//Function to map number quickly
int FMap(int in, int out, int inr, int sel)
{
  return ((in * (out >> 3)) >> inr) + ((255 - sel) >> 1);
}
