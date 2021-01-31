/* stack initialize the services in setup and
 * should serve in loop continueously
 */

#include "DeviceIotSensor.h"

DeviceIotSensor sensor;

void setup() {
  EwStack.initialize();
  __device_iot_service.initDeviceIotSensor(&sensor);
}

void loop() {
  EwStack.serve();
}
