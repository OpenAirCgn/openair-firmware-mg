#include "bme280.h"

#include "openairboard.h"

#define BME280_BASE_ADDR 0x76 /* add 1 if address pin is set */
#define BME280_CALIB1_BASE 0x88
#define BME280_CALIB1_SIZE 26
#define BME280_CALIB2_BASE 0xe1
#define BME280_CALIB2_SIZE 7

#define BME280_ID_ADDR 0xd0
#define BME280_ID_VAL 0xc0
#define BME280_RESET_ADDR 0xe0
#define BME280_RESET_VAL 0xb6
#define BME280_CTRL_HUM_ADDR 0xf2
#define BME280_STATUS_ADDR 0xf3
#define BME280_CTRL_MEAS_ADDR 0xf4
#define BME280_CONFIG_ADDR 0xf5

#define BME280_DATA_BASE 0xf7
#define BME280_DATA_SIZE 8

#define BME280_DONE_RETRIES 1000


bool bme280_init(BME280_Struct* bme, struct mgos_i2c *i2c, uint8_t idx) {
  bme->i2c = i2c;
  bme->idx = idx;
  bool ok = bme280_read_calib(bme);  //TODO: ID must be set for new HW
  if (!ok) {
    LOG(LL_ERROR, ("failed bme280_read_calib() idx: %d", idx));
    return ok;
  }

  ok = bme280_set_mode(bme,
    MEASURE_OS4,
    MEASURE_OS4,
    MEASURE_OS4,
    MODE_NORMAL,
    STANDBY_62MS5,
    FILTER_2);

  if (!ok) {
    LOG(LL_ERROR, ("failed bme_set_mode"));
  }

  return ok;
}

bool bme280_read_calib(BME280_Struct* bme) {
  uint8_t addr = BME280_BASE_ADDR + (bme->idx ? 1 : 0);
  uint8_t calib1[26]; // 0x88..0xa1
  uint8_t calib2[7];  // 0xe1..0xe7
  bool ok = mgos_i2c_read_reg_n(bme->i2c, addr, BME280_CALIB1_BASE, BME280_CALIB1_SIZE, &(calib1[0]));
  ok = ok && mgos_i2c_read_reg_n(bme->i2c, addr, BME280_CALIB2_BASE, BME280_CALIB2_SIZE, &(calib2[0]));
  if (ok) {
    bme->calib.dig_T1 = calib1[0] | (calib1[1] << 8);
    bme->calib.dig_T2 = calib1[2] | (calib1[3] << 8);
    bme->calib.dig_T3 = calib1[4] | (calib1[5] << 8);
    bme->calib.dig_P1 = calib1[6] | (calib1[7] << 8);
    bme->calib.dig_P2 = calib1[8] | (calib1[9] << 8);
    bme->calib.dig_P3 = calib1[10] | (calib1[11] << 8);
    bme->calib.dig_P4 = calib1[12] | (calib1[13] << 8);
    bme->calib.dig_P5 = calib1[14] | (calib1[15] << 8);
    bme->calib.dig_P6 = calib1[16] | (calib1[17] << 8);
    bme->calib.dig_P7 = calib1[18] | (calib1[19] << 8);
    bme->calib.dig_P8 = calib1[20] | (calib1[21] << 8);
    bme->calib.dig_P9 = calib1[22] | (calib1[23] << 8);
    bme->calib.dig_H1 = calib1[25];
    bme->calib.dig_H2 = calib2[0] | (calib2[1] << 8);
    bme->calib.dig_H3 = calib2[2];
    bme->calib.dig_H4 = (calib2[3] << 4) | (calib2[4] & 0x0f);
    bme->calib.dig_H5 = (calib2[4] >> 4) | (calib2[5] << 4);
    bme->calib.dig_H6 = calib2[6];
  } else {
    mgos_i2c_stop(bme->i2c);
  }
  return ok;
}


bool bme280_read_data(BME280_Struct* bme, uint32_t *outTemp, uint32_t* outPress, uint32_t *outHum) {
  uint8_t addr = BME280_BASE_ADDR + (bme->idx ? 1 : 0);
  uint8_t data[BME280_DATA_SIZE];  // 0xf7..0xfe
  bool ok = mgos_i2c_read_reg_n(bme->i2c, addr, BME280_DATA_BASE, BME280_DATA_SIZE, &(data[0]));
  if (ok) {
    uint32_t press = (data[2] >> 4) | (data[1] << 4) | (data[0] << 12);
    uint32_t temp = (data[5] >> 4) | (data[4] << 4) | (data[3] << 12);
    uint32_t hum = data[7] | (data[6] << 8);
    LOG(LL_DEBUG, ("press %i temp %i hum %i", press, temp, hum));
    if (outTemp) { *outTemp = temp; }
    if (outPress) { *outPress = press; }
    if (outHum) { *outHum = hum; }
  }
  return ok;
}

