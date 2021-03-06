#include <Arduino.h>

#include <Logging.h>
#include "data.h"

//#define STRINGIFY(x) #x
//#define TOSTRING(x) STRINGIFY(x)

#if defined(GROUND_SYSTEM)
#define THIS "GND_SYS: WIFI: " TOSTRING(__LINE__) ": "
#elif defined(SKY_SYSTEM)
#else
#define THIS "SKY_SYS: WIFI: " TOSTRING(__LINE__) ": ""
#endif

#define SSID        "D_ATOM_1" //"HHH7351HHH"
#define PASSWORD    "D_ATOM_1"

//#define SSID        "HHH7501HHH"
//#define PASSWORD    "hh1057hhh"

#if defined(GROUND_SYSTEM)
#define PEER_IP_ADDRESS "192.168.4.4" //"10.42.0.1" // "192.168.43.243" //"192.168.4.1" // SKY_SYSTEM address
#define PEER_PORT (10000) //(8090)
#elif defined(SKY_SYSTEM)
#define PEER_IP_ADDRESS "192.168.4.103" //"192.168.4.1" // SKY_SYSTEM address
#define PEER_PORT (10000) //(8090)
#else
#endif
#define WIFI_CMD_MODE_PIN (42)
#define WIFICOM (&Serial1)

void Delay(unsigned long ms)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(ms) );
}

void Delay(unsigned long ms, TickType_t xLastWakeTime)
{
  vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(ms) );
}

void Delay_tick(unsigned long tick, TickType_t xLastWakeTime)
{
  vTaskDelayUntil( &xLastWakeTime, (tick) );
}

static char buffer[32];

/**
* [selectCommandMode To select pin to change between transparent or cmd mode]
* @param _cmdMode [description]
*/
void selectCommandMode(int _cmdMode)
{
  if(0==_cmdMode) // select transparent mode
  {
    digitalWrite(WIFI_CMD_MODE_PIN, HIGH); // CMD mode
    //delay(150); // 100 requires but keeping 50 ms extra for co-operative tasking overheads in ESP8266 custom firmware.
    Delay(150) ;


  }
  else if(1==_cmdMode) // select transparent mode
  {
    digitalWrite(WIFI_CMD_MODE_PIN, LOW); // CMD mode
    //delay(150); // 100 requires but keeping 50 ms extra for co-operative tasking overheads in ESP8266 custom firmware.
    Delay(150) ;
  }
  else
  {
    Log.Error(THIS"wifi invalid data mode"CR);
  }
}

/**
* [readline read till new line]
* @param  readch [the char read by serial.read()]
* @param  buffer [buffer for chars to save]
* @param  len    [max len out]
* @return        [return len read]
*/

/**
* [readline read till new line]
* @param  readch [the char read by serial.read()]
* @param  buffer [buffer for chars to save]
* @param  tlen   [max len out]
* @param  len    [len read]
* @return        [whene it containing new line even if zero len apart from newline]
*/
inline int readline(int readch, char *buffer, const int tlen, int * len)
{
  static int pos = 0;
  static int linrec = -1;
  int temp = 0;
  linrec = -1;

  //Log.Verbose(THIS"readline : %c"CR, readch);

  //memset(buffer, 0, 32);
  //*len = int (-1);

  if (readch > 0) {
    switch (readch) {
      case '\r': // Ignore new-lines
      Log.Verbose(THIS"readline : [CR]"CR);
      break;
      case '\n': // Return on LF
      (*len) = pos;
      linrec = 1;
      pos = 0;  // Reset position index ready for next time
      Log.Debug(THIS"readline : [LF] %d %d (%s)"CR, linrec, (*len), buffer);
      for(temp = 0 ; temp<(*len) ; temp++)
      {
        Log.Verbose(" [%d %c (%x)]"CR, temp, buffer[temp], buffer[temp]);
      }
      Log.Verbose(CR);
      return linrec;
      default:
      if (pos < tlen - 1) {
        //Log.Verbose(THIS"readline : %c"CR, readch);
        buffer[pos++] = readch;
        buffer[pos] = 0;
      }
    }
  }
  // No end of line has been found, so return -1.
  return -1;
}


