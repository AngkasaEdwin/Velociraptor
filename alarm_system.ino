const int distance_sensor = A2;
const int buzzor = 4;
const int green_led = 5;
const int red_led = 8;

const int alarm_button = 3;
const int security_button1 = 6;
const int security_button2 = 7;

  int button_on = LOW;
  int sound_signal = 0;
  int sound_delay = 1000;
  int distance = 0;
  int scaled_dist = 0;
  bool alert_mode = false;

//Alarm loudness
  const int ALERT = 3000;
  const int ALERT2 = 1000;
  const int BEEP = 5000;

//Hold button variables
int current;         // Current state of the button
long millis_held;    // How long the button was held (milliseconds)
long secs_held;      // How long the button was held (seconds)
long prev_secs_held; // How long the button was held in the previous check
int previous = LOW;
unsigned long first_time; // how long since the button was first pressed 

  int alarm_button_state = 0;
  int security_button1_state = 0;
  int security_button2_state = 0;
  const int BUTTON_DELAY = 5000;          //Time allowed between button presses
  
String current_password = "00000";

int compute_alarm_sound_frequency()
{
    return ALERT + floor(2000*sin(millis()));
}

// Stop the alarm
void stop_alarm(){
  sound_signal = 0;
  noTone(buzzor);
}

// Sound the alarm
void sound_alarm(int volume, int delay_length = 0){
  sound_signal = volume;
  tone(buzzor, sound_signal);
  if(delay_length > 0){
    delay(delay_length);
    stop_alarm();
  }
}

// This function will listen for 3 consecutive clicks from the alarm button.
// It will prompt password change if that happens
bool listen_alarm_button()
{
  int counter = 0;
  unsigned long start_time = 0;
  unsigned long current_time = 0;
  unsigned long delta_time = 0;
  for(int i = 0 ; i < 2 ; i++)
  {
    start_time = millis();
    current_time = start_time;
    delta_time = 0;
    security_button1_state = digitalRead(3);
    int original_button_state = alarm_button_state;

    while(delta_time < 500 && alarm_button_state == original_button_state)
    {
      alarm_button_state = digitalRead(3);
      current_time = millis();
      delta_time = current_time - start_time;
    }
    
    if (alarm_button_state != original_button_state) // When button is clicked
    {
      counter++;
    }
    else
    {
      return false;
    }
    delay(250);
  }
  return true;
}

// This function will listen for and read 5 clicks from the security buttons. 
// It returns a string of which buttons were pressed: 0 for security_button1, 1 for security_button2
// Note: This function might need to be modified based on how button clicks are actually registered
String listen_security_buttons()
{
  String clicks = "";
  unsigned long start_time = 0;
  unsigned long current_time = 0;
  unsigned long delta_time = 0;
  for(int i = 0 ; i < 5 ; i++)
  {
    start_time = millis();
    current_time = start_time;
    delta_time = 0;
    security_button1_state = digitalRead(6);
    security_button2_state = digitalRead(7);
    int original_button1_state = security_button1_state;
    int original_button2_state = security_button2_state;

    while(delta_time < BUTTON_DELAY && security_button1_state == original_button1_state && security_button2_state == original_button2_state)
    {
      security_button1_state = digitalRead(6);
      security_button2_state = digitalRead(7);
      current_time = millis();
      delta_time = current_time - start_time;
    }
    
    if (security_button1_state != original_button1_state) // When button1 is clicked.
    {
      clicks += "0";
    }
    else if (security_button2_state != original_button2_state) // When button 2 is clicked
    {
      clicks += "1";
    }
    else
    {
      digitalWrite(red_led, HIGH);
      delay(1000);
      digitalWrite(red_led, LOW);
      return clicks;
    }
    delay(250);
  }
  return clicks;
}

// This function is used to confirm the old password. It should be used by the change_password function.
// Returns true if the old password is correct; false otherwise. 
// It should be the job of the change_password function to sound the alarm if needed.
bool confirm_old_password()
{
  const int MAX_ATTEMPTS = 3;

  String old_password_confirm;
  for(int i = 0 ; i < MAX_ATTEMPTS ; i++)
  {
    old_password_confirm = listen_security_buttons();
    if (old_password_confirm == "")
    {
      return false; // No button is pressed within 5 seconds
    }
    if (old_password_confirm == current_password)
    {
      return true; // Successful input of old password
    }
  }
  return false; // Incorrect password inputted MAX_ATTEMPTS number of times
}


