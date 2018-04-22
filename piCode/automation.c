#include "stdio.h"
#include <stdlib.h>
#include <math.h>
#include <wiringPi.h> //library used to configure GPIO pins
#include <softPwm.h>  //library used to configure GPIO pins
#include <unistd.h>

#include "setup.h"

/* README

Intall the following libraries (if using the Rasberry Pi):  
    
    1) download the wiringPi library by following the instructions on this website: 
    *   http://wiringpi.com/download-and-install/
		
	-----------------------------------------------------------------------------------------------------------
	 
    To obtain the reference pin of the wiringpi library insert "gpio readall" in the command terminal 
	
*/

char coordinates[9] = {'0', '0', '0', '0', '0', '0', '0', '0', '\0'}; //Coordinates array
char positioning[9] = {'1', '1', '1', '1', '1', '1', '1', '1', '\0'}; //Positioning array
int stageACompleted = 0;
int stageBCompleted = 0; // Stores which stages have been successfully completed
int stageCCompleted = 0;
int plankCrossed = 0;

int first20AtoC = 0;	 // will store total time available to press button A before automatically changing to button C
int first45secs = 0;	 // Variable that will store the length of time that will change button A to C once the start button is pressed (45 secs)

int encoderPosition = 0; // Keeps track of the encoder position
int lastDBCheck = 0;	 // Last time interuption for the rotary encoder was excecuted. Used to prevent bouncing
int lastResetCheck = 0;  // Last time int for reset

int flagPoints = 0;		 // Keeps track of the point for rising the flag (rotary encoder)
int startMatch = 0;		 // When set to 1 the match starts and starts sending the IR messegae
int resetMatch = 0;		 // When set to 1 the match will automatically stop and begin a new game to positioning phase
int startedCounting = 0; // Flag to determine wether the timer for the match has started
float points = 0;		 // Keeps track of all of the points of the game

void sendIR(char coordinates[]){

	digitalWrite(POSITIONING, LOW);

	if(coordinates[5]=='1'){
		digitalWrite(IRLed2, HIGH);
	}else{
		digitalWrite(IRLed2, LOW);
	}

	if(coordinates[6]=='1'){
		digitalWrite(IRLed1, HIGH);
	}else{
		digitalWrite(IRLed1, LOW);
	}

	if(coordinates[7]=='1'){
		digitalWrite(IRLed0, HIGH);
	}else{
		digitalWrite(IRLed0, LOW);
	}
}

void sendPositioningCode(){
	digitalWrite(POSITIONING, HIGH);
}

void shutdownIR(){
	digitalWrite(SEND_CODE, LOW); 
}

// Function takes care of lighting the RGB LED depending on the current position of the rotary encoder
void lightRGB(){
	float turns = (float)encoderPosition / 24.0;
	printf("Turns: %f\n", turns);
	if (turns >= 2.75 && turns <= 4.74)
	{
		if(flagPoints!=100){
			sendData(3, ENCODER);
		}
		softPwmWrite(RCLedRed, 0);
		softPwmWrite(RCLedBlue, 100);
		softPwmWrite(RCLedGreen, 0);
		flagPoints = 100;
		printf("Yellow\n");
		
	}
	else if (turns > 4.74 && turns <= 5.24)
	{
		if(flagPoints!=200){
			sendData(2, ENCODER);
		}
		softPwmWrite(RCLedRed, 100);
		softPwmWrite(RCLedBlue, 100);
		softPwmWrite(RCLedGreen, 0);
		flagPoints = 200;
		printf("Green\n");
	}
	else
	{
		if(flagPoints!=0){
			sendData(1, ENCODER);
		}
		softPwmWrite(RCLedRed, 0);
		softPwmWrite(RCLedBlue, 100);
		softPwmWrite(RCLedGreen, 100);
		flagPoints = 0;
		printf("Red\n");
	}
}

void turnONLeds(){
	digitalWrite(stageASLed, HIGH);
	digitalWrite(stageANLed, HIGH);
	digitalWrite(stageBSLed, HIGH);
	digitalWrite(stageBNLed, HIGH);
	digitalWrite(stageCSLed, HIGH);
	digitalWrite(stageCNLed, HIGH);
}

void turnOFFLeds(){
	digitalWrite(stageASLed, LOW);
	digitalWrite(stageANLed, LOW);
	digitalWrite(stageBSLed, LOW);
	digitalWrite(stageBNLed, LOW);
	digitalWrite(stageCSLed, LOW);
	digitalWrite(stageCNLed, LOW);
}

// Interruption script activated whenever the rotary encoder is rotated (The max is 126 which)
void interruptRotaryA(void){
	int a = digitalRead(RCPhaseA);
	int b = digitalRead(RCPhaseB);

	if (a == 0)
	{
		return;
	}

	if (micros() > lastDBCheck + 500 && startMatch == 1)
	{
		if (a != b)
		{
			encoderPosition++;
			if (encoderPosition >= 126)
			{
				encoderPosition = 0;
			}
			lightRGB();
		}
		lastDBCheck = micros();
	}
}

