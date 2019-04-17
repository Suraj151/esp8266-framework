#include <SmartLooHandler.h>

SmartLooWorker SLooWorker;

void setup() {
  SLooWorker.begin();
}

void loop() {
  SLooWorker.run();
}

//#include <EwingsEsp8266Stack.h>
//
//void setup() {
//  EwStack.initialize();
//}
//
//void loop() {
//  EwStack.serve();
//}
