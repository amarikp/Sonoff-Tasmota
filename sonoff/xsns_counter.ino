/*
  xsns_counter.ino - Counter sensors (water meters, electricity meters etc.) sensor support for Sonoff-Tasmota

  Copyright (C) 2017  Maarten Damen and Theo Arends

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*********************************************************************************************\
 * Counter sensors (water meters, electricity meters etc.)
\*********************************************************************************************/

unsigned long last_counter_timer[MAX_COUNTERS]; // Last counter time in milli seconds

void CounterUpdate(byte index)
{
  unsigned long counter_debounce_time = millis() - last_counter_timer[index -1];
  if (counter_debounce_time > Settings.pulse_counter_debounce) {
    last_counter_timer[index -1] = millis();
    if (bitRead(Settings.pulse_counter_type, index -1)) {
      RtcSettings.pulse_counter[index -1] = counter_debounce_time;
    } else {
      RtcSettings.pulse_counter[index -1]++;
    }

//    snprintf_P(log_data, sizeof(log_data), PSTR("CNTR: Interrupt %d"), index);
//    AddLog(LOG_LEVEL_DEBUG);
  }
}

void CounterUpdate1()
{
  CounterUpdate(1);
}

void CounterUpdate2()
{
  CounterUpdate(2);
}

void CounterUpdate3()
{
  CounterUpdate(3);
}

void CounterUpdate4()
{
  CounterUpdate(4);
}

void CounterSaveState()
{
  for (byte i = 0; i < MAX_COUNTERS; i++) {
    if (pin[GPIO_CNTR1 +i] < 99) {
      Settings.pulse_counter[i] = RtcSettings.pulse_counter[i];
    }
  }
}

void CounterInit()
{
  typedef void (*function) () ;
  function counter_callbacks[] = { CounterUpdate1, CounterUpdate2, CounterUpdate3, CounterUpdate4 };

  for (byte i = 0; i < MAX_COUNTERS; i++) {
    if (pin[GPIO_CNTR1 +i] < 99) {
      pinMode(pin[GPIO_CNTR1 +i], INPUT_PULLUP);
      attachInterrupt(pin[GPIO_CNTR1 +i], counter_callbacks[i], FALLING);
    }
  }
}

/*********************************************************************************************\
 * Presentation
\*********************************************************************************************/

void MqttShowCounter(uint8_t* djson)
{
  char stemp[16];

  byte dsxflg = 0;
  for (byte i = 0; i < MAX_COUNTERS; i++) {
    if (pin[GPIO_CNTR1 +i] < 99) {
      if (bitRead(Settings.pulse_counter_type, i)) {
        dtostrfd((double)RtcSettings.pulse_counter[i] / 1000, 3, stemp);
      } else {
        dsxflg++;
        dtostrfd(RtcSettings.pulse_counter[i], 0, stemp);
      }
      snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("%s, \"" D_COUNTER "%d\":%s"), mqtt_data, i +1, stemp);
      *djson = 1;
#ifdef USE_DOMOTICZ
      if (1 == dsxflg) {
        DomoticzSensor(DZ_COUNT, RtcSettings.pulse_counter[i]);
        dsxflg++;
      }
#endif  // USE_DOMOTICZ
    }
  }
}

#ifdef USE_WEBSERVER
const char HTTP_SNS_COUNTER[] PROGMEM =
  "<tr><th>" D_COUNTER "%d</th><td>%s%s</td></tr>";

String WebShowCounter()
{
  String page = "";
  char stemp[16];
  char sensor[80];

  for (byte i = 0; i < MAX_COUNTERS; i++) {
    if (pin[GPIO_CNTR1 +i] < 99) {
      if (bitRead(Settings.pulse_counter_type, i)) {
        dtostrfi((double)RtcSettings.pulse_counter[i] / 1000, 3, stemp);
      } else {
        dtostrfi(RtcSettings.pulse_counter[i], 0, stemp);
      }
      snprintf_P(sensor, sizeof(sensor), HTTP_SNS_COUNTER, i+1, stemp, (bitRead(Settings.pulse_counter_type, i)) ? " " D_UNIT_SECOND : "");
      page += sensor;
    }
  }
  return page;
}
#endif  // USE_WEBSERVER