// This function will read a few sets of 5 clicks, separated by beeps to change the password.
// Note: The code might need to be modified, based on how button clicks are actually registered.
// A description of the function and what it does: 
//
// The user can click and hold the "ON" button to change the password
// After the user clicks and holds the ON button, 3-5 sets of 5 clicks will be registered
// with the arduino. The ON button is the only button that looks like the two security buttons, but 
// are not the two security buttons. (Hopefully this description is enough?)
//
// The first 5 clicks will be the old password.
//    If the user sucessfully enters the old password, then one beep will sound.
//    If the user does not, then two beeps will sound and the user will have to enter the password again.
//        If the user fails MAX_ATTEMPTS times, then the change_password function will quit.
//        IF_TIME_PERMITS: We can cause a delay of 1 min before the user can try changing the time again.
//
// Assuming now that the user has entered the old password correctly, 
//    The next 5 clicks will be the new password. There will be a beep after these 5 clicks to tell the 
//      user that the new password has been successfully read.
//    Finally, the next 5 clicks after that will confirm the new password.
//        If the new_password inputted the first time and the new_password inputted the second time are not the same,
//          then, quit the change_password function, and beep twice to show that something failed.
//        Else: Beep once to show that the new password has been set to the new one
void change_password()
{
  digitalWrite(green_led, HIGH);
  digitalWrite(red_led, HIGH);
  // If this function is being called, then it is assumed that the user has pressed and held the ON button for at least (3?) seconds
  if ( confirm_old_password() )
  {
    // Blink twice to signal that the user has inputted the old password correctly.
    digitalWrite(green_led, HIGH);
    delay(100);
    digitalWrite(green_led, LOW);
    delay(100);
    digitalWrite(green_led, HIGH);
    delay(100);
    digitalWrite(green_led, LOW);
    
    String new_password = listen_security_buttons(); // The new password is inputted
    if(new_password.length() < 5){
      sound_alarm(BEEP,200);
      sound_alarm(BEEP,200);
      sound_alarm(BEEP,200);
    }
    // Beep once to signal that the new password has been read.
    sound_alarm(BEEP,2000);

    // Confirm that the user has inputted the right password
    String confirm_new_password = listen_security_buttons();
    if (new_password == confirm_new_password)
    {
      // Change the current password.
      // Note: We need to make sure that this next line is actually changing the current password
      //       and not just doing it by value and then discarding it.
      current_password = new_password;

      // Beep twice to signal to the user that the new password has been inputted successfully
      sound_alarm(BEEP,200);
      sound_alarm(BEEP,200);
    }
    else
    {
      //      Beep twice to signal that the new password has not been confirmed, and that the 
      //      change_password function has temininated. Although it is is possible to ask the 
      //      user again for the new password and its confirmation, I feel that this would create 
      //      too many beeps and then confuse the user.
      //      If you are in this branch, then that means that the user has not successfully confirmed the new password
      sound_alarm(BEEP,200);
      sound_alarm(BEEP,200);
    }

  }
  else
  {
    // TODO: IF TIME: If you are in this branch, then that means that the user has inputted the password incorrectly 
    //                MAX_ATTEMPTS number of times. (This is a constant int set to 3 inside the confirm_old_password 
    //                function. In this case, we may want to sound the alarm here.
    sound_alarm(ALERT);
    while(sound_signal > 0) // Alarm active
    {
      sound_alarm(ALERT);
      sound_alarm(ALERT2);
      if(confirm_old_password())
        stop_alarm(); 
    }
  }
}

int convert_gp2d120_range(int distance) {
  return (2914 / (distance + 5)) - 1;
} 

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(buzzor, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  
  pinMode(alarm_button, INPUT);
  pinMode(security_button1, INPUT);
  pinMode(security_button2, INPUT);

  first_time = 0;
}

void loop() 
{
  // put your main code here, to run repeatedly:
  distance = analogRead(distance_sensor);
  button_on = digitalRead(alarm_button); 
  scaled_dist = convert_gp2d120_range(distance);  // Calculates the scaled value

  //Serial.println(second_dist);
  delay(200);
  //Serial.println(scaled_dist);

  // if the button state changes to pressed, remember the start time 
  //if (button_on == HIGH && previous == LOW && (millis() - first_time) > 200) {
  //  first_time = millis();
  //}

  //millis_held = (millis() - first_time);
  //secs_held = millis_held / 1000;

  //If the buttonstate changes
  if (button_on == HIGH)
  {
    digitalWrite(red_led, HIGH);  // Turn red LED on  
    Serial.println(alert_mode);
    if(alert_mode)
    {
      digitalWrite(red_led, LOW);  // Turn red off
      delay(2000);
      change_password();
//      Serial.println("change_password finished");
    }
    else
    {
        alert_mode = true;
        delay(500);
    }
  }
  
  //Serial.println(millis_held);
  
  // If the security system is active, sense distance
  if (alert_mode && scaled_dist > 1 && scaled_dist < 20)         // Distance has been sensed
  {
    // Enter password within 5 seconds or the alarm rings
    if(!confirm_old_password()) { // Password is entered incorrectly
      tone(buzzor,compute_alarm_sound_frequency());
      while(true) // Alarm active
      {
        tone(buzzor,compute_alarm_sound_frequency());
        if(confirm_old_password()){
          stop_alarm();
          break;
        }
      }
    }
  }
  previous = button_on;
  //prev_secs_held = secs_held;
/*
  if (button_on == HIGH)       // Check if button is pressed
  {

    if (convert_gp2d120_range(distance) > 0)         // Distance has been sensed
    {
      digitalWrite(green_led, HIGH);  // Turn LED on
      delay(sound_delay);
      tone(buzzor, sound_signal);     // Sound alarm
    }
  }
  else{
    digitalWrite(green_led, LOW);   // Turn LED off
  }
*/
}