/**
* [wifi_setup setup wifi]
* @param _mode [setup mode]
*/
int wifi_setup()//int _mode)
{
  pinMode(WIFI_CMD_MODE_PIN, OUTPUT);
  //digitalWrite(WIFI_CMD_MODE_PIN, LOW); // Switch to command mode
  selectCommandMode(0);

  WIFICOM->begin(115200);

  // AT+RST
  if (0 == wifi_reset()) // no error
  {
    Log.Info(THIS"wifi restarted"CR);
  }
  else
  {
    Log.Error(THIS"wifi restart"CR);
    Log.Info(THIS"Please reset the controller"CR);
    return -1;
  }

  //delay(200);

  // AT
  if (0 == wifi_check())
  {
    Log.Info(THIS"wifi functional"CR);
  }
  else
  {
    Log.Error(THIS"wifi functional"CR);
    Log.Info(THIS"Please check for delay and/or implementation"CR);
    return -1;
  }

  // AT+SRIPP IP1.IP2.IP3.IP4 PORTT
  if (0 == wifi_set_remote_IP_PORT())
  {
    Log.Info(THIS"wifi remote ip port set"CR);
  }
  else
  {
    Log.Error(THIS"wifi remote ip port"CR);
    Log.Info(THIS"Please check for delay and/or implementation"CR);
    return -1;
  }

  // AT+START 0 / 1
  //
  #if (defined(SKY_SYSTEM) )

  if (0 == wifi_set_mode(0)) //
  {
    Log.Info(THIS"wifi mode set for SKY_SYSTEM"CR);
  }
  else
  {
    Log.Error(THIS"wifi mode"CR);
    Log.Info(THIS"Please check for delay and/or implementation"CR);
    return -1;
  }

  // AT+SRIPP IP1.IP2.IP3.IP4 PORTT
  if (0 == wifi_set_remote_IP_PORT())
  {
    Log.Info(THIS"wifi remote ip port set"CR);
  }
  else
  {
    Log.Error(THIS"wifi remote ip port"CR);
    Log.Info(THIS"Please check for delay and/or implementation"CR);
    return -1;
  }

  #elif ( defined(GROUND_SYSTEM) )

  if (0 == wifi_set_mode(1)) //
  {
    Log.Info(THIS"wifi mode set for GROUND_SYSTEM"CR);
  }
  else
  {
    Log.Error(THIS"wifi mode"CR);
    Log.Info(THIS"Please check for delay and/or implementation"CR);
    return -1;
  }

  #else

  #error no system mode selected

  #endif

  return 0;

}


/**
* [wifi_loop_send_Joystick_data keep sending data]
* @param  data [txGamePadData type]
* @return      [whether wifi module forwarded the data]
*/
//#error EOOROROEOROROEOEO

// #if (defined(USE_DATA_UNION))
// #define DATA (txGamePadORMPUData *) // * data
// #elif  (!defined(USE_DATA_UNION))
// #define DATA (txGamePadData *) // data
// #else
// #endif

