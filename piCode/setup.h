

// These values are for the Wiring Pi library reference, they correspond to the following pins:
#define startResetButton 6 	    //Pin 22
#define stageACNButton 16		//Pin 10
#define stageACSButton 0		//Pin 11
#define stageBNButton 1			//Pin 12
#define stageBSButton 2 		//Pin 13

#define stageANLed 3 			//Pin 15
#define stageASLed 12			//Pin 19
#define stageBNLed 13 			//Pin 21
#define stageBSLed 14			//Pin 23 
#define stageCNLed 10			//Pin 24
#define stageCSLed 26			//Pin 32

#define WPHNsensor 8			//Pin 3
#define WPHSsensor 9			//Pin 5
#define WPHsensor 15			//Pin 8

#define Lasers 21			    //Pin 29

#define IRLed0 4			    //Pin 16
#define IRLed1 5			    //Pin 18
#define IRLed2 11			    //Pin 26
#define POSITIONING 28			//Pin 38
#define SEND_CODE 7			    //Pin 7


#define STAGE_A 1
#define STAGE_B 2
#define STAGE_C 3

#define TIME 1
#define COORDINATES 2
#define STAGES 3
#define ENCODER 4
#define PLANK 5
#define SCORE 6
#define RESET 7

//Reasons to reset:
#define MATCH_START 1
#define MATCH_END 2
#define WATER 3
#define MANUAL 4
#define STAGES_COMPLETED 5


//Rotary Encoder
#define RCLedRed 27    			//Pin 36
#define RCLedGreen 25		    //Pin 37
#define RCLedBlue 29		    //Pin 40

#define RCPhaseA 24		    	//Pin 35
#define RCPhaseB 22			    //Pin 31
#define RCPhaseC 23             //Pin 33

void setupComm();

void sendStringData(char *, int);  

void sendData(float, int);  
