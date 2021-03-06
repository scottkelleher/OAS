   

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//To Do
//*Add file names
//
//
//
//
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////INA219/////////////////
#include "adafruit-ina219/adafruit-ina219.h"
Adafruit_INA219 ina219;
/////////////////////////////////////////////////////
////////////////////////INA219//////////////////

/////////////////////////////////////////
//////////////Fuel gauge/////////////////////
#include "SparkFunMAX17043/SparkFunMAX17043.h"
double battery_volts = 0; // Variable to keep track of LiPo voltage
double soc = 0; // Variable to keep track of LiPo state-of-charge (SOC)
bool alert; // Variable to keep track of whether alert has been triggered
////////////////////////////////////////////////
////////////////fuel gauge//////////////////////


/////////////////////////////////////////////
////////////////////SD///////////////////////{
/////////////////////////////////////////////
#include "SdFat/SdFat.h"
#define SPI_CONFIGURATION 0
//------------------------------------------------------------------------------
// Setup SPI configuration.
#if SPI_CONFIGURATION == 0
// Primary SPI with DMA
// SCK => A3, MISO => A4, MOSI => A5, SS => A2 (default)
SdFat sd;
const uint8_t chipSelect = SS;
#elif SPI_CONFIGURATION == 1
// Secondary SPI with DMA
// SCK => D4, MISO => D3, MOSI => D2, SS => D1
SdFat sd(1);
const uint8_t chipSelect = D1;
#elif SPI_CONFIGURATION == 2
// Primary SPI with Arduino SPI library style byte I/O.
// SCK => A3, MISO => A4, MOSI => A5, SS => A2 (default)
SdFatLibSpi sd;
const uint8_t chipSelect = SS;
#elif SPI_CONFIGURATION == 3
// Software SPI.  Use any digital pins.
// MISO => D5, MOSI => D6, SCK => D7, SS => D0
SdFatSoftSpi<D5, D6, D7> sd;
const uint8_t chipSelect = D0;
#endif  // SPI_CONFIGURATION

File myFile;
/////////////////////////////////////////////
////////////////////SD///////////////////////}
/////////////////////////////////////////////

/////////////////////////////////////////////
////////////////////BME//////////////////////{
/////////////////////////////////////////////
#include "SparkFunBME280/SparkFunBME280.h"

BME280 mySensor;

unsigned int sampleNumber = 0; //For counting number of CSV rows
/////////////////////////////////////////////
////////////////////BME//////////////////////}
/////////////////////////////////////////////

/////////////////////////////////////////////
////////////////////ads//////////////////////{
/////////////////////////////////////////////
#include "Adafruit_ADS1X15/Adafruit_ADS1X15.h"

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
double multiplier;
/////////////////////////////////////////////
////////////////////ads//////////////////////}
/////////////////////////////////////////////

/////////////////////////////////////////////
/////////////////Variabels///////////////////{
/////////////////////////////////////////////
unsigned long previousMillis = 0;        // will store last time LED was updated

const int nAverage = 15;        //How many readings should be averaged 0<X<512
float sharp_readings[nAverage]; //
int reading_index=0; 
float sharp_a=0;

float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;

FuelGauge fuel;
/////////////////////////////////////////////
/////////////////Variables///////////////////}
/////////////////////////////////////////////




SYSTEM_MODE(AUTOMATIC);
//Particle.connect();       //This will conect to the cloud
//Particle.process();       //This must then be called at least every 20s to maintain the conection