int wifi_loop_send_Joystick_or_mpu_data( void* _data )
{
  // Do not use serial/Log print here
  //#define DEEP_DEBUG_WIFI_SEND

  int ret = -1;
  int len = -1;
  int rec = -1;
  char *saveptr;
  char *sntstr, *numstr;
  int cnt = -1;
  TickType_t lastTick = 0;

  // while (WIFICOM->available())
  // {
  //   WIFICOM->read();
  // }

  #if defined(USE_DATA_UNION)

  txGamePadORMPUData * data = _data;
  data->data.stx = 0x02;
  data->data.header = 0xff;
  //data->data.data_len = (SIZE_OF_GPADDATA_STRUCT - 3);
  //data->data.data_type = 0x01;
  data->data.res3 = 0x00;
  data->data.etx = 0x03;

  WIFICOM->write(data->uc_data, SIZE_OF_GPADMDATA_STRUCT);

  #else

  txGamePadData * data = _data;
  data->gd.stx = 0x02;
  data->gd.header = 0xff;
  //data->gd.data_len = (SIZE_OF_GPADDATA_STRUCT - 3);
  //data->gd.data_type = 0x01;
  data->gd.res3 = 0x00;
  data->gd.etx = 0x03;

  WIFICOM->write(data->uc_data, SIZE_OF_GPADDATA_STRUCT);

  #endif

  WIFICOM->flush();

  lastTick = xTaskGetTickCount( );
  while (9>=WIFICOM->available()) // Time out can be put +sent 2\r\n
  {
    if( ( xTaskGetTickCount( ) - lastTick ) > 5)
    {
      lastTick = xTaskGetTickCount( ) - lastTick; // lastTick 's work is done here

      #if defined(DEEP_DEBUG_WIFI_SEND)
      Serial.print(__LINE__);
      Serial.print(" ");
      Serial.println(lastTick);
      #endif

      break;
    }
  }

  #if defined(DEEP_DEBUG_WIFI_SEND)
  Serial.print(__LINE__);
  Serial.print(" ");
  Serial.print(micros());
  Serial.print(" ");
  Serial.print((char)WIFICOM->peek());
  Serial.print(" ");
  #endif

  while (WIFICOM->available())
  {
    rec = readline(WIFICOM->read(), buffer, 32, &len);
    if (1==rec)
    {
      ret = ( NULL != strstr(buffer, "+sent") );
      if(1==ret)
      {


        sntstr = strtok_r(buffer, " ", &saveptr);
        numstr = strtok_r(NULL, " ", &saveptr);
        cnt = atoi(numstr);

        if(SIZE_OF_GPADDATA_STRUCT==cnt)
        {
          ret = 1;

          #if defined(DEEP_DEBUG_WIFI_SEND)
          Serial.println(__LINE__);
          #endif
        }
        else
        {
          ret = -1;

          #if defined(DEEP_DEBUG_WIFI_SEND)
          Serial.print(" ");
          Serial.print(__LINE__);
          Serial.print(" ");
          Serial.print(micros());
          Serial.print(" ");
          Serial.print(buffer);
          Serial.print(" ");
          Serial.print(numstr);
          Serial.print(" ");
          #endif
        }

        if(SIZE_OF_GPADDATA_STRUCT>cnt)
        {
          #if defined(DEEP_DEBUG_WIFI_SEND)
          Serial.println(__LINE__);
          #endif

          lastTick = xTaskGetTickCount( );
          while (9>=WIFICOM->available())
          {
            if( ( xTaskGetTickCount( ) - lastTick ) > 5)
            {
              lastTick = xTaskGetTickCount( ) - lastTick; // lastTick 's work is done here
              #if defined(DEEP_DEBUG_WIFI_SEND)
              Serial.print(__LINE__);
              Serial.print(" ");
              Serial.println(lastTick);
              #endif

              break;
            }
          }


        }
        else
        {
          #if defined(DEEP_DEBUG_WIFI_SEND)
          Serial.println(__LINE__);
          #endif
        }

        // all fine
        while (WIFICOM->available())
        {
          rec = readline(WIFICOM->read(), buffer, 32, &len);
          if(1==rec)
          {
            ret = ( NULL != strstr(buffer, "+sent") );
            if(1==ret)
            {
              sntstr = strtok_r(buffer, " ", &saveptr);
              numstr = strtok_r(NULL, " ", &saveptr);
              cnt += atoi(numstr);

              if(SIZE_OF_GPADDATA_STRUCT==cnt)
              {
                ret = 1;

                #if defined(DEEP_DEBUG_WIFI_SEND)
                Serial.println(__LINE__);
                #endif
              }
              else if(SIZE_OF_GPADDATA_STRUCT<cnt)
              {
                // error
                ret = -1; // already handled but keeping it here
              }
              else
              {
                ret = -1;

                #if defined(DEEP_DEBUG_WIFI_SEND)
                Serial.print(" ");
                Serial.print(__LINE__);
                Serial.print(" ");
                Serial.print(micros());
                Serial.print(" ");
                Serial.print(buffer);
                Serial.print(" ");
                Serial.print(numstr);
                Serial.print(" ");
                Serial.print(cnt);
                Serial.print(" ");
                #endif

              }
            }
          }
          //Delay(1);
        }


        if( (WIFICOM->available()))
        {
          // warn
          #if defined(DEEP_DEBUG_WIFI_SEND)
          Serial.println(__LINE__);
          #endif
        }

        while (WIFICOM->available())
        {
          #if defined(DEEP_DEBUG_WIFI_SEND)
          Serial.print(__LINE__);
          Serial.print(" ");
          Serial.print(micros());
          Serial.print(" ");
          Serial.println((char)WIFICOM->read());
          #endif
        }
        break;
      }
    }
  }
  //Serial.println(" ");
  return (1 == ret ? 0 : -1);
}

