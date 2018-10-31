// Notes: Set RTC using C# program by lucadentella.it
// A.E.Nuemann 10/31/2018 V1.0
// modified to detect shutdown of C# pgm that updates the time
// C# program modified to fix error, add shutdown notice, added day of week
//  when the C# RTC Setup program shuts down, it sends $$ to indicate same and 
//  this program then exits the clock setting routine and displays time continually on an lcd module

#include <DS3231.h>
#include <DS3232RTC.h>

#define BUFFER_SIZE 20
#define VERSION     "1.0"

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);
// Init a Time-data structure
Time  clock;

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
char daysOfTheWeek[8][12] = {"null", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Buffer for incoming data
char serial_buffer[BUFFER_SIZE];
int buffer_position;
bool stopSettingTime=false;

void setup()
{
   Serial.begin(57600);
   
  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);}

	buffer_position = 0;
  
  lcd.init();  //initialize the lcd
  lcd.backlight();  //open the backlight 
  lcd.setCursor ( 0, 0 );            // go to the top left corner
  lcd.print ("Time and Temp"); // write this string on the top row

  // Initialize the rtc object
  rtc.begin();
  clock = rtc.getTime();   
  lcdPrintTime();
} 
  
//--------------MAIN------------------------
void loop() 
{
  if (!stopSettingTime)
  {
  	checktime();
  }
  	delay(1000);
  	clock = rtc.getTime();   
    lcdPrintTime();
    lcdPrintTemp();

}
//---------------------------------------------

void checktime() 
{
	  char incoming_char="";
  	String time_string="";
  	
do{ // Loop here checking/setting clock time until C# setup prgrm exits
   
// Wait for incoming data on serial port
  if (Serial.available() > 0) 
    {   // Read the incoming character
        incoming_char = Serial.read();
       
        // End of line?
        if(incoming_char == '\n') 
            { // Parse the command
                if(serial_buffer[0] == '#' && serial_buffer[1] == '#')  // acknowledge
                  { Serial.println("!!");
                  }
                else if(serial_buffer[0] == '$' && serial_buffer[1] == '$') // C# RTCSetup shutdown
                  { lcd.setCursor (0,4);
                    lcd.print("End");
                    stopSettingTime=true;
                  }
                else if(serial_buffer[0] == '?' && serial_buffer[1] == 'V')
                  {   Serial.println(VERSION);
                  }
                else if(serial_buffer[0] == '?' && serial_buffer[1] == 'T') // send DS3231 time      
                  {   clock = rtc.getTime();   // Get data from the DS3231
                      time_string = String(clock.date, DEC);
                      time_string += ("/");
                      time_string += String(clock.mon, DEC);
                      time_string += ("/");
                      time_string += String(clock.year, DEC);
                      time_string += (" ");
                      time_string += String(clock.hour, DEC);
                      time_string += (":");
                      time_string += String(clock.min, DEC);
                      time_string += (":");
                      time_string += String(clock.sec, DEC);
                      Serial.println(time_string);
                  }
                else if(serial_buffer[0] == '!' && serial_buffer[1] == 'T') // get new time to set
                  {   time_string = serial_buffer;
                      int day = time_string.substring(2, 4).toInt();
                      int month = time_string.substring(4, 6).toInt();        
                      int year = time_string.substring(6, 10).toInt();
                      int hour = time_string.substring(10, 12).toInt();
                      int minute = time_string.substring(12, 14).toInt();
                      int second = time_string.substring(14, 16).toInt();
                      int dayofweek = time_string.substring(16, 17).toInt();

                      //DateTime set_time = DateTime(year, month, day, hour, minute, second);
                      rtc.setDOW(dayofweek);     // Set Day-of-Week
                      rtc.setTime(hour, minute, second);     // Set the time to 12:00:00 (24hr format)
                      rtc.setDate(day, month, year);   // Set the date to DD/MM/YYYY
                      
                      Serial.println("OK");
                      clock = rtc.getTime();   
                      lcdPrintTime();
                  }
                   // Reset the buffer
                   buffer_position = 0;
                }
     
        // Carriage return, do nothing
        else if(incoming_char == '\r');
          
        // Normal character
        else 
          {// Buffer full, we need to reset it
            if(buffer_position == BUFFER_SIZE - 1)
                {buffer_position = 0;}
            // Store the character in the buffer and move the index
            serial_buffer[buffer_position] = incoming_char;
            buffer_position++; 
          } 
      }
  }while (stopSettingTime==false);
} 
   
void lcdPrintTime(){
    lcd.setCursor ( 0, 0 );            // go to the top left corner
    printDigitsDatelcd(clock.mon);
    printDigitsDatelcd(clock.date);  // adj to 00 if only 0 add /
    lcd.print(clock.year);
    lcd.print("          ");  
    lcd.setCursor(11,0);
    lcd.print(daysOfTheWeek[clock.dow]);
    //lcd.print(clock.dow);
    
    lcd.setCursor (0,1);
    printDigitslcd(clock.hour, false);
    printDigitslcd(clock.min, true);	// adj to 00 if only 0 add :
    printDigitslcd(clock.sec, true);
}

void lcdPrintTemp(){
  lcd.setCursor (0,2);
  lcd.print(rtc.getTemp());
  lcd.print(" C  ");
  //convert to F  -  T(degF) = T(degC) D 1.8 + 32 
  float tempF = 0;
  tempF = ((rtc.getTemp() * 1.8) + 32);
  lcd.print(tempF);
  lcd.print(" F");
  }
 
void printDigitslcd(int digits, bool divider){
    // utility function for digital clock display: prints preceding colon and leading 0
    if (divider==true){
        lcd.print(":");}
    if(digits < 10){
        lcd.print("0");}
    lcd.print(digits);
}
void printDigitsDatelcd(int digits){
    // utility function for digital clock display: prints preceding colon and leading 0
    if(digits < 10){
        lcd.print("0");}
    lcd.print(digits);
    lcd.print("/");
}
