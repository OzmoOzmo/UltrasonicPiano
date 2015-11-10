#define HAND_MUST_MOVE_DISTANCE 200	//in uS ping time in we take as something near
#define MAXECHOTIME 1000	// Calculate the maximum distance in uS (no rounding).
#define DEBOUNCEmS 500		// millsec before can play a second note on that key
#define MIN_DISTANCE 2		// below this time in uS means either too close - or nothing near 

#define TRIGPIN 13			// All sonics connected to one pin

#define MAX_SENSOR_DELAY 500   		// Maximum uS it takes for sensor to start the ping

#define SONAR_NUM     9		// Number of detectors
#define SOUND_BANK_PIN  12  // Button to switch sound banks


//The Pins the sonar is attached to
int sonar[SONAR_NUM]=
{
	2, 3, 4, 5, 6, 7, 8, 9, 10
};

int result[SONAR_NUM];
int previousHits[SONAR_NUM];
long lasttime[SONAR_NUM];
int soundBank = LOW;		//Two sound bank choices - guitar and piano

//9 samples (must be wav unless you have installed the optional mp3 libraries on the edison)
//Some free samples can be got here: https://www.freesound.org/people/pinkyfinger/packs/4409/
//Wav files need be placed into the folders on the edison (use FTP) as indicated below.
//The trailing & is important
char* sounds[]=
{
	"aplay /home/root/piano/00_piano-g.wav &",
	"aplay /home/root/piano/01_piano-a.wav &",
	//"aplay /home/root/piano/02_piano-bb.wav &",
	"aplay /home/root/piano/03_piano-b.wav &",
	//"aplay /home/root/piano/04_piano-cc.wav &",
	"aplay /home/root/piano/05_piano-c.wav &",
	"aplay /home/root/piano/06_piano-d.wav &",
	//"aplay /home/root/piano/07_piano-eb.wav &",
	"aplay /home/root/piano/08_piano-e.wav &",
	"aplay /home/root/piano/09_piano-f.wav &",
	"aplay /home/root/piano/10_piano-f.wav &",
	"aplay /home/root/piano/11_piano-g.wav &"
};

//9 alternative samples
char* sounds2[]=
{
	"aplay /home/root/guitar/1.e.wav &",
	"aplay /home/root/guitar/2.f.wav &",
	"aplay /home/root/guitar/3.g.wav &",
	"aplay /home/root/guitar/4.a.wav &",
	"aplay /home/root/guitar/5.b.wav &",
	"aplay /home/root/guitar/6.d.wav &",
	"aplay /home/root/guitar/7.e2.wav &",
	"aplay /home/root/guitar/8.f2.wav &",
	"aplay /home/root/guitar/9.b.wav &"
};

void setup() {
	Serial.begin(115200);
	Serial.println("Start");
	
	for(int n=0;n<SONAR_NUM;n++)
	{//Out of time starting
		int ECHOPIN = sonar[n];
		pinMode(ECHOPIN, INPUT_FAST);
	}
	pinMode(TRIGPIN, OUTPUT_FAST);
	pinMode(12,INPUT_PULLUP);
}

void ping()
{
	for(int n=0;n<SONAR_NUM;n++)
		result[n] = 0;  //0 for no result
	
	digitalWrite(TRIGPIN, LOW);
	delayMicroseconds(2);			// Wait for pin to go low. (try 4 if having problems)
	digitalWrite(TRIGPIN, HIGH);	// Set trigger pin high, this tells the sensor to send out a ping.
	delayMicroseconds(10);			// Wait long enough for the sensor to realize the trigger pin is high. Sensor specs say to wait 10uS.
	digitalWrite(TRIGPIN,LOW);		// Set trigger pin back to low.
	
	long timeoutTime;
	
	// wait for ping to start
#ifdef true
	//Easy way
	delayMicroseconds(500);		//most detectors take about 450uS to 500uS SRF06 can take much much longer.
#else
	//hard way
	//long st = micros();
	timeoutTime = micros() + MAXECHOTIME  + MAX_SENSOR_DELAY; 
			
	for(int n=0;n<SONAR_NUM;n++)
	{
		int ECHOPIN = sonar[n];
		// Previous ping hasn't finished, abort.
		if(digitalRead(ECHOPIN) == HIGH)
			result[n] = -1; //"No Reply"
	}

	for(int n=0;n<SONAR_NUM;n++)
	{
		if (result[n]==0)
		{
			int ECHOPIN = sonar[n];
			// Wait for ping to start.
			while(digitalRead(ECHOPIN) == LOW)
				if (micros() > timeoutTime)
				{
					result[n] = -2;		//"Too Long To Start"
					break;
				}
		}
	}
	//Serial.println(micros()-st);
#endif	
	
	long startTime = micros();
	timeoutTime = startTime + MAXECHOTIME ; // Ping started, set the time-out.
	
	boolean bExit;
	do
	{
		bExit=true;
		for(int n=0;n<SONAR_NUM;n++)
		{
			if (result[n]==0)
			{
				int ECHOPIN = sonar[n];
				if(digitalRead(ECHOPIN) == LOW)  // ping end echo
					result[n] =	(int)((micros() - startTime)+.05); // Dont bother calc distance - ping time is enough for us.

				else if (micros() > timeoutTime)
					result[n] = -3;  // record error if we're beyond the set maximum distance.
				
				bExit=false;
			}
		}
	}while(bExit == false);
}

void loop() 
{
	ping();

	//All done - display
	for(int n=0;n<SONAR_NUM;n++)
	{
		float dist = result[n]; //in time
		//Serial.print(dist);Serial.print(',');
		
		if (dist <= MIN_DISTANCE)
		{//person is too close or has gone away
			previousHits[n]=dist;  //reset
			continue;
		}
		
		int previous = previousHits[n];
		if (abs(dist - previous) > HAND_MUST_MOVE_DISTANCE)
		{//New key pressed!
			previousHits[n]=dist;
			
			long now = millis();
			if ((now - lasttime[n]) > DEBOUNCEmS) //debounce 333mS
			{
				char* sound = soundBank == LOW? sounds[n]:sounds2[n];
				Serial.println(sound);
				system(sound);
			}
			lasttime[n] = now;
		}
	}
	//Serial.println('.');
	
	if (digitalRead(12) == LOW)
	{//Switch to second band - have settle time.
		soundBank = !soundBank;
		Serial.print(soundBank);
		delay(1000);//debounce
	}
	
	delay(20);
}

 
  