/**
* [wifi_loop_recv_joystick_data keep recieving data]
* @param  gd [data to update]
* @return    [whether data was valid]
*/
int wifi_loop_recv_joystick_data(void * _gd, int *remlen)
{
  // If performing serial print must be inside semaphore
  #define DEEP_DEBUG_WIFI_RECV

  static boolean timestampCapd = false;
  static int32_t timediff = 0;

  int ret = -1;
  int recvlen = 0;
  int i=0;
  byte c = 0;

  *remlen = 0;

  #if defined(USE_DATA_UNION)
  txGamePadORMPUData * gd = _gd;
  //while(SIZE_OF_GPADMDATA_STRUCT!=WIFICOM->available());
  #else
  txGamePadData * gd = _gd;
  //while(SIZE_OF_GPADDATA_STRUCT!=WIFICOM->available());
  #endif
  while(5>=WIFICOM->available())
  {
    // if time is more than as in practice break or return
    //Delay(1000);
    //Serial.println(__LINE__);

  }

  gd->uc_data[0] = WIFICOM->read();
  #if defined(USE_DATA_UNION)
  if(0x02!=gd->data.stx)
  #else
  if(0x02!=gd->gd.stx)
  #endif
  {
    #if defined(DEEP_DEBUG_WIFI_RECV)
    Serial.println(__LINE__);
    #endif
    while(WIFICOM->available())
    {
      c = WIFICOM->read();
      #if defined(DEEP_DEBUG_WIFI_RECV)
      Serial.print(c, HEX);Serial.print(" ");
      #endif
      // if time is more than as in practice break or return
      //Delay(1000);
      //Serial.println(__LINE__);

    }
    #if defined(DEEP_DEBUG_WIFI_RECV)
    Serial.println();
    #endif
    return (1 == ret ? 0 : -1);
  }

  //while(1>=WIFICOM->available());
  gd->uc_data[1] = WIFICOM->read();

  #if defined(USE_DATA_UNION)
  if(0xFF!=gd->data.header)
  #else
  if(0xFF!=gd->gd.header)
  #endif
  {
    #if defined(DEEP_DEBUG_WIFI_RECV)
    Serial.println(__LINE__);
    #endif
    while(WIFICOM->available())
    {
      c = WIFICOM->read();
      #if defined(DEEP_DEBUG_WIFI_RECV)
      Serial.print(c, HEX);Serial.print(" ");
      #endif
      // if time is more than as in practice break or return
      //Delay(1000);
      //Serial.println(__LINE__);

    }
    #if defined(DEEP_DEBUG_WIFI_RECV)
    Serial.println();
    #endif
    return (1 == ret ? 0 : -1);
  }
  else
  {

  }

  //while(1>=WIFICOM->available());
  gd->uc_data[2] = WIFICOM->read();

  #if defined(USE_DATA_UNION)

  if( ( (SIZE_OF_MDATA_STRUCT) != gd->data.data_len ) || ( (SIZE_OF_JDATA_STRUCT) != gd->data.data_len ) )

  #else

  if( (SIZE_OF_JDATA_STRUCT) != gd->gd.data_len )

  #endif
  {
    #if defined(DEEP_DEBUG_WIFI_RECV)
    Serial.print(__LINE__); // C/2 | 16 | 15
    Serial.print(" [2]: ");
    Serial.print((int)gd->uc_data[2]);
    Serial.print(" |len: ");
    #if defined(USE_DATA_UNION)
    Serial.print(gd->data.data_len);
    #else
    Serial.print(gd->gd.data_len);
    #endif
    Serial.print(" |mcro: ");
    Serial.println(SIZE_OF_GPADDATA_STRUCT);
    #endif

    while(WIFICOM->available())
    {
      c = WIFICOM->read();
      #if defined(DEEP_DEBUG_WIFI_RECV)
      Serial.print(c, HEX);Serial.print(" ");
      #endif
      // if time is more than as in practice break or return
      //Delay(1000);
      //Serial.println(__LINE__);

    }
    #if defined(DEEP_DEBUG_WIFI_RECV)
    Serial.println();
    #endif

    return (1 == ret ? 0 : -1);
  }

  #if defined(USE_DATA_UNION)

  //while(1>=WIFICOM->available());
  gd->uc_data[3] = WIFICOM->read();
  if( DATATYPE_JOY == gd->data.data_type || DATATYPE_MPU == gd->data.data_type )

  #else

  //while(1>=WIFICOM->available());
  gd->uc_data[3] = WIFICOM->read();
  if( DATATYPE_JOY == gd->gd.data_type )

  #endif
  {
    //ret = 0;
  }
  else
  {
    #if defined(DEEP_DEBUG_WIFI_RECV)
    Serial.println(__LINE__);
    #endif
    while(WIFICOM->available())
    {
      c = WIFICOM->read();
      #if defined(DEEP_DEBUG_WIFI_RECV)
      Serial.print(c, HEX);Serial.print(" ");
      #endif
      // if time is more than as in practice break or return
      //Delay(1000);
      //Serial.println(__LINE__);

    }
    #if defined(DEEP_DEBUG_WIFI_RECV)
    Serial.println();
    #endif

    return (1 == ret ? 0 : -1);
  }

  //while(1>=WIFICOM->available());
  gd->uc_data[4] = WIFICOM->read(); // res
  //recvlen = WIFICOM->available();

  // Above lines take care of data fragments

  #if defined(USE_DATA_UNION)
  while(SIZE_OF_GPADMDATA_STRUCT-5>WIFICOM->available());
  #else
  while(SIZE_OF_GPADDATA_STRUCT-5>WIFICOM->available());
  #endif

  recvlen = WIFICOM->available();

  if(recvlen)
  {

    #if defined(DEEP_DEBUG_WIFI_RECV)
    Serial.print(__LINE__);
    Serial.print(" reclen: ");
    Serial.print(recvlen);
    Serial.print(" | data macro: ");
    #endif

    #if defined(USE_DATA_UNION)
    //ret = recvlen == (SIZE_OF_MDATA_STRUCT+1); // size of .etx
    #if defined(DEEP_DEBUG_WIFI_RECV)
    Serial.print(SIZE_OF_MDATA_STRUCT);
    Serial.print(" | ");
    #endif // #if defined(LOW_RAM_DEBUG)
    #else
    //ret = recvlen == (SIZE_OF_JDATA_STRUCT+1); // size of .etx
    #if defined(DEEP_DEBUG_WIFI_RECV)
    Serial.print(SIZE_OF_JDATA_STRUCT);
    Serial.print(" | ");
    #endif // #if defined(LOW_RAM_DEBUG)
    #endif

    if((recvlen >= (SIZE_OF_GPADDATA_STRUCT-5)))
    {

      WIFICOM->readBytes((&gd->uc_data[5]), (SIZE_OF_GPADDATA_STRUCT-5-1));
      // , (recvlen)
      // >= (SIZE_OF_GPADDATA_STRUCT-5-1)
      // ? (SIZE_OF_GPADDATA_STRUCT-5-1) : recvlen);

      gd->gd.etx = WIFICOM->read();

      *remlen = WIFICOM->available()%SIZE_OF_GPADDATA_STRUCT;

      // Validate

      ret = gd->gd.stx == 0x02 &&
      gd->gd.header == 0xFF &&
      gd->gd.data_len == SIZE_OF_JDATA_STRUCT && //(SIZE_OF_GPADDATA_STRUCT - SIZE_OF_JDATA_STRUCT);
      gd->gd.data_type == DATATYPE_JOY &&
      gd->gd.res3 == 0x00 &&
      gd->gd.etx == 0x03;

      #if defined(DEEP_DEBUG_WIFI_RECV)
      //if(1!=ret)
      {
        if(false==timestampCapd)
        {
          timediff = (gd->gd.gd.timestamp-millis());
          timestampCapd = true;
        }

        Serial.print((int32_t)(gd->gd.gd.timestamp-millis()-timediff));
        Serial.print(" DATA: ");
        #if defined(USE_DATA_UNION)
        for(i=0 ; i<SIZE_OF_GPADMDATA_STRUCT ; i++)
        #else
        for(i=0 ; i<SIZE_OF_GPADDATA_STRUCT ; i++)
        #endif
        {
          Serial.print(gd->uc_data[i], HEX);
          Serial.print(" ");
        }
        Serial.print(" | ");
      }
      Serial.print(__LINE__);
      Serial.print (" ");
      Serial.println(ret);

      #endif
    }
    else
    {
      #if defined(DEEP_DEBUG_WIFI_RECV)
      Serial.print(__LINE__);
      Serial.print(" ");
      #endif
      while(WIFICOM->available())
      {
        c = WIFICOM->read();
        Serial.print(c, HEX);Serial.print(" ");
        // if time is more than as in practice break or return
        //Delay(1000);
        //Serial.println(__LINE__);

      }
      Serial.println();
      //return (1 == ret ? 0 : -1);
    }



    // Can be uncommented to debug
    // if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    // {
    //
    //   Log.Verbose(THIS"len %d %d %d"CR, recvlen, ret, SIZE_OF_GPADDATA_STRUCT);
    //
    //   if(0==ret)
    //   {
    //     Log.Warning(THIS"ret %d (%d/%d)"CR, ret, recvlen, SIZE_OF_GPADDATA_STRUCT);
    //   }
    //   else
    //   {
    //     Log.Debug(THIS"rec fine"CR);
    //   }
    //
    //   if ( ( xSerialSemaphore ) != NULL )
    //   xSemaphoreGive( ( xSerialSemaphore ) );  // make the Serial Port available
    // }



//     if((recvlen >= (SIZE_OF_MDATA_STRUCT+1)))
//     {
//
// #if defined(USE_DATA_UNION)
//       WIFICOM->readBytes((&gd->uc_data[5])
//       , (recvlen)
//       >= (SIZE_OF_MDATA_STRUCT)
//       ? (SIZE_OF_MDATA_STRUCT) : recvlen);
//
//       gd->data.etx = WIFICOM->read();
//
//       *remlen = WIFICOM->available()%SIZE_OF_GPADMDATA_STRUCT;
//
//       // Validate
//
//       ret = gd->data.stx == 0x02 &&
//       gd->data.header == 0xFF &&
//       gd->data.data_len == SIZE_OF_MDATA_STRUCT &&
//       (gd->data.data_type == DATATYPE_MPU || gd->data.data_type == DATATYPE_JOY) &&
//       gd->data.res3 == 0x00 &&
//       gd->data.etx == 0x03;
//
//       #else
//
//       WIFICOM->readBytes((&gd->uc_data[5])
//       , (recvlen)
//       >= (SIZE_OF_GPADDATA_STRUCT-5-1)
//       ? (SIZE_OF_GPADDATA_STRUCT-5-1) : recvlen);
//
//       gd->gd.etx = WIFICOM->read();
//
//       *remlen = WIFICOM->available()%SIZE_OF_GPADDATA_STRUCT;
//
//       // Validate
//
//       ret = gd->gd.stx == 0x02 &&
//       gd->gd.header == 0xFF &&
//       gd->gd.data_len == SIZE_OF_JDATA_STRUCT && //(SIZE_OF_GPADDATA_STRUCT - SIZE_OF_JDATA_STRUCT);
//       gd->gd.data_type == DATATYPE_JOY &&
//       gd->gd.res3 == 0x00 &&
//       gd->gd.etx == 0x03;
//
//       #endif
//
//       #if defined(DEEP_DEBUG_WIFI_RECV)
//       //if(1!=ret)
//       {
//         Serial.print(" DATA: ");
//         #if defined(USE_DATA_UNION)
//         for(i=0 ; i<SIZE_OF_GPADMDATA_STRUCT ; i++)
//         #else
//         for(i=0 ; i<SIZE_OF_GPADDATA_STRUCT ; i++)
//         #endif
//         {
//           Serial.print(gd->uc_data[i], HEX);
//           Serial.print(" ");
//         }
//         Serial.println();
//       }
//       Serial.print(__LINE__);
//       Serial.print (" ");
//       Serial.println(ret);
//
//       #endif
//
//       // if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
//       // {
//       //
//       //   for ( i = 0; i < recvlen; i++)
//       //   {
//       //     Log.Verbose(THIS"[%d] %X"CR, i, gd->uc_data[i]);
//       //   }
//       //   Log.Verbose(THIS"(ASCII: %s)]"CR, gd->uc_data);
//       //
//       //   if ( ( xSerialSemaphore ) != NULL )
//       //   xSemaphoreGive( ( xSerialSemaphore ) );  // make the Serial Port available
//       // }
//
//       //break;
//     }
//     else
//     {
//       #if defined(DEEP_DEBUG_WIFI_RECV)
//       Serial.println(__LINE__);
//       #endif
//       while(WIFICOM->available())
//       {
//         WIFICOM->read();
//         // if time is more than as in practice break or return
//         //Delay(1000);
//         //Serial.println(__LINE__);
//
//       }
//       //return (1 == ret ? 0 : -1);
//     }

    //recvlen = WIFICOM->available();

  }

  return (1 == ret ? 0 : -1);

}


