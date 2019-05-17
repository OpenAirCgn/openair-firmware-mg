#ifndef _BME280_H_
#define _BME280_H_

#include "mgos.h"
#include "mgos_i2c.h"

bool bme280_read_calib(struct mgos_i2c *i2c, bool addrPin);

bool bme280_read_data(struct mgos_i2c *i2c, bool addrPin);

typedef enum {
  MEASURE_OFF = 0,
  MEASURE_OS1 = 1,
  MEASURE_OS2 = 2,
  MEASURE_OS4 = 3,
  MEASURE_OS8 = 4,
  MEASURE_OS16 = 8
} BME280_Measure_Mode;

typedef enum {
  MODE_SLEEP = 0,
  MODE_FORCED = 1,
  MODE_NORMAL = 3
} BME280_Operation_Mode;

typedef enum {
  STANDBY_0MS5 = 0,
  STANDBY_62MS5 = 1,
  STANDBY_125MS = 2,
  STANDBY_250MS = 3,
  STANDBY_500MS = 4,
  STANDBY_1000MS = 5,
  STANDBY_10MS = 6,
  STANDBY_20MS = 7
} BME280_Standby_Time;

typedef enum {
  FILTER_OFF = 0,
  FILTER_2 = 1,
  FILTER_4 = 2,
  FILTER_8 = 3,
  FILTER_16 = 4
} BME280_Filter_Mode;

bool bme280_set_mode(struct mgos_i2c *i2c, bool addrPin,
  BME280_Measure_Mode humidity_mode,
  BME280_Measure_Mode pressure_mode,
  BME280_Measure_Mode temperature_mode,
  BME280_Operation_Mode operation,
  BME280_Standby_Time pause,
  BME280_Filter_Mode filter);


void bme280_compensate(int32_t temp, int32_t press, int32_t hum);

#endif