/* Start button pressed*/
void didPressStart(void){

	if(lastResetCheck +1000 > millis()){
		return;
	}

	int buttonVal = digitalRead(startResetButton);
	lastResetCheck = millis();

	//If the match hasn't begun start
	if (startMatch == 0)
	{
		digitalWrite(Lasers, HIGH);
		delay(100);
		startMatch = 1; //If button0 (pin:24) is pressed, the match will begin
	}
	else
	{
		resetMatch = 1;
		startMatch = 0;
		sendData(MANUAL, RESET);
	}
	
}

// Interrupt excecuted whenever any light sensor for the water reports a change.
void waterCrossed(void){
	int WPHNsensorVal = digitalRead(WPHNsensor);
	int WPHSsensorVal = digitalRead(WPHSsensor);
	int WPHsensorVal = digitalRead(WPHNsensor);

	if (startMatch == 1)
	{
		//if(WPHNsensorVal == 1 || WPHSsensorVal == 1 || WPHsensorVal == 1){
			resetMatch = 1;
			startMatch = 0;
			sendData(WATER, RESET);
		//}
	}
}

void didPressStageACN(void){
	if(digitalRead(stageACSButton)){
		turnOFFLeds();
		system("reboot");
	}
	if (startMatch)
	{
		int time = millis();
		if (coordinates[5] == '0' && stageACompleted == 0 && time < first45secs && stageBCompleted == 0)
		{
			stageACompleted = 1;
			points += 50;
			digitalWrite(stageANLed, HIGH);
			sendData(STAGE_A, STAGES);
			first20AtoC = millis() + 20 * 1000;
		}
		//Button C is enabled if the proper coordinates is pressed and stage C is not completed and
		//either stage A is completed and 20 secs have passed or 45 total seconds have passed since start button have been pressed
		//or stage B is completed
		if (coordinates[7] == '0' && stageCCompleted == 0 &&
			((stageACompleted == 1 && time >= first20AtoC) || time >= first45secs || stageBCompleted == 1))
		{
			stageCCompleted = 1;
			points += 100;
			digitalWrite(stageCNLed, HIGH);
			sendData(STAGE_C, STAGES);
			resetMatch = 1;
		}

		// Same but wrong C
		if (coordinates[7] == '1' && stageCCompleted == 0 &&
			((stageACompleted == 1 && time >= first20AtoC) || time >= first45secs || stageBCompleted == 1))
		{
			resetMatch = 1;
		}
	}
}

void didPressStageACS(void){
	if(digitalRead(stageACNButton)){
		turnOFFLeds();
		system("reboot");
	}
	if (startMatch)
	{
		int time = millis();
		if (coordinates[5] == '1' && stageACompleted == 0 && time < first45secs && stageBCompleted == 0)
		{
			stageACompleted = 1;
			points += 50;
			digitalWrite(stageASLed, HIGH);
			sendData(STAGE_A, STAGES);
			first20AtoC = millis() + 20 * 1000;
		}
		//Button C is enabled if the proper coordinates is pressed and stage C is not completed and
		//either stage A is completed and 20 secs have passed or 45 total seconds have passed since start button have been pressed
		//or stage B is completed
		if (coordinates[7] == '1' && stageCCompleted == 0 &&
			((stageACompleted == 1 && time >= first20AtoC) || time >= first45secs || stageBCompleted == 1))
		{
			stageCCompleted = 1;
			points += 100;
			digitalWrite(stageCSLed, HIGH);
			sendData(STAGE_C, STAGES);
			sendData(STAGES_COMPLETED, RESET);
			resetMatch = 1;
		}
		
		// Same but wrong C
		if (coordinates[7] == '0' && stageCCompleted == 0 &&
			((stageACompleted == 1 && time >= first20AtoC) || time >= first45secs || stageBCompleted == 1))
		{
			sendData(STAGES_COMPLETED, RESET);
			resetMatch = 1;
		}
	}
}

void didPressStageBN(void){
	if (startMatch)
	{
		if (coordinates[6] == '0' && stageBCompleted == 0)
		{
			stageBCompleted = 1;
			points += 200;
			digitalWrite(stageBNLed, HIGH);
			sendData(STAGE_B, STAGES);
		}
	}
}

void didPressStageBS(void){
	if (startMatch)
	{
		if (coordinates[6] == '1' && stageBCompleted == 0)
		{
			stageBCompleted = 1;
			points += 200;
			digitalWrite(stageBSLed, HIGH);
			sendData(STAGE_B, STAGES);
		}
	}
}