/*
10 lines inlcuding <- this emptyline
ets Jan  8 2013,rst cause:2, boot mode:(3,6)

load 0x4010f000, len 1384, room 16
tail 8

chksum 0x2d
csum 0x2d
v09f0c112
~ld
*/

/**
* [wifi_reset reset the wifi module]
* @return [confirm whether wifi module is reset]
*/
int wifi_reset()
{

  int ret = -1;
  int rep = 0;
  int rec = 0;
  int len = -1;
  int temp = -1;

  selectCommandMode(1);  // CMD mode on

  WIFICOM->println("AT+RST");

  while (4>WIFICOM->available());

  while (WIFICOM->available())
  {
    rec = readline(WIFICOM->read(), buffer, 32, &len);
    if (1==rec && 0<=len)
    {
      ret = ( NULL != strstr(buffer, "OK") );

      //Log.Verbose(THIS"check %d %s"CR, ret, buffer);
      if(0!=ret)
      {
        // getout of mode or else ESP8266 will go into bootloader mode.
        // within 100 ms
        // Here it is done instantly
        //
        selectCommandMode(0); // CMD mode off
        break;
      }
      else
      {
        selectCommandMode(0);
        Log.Error(THIS"wifi implementation %s %d"CR, __func__, __LINE__);
        return -1;
      }
    }
  }

  Delay(100);
  selectCommandMode(1);

  memset(buffer, 0, 32);
  while (2>WIFICOM->available());

  while (WIFICOM->available())
  {
    /*
    10 lines inlcuding <- this emptyline as well
    ets Jan  8 2013,rst cause:2, boot mode:(3,6)

    load 0x4010f000, len 1384, room 16
    tail 8

    chksum 0x2d
    csum 0x2d
    v09f0c112
    ~ld
    */
    rec = readline(WIFICOM->read(), buffer, 32, &len);
    if (1==rec && 0<=len)
    {
      ret = ( NULL != strstr(buffer, "Initialized") );

      if( (0 == ret) )
      {
        rep++;
        Log.Verbose(THIS"response %d %d %s"CR, rep, len, buffer);
      }
      else
      {
        Log.Verbose(THIS"response %d %d %s"CR, rep, len, buffer);
        break;
      }
      memset(buffer, 0, 32);
    }
  }

  selectCommandMode(0); // Back in transparent mode

  return (1 == ret ? 0 : -1);

}

