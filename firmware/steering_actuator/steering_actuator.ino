#include "src/CAN.h"

// INLAND / KS0411 CAN shield
// these may not be needed
#define CS 10
#define INT 8

// CAN port
#define CAN_ID 0x51
#define CAN_INPUT 0x50
#define COUNTER_CYCLE 0xFU
const uint8_t crc_poly = 0xD5;
uint8_t pkt_idx = 0;
// MASTER_NODE sets the "clock"
// #define MASTER_NODE
// Serial debugging
#define SERIAL_DEBUG

int pwm_value = 0;

int timeout = 0;
#define TIMEOUT_MAX 25

// error states
#define NO_ERROR 0
#define TIMEOUT 1
#define BAD_CHECKSUM 2
#define STARTUP 3

int state = STARTUP;

// CAN packet
uint8_t dat[8] = {0x0};

void set_hbridge(bool mode) {
  // relay
  digitalWrite(3, mode);
  // R_EN, L_EN
  digitalWrite(4, mode);
  digitalWrite(7, mode);
}

void setup() {
  // put your setup code here, to run once:

#ifdef SERIAL_DEBUG
  Serial.begin(115200);
#endif

  // CAN setup
  if (!CAN.begin(500E3)) {
    Serial.println("CAN FAIL");
  }
  CAN.filter(CAN_INPUT);
  Serial.println("SETUP COMPLETE");

//#ifdef MASTER_NODE
  // TIMER 1 for interrupt frequency 20 Hz:
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 20 Hz increments
  OCR1A = 12499; // = 16000000 / (64 * 20) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 64 prescaler
  TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts
//#else
  CAN.onReceive(CAN_RX_IRQHandler);
//#endif

  // TIMER 2 for interrupt frequency 1000 Hz:
  cli(); // stop interrupts
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2  = 0; // initialize counter value to 0
  // set compare match register for 1000 Hz increments
  OCR2A = 249; // = 16000000 / (64 * 1000) - 1 (must be <256)
  // turn on CTC mode
  TCCR2B |= (1 << WGM21);
  // Set CS22, CS21 and CS20 bits for 64 prescaler
  TCCR2B |= (1 << CS22) | (0 << CS21) | (0 << CS20);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  sei(); // allow interrupts

  // relay pins
  pinMode(3, OUTPUT);

  // H-Bridge pins
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  // R_PWM, L_PWM
  analogWrite(5, 127);
  analogWrite(6, 127);
}

// checksum algo ripped from the Comma Pedal code
uint8_t crc_checksum(uint8_t *dat, int len, const uint8_t poly) {
  uint8_t crc = 0xFF;
  int i, j;
  for (i = len - 1; i >= 0; i--) {
    crc ^= dat[i];
    for (j = 0; j < 8; j++) {
      if ((crc & 0x80U) != 0U) {
        crc = (uint8_t)((crc << 1) ^ poly);
      }
      else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

void send_can() {
  Serial.println("CAN TX");
  dat[6] = ((0x0 & 0xFU) << 4) | pkt_idx;
  dat[7] = crc_checksum(dat, 7, crc_poly);
  CAN.beginPacket(CAN_ID);
  CAN.write(dat, 8);
  CAN.endPacket();
  ++pkt_idx;
  pkt_idx &= COUNTER_CYCLE;
}

uint8_t can_buf[8] = {0};

// CAN RX interrupt (slave nodes)
void CAN_RX_IRQHandler(int packetSize) {
  cli();
  for (int ii = 0; ii <= (packetSize - 1); ii++) {
    can_buf[ii] = CAN.read();
  }
  // TODO: do a checksum and enable check
  uint8_t cksum = can_buf[7];
  if (1) {
    //bool enable = (can_buf[2] >> 7);
    if (1) {
      pwm_value = ((can_buf[0] << 8) | can_buf[1]) / 170;
      set_hbridge(1);
    } else {
      pwm_value = 0;
    }
    state = 0;
    timeout = 0;
  }
  Serial.println("CAN_RX!");
  Serial.println(pwm_value / 170);
  sei();
}

bool reverse = 0;

void actuator() {
  
  // main code goes here
  if (state == NO_ERROR){
    // clip to +- 48
    // set_hbridge(1);
    analogWrite(5, (128 + pwm_value));
    analogWrite(6, (128 - pwm_value));
  } else {
    // error handling
    pwm_value = 0;
    set_hbridge(0);
  }

  if (timeout >= TIMEOUT_MAX){
    state = TIMEOUT;
    pwm_value = 0;
    set_hbridge(0);
  }
  
  Serial.print("STATE: ");
  Serial.print(state);
  Serial.print(" PWM_VALUE: ");
  Serial.print(pwm_value);
  Serial.print(" TIMEOUT: ");
  Serial.println(timeout);
  
}

// 50Hz timer interrupt (master node)
ISR(TIMER1_COMPA_vect) {
  //cli();
  // send_can();
  //sei();
  // up timeout counter
  if (timeout <= TIMEOUT_MAX){
    timeout++;
  }
}

// 1000Hz timer interrupt (main loop)
ISR(TIMER2_COMPA_vect) {
  // cli();
  actuator();
  // sei();
}

void loop() {
  // not used
}
