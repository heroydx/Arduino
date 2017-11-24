extern "C" {
#include <AppleBase64.h>
}

char testSendData[5][40] = {
  "HELLO WORLD",
  "01234567890",
  "ABCDEFGHIJKLMN\n",
  "abcdefghijklmnopqrstuvwxyz\n",
  "AAABBBCCCDDD\n",
};

void setup()
{
  // start serial port at 115200 bps:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB only
  }

  // encoding
  char inputString[] = "Base64EncodeExample";
  int inputStringLength = sizeof(inputString);

  Serial.print("Input string is:\t");
  Serial.println(inputString);
  Serial.println(inputStringLength);

  Serial.println();

  char outputString[200];
  int outputStringLenght;
  outputStringLenght = Base64encode(outputString, inputString, inputStringLength);

  Serial.print("Output string is:\t");
  Serial.println(outputString);
  Serial.println(outputStringLenght);

  Serial.println();

  char decodeString[200];
  int decodeStringLenght;
  decodeStringLenght = Base64decode(decodeString, outputString);

  Serial.print("decode string is:\t");
  Serial.println(decodeString);
  Serial.println(decodeStringLenght);

  Serial.println();

  //more
  int i, k;
  for (i = 0; i < 5; i++) {
    char *ptr;
    ptr = testSendData[i];
    inputStringLength = strlen(ptr);
    outputStringLenght = Base64encode(outputString, ptr, inputStringLength);
    Serial.print("input string is:\t");
    Serial.println(ptr);

    Serial.print("Output string is:\t");
    Serial.println(outputString);
    Serial.println(outputStringLenght);

    decodeStringLenght = Base64decode(decodeString, outputString);
    Serial.print("decode string is:\t");
    Serial.println(decodeString);
    Serial.println(decodeStringLenght);
  }
}


void loop() {


}

