/**
ECE 2036 lab assignment

This is an implementation of a thermostat developed on an mbed.

Created by Varun Malhotra
**/

#include "mbed.h"
#include "TMP36.h"
#include "SDFileSystem.h"
#include "uLCD_4DGL.h"
#include "PinDetect.h"
#include "Speaker.h"
#include "Shiftbrite.h"
#include "RGBled.h"



// use class to setup temperature sensor pins
TMP36 myTMP36(p17);  //Analog in

// use class to setup microSD card filesystem

// use class to setup the  Color LCD
uLCD_4DGL uLCD(p28, p27, p29); // create a global uLCD object

// use class to setup pushbuttons pins
PinDetect pb1(p8);
PinDetect pb2(p7);
PinDetect pb3(p25);

// use class to setup speaker pin
Speaker mySpeaker(p21); //PWM out

// use class to setup Shiftbrite pins
Shiftbrite myShiftbrite(p9, p10, p11, p12, p13);// ei li di n/c ci

// use class to setup Mbed's four on-board LEDs
DigitalOut myLED1(LED1);
DigitalOut myLED2(LED2);
DigitalOut myLED3(LED3);
DigitalOut myLED4(LED4);



//also setting any unused analog input pins to digital outputs reduces A/D noise a bit
//see http://mbed.org/users/chris/notebook/Getting-best-ADC-performance/
DigitalOut P16(p16);
DigitalOut P17(p15);
DigitalOut P18(p18);
DigitalOut P19(p19);
DigitalOut P20(p20);

RGBled myLed (p22, p23, p24);



// Global variables used in callbacks and main program
// C variables in interrupt routines should use volatile keyword
int volatile heat_setting=65; // heat to temp
int volatile cool_setting=68; // cool to temp
int volatile heat_setpoint_night = heat_setting - 5;
int volatile cool_setpoint_night = cool_setting + 5;

volatile int* pTemperature = &heat_setting;

//enum Mode { mode_off = 0, mode_heat = 1, mode_cool = 2 };
//volatile Mode mode = mode_off;
volatile int mode = 0;
enum Statetype { off = 0, Heat_off = 1, Heat_on = 2, Cool_off = 3, Cool_on =4 };
Statetype state = off;


//bool volatile mode=false; // heat or cool mode

// Callback routine is interrupt activated by a debounced pb1 hit
void pb1_hit_callback (void)
{
(*pTemperature)--;
mySpeaker.PlayNote(500.0, .1, .2);

}
// Callback routine is interrupt activated by a debounced pb2 hit
void pb2_hit_callback (void)
{
(*pTemperature)++;
mySpeaker.PlayNote(500.0, .1, .2);
}


// Callback routine is interrupt activated by a debounced pb3 hit
void pb3_hit_callback (void)
{
    mode = (mode + 1)%3;
    mySpeaker.PlayNote(500.0, .1, .2);


}