void setup(){
	delay(15000);
	time_t t;		 //argument used to allow random generation of coordinates
	wiringPiSetup(); //Set interruption for restart button
	pinMode(startResetButton, INPUT);
	pinMode(stageACNButton, INPUT);
	pinMode(stageACSButton, INPUT);
	pinMode(stageBNButton, INPUT);
	pinMode(stageBSButton, INPUT);
	pinMode(stageANLed, OUTPUT);
	pinMode(stageASLed, OUTPUT);
	pinMode(stageBNLed, OUTPUT);
	pinMode(stageBSLed, OUTPUT);
	pinMode(stageCNLed, OUTPUT);
	pinMode(stageCSLed, OUTPUT);
	pinMode(WPHNsensor, INPUT);
	pinMode(WPHSsensor, INPUT);
	pinMode(WPHsensor, INPUT);
	softPwmCreate(RCLedBlue, 100, 100);
	softPwmCreate(RCLedGreen, 100, 100);
	softPwmCreate(RCLedRed, 0, 100);
	pinMode(RCPhaseA, INPUT);
	pinMode(RCPhaseB, INPUT);
	pinMode(RCPhaseC, OUTPUT);
	pinMode(Lasers, OUTPUT);

	pinMode(IRLed0, OUTPUT);
	pinMode(IRLed1, OUTPUT);
	pinMode(IRLed2, OUTPUT);
	pinMode(POSITIONING, OUTPUT);
	pinMode(SEND_CODE, OUTPUT);


	digitalWrite(RCPhaseC, LOW);

	pullUpDnControl(RCPhaseA, PUD_UP); // pullup internal resistor
	pullUpDnControl(RCPhaseB, PUD_UP); // pullup internal resistor

	wiringPiISR(startResetButton, INT_EDGE_BOTH, &didPressStart); //set up inturruption for start button
	wiringPiISR(stageACNButton, INT_EDGE_BOTH, &didPressStageACN);
	wiringPiISR(stageACSButton, INT_EDGE_BOTH, &didPressStageACS);
	wiringPiISR(stageBNButton, INT_EDGE_BOTH, &didPressStageBN);
	wiringPiISR(stageBSButton, INT_EDGE_BOTH, &didPressStageBS);

	wiringPiISR(WPHNsensor, INT_EDGE_BOTH, &waterCrossed);
	wiringPiISR(WPHSsensor, INT_EDGE_BOTH, &waterCrossed);
	wiringPiISR(WPHsensor, INT_EDGE_BOTH, &waterCrossed);

	wiringPiISR(RCPhaseA, INT_EDGE_RISING, &interruptRotaryA);

	digitalWrite(RCLedGreen, HIGH);
	srand((unsigned)time(NULL)); //allows use of rand()
	setupComm();
}

void blink(int r){
	int i = 0;
	for (i = 0; i < r; i++)
	{
		turnONLeds();
		delay(300);
		turnOFFLeds();
		delay(300);
	}
}

/* Start a new game*/
void game(){
	startMatch = 0;
	resetMatch = 0;
	points = 0;
	encoderPosition = 0;
	stageACompleted = 0;
	stageBCompleted = 0; // Stores which stages have been successfully completed
	stageCCompleted = 0;
	int startedCounting = 0;
	int i = 0;
	int finish, first30secs, time;
	int bit;

	turnONLeds();
	digitalWrite(Lasers, LOW);

	// Turn Rotarty encoder LED red
	softPwmWrite(RCLedRed, 0);
	softPwmWrite(RCLedBlue, 100);
	softPwmWrite(RCLedGreen, 100);

	for (i = 0; i < 3; i++) //randomizing the bits for the coordinates
	{
		bit = rand() % 2;			   //outputs either 1's or 0's
		coordinates[i + 5] = bit + 48; //manipulates and interprets the bit 5-7 interger value in the array into ASCII
	}

	digitalWrite(SEND_CODE, HIGH);		//Enable IR

	while (!resetMatch)
	{ 
		if (startMatch == 0)
		{
			sendPositioningCode();
		}
		else
		{
			time = millis(); // function from wiring pi that returns a number representing the number milliseconds since any of the wiringPiSetup functions were called

			// send code only for 30 sec
			if (time <= first30secs){
				sendIR(coordinates); //send coordinates code
			}else{
				shutdownIR();
			}

			if (!startedCounting)
			{
				turnOFFLeds();
				finish = (240 * 1000) + time;	 // start the timer for the 4 min (total time of the match)
				first30secs = (30 * 1000) + time; // 30 seconds limit for sending the IR coordinates code (once start is prssed)
				first45secs = (45 * 1000) + time; // 45 seconds limit once the start button is pressed (Time before pressing stage C skiping B)
				startedCounting = 1;
				sendData(MATCH_START, RESET);	  //Inform the server that the match has begun
				sendStringData(coordinates, COORDINATES); // Send the game coordinates to the server
				printf("%s\n", coordinates);
			}
			else
			{

				int currentTime = -(finish - time) + (240 * 1000); //Calculate the current time in ms starting from 0
				sendData(currentTime, TIME);				   //Send the time data to the server
				if (time >= finish)								   //if time is over, end match
				{
					sendData(MATCH_END, RESET);
					break;
				}
			}
		}
		delay(1000);
	}
	/*Count points*/

	float timePoints = (float)(finish - time) / 1000.0; //  Calculate point for time remaining

	//Prevent negative time points
	if (timePoints < 0)
	{
		timePoints = 0;
	}

	points += timePoints;
	points += flagPoints;
	printf("points: %f \n\n", points);
	sendData(points, SCORE);

	if(stageCCompleted){
		delay(5000);
	}
	blink(10); //Visual indicator that the match is over
	game(); //new game started
}

//Main thread function
int main(int argc, char *argv[]){
	setup();
	game();
	return 0;
}