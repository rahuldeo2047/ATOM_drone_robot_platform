#include <Arduino.h>

/* Simplified Logitech Extreme 3D Pro Joystick Report Parser */
#include <Logging.h>
//#include <hid.h>
#include <hiduniversal.h>
#include <usbhub.h>

#include "data.h"
#include "le3dp_rptparser.h"

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

USB                                             Usb;
USBHub                                          Hub(&Usb);
HIDUniversal                                    Hid(&Usb);
JoystickEvents                                  JoyEvents;
JoystickReportParser                            Joy(&JoyEvents);
GamePadEventData_Simple                         gpadevt;

#if defined(GROUND_SYSTEM)
#define THIS "GND_SYS: JOYS: " TOSTRING(__LINE__) ": "
#elif defined(SKY_SYSTEM)
#else
#define THIS "SKY_SYS: JOYS: " TOSTRING(__LINE__) ": ""
#endif
 
void joystick_setup()
{
  //  //analogWrite(13, 200);
  //  //Serial.begin( 250000 );
  //
  //#if !defined(__MIPSEL__)
  //  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  //#endif
  Log.Info(THIS"setup begins"CR);

  if (Usb.Init() == -1)
  {
    //analogWrite(13, 1);
    Log.Error(THIS"OSC did not start"CR);
  }
  else
  {
    Log.Info(THIS"started succesfully"CR);
  }

  delay( 200 );
  //analogWrite(13, 0);

  if (!Hid.SetReportParser(0, &Joy))
    ErrorMessage<uint8_t>(PSTR("SetReportParser"), 1  );

  Log.Info(THIS"setup ends"CR);
}

const GamePadEventData_Simple joystick_loop()
{
  Usb.Task();
  return gpadevt;
}


void JoystickEvents::OnGamePadChanged(const GamePadEventData *evt)
{
  //GamePadEventData_Simple

  gpadevt.x = evt->x;
  gpadevt.y = evt->y;
  gpadevt.slider = evt->slider;

  int x = (gpadevt.x);
  int y = (gpadevt.y);
  int z = (gpadevt.slider);

  gpadevt.twist = evt->twist;
  gpadevt.hat = evt->hat;
  gpadevt.buttons_a = evt->buttons_a;
  gpadevt.buttons_b = evt->buttons_b;

  //gpadevt = *evt;
  // 0 to 0d1023 0x03FF
  //Log.Verbose(THIS"X: %d Y: %d Z: %d Yaw: %d"CR, gpadevt.x, gpadevt.x, evt->slider, gpadevt.twist); // 0 to 0d1023 0x03FF
  Log.Verbose(THIS"X: %d Y: %d Z: %d Yaw: %d, button_a: %d button_b: %d hat: %d x: %d y: %d z: %d"CR
              , gpadevt.x, gpadevt.y, gpadevt.slider, gpadevt.twist
              , gpadevt.buttons_a, gpadevt.buttons_b, gpadevt.hat, x, y, z);
  //hat : default 8 , 0(top),1(t-r),2(right),3,4,5,6,7

  //  Serial.print("X: ");
  //  PrintHex<uint16_t>(evt->x, 0x80);
  //  Serial.print(" Y: ");
  //  PrintHex<uint16_t>(evt->y, 0x80);
  //  Serial.print(" Hat Switch: ");
  //  PrintHex<uint8_t>(evt->hat, 0x80);
  //  Serial.print(" Twist: ");
  //  PrintHex<uint8_t>(evt->twist, 0x80);
  //  Serial.print(" Slider: ");
  //  PrintHex<uint8_t>(evt->slider, 0x80);
  //  Serial.print(" Buttons A: ");
  //  PrintHex<uint8_t>(evt->buttons_a, 0x80);
  //  Serial.print(" Buttons B: ");
  //  PrintHex<uint8_t>(evt->buttons_b, 0x80);
  //  Serial.println("");
}
