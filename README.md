# SuperFreezer3000


A STC-1000 like on a Raspbeery Pico

# TODO





    |          |         | [USB] |         |          |
    | I2C0 SDA |   GP0   |1    40| VBUS    |          |
    | I2C0 SCL |   GP1   |2    39| VSYS    |          |
    |          |    GND1 |3    38| GND8    |          |
    |          |   GP2   |4    37| 3V3_EN  |          |
    |          |   GP3   |5    36| 3V3 OUT |          |
    |          |   GP4   |6    35|         |          |
    |          |   GP5   |7    34|  GP28   |          |
    |          |    GND2 |8    33| GND7    |          |
    |          |   GP6   |9    32|  GP27   |          |
    |          |   GP7   |10   31|  GP26   |          |
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


* components:
  * pico (Raspberry Pi Pico 2 W - RP2350)
  * temp - resistor type K
    * MAX 6675
  * relay (x2) - ENMG solid state relay PG5A2032
  * screen - NFP1315-45AY (128x64)
  * buttons - x5 combo


* thermo-MAX (SPI):
  * VCC
  * GND [38]
  * SCK [24]
  * CS  [22]
  * SO  [21]

* screen (I2C):
  * VCC
  * GND [3]
  * SCL [2]
  * SDA [1]

* buttons: (GPIO)
  * GND [3]
  * IN1 [15]
  * IN2 [16]
  * IN3 [17]
  * IN4 [19]
  * IN5 [20]

* relays: (GPIO)
  * VCC
  * GND [28]
  * IN1 [29]
  * IN2 [27]


* software features:
  * config target temp
  * toggle relays based on temp + config
  * display:
    * current temp
    * target temp
    * relays state
