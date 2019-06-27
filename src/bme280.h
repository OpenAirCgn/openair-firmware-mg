#ifndef _BME280_H_
#define _BME280_H_

#include "mgos.h"
#include "mgos_i2c.h"

typedef struct {
  uint16_t dig_T1;
  int16_t  dig_T2;
  int16_t  dig_T3;
  uint16_t dig_P1;
  int16_t  dig_P2;
  int16_t  dig_P3;
  int16_t  dig_P4;
  int16_t  dig_P5;
  int16_t  dig_P6;
  int16_t  dig_P7;
  int16_t  dig_P8;
  int16_t  dig_P9;
  uint8_t  dig_H1;
  int16_t  dig_H2;
  uint8_t  dig_H3;
  int16_t  dig_H4;
  int16_t  dig_H5;
  int8_t   dig_H6;
} BME280_Calib;

typedef struct {
  struct mgos_i2c *i2c;
  uint8_t idx;
  BME280_Calib calib;
} BME280_Struct;

bool bme280_init(BME280_Struct* bme, struct mgos_i2c *i2c, uint8_t idx);

bool bme280_read_calib(BME280_Struct* bme);

bool bme280_read_data(BME280_Struct* bme, uint32_t *outTemp, uint32_t* outPress, uint32_t *outHum);

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


bool bme280_set_mode(BME280_Struct* bme,
  BME280_Measure_Mode humidity_mode,
  BME280_Measure_Mode pressure_mode,
  BME280_Measure_Mode temperature_mode,
  BME280_Operation_Mode operation,
  BME280_Standby_Time pause,
  BME280_Filter_Mode filter);

void bme280_compensate(BME280_Struct* bme, int32_t temp, int32_t press, int32_t hum,
  float *outTemp, float *outPress, float *outHum);

#endif