void setup() {
    Serial.begin(115200);
    //while (!Serial) {     //wait for the Seial com port to be opened before running anything
    //    SysCall::yield();
    //}
    Time.zone(-6);
   Time.format(Time.now(), "%H:%M:%S");
    
    
    
    pinMode(D7, OUTPUT);
    digitalWrite(D7, HIGH);
    pinMode(D5, OUTPUT);
    digitalWrite(D5, HIGH);
    pinMode(D6, OUTPUT);
    digitalWrite(D6, HIGH);
    pinMode(D4, OUTPUT);
    digitalWrite(D4, LOW);
    
    /////////////////////////////////////////////
    ////////////////////SD///////////////////////{
    /////////////////////////////////////////////
    // Initialize SdFat or print a detailed error message and halt
    // Use half speed like the native library.
    // Change to SPI_FULL_SPEED for more performance.
    if (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
        sd.initErrorHalt();
    }
    if (!myFile.open("AOS_Test.txt", O_RDWR | O_CREAT | O_AT_END)) {
        sd.errorHalt("opening test.txt for write failed");
    }
    
    myFile.close();
    /////////////////////////////////////////////
    ////////////////////SD///////////////////////}
    /////////////////////////////////////////////
    
    /////////////////////////////////////////////
    ////////////////////BME//////////////////////{
    /////////////////////////////////////////////
    //***Driver settings********************************//
	//commInterface can be I2C_MODE or SPI_MODE
	//specify chipSelectPin using arduino pin names
	//specify I2C address.  Can be 0x77(default) or 0x76

	//For I2C, enable the following and disable the SPI section
	mySensor.settings.commInterface = I2C_MODE;
	mySensor.settings.I2CAddress = 0x76;

	//For SPI enable the following and dissable the I2C section0
	//mySensor.settings.commInterface = SPI_MODE;
	//mySensor.settings.chipSelectPin = 10;


	//***Operation settings*****************************//
	mySensor.settings.runMode = 3; //  3, Normal mode
	mySensor.settings.tStandby = 0; //  0, 0.5ms
	mySensor.settings.filter = 0; //  0, filter off
	//tempOverSample can be:
	//  0, skipped
	//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
	mySensor.settings.tempOverSample = 1;
	//pressOverSample can be:
	//  0, skipped
	//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
    mySensor.settings.pressOverSample = 1;
	//humidOverSample can be:
	//  0, skipped
	//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
	mySensor.settings.humidOverSample = 1;
	
	mySensor.begin();
    /////////////////////////////////////////////
    ////////////////////BME//////////////////////}
    /////////////////////////////////////////////
    
    /////////////////////////////////////////////
    ////////////////////ads//////////////////////{
    /////////////////////////////////////////////
    //ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit =       3mV       0.1875mV (DEFAULT)
    //multiplier =  0.1875F;
    ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit =     2mV       0.125mV
    multiplier =  0.125F;
    //ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit =     1mV       0.0625mV
    //multiplier =  0.0625F;
    //ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit =     0.5mV     0.03125mV
    //multiplier =  0.03125F;
    //ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit =     0.25mV    0.015625mV
    //multiplier =  0.015625F;
    //ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit =     0.125mV   0.0078125mV  
    //multiplier =  0.0078125F;
    ads.begin();
    /////////////////////////////////////////////
    ////////////////////ads//////////////////////}
    /////////////////////////////////////////////
    
    
    ///////////////fuel gauge/////////////////


lipo.begin(); // Initialize the MAX17043 LiPo fuel gauge

	// Quick start restarts the MAX17043 in hopes of getting a more accurate
	// guess for the SOC.
	lipo.quickStart();

	// We can set an interrupt to alert when the battery SoC gets too low.
	// We can alert at anywhere between 1% - 32%:
//	lipo.setThreshold(20); // Set alert threshold to 20%.
    ////////////// fuel gauge //////////////////////////
    
    
    
    //////////////////////////////////////////////////
    ////////////////////ads115////////////////////////////
    //uint32_t currentFrequency;
    ina219.begin();
    /////////////////////////////////////////////////
    /////////////////ads115///////////////////////
    
}

void log_data() {
    double sharp0, sharp1, sharp2, sharp3;
    sharp0 = read_sharp(0);
    double particle_analogsignal;
    particle_analogsignal=voltageRaw();

    float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;

   float sharp_a = read_sharp_averaged();
   
   
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  

    
    //sharp1 = read_sharp(1);
    //sharp2 = read_sharp(2);
    //sharp3 = read_sharp(3);.
    
    float tempC, press, altM, RH;
    tempC = mySensor.readTempC();
    press = mySensor.readFloatPressure();
    altM  = mySensor.readFloatAltitudeMeters();
    RH    = mySensor.readFloatHumidity();
    
    if (!myFile.open("AOS_Test.txt", O_RDWR | O_AT_END)) {
        sd.errorHalt("opening test.txt for write failed");
    }
    
    myFile.print(tempC, 3); myFile.print(",");
    myFile.print(press, 1); myFile.print(",");
    myFile.print(altM,  3); myFile.print(",");
    myFile.print(RH,    3); myFile.print(",");
    //myFile.print(sharp0,3); myFile.print(",");
    myFile.print(particle_analogsignal,3);myFile.print(",");
    //myFile.print(sharp1,3); myFile.print(",");
   // myFile.print(sharp2,3); myFile.print(",");
    //myFile.print(sharp3,3); myFile.println(",");
    myFile.print(sharp_a,1); myFile.print(",");
    myFile.print(String::format( "%02.2f %03.2f", fuel.getVCell(), fuel.getSoC()));
    myFile.print(busvoltage,2); myFile.print(",");
    myFile.print(shuntvoltage,2); myFile.print(",");
    myFile.print(loadvoltage,2); myFile.print(",");
    myFile.print(current_mA,3); myFile.print(",");
   
    myFile.print(String::format("%d-", Time.year()));
    myFile.print(String::format("%d-", Time.month()));
    myFile.print(String::format("%d,", Time.day()));
    myFile.print(String::format("%d:", Time.hour()));
    myFile.print(String::format("%d:", Time.minute()));
     myFile.println(String::format("%d", Time.second()));
   // myFile.print(battery_volts,3);myFile.print(",");
   // myFile.println(soc,3);
    myFile.close();
    
    
}


