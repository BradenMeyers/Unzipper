#ifndef testCases
#define testCases

#include <string.h>
#include <serverESP.h>

void accelerometerLoop();

void odometerSetup();
void odometerLoop();

void motorLoop();

void testSelect();

void web_select(int testSel);

extern Blogger testLogger;

#endif