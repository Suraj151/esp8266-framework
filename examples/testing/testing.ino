#include <EwingsEsp8266Stack.h>

void setup() {
 EwStack.initialize();
}

void loop() {
 EwStack.serve();
}
