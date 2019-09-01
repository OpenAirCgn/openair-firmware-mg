#include "mgos.h"
#include "alphasense.h"



/** maps sensor types to recommended compensation algorithm */
static const AlphaTempCompAlgorithm ALPHA_RECOMMENDED_COMPENSATION_ALGORITHM[10] = {
  ALPHA_COEFF_KT1,
  ALPHA_COEFF_KT,
  ALPHA_COEFF_KT1,
  ALPHA_COEFF_KT,
  ALPHA_COEFF_KT1,
  ALPHA_COEFF_NT,
  ALPHA_COEFF_NT,
  ALPHA_COEFF_NT,
  ALPHA_COEFF_KT2,
  ALPHA_COEFF_KT2
};

#define ALPHA_TEMP_COMP_TEMPERATURE_COUNT 9
//temperature lookup points for tempCompFactors
static const float ALPHA_TEMP_COMP_TEMPERATURES[ALPHA_TEMP_COMP_TEMPERATURE_COUNT] = {
  -30, -20, -10, 0, 10, 20, 30, 40, 50
};

/** Three-dimensional array of compensation coefficients:
first index: sensor model (see AlphaSensorType)
second index: temperature compensation model (see AlphaTempCompAlgorithm)
third index: temperature (see ALPHA_TEMP_COMP_TEMPERATURES)
*/
static const float tempCompFactors[10][4][ALPHA_TEMP_COMP_TEMPERATURE_COUNT] = {
  {//0: NO-A4
    {1.7, 1.7, 1.6, 1.5, 1.5, 1.5, 1.5, 1.6, 1.7},
    {1.1, 1.1, 1.1, 1.0, 1.0, 1.0, 1.0, 1.1, 1.1},
    {0.7, 0.7, 0.7, 0.7, 0.8, 1.0, 1.2, 1.4, 1.6},
    {-25, -25, -25, -25, -16, 0.0, 56, 200, 615}
  },

  {//1: NO-B4
    {2.9, 2.9, 2.2, 1.8, 1.7, 1.6, 1.5, 1.4, 1.3},
    {1.8, 1.8, 1.4, 1.1, 1.1, 1.0, 0.9, 0.9, 0.8},
    {0.8, 0.8, 0.8, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3},
    {-25, -25, -25, -25, -16, 0.0, 56, 200, 615}
  },

  {//2: NO-A43F
    {0.8, 0.8, 1.0, 1.2, 1.6, 1.8, 1.9, 2.5, 3.6},
    {0.4, 0.4, 0.6, 0.7, 0.9, 1.0, 1.1, 1.4, 2.0},
    {0.2, 0.2, 0.2, 0.2, 0.7, 1.0, 1.3, 2.1, 3.5},
    {-4, -4, -4, -4, -2, 0, 10, 35, 132}
  },

  {//3: NO-B43F
    {1.3, 1.3, 1.3, 1.3, 1.0, 0.6, 0.4, 0.2, -1.5},
    {2.2, 2.2, 2.2, 2.2, 1.7, 1.0, 0.7, 0.3, -2.5},
    {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.4, -0.1, -4},
    {7.0, 7.0, 7.0, 7.0, 4.0, 0.0, 0.5, 5.0, 67}
  },

  {//4: OX-A431
    {1.0, 1.2, 1.2, 1.6, 1.7, 2.0, 2.1, 3.4, 4.6},
    {0.5, 0.6, 0.6, 0.8, 0.9, 1.0, 1.1, 1.7, 2.3},
    {0.1, 0.1, 0.2, 0.3, 0.7, 1.0, 1.7, 3.0, 4.0},
    {-5, -5, -4, -3, 0.5, 0.0, 9.0, 42, 134}
  },

  {//5: OX-B431
    {0.9, 0.9, 1.0, 1.3, .15, .17, 2.0, 2.5, 3.7},
    {0.5, 0.5, 0.6, 0.8, 0.9, 1.0, 1.2, 1.5, 2.2},
    {0.5, 0.5, 0.5, 0.6, 0.6, 1.0, 2.8, 5.0, 5.3},
    {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 8.5, 23, 103}
  },

  {//6: CO-A4
    {1.0, 1.0, 1.0, 1.0, -0.2, -0.9, -1.5, -1.5, -1.5},
    {-1.1, -1.1, -1.1, -1.1, 0.2, 1.0, 1.7, 1.7, 1.7},
    {1.9, 2.9, 2.7, 3.9, 2.1, 1.0, -0.6, -0.3, -0.5},
    {13, 12, 16, 11, 40, -15, -18, -36}
  },

  {//7: CO-B4
    {0.7, 0.7, 0.7, 0.7, 1.0, 3.0, 3.5, 4.0, 4.5},
    {0.2, 0.2, 0.2, 0.2, 0.3, 1.0, 1.2, 1.3, 1.5, },
    {-1.0, -0.5, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0},
    {55, 55, 55, 50, 31, 0, -50, -150, -250}
  },

  {//8: SO2-A4
    {1.3, 1.3, 1.3, 1.2, 0.9, 0.4, 0.4, 0.4, 0.4},
    {3.3, 3.3, 3.3, 3.0, 2.3, 1.0, 1.0, 1.0, 1.0},
    {1.5, 1.5, 1.5, 1.5, 1.0, 1.0, 1.0, 1.0, 1.0},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 25, 45}
  },

  {//9: SO2-B4
    {1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.9, 3.0, 5.8},
    {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.2, 1.9, 3.6},
    {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 3.5, 7.0},
    {-4, -4, -4, -4, -4, 0.0, 20, 140, 450}
  }
};


