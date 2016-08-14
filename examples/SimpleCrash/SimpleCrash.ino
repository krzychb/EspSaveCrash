#include "EspSaveCrash.h"

void setup(void)
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("SimpleCrash.ino");
  SaveCrash.print();

  Serial.println();
  Serial.println("Type a letter + <enter>");
  Serial.println("0 : attempt to divide by zero");
  Serial.println("e : attempt to read through a pointer to no object");
  Serial.println("c : clear crash information");
}


void loop(void)
{
  if (Serial.available() > 0)
  {
    char inChar = Serial.read();
    switch (inChar)
    {
      case '0':
        Serial.println("Attempting to divide one by zero ...");
        Serial.println(millis());
        int result;
        result = 1 / 0;
        Serial.print("Result = ");
        Serial.println(result);
        break;
      case 'e':
        Serial.println("Attempting to read through a pointer to no object ...");
        int* nullPointer;
        nullPointer = NULL;
        // null pointer dereference - read
        // attempt to read a value through a null pointer
        Serial.print(*nullPointer);
        break;
      case 'c':
        SaveCrash.clear();
        Serial.println("Crash information cleared");
        break;
      default:
        Serial.printf("%c typed\n", inChar);
        break;
    }
  }
}

