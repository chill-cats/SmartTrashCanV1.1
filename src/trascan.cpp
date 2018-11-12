#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include "NewPing.h"
#include "Bounce2.h"

static const uint8_t TRIG_PIN_HAND = 5;
static const uint8_t ECHO_PIN_HAND = 4;

static const uint8_t TRIG_PIN_TRASH = 13;
static const uint8_t ECHO_PIN_TRASH = 12;

static const uint8_t HOME_SWITCH_PIN = 11;
static const uint8_t ACTUATOR_PIN_1 = 10;
static const uint8_t ACTUATOR_PIN_2 = 9;

static const uint8_t LOCK_LED = 2;
static const uint8_t RESET_BUTTON = 3;
static const uint8_t MANUALLY_OPEN_BUTTON = 23;

static const int DEBUG_MODE_PIN = 22;

int flag_isOpenned = 0;
int flag_isFull = 0;
int flag_debug_mode = 0;
int flag_manuallyOpened = 0;

NewPing sonarHand(TRIG_PIN_HAND, ECHO_PIN_HAND, 200);
NewPing sonarTrash(TRIG_PIN_TRASH, ECHO_PIN_TRASH, 400);
DFRobotDFPlayerMini myDFPlayer;
Bounce debouncerResetButton = Bounce();
Bounce debouncerManuallyOpenButton = Bounce();

void sendMessage() {

	Serial.println("AT");
	delay(1000);
	if (flag_debug_mode) {
		Serial.print("AT");
		Serial.print("Response: ");
		Serial.println(Serial2.readString());
	}
	delay(100);

	Serial2.println("AT+CMGF=1");
	delay(1000);
	if (flag_debug_mode) {
		Serial.println("AT+CMGF=1");
		Serial.print("Response: ");
		Serial.println(Serial2.readString());
	}
	delay(100);

	Serial2.println("AT+CMGS=\"+84929341350\"");
	delay(1000);
	if (flag_debug_mode) {
		Serial.println("AT+CMGS=\"+84929341350\"");
		Serial.print("Response: ");
		Serial.println(Serial2.readString());
	}
	delay(100);

	Serial2.println("Thung rac 1 da day");
	delay(1000);
	if (flag_debug_mode) {
		Serial.println("Thung rac 1 da day");
		Serial.print("Response: ");
		Serial.println(Serial2.readString());
	}
	delay(100);

	Serial2.println((char) 26);
	delay(1000);
	if (flag_debug_mode) {
		Serial.println("Ctrl + Z");
		Serial.print("Response: ");
		Serial.println(Serial2.readString());
	}
}

void checkTrash() {
	int d = sonarTrash.ping_cm();
	if (flag_debug_mode) {
		Serial.print("SonarTrash measurement in cm: ");
		Serial.println(d);
	}
	if (d <= 15 && d != 0) {
		if (flag_debug_mode) {
			Serial.println("Locked the trash can");
		}
		flag_isFull = 1;
		if (flag_debug_mode) {
			Serial.println("Set flag_isFull to 1");
		}
		sendMessage();
		if (flag_debug_mode) {
			Serial.println("sendMessage() called");
		}
	}
}

void moveActuator(int action) {
	if (flag_debug_mode) {
		Serial.print("argument for the move Actuator Method is ");
		Serial.println(action);
	}
	switch (action) {
	case 1: {
		digitalWrite(ACTUATOR_PIN_1, HIGH);
		digitalWrite(ACTUATOR_PIN_2, LOW);
		if (flag_debug_mode) {
			Serial.println("Case 1 called");
		}
		break;
	}
	case 2: {
		digitalWrite(ACTUATOR_PIN_1, LOW);
		digitalWrite(ACTUATOR_PIN_2, HIGH);
		if (flag_debug_mode) {
			Serial.println("Case 2 called");
		}
		break;
	}
	}
}
void close() {
	if (flag_isOpenned) {
		if (!flag_manuallyOpened) {
			moveActuator(2);
			if (flag_debug_mode) {
				Serial.println("Closing the lid");
			}
			flag_isOpenned = 0;
		}
	}
	checkTrash();
	if (flag_debug_mode) {
		Serial.println("checkTrash() called");
	}
}
void open() {
	if (!flag_isFull) {
		if (!flag_isOpenned) {
			if (!flag_manuallyOpened) {
				moveActuator(1);
				myDFPlayer.play(2);
				if (flag_debug_mode) {
					Serial.println("Open the lid");
				}
			}

		}

		if (flag_debug_mode) {
			Serial.println("Playing sound");
		}
		flag_isOpenned = 1;
	} else {
		myDFPlayer.play(1);
	}
	delay(10000);
	if (flag_debug_mode) {
		Serial.println("calling close()");
	}
	close();

}