AlphaTempCompAlgorithm alphaRecommendedAlgorithmForSensor(AlphaSensorType sensor) {
  return ALPHA_RECOMMENDED_COMPENSATION_ALGORITHM[(uint8_t)sensor];
}

float alphaCompensationFactor(AlphaSensorType sensor, AlphaTempCompAlgorithm type, float celsius) {
  int upperIdx;
  for (upperIdx = 0; upperIdx < ALPHA_TEMP_COMP_TEMPERATURE_COUNT; upperIdx++) {
    if (ALPHA_TEMP_COMP_TEMPERATURES[upperIdx] >= celsius) break;
  }
  if (upperIdx <= 0) {  //temp is smaller than min temp
    return tempCompFactors[sensor][type][0];
  } else if (upperIdx >= ALPHA_TEMP_COMP_TEMPERATURE_COUNT) {  //temp is larger than max temp
    return tempCompFactors[sensor][type][ALPHA_TEMP_COMP_TEMPERATURE_COUNT-1];
  } else {
    int lowerIdx = upperIdx - 1;
    float upperTemp = ALPHA_TEMP_COMP_TEMPERATURES[upperIdx];
    float lowerTemp = ALPHA_TEMP_COMP_TEMPERATURES[lowerIdx];
    float wUpper = (celsius-lowerTemp) / (upperTemp-lowerTemp);
    float wLower = 1.0 - wUpper;
    float upperVal = tempCompFactors[sensor][type][upperIdx];
    float lowerVal = tempCompFactors[sensor][type][lowerIdx];
    return wUpper * upperVal + wLower * lowerVal;
  }
}

float alphaCompensateTemperature(AlphaSensorType sensor, AlphaTempCompAlgorithm type, float celsius, float weu, float aeu, float wee, float aee, float we0, float ae0) {
  float factor = alphaCompensationFactor(sensor, type, celsius);
  switch (type) {
    case ALPHA_COEFF_NT:
      return (weu - wee) - factor * (aeu - aee);
      break;
    case ALPHA_COEFF_KT:
      return (weu - wee) - factor * (we0 / ae0) * (aeu - aee);
      break;
    case ALPHA_COEFF_KT1:
      return (weu - wee)- (we0 - ae0) - factor * (aeu - aee);
      break;
    case ALPHA_COEFF_KT2:
      return (weu - wee) - we0 - factor;
      break;
    default:
      LOG(LL_ERROR, ("Unknown compensation type %i", type));
      return 0;
  }
}

float alphaCalculatePPM_(AlphaSensorType sensor, AlphaTempCompAlgorithm type, float celsius, float weu, float aeu, float wee, float aee, float we0, float ae0, float sensitivity) {
  return alphaCompensateTemperature(sensor, type, celsius, weu, aeu, wee, aee, we0, ae0) * sensitivity;
}


float alphaCalculatePPM(AlphaSensorType sensor, float celsius, float weu, float aeu, float wee, float aee, float we0, float ae0, float sensitivity) {

	AlphaTempCompAlgorithm algo = alphaRecommendedAlgorithmForSensor(sensor);
  	return alphaCompensateTemperature(sensor, algo, celsius, weu, aeu, wee, aee, we0, ae0) * sensitivity;


}
