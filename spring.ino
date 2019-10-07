#include "HX711.h"
#include "Wire.h"
#include "Adafruit_MCP23017.h"

#define MANOMETER_DOUT_PIN1  4
#define MANOMETER_SCK_PIN1   5

#define MANOMETER_DOUT_PIN2  6
#define MANOMETER_SCK_PIN2   7

#define MANOMETER_DOUT_PIN3  8
#define MANOMETER_SCK_PIN3   9

#define KOMPRESOR_PIN 2

#define SPUST1    A3
#define SPUST2    A2
#define SPUST3    A1

#define WPUST1    12
#define WPUST2    13
#define WPUST3    A0

int LED_PIN[] = {10,9,8};
#define LED_GRN 0
#define LED_YLW 1
#define LED_RED 2

Adafruit_MCP23017 mcp;
/*
HX711 mankiet1;
HX711 mankiet2;
HX711 mankiet3;
*/
/*
 *     K - kompresor        - OFF/ON
 * W1-W3 - zawór wpustowy   - CLOSE/OPEN
 * S1-S3 - zawór spustowy   - CLOSE/OPEN
 * P1-P3 - odczyt manometru - [mmHg]
 * T1,T2 - odczyt timera    - [s]
 * T1R,T2R - reset timera   - DONT/RESET
 */

#define LOW_INTENSITY    0
#define MEDIUM_INTENSITY 1
#define HIGH_INTENSITY   2

#define CONTINUUS_PROGRAM 0
#define PULSATE_PROGRAM   1


int last_click_time[] = {0,0};
int last_click_val[]  = {1,1};
int click_pin[]       = {3,4}; //pins 3 and 4 are input pins on WCMCU 2317


#define INTENSITY_PIN_IDX 0
#define PROGRAM_PIN_IDX 1


#define NO_CLICK_INTERVAL 200



//[program][intensity][cycle_step]{K, W1,W2,W3,S1,S2,S3, P1, P2, P3, T1,  T2,  T1R,T2R}

                                    //CONTINUUS_PROGRAM
                                    //LOW_INTENSITY
unsigned char prog[][3][5][14] = {{{{1, 0, 0, 1, 0, 0, 0, 200,200, 50, 100, 100, 1,  1  },
                                    {1, 0, 1, 0, 0, 0, 0, 200, 40,200, 100, 100, 1,  1  },
                                    {1, 1, 0, 0, 0, 0, 0,  30,200,200, 100, 100, 1,  1  },
                                    {0, 0, 1, 0, 0, 0, 0, 200,200,200, 10,  100, 1,  0  },
                                    {0, 1, 0, 0, 1, 1, 1, 200,200,200, 100, 10 , 0,  1  }},
                                    
                                    //MEDIUM_INTENSITY
                                   {{1, 0, 0, 1, 0, 0, 0, 200,200, 80, 100, 100, 1,  1  },
                                    {1, 0, 1, 0, 0, 0, 0, 200, 70,200, 100, 100, 1,  1  },
                                    {1, 1, 0, 0, 0, 0, 0,  60,200,200, 100, 100, 1,  1  },
                                    {0, 0, 1, 0, 0, 0, 0, 200,200,200, 10,  100, 1,  0  },
                                    {0, 1, 0, 0, 1, 1, 1, 200,200,200, 100, 10 , 0,  1  }},
        
                                    //HIGH_INTENSITY
                                   {{1, 0, 0, 1, 0, 0, 0, 200,200,110, 100, 100, 1,  1  },
                                    {1, 0, 1, 0, 0, 0, 0, 200,100,200, 100, 100, 1,  1  },
                                    {1, 1, 0, 0, 0, 0, 0,  90,200,200, 100, 100, 1,  1  },
                                    {0, 0, 1, 0, 0, 0, 0, 200,200,200, 10,  100, 1,  0  },
                                    {0, 1, 0, 0, 1, 1, 1, 200,200,200, 100, 10 , 0,  1  }}},
        
                                 
                                    //PULSATE_PROGRAM
                                    //LOW_INTENSITY
                                  {{{1, 0, 0, 1, 0, 0, 0, 200,200, 50, 100, 100, 1,  1  },
                                    {1, 0, 1, 0, 0, 0, 1, 200, 40,200, 100, 100, 1,  1  },
                                    {1, 1, 0, 0, 0, 1, 1,  30,200,200, 100, 100, 1,  1  },
                                    {0, 0, 1, 0, 1, 1, 1, 200,200,200, 10,  100, 1,  0  },
                                    {0, 1, 0, 0, 1, 1, 1, 200,200,200, 100, 0,   0,  1  }},
                                    
                                    //MEDIUM_INTENSITY
                                   {{1, 0, 0, 1, 0, 0, 0, 200,200, 80, 100, 100, 1,  1  },
                                    {1, 0, 1, 0, 0, 0, 1, 200, 70,200, 100, 100, 1,  1  },
                                    {1, 1, 0, 0, 0, 1, 1,  60,200,200, 100, 100, 1,  1  },
                                    {0, 0, 1, 0, 1, 1, 1, 200,200,200, 10,  100, 1,  0  },
                                    {0, 1, 0, 0, 1, 1, 1, 200,200,200, 100, 0,   0,  1  }},
                                    
                                    //HIGH_INTENSITY
                                   {{1, 0, 0, 1, 0, 0, 0, 200,200,110, 100, 100, 1,  1  },
                                    {1, 0, 1, 0, 0, 0, 1, 200,100,200, 100, 100, 1,  1  },
                                    {1, 1, 0, 0, 0, 1, 1,  90,200,200, 100, 100, 1,  1  },
                                    {0, 0, 1, 0, 1, 1, 1, 200,200,200, 10,  100, 1,  0  },
                                    {0, 1, 0, 0, 1, 1, 1, 200,200,200, 100, 0,   0,  1  }}}};
        