void checkHand() {
	int distance = sonarHand.ping_cm(400);
	if (flag_debug_mode) {
		Serial.print("Sonar hand measurement in cm ");
		Serial.println(distance);
	}
	if (distance <= 15 && distance != 0) {
		if (flag_debug_mode) {
			Serial.println("Calling open()");
		}
		open();
	}

}
void resetTrashCan() {
	if (flag_debug_mode) {
		Serial.println("resetTrashCan() is running");
	}
	flag_isFull = 0;
	if (flag_debug_mode) {
		Serial.println("Set flag_isFull to 0");
	}
	moveActuator(2);
}
void checkResetButton() {
	if (flag_debug_mode) {
		Serial.println("checkResetButton() is running");
	}
	debouncerResetButton.update();
	if (flag_debug_mode) {
		Serial.println("debouncerResetButton updated");
	}
	int ResetButtonvalue = debouncerResetButton.read();
	if (ResetButtonvalue == LOW) {
		if (flag_debug_mode) {
			Serial.println("Reset the trashcan");
		}
		resetTrashCan();
	}
}

void checkManuallyOpenButton() {
	if (flag_debug_mode) {
		Serial.println("Update debouncerManuallyOpenButton");
	}
	debouncerManuallyOpenButton.update();
	int ManuallyOpenButtonvalue = debouncerManuallyOpenButton.read();
	if (flag_debug_mode) {
		Serial.print("ManuallyOpenButton Value is: ");
		Serial.println(ManuallyOpenButtonvalue);
	}
	if (ManuallyOpenButtonvalue == LOW) {
		if (flag_manuallyOpened == 0) {
			moveActuator(1);
			flag_manuallyOpened = 1;
			delay(100);
		} else {
			moveActuator(2);
			flag_manuallyOpened = 0;
			delay(100);
		}
	}
}

void setup() {
	Serial.begin(115200);
	Serial1.begin(9600);
	Serial2.begin(57600);

	pinMode(TRIG_PIN_HAND, OUTPUT);
	pinMode(ECHO_PIN_HAND, INPUT);
	pinMode(TRIG_PIN_TRASH, OUTPUT);
	pinMode(ECHO_PIN_TRASH, INPUT);
	pinMode(HOME_SWITCH_PIN, INPUT);
	pinMode(ACTUATOR_PIN_1, OUTPUT);
	pinMode(ACTUATOR_PIN_2, OUTPUT);
	pinMode(RESET_BUTTON, INPUT_PULLUP);
	pinMode(MANUALLY_OPEN_BUTTON, INPUT_PULLUP);
	pinMode(DEBUG_MODE_PIN, INPUT_PULLUP);

	debouncerResetButton.attach(RESET_BUTTON);
	debouncerManuallyOpenButton.attach(MANUALLY_OPEN_BUTTON);
	debouncerResetButton.interval(10);
	debouncerManuallyOpenButton.interval(10);

	if (!myDFPlayer.begin(Serial1)) {
		Serial.println(F("Unable to begin:"));
		Serial.println(F("1.Please rechecks the connection!"));
		Serial.println(F("2.Please insert the SD card!"));
	} else {
		Serial.println(F("DFPlayer Mini online."));
	}
	myDFPlayer.volume(10);

	int a = digitalRead(DEBUG_MODE_PIN);
	if (a == 0) {
		flag_debug_mode = 1;
		Serial.println("Activated debug mode");
	}
}
void loop() {
	checkHand();
	checkManuallyOpenButton();
	checkResetButton();

	if (flag_debug_mode) {
		delay(1000);
	}
	if (flag_isFull == 1) {
		digitalWrite(LOCK_LED, HIGH);
	} else {
		digitalWrite(LOCK_LED, LOW);
	}
}