bool bme280_set_mode(BME280_Struct* bme,
  BME280_Measure_Mode humidity_mode,
  BME280_Measure_Mode pressure_mode,
  BME280_Measure_Mode temperature_mode,
  BME280_Operation_Mode operation,
  BME280_Standby_Time pause,
  BME280_Filter_Mode filter) {

  uint8_t addr = BME280_BASE_ADDR + (bme->idx ? 1 : 0);
  //set to sleep mode to allow 
  bool ok = mgos_i2c_write_reg_b(bme->i2c, addr, BME280_CTRL_MEAS_ADDR, 0x00);
  if (!ok) {
    LOG(LL_ERROR, ("Could not pause BME280"));
    return false;
  }

  int i;
  for (i=0; i<BME280_DONE_RETRIES; i++) {
    int val = mgos_i2c_read_reg_b(bme->i2c, addr, BME280_STATUS_ADDR);
    if (val < 0) {
      LOG(LL_ERROR, ("Failed to read BME280 status"));
      return false;
    }
    if (!(val & 0x09)) {
      break;
    }
  }
  if (i >= BME280_DONE_RETRIES) {
    LOG(LL_ERROR, ("Wait for BME280 to finish timed out"));
    return false;
  }

  ok = mgos_i2c_write_reg_b(bme->i2c, addr, BME280_CTRL_HUM_ADDR, humidity_mode);
  
  uint8_t conf = (((uint8_t)pause) << 5) | (((uint8_t)filter) << 2);  
  ok = ok && mgos_i2c_write_reg_b(bme->i2c, addr, BME280_CONFIG_ADDR, conf);

  uint8_t ctrl = (((uint8_t)temperature_mode) << 5) | (((uint8_t)pressure_mode) << 2) | ((uint8_t) operation);  
  ok = ok && mgos_i2c_write_reg_b(bme->i2c, addr, BME280_CTRL_MEAS_ADDR, ctrl);

  if (!ok) {
    LOG(LL_ERROR, ("Failed to set BME280 configuration"));
  }
  return ok;
}

//formulas taken from datasheet
void bme280_compensate(BME280_Struct* bme, int32_t temp, int32_t press, int32_t hum,
  float *outTemp, float *outPress, float *outHum) {

  int32_t t_fine;
  float temperature = 0.0f;
  float pressure = 0.0f;  //hPa
  float humidity = 0.0f;

  //temperature
  {
    int32_t var1, var2;
    var1 = ((((temp>>3) - ((int32_t)(bme->calib.dig_T1)<<1))) * ((int32_t)(bme->calib.dig_T2))) >> 11;
    var2 = (((((temp>>4) - ((int32_t)(bme->calib.dig_T1))) * ((temp>>4) - ((int32_t)(bme->calib.dig_T1)))) >> 12) * ((int32_t)(bme->calib.dig_T3))) >> 14;
    t_fine = var1 + var2;
    temperature = ((t_fine*5+128)>>8) * 0.01;
  }

  //pressure
  {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)(bme->calib.dig_P6);
    var2 = var2 + ((var1*(int64_t)(bme->calib.dig_P5))<<17);
    var2 = var2 + (((int64_t)(bme->calib.dig_P4))<<35);
    var1 = ((var1 * var1 * (int64_t)(bme->calib.dig_P3))>>8);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)(bme->calib.dig_P1))>>33;
    if (var1 != 0) {
      p = 1048576-press;
      p = (((p<<31)-var2)*3125)/var1;
      var1 = (((int64_t)(bme->calib.dig_P9)) * (p>>13) * (p>>13)) >> 25; var2 = (((int64_t)(bme->calib.dig_P8)) * p) >> 19;
      p = ((p + var1 + var2) >> 8) + (((int64_t)(bme->calib.dig_P7))<<4);
      pressure = p / 25600.0;
    }
  }

  //humidity
  int32_t v_x1_u32r;
  v_x1_u32r = (t_fine - ((int32_t)76800));
  v_x1_u32r = (((((hum << 14) - (((int32_t)(bme->calib.dig_H4)) << 20) - (((int32_t)(bme->calib.dig_H5)) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)(bme->calib.dig_H6))) >> 10) * (((v_x1_u32r * ((int32_t)(bme->calib.dig_H3))) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)(bme->calib.dig_H2)) + 8192) >> 14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)(bme->calib.dig_H1))) >> 4));
  v_x1_u32r = ((v_x1_u32r < 0) ? 0 : v_x1_u32r);
  v_x1_u32r = ((v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r);
  humidity = (v_x1_u32r>>12) / 1024.0;

  LOG(LL_DEBUG, ("press %f temp %f hum %f", pressure, temperature, humidity));
  if (outPress) {
    *outPress = pressure;
  }

  if (outTemp) {
    *outTemp = temperature;
  }

  if (outHum) {
    *outHum = humidity;
  }
}
// vim: et:sw=2:ts=2
