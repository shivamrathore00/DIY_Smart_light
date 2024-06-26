#include <Wire.h>
#include <BH1750.h>
#include <SoftwareSerial.h>
#include <math.h>

BH1750 lightInput;
const int DEFAULT_BRIGHTNESS = 196;  //Default brightness while LED starting
const int DEFAULT_TEMP_INDEX = 2;  //Default functuion to use for processing rgb temperature
float LUX = 0.0;  //Initializing BH1750 input float variable to 0.0

/*Function definitions for controlling brightness*/

int ConstantBrightness(float y){  //For constant brightness output
  return 255.0*y/9;
}

int LogBrightness(float x){  //The brightness function
  if(x>100)
    return 0;
  return int((100.589-19.811*log(x+1.5))*2.55);
}

//
int Temperatures[11][3] = {{200,245,254},{240,255,255},{255,255,255},{255,255,200},{248,255,183},{255,248,167},{255,234,144},{255,218,122},{255,182,78},{255,139,39},{255,89,11}};  //rgb values for specific temperatures 
int (*Brightness_Functions[2])(float) = {LogBrightness, ConstantBrightness};  //array of pointers to brightness functions

class LED{  //class to store the variables and functions corresponding to the operating light
    public:
        int Pins[2] = {3, 5};  //Pins controlling SMD LEDs
        int RGB_Pins[3] = {9, 10, 11};  //Pins controlling the r, g and b values of a RGB LED respectively
        int brightness = DEFAULT_BRIGHTNESS;
        int brightness_function_index = 0;  //Index of the brightness function in use
        int temp_index = DEFAULT_TEMP_INDEX;  //Current operating index of Temperatures array
        int r = Temperatures[temp_index][0]*(brightness/255.0);
        int g = Temperatures[temp_index][1]*(brightness/255.0);
        int b = Temperatures[temp_index][2]*(brightness/255.0);
  
        void glow(){  //Glows the Light at set brightness and temperature
            for (int i = 0; i < 2; i++){
                analogWrite(Pins[i], brightness);
              	delay(50);
			      }
            analogWrite(RGB_Pins[0], r);
        		delay(50);
        		analogWrite(RGB_Pins[1], g);
        		delay(50);
        		analogWrite(RGB_Pins[2], b);
        		delay(50);
        }
  
        void dim(){  //Turns off the Light
            for (int i = 0; i < 2; i++){
                digitalWrite(Pins[i], LOW);
              	delay(50);
            }
            for (int i = 0; i < 3; i++){
                digitalWrite(RGB_Pins[i], LOW);
              	delay(50);
            }
        }
  
        void update(){  //Updates the Light brightness and temperature according to LUX and User input
            brightness = (*Brightness_Functions[0])(LUX/32.0);
            r = Temperatures[temp_index][0]*(brightness/255.0);
            g = Temperatures[temp_index][1]*(brightness/255.0);
            b = Temperatures[temp_index][2]*(brightness/255.0);
            glow();
        }
		
		    void update_with_const_brightness(int level){  //Updates the brightness according to user input
			      brightness_function_index = 1;
		  	    brightness = (*Brightness_Functions[brightness_function_index])(level);
		  	    r = Temperatures[temp_index][0]*(brightness/255.0);
            g = Temperatures[temp_index][1]*(brightness/255.0);
            b = Temperatures[temp_index][2]*(brightness/255.0);
            glow();
	    	}
  
        char Hotter(){  //Increases light temperature
            if(temp_index < 10){
                temp_index++;
            }
            update();
			      return '0';
        }
  
        char Cooler(){  //Decreases light temperature
            if(temp_index > 0){
                temp_index--;
            }
        update();
			  return '0';
        }
};

char data[5] = {'1', '1','0', '0', '7'};  //array to store transmitted information
const int data_size = 5;
LED Light;


void setup(){
    Serial.begin(9600);
    Wire.begin();
    lightInput.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
    pinMode(3, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);

    Light.brightness = DEFAULT_BRIGHTNESS;
    LUX = lightInput.readLightLevel();
    Light.glow();
}

void loop(){
    lightInput.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
    LUX = lightInput.readLightLevel();
    if(Serial.available())
      Serial.readBytes(data, data_size);
    if(data[0]=='1'){
      Light.glow();
      if(data[1]=='0')
        Light.update_with_const_brightness(data[4]-'0');
      if(data[1]=='1')
        Light.update();
      if(data[3]=='1')
        data[3] = Light.Hotter();
      if(data[3]=='2')
        data[3] = Light.Cooler();
    }
    else if(data[0]=='0')
      Light.dim();
    Serial.println(100-Light.brightness/2.55);
    delay(500);
}
