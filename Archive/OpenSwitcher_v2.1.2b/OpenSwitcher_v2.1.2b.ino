//Amiga OpenSwitcher firmware. (c)2021 Matthew Bostock.
// v2.1.2c

//Operating Guide

//>>>>>>START<<<<<<
//Upon power on, unconnected (ie plugged into PC), red light will be flashing. This means successfully flashed... Please plug into OpenSwitcher.
//------------------------------
//This will ONLY HAPPEN ONCE on FIRST INSTALL INTO OpenSwitcher! Upon Amiga power-on, white LED should flash 3 times. Identify relay, program virtual Eeprom with default values. When white LED flash 3 times This indicates setup is correct.
//------------------------------
//Upon Green LED, drive ID successful, booting in a stock unswapped condition
//Upon Blue LED, drive ID successful, booting in swapped condition.
//LED off after 8 seconds approx = OpenSwitcher is asleep.
//
//Upon Amiga RESET, pink LED, reboot successful, go back to start.
//If Amiga RESET is HELD for approx 3 seconds, White LED will blink 3 times. If in a swapped condition, will become unswapped. If unswapped, will be swapped. Stored to Eeprom.
//That is all

#include <Adafruit_DotStar.h>
#include <FlashStorage.h>
#define cia_sel1 4
#define cia_sel0 3
#define switch_request 2
#define relaypin2 1
#define relaypin1 0

//#define TEST_MODE uncomment to test board via serial monitor. Comment out again to restore operation.
//#define SHOW_SEL_ACTIVITY
#define USER_TIMER_SETUP 3
#define SLEEP_TIMEOUT 8
#define START_MODE 1
FlashStorage(sys_prep, bool);
FlashStorage(swapdrives_active, bool);
FlashStorage(relay_latching, bool);
FlashStorage(user_timer, int);

Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
unsigned long ticktimer1;
unsigned long ticktimer2;
static bool swapactive;
static bool relaylatching;
static int usertimer;

void led_show(const int red, const int green, const int blue)
{
  strip.setPixelColor(0, red, green, blue);
  strip.show();
}

void led_flash(const int red, const int green, const int blue, const int count, uint32_t pause1, uint32_t pause2)
{
  if (pause2 == 0) pause2 = pause1;
  for (int i = 0; i < count; ++i)
  { strip.setPixelColor(0, red, green, blue);
    strip.show();
    delayMicroseconds(pause1);
    strip.setPixelColor(0, 0, 0, 0);
    strip.show();
    delayMicroseconds(pause2);
  }
}

int read_eeprom_drive()
{
  int val = 0;
  val = swapdrives_active.read();
  //val = test_mode_swap;
  return val;
}

int read_eeprom_relay()
{
  int val = 0;
  val = relay_latching.read();
  //val = test_mode_swap;
  return val;
}

int read_eeprom_usertimer()
{
  int val = 0;
  val = user_timer.read();
  //val = test_mode_swap;
  return val;
}

int write_eeprom(const int value)
{
  swapdrives_active.write(value);
}

void reset()
{
  SCB->AIRCR = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk);
}

void first_boot_lock()
{
  pinMode(cia_sel0, INPUT_PULLUP);
  if (digitalRead(cia_sel0) == HIGH) standby(1);
}

int newflash_initial_settings()
{


  if (sys_prep.read() == 1) return 0;
  else
  {
    led_show(0, 0, 0);
    pinMode(relaypin2, INPUT_PULLUP);
    if (digitalRead(relaypin2) != HIGH)
    {
      //led_flash(64, 64, 0, 3, 50000, 300000);
      relay_latching.write(1);
    }
    else
    {
      //led_flash(0, 64, 64, 3, 50000, 300000);
      relay_latching.write(0);
    }

    swapdrives_active.write(START_MODE);
    user_timer.write(USER_TIMER_SETUP);
    sys_prep.write(1);
    delayMicroseconds(500000);
    led_flash(64, 64, 64, 3, 50000, 200000);
  }
  if (swapdrives_active.read() != START_MODE) return 1;
  return 0;
}

void standby(const int option)
{
  if (option)
  {
    while (true) led_flash(2, 0, 0, 1, 50000, 950000);
    //attachInterrupt(switch_request, reset, FALLING);
  }
  else
  {
    led_show(0, 0, 0);
    //attachInterrupt(switch_request, reset, RISING);
  }
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  GCLK->CLKCTRL.reg = uint16_t(
                        GCLK_CLKCTRL_CLKEN |
                        GCLK_CLKCTRL_GEN_GCLK2 |
                        GCLK_CLKCTRL_ID( GCLK_CLKCTRL_ID_EIC_Val )
                      );
  while (GCLK->STATUS.bit.SYNCBUSY) {}

  __DSB();
  __WFI();
}