long unsigned timer_begin[] = {0,0,0};
int state = 0;
int cycle_len = 5;
int intensity = LOW_INTENSITY;
int program   = CONTINUUS_PROGRAM;

void timer_reset(int nr) {
  timer_begin[nr] = millis()/1000;
}


long unsigned get_timer_val  (int nr) {
  return millis()/1000 - timer_begin[nr];
}

//                          {K, W1,W2,W3,S1,S2,S3, P1, P2, P3, T1,  T2,  T1R,T2R}
void set_state () {

  Serial.print("ustawiam stan: ");
  Serial.println(state);
  
  if (prog[program][intensity][state][0]) {
    digitalWrite(KOMPRESOR_PIN, LOW );
    Serial.println("kompresor\t\tON");
  }
  else {
    digitalWrite(KOMPRESOR_PIN, HIGH);
    Serial.println("kompresor\t\tOFF");
  }
  
  if (prog[program][intensity][state][1]) {
    digitalWrite(WPUST1,    HIGH);
    Serial.println("WPUST1\t\tOPEN");
  }
  else {
    digitalWrite(WPUST1,    LOW );
    Serial.println("WPUST1\t\tCLOSE");
  }
  if (prog[program][intensity][state][2]) {
    digitalWrite(WPUST2,    HIGH);
    Serial.println("WPUST2\t\tOPEN");
  }
  else {
    digitalWrite(WPUST2,    LOW );
    Serial.println("WPUST2\t\tCLOSE");
  }
  if (prog[program][intensity][state][3]) {
    digitalWrite(WPUST3,    HIGH);
    Serial.println("WPUST3\t\tOPEN");
  }
  else {
    digitalWrite(WPUST3,    LOW );
    Serial.println("WPUST3\t\tCLOSE");
  }
  if (prog[program][intensity][state][4]) {
    digitalWrite(SPUST1,    LOW );
    Serial.println("SPUST1\t\tOPEN");
  }
  else {
    digitalWrite(SPUST1,    HIGH);
    Serial.println("SPUST1\t\tCLOSE");
  }
  if (prog[program][intensity][state][5]) {
    digitalWrite(SPUST2,    LOW );
    Serial.println("SPUST2\t\tOPEN");
  }
  else {
    digitalWrite(SPUST2,    HIGH);
    Serial.println("SPUST2\t\tCLOSE");
  }
  if (prog[program][intensity][state][6]) {
    digitalWrite(SPUST3,    LOW );
    Serial.println("SPUST3\t\tOPEN");
  }
  else {
    digitalWrite(SPUST3,    HIGH);
    Serial.println("SPUST3\t\tCLOSE");
  }
  if (prog[program][intensity][state][12]) {
    timer_reset(1);
    Serial.println("TIMER1\t\tRESET");
  }
  if (prog[program][intensity][state][13]) {
    timer_reset(2);
    Serial.println("TIMER2\t\tRESET");
  }
}