int main()
{
    float Current_temp=0.0;
    myLed.write(0.0,0.0,0.0);
    set_time(1356729737); // Set RTC time to Wed, 28 Oct 2009 11:35:37



    // Use internal pullups for the three pushbuttons
    pb1.mode(PullUp);
    pb2.mode(PullUp);
    pb3.mode(PullUp);
    // Delay for initial pullup to take effect
    wait(.01);
    // Setup Interrupt callback functions for a pb hit
    pb1.attach_deasserted(&pb1_hit_callback);
    pb2.attach_deasserted(&pb2_hit_callback);
    pb3.attach_deasserted(&pb3_hit_callback);
    // Start sampling pb inputs using interrupts
    pb1.setSampleFrequency();
    pb2.setSampleFrequency();
    pb3.setSampleFrequency();
    // pushbuttons now setup and running






    bool flagOff = 1;
    bool flagOn = 1;
    bool night  = false;
    float temp = myTMP36.read(); //Read temp sensor
    float tempToDisplay = heat_setting;
    uLCD.color(WHITE);

                        
    uLCD.background_color(GREEN);
    uLCD.textbackground_color(GREEN);
    uLCD.cls();
    uLCD.locate(7, 6);
    uLCD.printf("Off");
    myLed.write(0.0,1.0,0.0);
    myLED1 = 0;
    myLED2 = 0;
    

    time_t seconds = time(NULL);


    // State machine code below will need changes and additions
    while (1) {

            

                time_t seconds = time(NULL);
                tm * now = localtime( &seconds);
                now->tm_hour = now->tm_hour;


                    
                switch (state) {
                    
                    case off:

                        switch(mode) {
                        
                        case 0:
                            state = off;



                            break;
                        case 1:
                            state = Heat_off;
                            uLCD.background_color(0x000000);
                            uLCD.textbackground_color(BLACK);
                            uLCD.cls();
                            uLCD.locate(7, 6);
                            uLCD.printf("Heat");
                            myLed.write(0.0,0.0,0.0);
                            myLED1 = 1;
                            myLED2 = 0;
                            myLED3 = 0;
                            myLED4 = 0;

                            
                            break;

                        default:
                            state = off;
 
                        }
                        break;
                        
            
                           
                    case Heat_off:
                        if (now->tm_hour >= 23 && now->tm_hour < 6) {
                            pTemperature = &heat_setpoint_night;
                            uLCD.locate(1,1);
                            uLCD.filled_circle(10, 10, 5, WHITE);
                            night  = true;    
    
                        } else {
                            pTemperature = &heat_setting;
                            uLCD.locate(1,1);
                            uLCD.filled_circle(10, 10, 5, BLACK);
                            night  = false;    
                        }

    

                        

                        
                        if (mode == 2){
                            state = Cool_off;
                            uLCD.background_color(0x000000);
                            uLCD.textbackground_color(BLACK);
                            uLCD.cls();
                            uLCD.locate(7, 6);
                            uLCD.printf("Cool");
                            myLed.write(0.0,0.0,0.0);
                            myLED4 = 0;
                            myLED3 = 0;
                            myLED1 = 0;
                            myLED2 = 1; 
                            

                            break;   
                        }
                        if (temp <= *pTemperature){ 
                            state = Heat_on;
                            mySpeaker.PlayNote(100.0, .5, .2);            
                            uLCD.background_color(0xFF0000);
                            uLCD.textbackground_color(RED);
                            uLCD.cls();
                            uLCD.locate(6, 6);
                            uLCD.printf("Heating");
                            myLed.write(1.0,0.0,0.0);
                            myLED3 = 1;
                            myLED4 = 0;
                            myLED1 = 1;
                            myLED2 = 0;
                            if (night){
                            uLCD.locate(1,1);
                            uLCD.filled_circle(10, 10, 5, WHITE);
                            }
                                
                        }                            
                        break;
                    case Heat_on:
                        
                        if (mode == 2){
                            state = Cool_off;
                            uLCD.background_color(0x000000);
                            uLCD.textbackground_color(BLACK);
                            uLCD.cls();
                            uLCD.locate(7, 6);
                            uLCD.printf("Cool");
                            myLed.write(0.0,0.0,0.0);
                            myLED3 = 0;
                            myLED4 = 0;
                            myLED1 = 0;
                            myLED2 = 1;                            

                            break;   
                        }
                        

                        
                        if (temp >= *pTemperature){
                            state = Heat_off;
                            mySpeaker.PlayNote(100.0, .5, .2);            
                            uLCD.background_color(0x000000);
                            uLCD.textbackground_color(BLACK);
                            uLCD.cls();
                            uLCD.locate(7, 6);
                            uLCD.printf("Heat");
                            myLed.write(0.0,0.0,0.0);
                            myLED3 = 0;
                            myLED4 = 0;
                            myLED1 = 1;
                            myLED2 = 0;


                        }
                        break;
                        
                        
                    case Cool_off:
                        if (now->tm_hour >= 23 && now->tm_hour < 6) {
                            pTemperature = &cool_setpoint_night;
                            uLCD.locate(1,1);
                            uLCD.filled_circle(10, 10, 5, WHITE);
                            night = true;    
                        } else {
                            pTemperature = &cool_setting;
                            uLCD.locate(1,1);
                            uLCD.filled_circle(10, 10, 5, BLACK);
                            night = false; 
                        }
      


                       if (mode == 0){
                            state = off;
                            uLCD.background_color(GREEN);
                            uLCD.textbackground_color(GREEN);
                            uLCD.cls();
                            uLCD.locate(7, 6);
                            uLCD.printf("Off");
                            myLed.write(0.0,1.0,0.0);
                            myLED3 = 0;
                            myLED4 = 0;
                            myLED1 = 0;
                            myLED2 = 0;

                            break;                                                                                   
                        }       
                        

                        if (temp >= *pTemperature){
                            state = Cool_on;
                            mySpeaker.PlayNote(100.0, .5, .2);            
                            uLCD.background_color(BLUE);
                            uLCD.textbackground_color(BLUE);
                            uLCD.cls();
                            uLCD.locate(6, 6);
                            uLCD.printf("Cooling");
                            myLed.write(0.0,0.0,1.0);
                            myLED3 = 0;
                            myLED4 = 1;
                            myLED1 = 0;
                            myLED2 = 1;
                            if (night) {
                            uLCD.locate(1,1);
                            uLCD.filled_circle(10, 10, 5, WHITE);
                            }
                        }
                        
                                         
                        break;
                        
                    case Cool_on:


                        
                        if (mode == 0){
                            state = off;
                            uLCD.background_color(GREEN);
                            uLCD.textbackground_color(GREEN);
                            uLCD.cls();
                            uLCD.locate(7, 6);
                            uLCD.printf("Off");
                            myLed.write(0.0,1.0,0.0);
                            myLED3 = 0;
                            myLED4 = 0;
                            myLED1 = 0;
                            myLED2 = 0;
   

                            break;                                                                                   
                        }
                        
                        if (temp <= *pTemperature){
                            state = Cool_off;
                            mySpeaker.PlayNote(100.0, .5, .2);            
                            uLCD.background_color(0x000000);
                            uLCD.textbackground_color(BLACK);
                            uLCD.cls();
                            uLCD.locate(7, 6);
                            uLCD.printf("Cool");
                            myLed.write(0.0,0.0,0.0);
                            myLED3 = 0;
                            myLED4 = 0;
                            myLED1 = 0;
                            myLED2 = 1;
                        }
                }

                temp = myTMP36.read();
                float tempF = (9.0*temp)/5.0 + 32.0;
                uLCD.color(WHITE);
                uLCD.locate(6,1);
                uLCD.printf("Set to");
                uLCD.locate(7, 3);
                uLCD.text_width(2); //2X size text
                uLCD.text_height(2);  
                uLCD.printf("%d", *pTemperature);
                uLCD.text_width(1); //2X size text
                uLCD.text_height(1);                  
                uLCD.locate(6, 10);
                uLCD.printf("Inside");
                uLCD.locate(7, 12);
                uLCD.text_width(2); //2X size text
                uLCD.text_height(2);                  
                uLCD.printf("%.0f", temp);
                uLCD.text_width(1); //2X size text
                uLCD.text_height(1); 
                uLCD.locate(5,8);
                uLCD.printf("%d:%d:%d", now->tm_hour, now->tm_min, now->tm_sec);




                wait(1);

                // heartbeat LED - common debug tool
                // blinks as long as code is running and not locked up
                
                

            
        }
    }