/**
* [wifi_set_server_mode set server mode]
* @param  _mode [whether to set wifi module in ap or station mode]
* @return       [return whether expected mode is selected]
*/
int wifi_set_remote_IP_PORT()
{

  int ret = -1;
  int rep = 0;
  int rec = 0;
  int len = -1;
  int temp = -1;

  selectCommandMode(1);  // CMD mode on

  memset(buffer, 0, 32);
  // AT+SRIPP 255.255.255.255 12345
  // 123456789012345678901234567890
  // 30
  sprintf(buffer, "AT+SRIPP %s %d", PEER_IP_ADDRESS, PEER_PORT); // check just filling up the buffer
  Log.Verbose(THIS"wifi implementation %s %d %s"CR, __func__, __LINE__, buffer);
  WIFICOM->println(buffer);
  memset(buffer, 0, 32);

  selectCommandMode(0); // CMD mode off

  while (4>WIFICOM->available());
  Log.Verbose(THIS"wifi implementation %d"CR, WIFICOM->available());
  // Issue
  // WIFI: 155: wifi mode set for SKY_SYSTEM
  // WIFI: 448: check
  // ERROR: WIFI: 170: wifi remote ip port
  // WIFI: 171: Please check for delay and/or implementation
  // ERROR: MAIN: 201: WIFI fault
  //

  while (WIFICOM->available())
  {
    rec = readline(WIFICOM->read(), buffer, 32, &len);
    //Log.Verbose(THIS"check %d",rec);
    ret = ( NULL != strstr(buffer, "OK") );

    if (1==rec && 0<=len && 1==ret)
    {
      // ret = ( NULL != strstr(buffer, "OK") );
      Log.Verbose(THIS"wifi implementation %s %d %s"CR, __func__, __LINE__, buffer);
      selectCommandMode(0); // CMD mode off

      break;
    }
    // else if (1==rec)
    // {
    //   Log.Verbose(THIS"wifi implementation %s %d %s"CR, __func__, __LINE__, buffer);
    //   memset(buffer, 0, 32);
    // }
    // else
    // {
    //   //Log.Verbose(THIS"wifi implementation %d %d %d %s"CR, rec, len, ret, buffer);
    // }

  }
  //while ( (0==rec && 0==ret) );//|| WIFICOM->available() );

  return (1 == ret ? 0 : -1);

  // while ( (0==rec) || WIFICOM->available() )
  // {
  //   rec = readline(WIFICOM->read(), buffer, 32, &len);
  //   //Log.Verbose(THIS"check %d",rec);
  //
  //   if (1==rec && 0<=len)
  //   {
  //     // ret = ( NULL != strstr(buffer, "OK") );
  //     Log.Verbose(THIS"wifi implementation %s %d %s"CR, __func__, __LINE__, buffer);
  //     //
  //     // //Log.Verbose(THIS"check %d %s"CR, ret, buffer);
  //     // if(0!=ret)
  //     // {
  //     //   // getout of mode or else ESP8266 will go into bootloader mode.
  //     //   // within 100 ms
  //     //   // Here it is done instantly
  //     //   //
  //     //   selectCommandMode(0); // CMD mode off
  //     //   break;
  //     // }
  //     // else
  //     // {
  //     //   selectCommandMode(0);
  //     //   return -1;
  //     // }
  //   }
  // }

  // NOW OK will be received
  // Log.Verbose(THIS"check %s"CR, buffer);
  // memset(buffer, 0, 32);
  //
  // while (4>WIFICOM->available());
  //
  // while (WIFICOM->available())
  // {
  //   rec = readline(WIFICOM->read(), buffer, 32, &len);
  //   if (1==rec && 0<=len)
  //   {
  //     ret = ( NULL != strstr(buffer, "OK") );
  //
  //     //Log.Verbose(THIS"check %d %s"CR, ret, buffer);
  //     if(0!=ret)
  //     {
  //       // getout of mode or else ESP8266 will go into bootloader mode.
  //       // within 100 ms
  //       // Here it is done instantly
  //       //
  //       selectCommandMode(0); // CMD mode off
  //       break;
  //     }
  //     else
  //     {
  //       selectCommandMode(0);
  //       Log.Error(THIS"wifi implementation %s %d"CR, __func__, __LINE__);
  //       return -1;
  //     }
  //   }
  // }
  //
  //
  // selectCommandMode(0); // digitalWrite(WIFI_CMD_MODE_PIN, HIGH); // Back in transparent mode
  //
  // return (1 == ret ? 0 : -1);
  //
  //
  // //int ret = 0;
  // //int len = -1;
  //
  // selectCommandMode(1); // digitalWrite(WIFI_CMD_MODE_PIN, LOW); // Switch to command mode
  //
  // memset(buffer, 0, 32);
  // sprintf(buffer, "AT+SRIPP %s %d", PEER_IP_ADDRESS, PEER_PORT); // check just filling up the buffer
  //
  // WIFICOM->println(buffer);
  //
  // memset(buffer, 0, 32);
  //
  // while (WIFICOM->available())
  // {
  //   if (0 < readline(WIFICOM->read(), buffer, 32, &len))
  //   {
  //     break;
  //     // This line will contain the IP1.IP2.IP3.IP4 PORTT/r/n ignore it for now
  //   }
  // }
  //
  // memset(buffer, 0, 32);
  //
  // while (WIFICOM->available())
  // {
  //   if (0 < readline(WIFICOM->read(), buffer, 32, &len))
  //   {
  //     ret = ( NULL != strstr(buffer, "OK") );
  //     break;
  //   }
  // }
  //
  // selectCommandMode(0); // digitalWrite(WIFI_CMD_MODE_PIN, HIGH); // Back in transparent mode
  //
  // return (1 == ret ? 0 : -1);
}