void board_test()
{
  Serial.begin(115200);
  while (!Serial) ;

  Serial.println("Welcome to OpenSwitcher Board Test. Please run this with even CIA chip removed from OpenSwitcher");
  Serial.println("You will require 1 or preferably 2 jumper wires. You also need to know where 5V and GND are.");
  Serial.println("Each test is run in order. If you get to the end, your unit is working.");
  Serial.println("Have your OpenSwitcher in front of you with OpenSwitcher text facing you. Look at the CIA socket.");
  Serial.println("Pin 1 is GND. This is the TOP RIGHT PIN.");
  Serial.println("Pin 20 is 5V. This is the TOP LEFT PIN.");
  Serial.println("First test: SEL Lines. These are:");
  Serial.println("SEL0 = Pin 13, SEL1 = Pin 14");
  Serial.println("Pulling SEL0 high and setting to input...");
  pinMode (cia_sel0, INPUT_PULLUP);
  Serial.println("Touch SEL0 pin to 5V");
  while (digitalRead(cia_sel0) == HIGH);
  Serial.println("Success. Disconnect.");
  while (digitalRead(cia_sel0) == LOW);
  Serial.println("Pulling SEL1 high and setting to input...");
  pinMode (cia_sel1, INPUT_PULLUP);
  Serial.println("Touch SEL1 pin to 5V");
  while (digitalRead(cia_sel1) == HIGH);
  Serial.println("Success. Disconnect");
  while (digitalRead(cia_sel1) == LOW);
  Serial.println("Second Test: Keyboard Reset Line (J1 header)");
  Serial.println("WARNING: Early production boards (v2.1.2) have a printing error. KB_RST and 5V ports are labelled incorrectly.");
  Serial.println("The LOWER pin is 5V, the upper one is KB_RST on these boards");
  Serial.println("Pulling KB_RST high and setting to input...");
  pinMode (switch_request, INPUT_PULLUP);
  Serial.println("Link or touch KB_RST to 5V");
  while (digitalRead(switch_request) == HIGH);
  Serial.println("Success. Disconnect.");
  while (digitalRead(switch_request) == LOW);
  Serial.println("Third Test: Relay. Reading JP1...");
  pinMode (relaypin2, INPUT_PULLUP);
  pinMode (relaypin1, OUTPUT);
  if (digitalRead(relaypin2) == HIGH)
  {
    Serial.println("JP1 appears HIGH. Non-Latching relay probably installed. If this is wrong, CHECK YOUR BOARD! Testing...");
    Serial.println("You should hear your relay ticking when I update the screen. This will loop forever. If you hear the relay");
    Serial.println("ticking when the LED changes colour, it is working...");
    while (true)
    {
      led_show (0, 64, 0); digitalWrite(relaypin1, HIGH); Serial.println("Relay on...");
      delayMicroseconds(1000000);
      led_show (0, 0, 0); digitalWrite(relaypin1, LOW); Serial.println("Relay off...");
      delayMicroseconds(1000000);
    }
  }
  else
  {
    pinMode (relaypin2, OUTPUT);
    Serial.println("JP1 appears LOW. Latching relay probably installed. If this is wrong, CHECK YOUR BOARD! Testing...");
    Serial.println("You should hear your relay ticking when I update the screen. This will loop forever. If you hear the relay");
    Serial.println("ticking when the LED changes colour, it is working...");
    while (true)
    {
      led_show (0, 64, 0); digitalWrite(relaypin1, HIGH); Serial.println("Relay active latch... Power on...");
      delayMicroseconds(1000000);
      digitalWrite(relaypin1, LOW);
      Serial.println("Relay active latch... Power off...");
      delayMicroseconds(1000000);
      led_show (0, 0, 0); digitalWrite(relaypin2, HIGH); Serial.println("Relay deactive latch... Power on...");
      delayMicroseconds(1000000);
      digitalWrite(relaypin2, LOW); Serial.println("Relay deactive latch... Power off...");
      delayMicroseconds(1000000);

    }
  }


  while (true);
}
int reset_handler()
{
  static int res = 0;
  while (digitalRead(switch_request) == HIGH)
  {
    ticktimer2 = millis();

    if (ticktimer2 >= (ticktimer1 + (usertimer * 1000)) && (digitalRead(switch_request) == HIGH))
    {
      if (!res)
      {
        res = 1;
        led_flash (64, 64, 64, 1, 500000, 0);
      }
      if (digitalRead(switch_request) == LOW) return res;
    }
    if (ticktimer2 >= (ticktimer1 + ((usertimer + 2) * 1000)) && (digitalRead(switch_request) == HIGH))
    {
      if (res == 1)
      {
        led_flash (64, 64, 0, 1, 500000, 0);
        res = 2;
        if (digitalRead(switch_request) == LOW) return res;
      }
    }
  }
  //if(res) ticktimer1 = millis();
  return 0;
}

