/* stack initialize the services in setup and
 * should serve in loop continueously
 */

#include <EwingsEspStack.h>

void setup() {
 EwStack.initialize();
}

void loop() {
 EwStack.serve();
}
