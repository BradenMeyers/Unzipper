#ifndef testCases
#define testCases

#include <string.h>
#include <serverESP.h>

void accelerometerSetup();
void accelerometerLoop();

void hallSetup();
void hallLoop();

void servoSetupTest();
void servoLoop();

void motorSetup();
void motorLoop();

void testSelect();

void web_select(int testSel);

extern Blogger testLogger;

#endif