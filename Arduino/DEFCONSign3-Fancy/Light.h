
class Light {

  public :
    Light() {
      red = green = blue = 0;
      baseColour = colour = 0;
      targetRed = targetGreen = targetBlue = 0;

    }
    setBase(uint32_t col) {
      targetRed = red = (col >> 16);
      targetGreen = green = (col >> 8) ;
      targetBlue = blue = col ;
      baseColour = col;
      dirty = true;
    };

    set(uint32_t col) {
      targetRed = (col >> 16) ;
      targetGreen = (col >> 8) ;
      targetBlue = col;

      //        red = targetRed = (col >> 16) ;
      //      green = targetGreen = (col >> 8) ;
      //      blue = targetBlue = col;
      //  dirty = true;
    };

    void update() {

      interpolateColour(red, targetRed, 12, 2);
      interpolateColour(green, targetGreen, 10, 2);
      interpolateColour(blue, targetBlue, 8, 4);

      if (dirty) colour = ((uint32_t)Adafruit_DotStar::gamma8(red) << 16) | ((uint32_t)Adafruit_DotStar::gamma8(green) << 8) | Adafruit_DotStar::gamma8(blue);
      dirty = false;
    }

    void interpolateColour(uint8_t& component, uint8_t& target, int upspeed, int downspeed) {
      if (target == component) return;

      if (target > component) {
        if (255 - component < upspeed) component = target;
        else component += upspeed;

      } else if (target < component) {
        if (component < downspeed) component = target;
        else component -= downspeed;
      }

      dirty = true;
    }

    void turnOn() {
      set(baseColour);
      onState = true;
    }
    void turnOff() {
      set(0x000000);
      onState = false;
    }
    bool isOn() {
      return onState;
    }
    uint32_t getColour() {
      return colour;
    }
    uint64_t colour, baseColour;
    uint8_t red, green, blue, targetRed, targetGreen, targetBlue;
    bool dirty;
    bool onState = false;

};