bool state_invalid () {return false; }
/*  long  P1 = mankiet1.get_units(5);
  long  P2 = mankiet2.get_units(5);
  long  P3 = mankiet3.get_units(5);
  long unsigned T1 = get_timer_val(1);
  long unsigned T2 = get_timer_val(2);

  Serial.print(P1);
  Serial.print(",");
  Serial.print(P2);
  Serial.print(",");
  Serial.print(P3);

  Serial.print("\n");

  return (P1 > prog[program][intensity][state][7]  ||
          P2 > prog[program][intensity][state][8]  ||
          P3 > prog[program][intensity][state][9]  ||
          T1 > prog[program][intensity][state][10] ||
          T2 > prog[program][intensity][state][11]   );
}
*/
void setup() {

  Serial.begin(38400);
  Serial.println("elo");
  mcp.begin();      // use default address 0
  
  mcp.pinMode(click_pin[INTENSITY_PIN_IDX], INPUT);
  mcp.pinMode(click_pin[PROGRAM_PIN_IDX]  , INPUT);

  mcp.pinMode(LED_PIN[0], OUTPUT);
  mcp.pinMode(LED_PIN[1], OUTPUT);
  mcp.pinMode(LED_PIN[2], OUTPUT);

  pinMode (SPUST1,OUTPUT);
  pinMode (SPUST2,OUTPUT);
  pinMode (SPUST3,OUTPUT);
  
  pinMode (WPUST1,OUTPUT);
  pinMode (WPUST2,OUTPUT);
  pinMode (WPUST3,OUTPUT);
  
  pinMode (KOMPRESOR_PIN,OUTPUT);

  digitalWrite(WPUST1, LOW);
  digitalWrite(WPUST2, LOW);
  digitalWrite(WPUST3, LOW);

  digitalWrite(SPUST1, LOW);
  digitalWrite(SPUST2, LOW);
  digitalWrite(SPUST3, LOW);

  digitalWrite(KOMPRESOR_PIN, HIGH);
/*
  mankiet1.begin(MANOMETER_DOUT_PIN1, MANOMETER_SCK_PIN1, 64);
  mankiet2.begin(MANOMETER_DOUT_PIN2, MANOMETER_SCK_PIN2, 64);
  mankiet3.begin(MANOMETER_DOUT_PIN3, MANOMETER_SCK_PIN3, 64); 

  delay(2000);


  mankiet1.set_scale(51655);
  mankiet2.set_scale(51655);
  mankiet3.set_scale(51655);
  
  mankiet1.tare();
  mankiet2.tare();
  mankiet3.tare();
  
  delay(2000);
  set_state();
*/
}


bool get_click(int click_idx) {
  int current_click_val = mcp.digitalRead(click_pin[click_idx]);
  bool is_click_valid = false;
  
  if (millis() - last_click_time[click_idx] > NO_CLICK_INTERVAL) {
    if (last_click_val[click_idx] == 0 && current_click_val == 1) {
      is_click_valid = true;
      last_click_time[click_idx] = millis();
    } 
  }
  last_click_val[click_idx] = current_click_val;
  return is_click_valid;
}



void display_intensity() {

  mcp.digitalWrite(LED_PIN[intensity],LOW);
  mcp.digitalWrite(LED_PIN[(intensity+1)%3],HIGH);
  mcp.digitalWrite(LED_PIN[(intensity+2)%3],HIGH);
}

void loop() {
 
  intensity = ( intensity + get_click(INTENSITY_PIN_IDX) ) % 3;
  program   = ( program   + get_click(PROGRAM_PIN_IDX  ) ) % 2;

  display_intensity();

  Serial.print(intensity);
  Serial.print(" ");
  Serial.print(program);
  Serial.print("\n");

  if (state_invalid()) {
    state = ( state + 1 ) % cycle_len;
    set_state();
  }
  
/* example from wcmcu_2317_util lib to implement actual remote control
  bool isPushed = BUS_read (BUS_B,4);
  Serial.println(isPushed);
  BUS_write(BUS_A,1,isPushed);
  */
  
}
