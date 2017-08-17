/*
 * ####                                                ####
 * ####                                                ####
 * ####                                                ####      ##
 * ####                                                ####    ####
 * ####  ############  ############  ####  ##########  ####  ####
 * ####  ####    ####  ####    ####  ####  ####        ########
 * ####  ####    ####  ####    ####  ####  ####        ########
 * ####  ####    ####  ####    ####  ####  ####        ####  ####
 * ####  ####    ####  ####    ####  ####  ####        ####    ####
 * ####  ############  ############  ####  ##########  ####      ####
 *                             ####                                ####
 * ################################                                  ####
 *            __      __              __              __      __       ####
 *   |  |    |  |    [__)    |_/     (__     |__|    |  |    [__)        ####
 *   |/\|    |__|    |  \    |  \    .__)    |  |    |__|    |             ##
 *
 *
 * DU-INO Arduino Library - Microchip MCP4922 SPI Dual 12-Bit DAC Module
 * Aaron Mavrinac <aaron@logick.ca>
 */

#ifndef DUINO_MCP4922_H_
#define DUINO_MCP4922_H_

#include "Arduino.h"

class DUINO_MCP4922 {
 public:
  enum Channel
  {
    A = 0,
    B = 1
  };

 public:
  DUINO_MCP4922(uint8_t ss);

  void begin();

  void output(Channel channel, uint16_t data);

 private:
  const uint8_t pin_ss;
};

#endif // DUINO_FUNCTION_H_