#ifndef knob_h
#define knob_h

class knob {
  private:
    int8_t knobposition;
    int8_t knobpreviousvalue;
    int8_t upper_limit;
    int8_t lower_limit;
    int8_t prevrot = 0;
    bool buttonState;
    bool toggleMode; //sets it to push/release default
    bool prevButtonPress; //to stop value from toggling non-stop when button is held down a little too long
  public:
    knob() {
      knobposition = 0;
      upper_limit = 16;
      lower_limit = -16;
      buttonState=0;
      toggleMode = 0;
    }
    bool get_buttonState(){
      return buttonState;
    }
    void setToggle(bool setter){ //called in the setup() function, sets buttons to toggle rather than push/release
      if (setter==1){
        toggleMode = 1; //sets it to toggle rather than push/release
      }
      if (setter==0){
        toggleMode = 0;
      }
    }
    void update_buttonState(bool currButtonVal){
      bool pressed = !currButtonVal; //button values are normally 1 while not pressed, 0 while pressed, so this is just flipping it
      //toggleMode = 1; //for testing
      if (toggleMode ==1){
        if (pressed==1){
          if (prevButtonPress!= 1){ //stops value from non-stop toggling when held down
             buttonState= !buttonState;
          }
        }
        prevButtonPress = pressed;
      }
      else if (toggleMode == 0){
        buttonState = pressed; //if not toggle mode set, button value = whether it is currently pressed or not
      }
    }

    int8_t get_knob_position() {
      return knobposition;
    }
    void reset_knob_position(){
      knobposition =0;
      knobpreviousvalue = 0;
    }
    uint8_t get_previous_value() {
      return knobpreviousvalue;
    }
    void set_previous_position(uint8_t new_prev) {
      knobpreviousvalue = new_prev;
    }
    void set_upper_limit(int8_t newupper){
      upper_limit = newupper;
    }
    void set_lower_limit(int8_t newlower){
      lower_limit = newlower;
    }
    void knobdecoder(uint8_t localCurrentKnob) {
      int8_t rotation = 0;
      if ((knobpreviousvalue == 0x0) && (localCurrentKnob == 0x1)) {
        rotation = 1;
        prevrot = 1;
      }
      else if ((knobpreviousvalue == 0x0) && (localCurrentKnob == 0x2)) {
        rotation = -1;
        prevrot = -1;
      }
      /*else if ((knobpreviousvalue == 0x0) && (localCurrentKnob == 0x3)) {
        rotation = 2*prevrot;
      }*/
      else if ((knobpreviousvalue == 0x1) && (localCurrentKnob == 0x0)) {
        rotation = -1;
        prevrot = -1;
      }
      /*else if ((knobpreviousvalue == 0x1) && (localCurrentKnob == 0x1)) {
        rotation = 2*prevrot;
      }*/
      else if ((knobpreviousvalue == 0x1) && (localCurrentKnob == 0x3)) {
        rotation = 1;
        prevrot = 1;
      }
      else if ((knobpreviousvalue == 0x2) && (localCurrentKnob== 0x0)) {
        rotation = 1;
        prevrot = 1;
      }
      /*else if ((knobpreviousvalue == 0x2) && (localCurrentKnob == 0x1)) {
        rotation = 2*prevrot;
      }*/
      else if ((knobpreviousvalue == 0x2) && (localCurrentKnob == 0x3)) {
        rotation = -1;
        prevrot = -1;
      }
      /*else if ((knobpreviousvalue == 0x3) && (localCurrentKnob == 0x0)) {
        rotation = 2*prevrot;
      }*/
      else if ((knobpreviousvalue == 0x3) && (localCurrentKnob == 0x1)) {
        rotation = -1;
        prevrot = -1;
      }
      else if ((knobpreviousvalue == 0x3) && (localCurrentKnob == 0x2)) {
        rotation = 1;
        prevrot = 1;
      }
      if ((knobposition >= upper_limit) && (rotation == 1)) {
        knobposition = upper_limit;
        return;
      }
      if ((knobposition <= lower_limit) && (rotation == -1)) {
        knobposition = lower_limit;
        return;
      }
      knobposition += rotation;
    }
};

#endif
