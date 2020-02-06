/* stack initialize the services in setup and
 * should serve in loop continueously
 */

#include <EwingsEsp8266Stack.h>

void setup() {
 EwStack.initialize();
}

void loop() {
 EwStack.serve();
}
