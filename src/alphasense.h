
/** Alphasense sensor calibration / temperature compensation.
Most taken from Alphasense Application Note AAN 803-05. */


/** sensor model for tempCompFactors */
typedef enum {
  ALPHA_SENS_NO_A4 = 0,
  ALPHA_SENS_NO_B4 = 1,
  ALPHA_SENS_NO_A43F = 2,
  ALPHA_SENS_NO_B43F = 3,
  ALPHA_SENS_OX_A431 = 4,
  ALPHA_SENS_OX_B431 = 5,
  ALPHA_SENS_CO_A4 = 6,
  ALPHA_SENS_CO_B4 = 7,
  ALPHA_SENS_SO2_A4 = 8,
  ALPHA_SENS_SO2_B4 = 9
} AlphaSensorType;

/** sensor temperature compensation coefficient / algorithm for tempCompFactors */
typedef enum {
  ALPHA_COEFF_NT = 0,
  ALPHA_COEFF_KT = 1,
  ALPHA_COEFF_KT1 = 2,
  ALPHA_COEFF_KT2 = 3
} AlphaTempCompAlgorithm;

/** return the recommended compensation algorithm for a given sensor type
@param sensor sensor type
@return the manufacturer recommended compensation algorithm
*/
AlphaTempCompAlgorithm alphaRecommendedAlgorithmForSensor(AlphaSensorType sensor);

/** find a compensation factor for a sensor, a compensation algorithm and a temperature 
@param sensor sensor type
@param type compensation type
@param celsius temperature in degrees celsius
@return the appropriate factor for application in compensation algorithm type
*/
float alphaCompensationFactor(AlphaSensorType sensor, AlphaTempCompAlgorithm type, float celsius);

/** apply a compensation algorithm to sensor readout values
@param sensor sensor type
@param type compensation type
@param celsius temperature in degrees celsius
@param weu uncompensated working electrode value
@param weu uncompensated aux electrode value
@param wee working electrode electronic offset
@param aee aux electrode electronic offset
@param we0 working electrode zero
@param ae0 aux electrode zero
@return wec working electrode calibrated value
 */
float alphaCompensateTemperature(AlphaSensorType sensor, AlphaTempCompAlgorithm type, float celsius, float weu, float aeu, float wee, float aee, float we0, float ae0);

/** calculate a gas concentration from sensor values,
 * with control over the temp compensation alorithm used.
@param sensor sensor type
@param type compensation type
@param celsius temperature in degrees celsius
@param weu uncompensated working electrode value
@param weu uncompensated aux electrode value
@param wee working electrode electronic offset
@param aee aux electrode electronic offset
@param we0 working electrode zero
@param ae0 aux electrode zero
@param sensitivity sensor sensitivity
@return ppm value
 */
float alphaCalculatePPM(AlphaSensorType sensor, AlphaTempCompAlgorithm type, float celsius, float weu, float aeu, float wee, float aee, float we0, float ae0, float sensitivity);

/** calculate a gas concentration from sensor values
 * using the recommended algorithm
@param sensor sensor type
@param celsius temperature in degrees celsius
@param weu uncompensated working electrode value
@param weu uncompensated aux electrode value
@param wee working electrode electronic offset
@param aee aux electrode electronic offset
@param we0 working electrode zero
@param ae0 aux electrode zero
@param sensitivity sensor sensitivity
@return ppm value
 */
float alphaCalculatePPM(AlphaSensorType sensor, float celsius, float weu, float aeu, float wee, float aee, float we0, float ae0, float sensitivity);