int swap_drives()
{
  while (digitalRead(switch_request) == HIGH);
  int res = 0;
  if (swapactive)
  {

    //led_show(0, 64, 0);
    if (swapdrives_active.read() != 0) swapdrives_active.write(0);
    res = 1;

  } else
  {
    //led_show(0, 0, 64);
    if (swapdrives_active.read() != 1) swapdrives_active.write(1);
    res = 2;
  }
  return res;
}

int set_user_timer()
{
  usertimer = 0;
  static int res = 0;
  ticktimer1 = millis();
  while (digitalRead(switch_request) == HIGH);
  while (true)
  {
    ticktimer2 = millis();
    if (ticktimer2 > (ticktimer1 + 1000))
    {
      led_flash (64, 64, 0, 1, 100000, 100000);
      ++usertimer;
      ticktimer1 = millis();
    }
    if ((digitalRead(switch_request) == HIGH) && (usertimer <= 10))
    {
      if (usertimer <= 1) usertimer = 1;


      user_timer.write(usertimer);
      //led_flash (64, 64, 0, usertimer, 50000, 100000);
      res = 1;
    }
    if (res) break;
    if (usertimer >= 10)
    {
      //default values
      sys_prep.write(0);
      led_flash (64, 0, 0, 10, 50000, 50000);
      attachInterrupt(switch_request, reset, RISING);
      standby(0);
    }

  }
  return res;
  //reset();
}

void blue_led()
{led_show(0,0,64);}

void green_led()
{led_show(0,64,0);}

void setup() {
#ifdef TEST_MODE
  board_test();
#endif
  ticktimer1 = millis();
  //lock system if not plugged in to Amiga. Link keyboard reset and 5v to bypass. Set reset sense pin.
  first_boot_lock();
  //write initial values. Check type of relay. Done once.
  newflash_initial_settings();
  //grab eeprom values globally
  swapactive = read_eeprom_drive();
  relaylatching = read_eeprom_relay();
  usertimer = (read_eeprom_usertimer() - 1);
  if (usertimer < 1) usertimer = 1;
  led_show(0, 0, 0);
  //set other pins

  pinMode(switch_request, INPUT_PULLUP);
  pinMode(cia_sel1, INPUT_PULLUP);
  pinMode(relaypin1, OUTPUT);

  //Reset relay into default position if latching
  if (relaylatching)
  {
    pinMode(relaypin2, OUTPUT);
    digitalWrite(relaypin2, HIGH); delayMicroseconds (100000); digitalWrite(relaypin2, LOW);
  }
  else pinMode(relaypin2, INPUT_PULLUP);

}

void loop() {
  static bool coil1_active = false;
  static bool coil2_active = false;
  static bool first_timeout_reached = false;
  static bool second_timeout_reached = false;
  //swapactive = read_eeprom_drive();
  //relaylatching = read_eeprom_relay();

  if (digitalRead(switch_request) == HIGH)
  {
    int res = reset_handler();
    if (res == 1)
    {
      if (swap_drives())
      {
      led_flash (64, 64, 64, 10, 25000, 25000);
      reset();
      }
    }
    else if (res == 2)
    {
      if (set_user_timer())
      {
        led_flash (64, 64, 0, 10, 25000, 25000);
      reset();
      }
    }
    else reset();
  }
  while (digitalRead(cia_sel0) == LOW);
  delayMicroseconds(690);


  if (swapactive)
  {

    if (relaylatching)
    {
      if (!coil1_active)
      {
        digitalWrite(relaypin1, HIGH); delayMicroseconds (500000); digitalWrite(relaypin1, LOW);
        coil1_active = true;
        coil2_active = false;
      }
//      else
//      {
//        led_show (64, 0, 0);
//        delayMicroseconds (500000);
//      }
    }
    else
    {
      digitalWrite(relaypin1, HIGH);
      coil1_active = true;
      coil2_active = false;
    }
  }
  else
  {
    if (relaylatching)
    {
      if (!coil2_active)
      {
        digitalWrite(relaypin2, HIGH); delayMicroseconds (500000); digitalWrite(relaypin2, LOW);
        coil2_active = true;
        coil1_active = false;
      }
//      else
//      {
//        led_show (64, 0, 0);
//        delayMicroseconds (500000);
//      }
    }
    else
    {
      digitalWrite(relaypin1, LOW);
      coil1_active = false;
      coil2_active = false;
    }
  }
  attachInterrupt(switch_request, reset, RISING);
  #ifdef SHOW_SEL_ACTIVITY
  attachInterrupt(cia_sel0, green_led, RISING);
  attachInterrupt(cia_sel1, blue_led, RISING);
  #endif
  delayMicroseconds(500000);
  if (swapactive) led_show(0, 0, 64);
  else led_show(0, 64, 0);

  while (ticktimer2 <= (ticktimer1 + (SLEEP_TIMEOUT * 1000)))
  {
    ticktimer2 = millis();
  }
  led_show(0, 0, 0);
  standby(0);
}
