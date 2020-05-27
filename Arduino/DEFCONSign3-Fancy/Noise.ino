int range = 2048; 
int halfRange = range/2; 
int currentNoise = halfRange; 
int divider = 8; 
int variation = range/divider*1.5;
unsigned long lastNoiseChange = 0; 

int getNoise() {

  if(millis()-lastNoiseChange>10) { 
    int offset = (halfRange - currentNoise) /divider; // should be between -128 and 128 
    int noiseChange = random(offset-variation, offset+variation);
    currentNoise+=noiseChange; 
    if(currentNoise>range) currentNoise = range; 
    else if(currentNoise<0) currentNoise = 0; 
    lastNoiseChange = millis();
  }

  return currentNoise/8; 
   
}
