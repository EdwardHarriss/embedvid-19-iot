//LOW FREQUENCY OSCILLATOR CLASS
//there are 2 of these objects implemented: one of these for controlling the incrementing/decrementing counter that is added to the output frequency for vibrato
//and one for the counter added to amplitude for tremolo
#ifndef LFO_h
#define LFO_h

class LFO {
  private:
    int max_val; //counter ranges from 0 to max_val when incrementing/decrementing
    int counter;
    int counterprev; //helps determine when the counter is increasing vs decreasing, amd whether to increment or decrement the counter value next
    int counter_incr; //integer that determines the value to add/subtract from counter when increasing/decreasing - essentially sets the speed for the fluctuation of the LFO

  public:
    LFO() {//default values for fluctuation
      max_val = 100;
      counter = 0;
      counterprev = 0;
      counter_incr = 1;
    }

    void set_max(int newsetmax){ //function only ever called from setup()
      max_val = newsetmax;
    }

    void change_max(int maxvalchange){ //max_val is only ever changed via adding or subtracting from the max value (by knob_1)
      int new_max = max_val + maxvalchange;
      if ((new_max >=0)&&(new_max<10000)){ //puts upper limit on fluctuation range
         max_val = new_max;
      }
    }

    void change_counterIncr(int incr_change){//changes speed at which counter counts up and down (therfore speed of fluctuation in freq)
      int new_counter_incr = counter_incr + incr_change;
      if ((!(new_counter_incr > max_val/4))&&(!(new_counter_incr<=0))){ //cant have the increment too large or small otherwise it doesn't work
        counter_incr = new_counter_incr;
      }
    }

    void reset_counter(){//used when the joystick button is pressed - resets vibrato settings
      counter = 0;
      counterprev =0;
      counter_incr = 1;
      max_val = 100;
    }

    int get_counter(){
      return counter;
    }

    int get_incr(){
      return counter_incr;
    }

    void update_counter(){ //increments/decrements counter
      if ((counter <= 0)||((counter>counterprev)&&(counter<max_val))){
        counterprev = counter;
        counter+=counter_incr;
     }
      else if((counter >= max_val)||(counter < counterprev)){
        counterprev = counter;
        counter-=counter_incr;
     }
    }
};

#endif