/**
* [wifi_check send AT]
* @return [return 0 if all fine -1 otherwise]
*/
int wifi_check()
{
  int ret = 0;
  int rec = 0;
  int len = -1;

  memset(buffer, 0, 32);

  selectCommandMode(1);  // CMD mode on

  WIFICOM->println("AT");

  while (4>WIFICOM->available());

  while (WIFICOM->available())
  {
    rec = readline(WIFICOM->read(), buffer, 32, &len);
    if (1==rec && 0<=len)
    {
      ret = ( NULL != strstr(buffer, "OK") );

      //Log.Verbose(THIS"check %d %s"CR, ret, buffer);
      if(0!=ret)
      {
        // getout of mode or else ESP8266 will go into bootloader mode.
        // within 100 ms
        // Here it is done instantly
        //
        selectCommandMode(0); // CMD mode off
        break;
      }
      else
      {
        selectCommandMode(0);
        Log.Error(THIS"wifi implementation %s %d"CR, __func__, __LINE__);
        return -1;
      }
    }
  }

  selectCommandMode(0); // digitalWrite(WIFI_CMD_MODE_PIN, HIGH); // CMD mode

  return (1 == ret ? 0 : -1);

}

/**
* /**
* [wifi_set_mode to start the wifi module in ap or station mode]
* AT+START 0 AP mode
* AT+START 1 STA mdoe
* [wifi_set_mode description]
* @param  _mode [0 or 1]
* @return       [return 0 if all fine -1 otherwise]
*
* OK
* IP
* IP
* ready
*
*/
int wifi_set_mode(int _mode)
{


  int ret = -1;
  int rec = -1;
  int len = -1;

  memset(buffer, 0, 32);

  selectCommandMode(1);  // CMD mode on

  sprintf(buffer, "AT+START %d", _mode); // check just filling up the buffer
  WIFICOM->println(buffer);

  while (4>WIFICOM->available());
  memset(buffer, 0, 32);

  while (WIFICOM->available())
  {
    rec = readline(WIFICOM->read(), buffer, 32, &len);
    if (1==rec && 0<=len)
    {
      ret = ( NULL != strstr(buffer, "OK") );

      //Log.Verbose(THIS"check %d %s"CR, ret, buffer);
      if(0!=ret)
      {
        // getout of mode or else ESP8266 will go into bootloader mode.
        // within 100 ms
        // Here it is done instantly
        //
        selectCommandMode(0); // CMD mode off
        break;
      }
      else
      {
        selectCommandMode(0);
        Log.Error(THIS"wifi implementation %s %d"CR, __func__, __LINE__);
        return -1;
      }
    }
  }

  selectCommandMode(0); // CMD mode off

  memset(buffer, 0, 32);

  while (WIFICOM->available())
  {
    rec = readline(WIFICOM->read(), buffer, 32, &len);
    if (1 == rec)
    {
      Log.Verbose(THIS"wifi implementation pass for %s() %d : %s"CR, __func__, __LINE__, buffer);
      break;
      // This line will contain the IP1.IP2.IP3.IP4/r/n ignore it for now
    }
    else
    {
      //Log.Error(THIS"wifi implementation %s %d : %s"CR, __func__, __LINE__, buffer);
    }
  }

  memset(buffer, 0, 32);

  while (WIFICOM->available())
  {
    rec = readline(WIFICOM->read(), buffer, 32, &len);
    if (1 == rec)
    {
      Log.Verbose(THIS"wifi implementation pass for %s() %d : %s"CR, __func__, __LINE__, buffer);
      break;
      // This line will contain the IP1.IP2.IP3.IP4/r/n ignore it for now
    }
    else
    {
      //Log.Error(THIS"wifi implementation %s %d : %s"CR, __func__, __LINE__, buffer);
    }
  }

  memset(buffer, 0, 32);
  ret = 0;

  while (WIFICOM->available() || 0==ret)
  {
    rec = readline(WIFICOM->read(), buffer, 32, &len);
    if (1 == rec)
    {
      ret = ( NULL != strstr(buffer, "ready") );
      if(1==ret)
      {
        break;
      }
    }
  }

  selectCommandMode(0); // digitalWrite(WIFI_CMD_MODE_PIN, HIGH); // CMD mode

  return (1 == ret ? 0 : -1);
}