float read_sharp(int num){
    digitalWrite(D6, 0);
    delayMicroseconds(280);
    float voltage = multiplier * ads.readADC_SingleEnded(0);
    delayMicroseconds(40);
    digitalWrite(D6, 1);
    // Add transfer funtion here
    //delay(12);
    return voltage;
}  
    
float voltageRaw (){
    digitalWrite(D7, 0);
    delayMicroseconds(280);
    int reading = analogRead(0);
    delayMicroseconds(40);
    digitalWrite(D7, 1);
    // Add transfer funtion here
    //delay(12);
    return reading;
}

float read_sharp_averaged(){
    reading_index++;
    if(reading_index >= nAverage){
        reading_index = 0;
    }
    sharp_readings[reading_index] = analogRead(0);
    //Serial.print(String::format("Index:%d Value:%f,", reading_index, sharp_readings[reading_index]));
    float sum  = 0;
    for(int i = 0; i < nAverage; i++){
        sum = sum + sharp_readings[i];
    }
    return sum/nAverage;



}



    
    
void loop() {
    const long interval = 2000;           // interval at which to blink (milliseconds)
    
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;   
        log_data();
        
        
        float raw=voltageRaw();
        //Average();
       ;
        float tempC = mySensor.readTempC();
        float sharp_a = read_sharp_averaged();
        float sharp0=read_sharp(0);
        Serial.print(String::format("%f, %f,", tempC, sharp_a));
      //Serial.print(String::format("%f,", sharp0));
      //Serial.print(Time.timeStr());
       Serial.print(String::format("%d-", Time.year()));
       Serial.print(String::format("%d-", Time.month()));
       Serial.print(String::format("%d,", Time.day()));
       Serial.print(String::format("%d:", Time.hour()));
       Serial.print(String::format("%d:", Time.minute()));
       Serial.println(String::format("%d", Time.second()));
       
        FuelGauge fuel;
    Serial.print(String::format( "%02.2f %03.2f", fuel.getVCell(), fuel.getSoC()));
    //Particle.publish("e1", publishStr, 60, PRIVATE);    
    
   float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.println("Current:       "); Serial.print(current_mA); Serial.println(" mA");
 
    
     if(sharp_a>500) {
          digitalWrite(D4,HIGH);
    }
    else {
          digitalWrite(D4, LOW);
    }
       
    }
    
    
   
    
}
    /////////////////////////////////////////////////////////////////////
    
    
    
 
    
   



// lipo.getVoltage() returns a voltage value (e.g. 3.93)
        //battery_volts = lipo.getVoltage();
	// lipo.getSOC() returns the estimated state of charge (e.g. 79%)
        //soc = lipo.getSOC();
	// lipo.getAlert() returns a 0 or 1 (0=alert not triggered)
	//alert = lipo.getAlert();

	// Those variables will update to the Spark Cloud, but we'll also print them
	// locally over serial for debugging:
	/*Serial.print("battery volts: ");
	Serial.print(battery_volts);  // Print the battery voltage
	Serial.println(" V");

	Serial.print("Alert: ");
	Serial.println(alert);

	Serial.print("Percentage: ");
	Serial.print(soc); // Print the battery state of charge
	Serial.println(" %");
	Serial.println();
byte getVbusStat();*/
	
	




