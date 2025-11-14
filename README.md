# SuperFreezer3000


A STC-1000 like on a Raspbeery Pico

* TODO: doc on build / flash / console
* TODO: doc on PinConfig.h


# TODO

    |          |         | [USB] |         |          |
    | I2C0 SDA |   GP0   |1    40| VBUS    |          |
    | I2C0 SCL |   GP1   |2    39| VSYS    |          |
    |          |    GND1 |3    38| GND8    |          |
    |          |   GP2   |4    37| 3V3_EN  |          |
    |          |   GP3   |5    36| 3V3 OUT |          |
    |          |   GP4   |6    35|         | ADC_VREF |
    |          |   GP5   |7    34|  GP28   | ADC2     |
    |          |    GND2 |8    33| GND7    |  AGND    |
    |          |   GP6   |9    32|  GP27   | ADC1     |
    |          |   GP7   |10   31|  GP26   | ADC0     |
    |          |   GP8   |11   30| RUN     |          |
    |          |   GP9   |12   29|  GP22   |          |
    |          |    GND3 |13   28| GND6    |          |
    |          |   GP10  |14   27|  GP21   |          |
    |          |   GP11  |15   26|  GP20   |          |
    |          |   GP12  |16   25|  GP19   | SPI0 TX  |
    |          |   GP13  |17   24|  GP18   | SPI0 SCK |
    |          |    GND4 |18   23| GND5    |          |
    |          |   GP14  |19   22|  GP17   | SPI0 CSn |
    |          |   GP15  |20   21|  GP16   | SPI0 RX  |

ADC4 == internal temp
ADC3 == GP29 == led, but also wifi internal use -> "reserved"


* components:
  * pico (Raspberry Pi Pico 2 W - RP2350 A2 QFN60)
  * temp - resistor type K
    * MAX 6675
  * relay (x2) - ENMG solid state relay PG5A2032
  * screen - NFP1315-45AY (128x64)
  * buttons - x5 combo
  * TODO:
    * button
    * dc relay
    * photoresistor
    * fan
    * 12v converter


* thermo-MAX (SPI):
  * VCC
  * GND [38]
  * SCK [24]
  * CS  [22]
  * SO  [21]
* OR thermo dht22 (GPIO):
  * VCC
  * GND [38]
  * IN1 [24]

* screen (I2C):
  * VCC
  * GND [3]
  * SCL [2]
  * SDA [1]

* buttons control: (GPIO)
  * GND [3]
  * IN1 [15]
  * IN2 [16]
  * IN3 [17]
  * IN4 [19]
  * IN5 [20]

* relays AC: (GPIO)
  * VCC
  * GND [28]
  * IN1 [29] (hot)
  * IN2 [27] (cold)

* relay DC: (GPIO)
  * VCC
  * GND [28]
  * IN1 [26] (fan)

* button reset: (RUN)
  * RUN [30]
  * GND [38]

* photoresistor: (ADC)
  * VCC
  * GND
  * ADC2 [34]


* software features:
  * config target temp
  * toggle relays based on temp + config
  * display:
    * current temp
    * target temp
    * relays state
