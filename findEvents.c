typedef struct{
  int noteOn;
  int noteOff;
  int afterTouch;
  int controlChange;
  int programChange;
  int channelPressure;
  int pitchWheel;
} eventPlacement;

int main(void){
  int ticks[numbersInText];
  return 0;
}

void findEvents(int numbersInText, int hex[], eventPlacement placement[], note noteAr[], int ticks[]){
  int noteOff = 0, noteOn = 0, afterTouch = 0, controlChange = 0,
      programChange = 0, channelPressure = 0, pitchWheel = 0, i = 0, n = 0;

  for(int j = 0; j < numbersInText; j++){
    switch (hex[j]){
      case 0x90: insertPlacement1(hex, &placement[noteOn++].noteOn, j, noteAr, &n);               break;
      case 0x80: insertPlacement1(hex, &placement[noteOff++].noteOff, j, noteAr, &n);             break;
      case 0xA0: insertPlacement1(hex, &placement[afterTouch++].afterTouch, j, noteAr, &n);       break;
      case 0xB0: insertPlacement1(hex, &placement[controlChange++].controlChange, j, noteAr, &n); break;
      case 0xC0: insertPlacement2(hex, &placement[programChange++].programChange, j);             break;
      case 0xD0: insertPlacement2(hex, &placement[channelPressure++].channelPressure, j);         break;
      case 0xE0: insertPlacement1(hex, &placement[pitchWheel++].pitchWheel, j, noteAr, &n);       break;
      default  :                                                                                  break;
    }
  }
  findTicks(numbersInText, hex, placement, noteAr, ticks);
}

void insertPlacement1(int hex[], int *place, int j, note noteAr[], int *n){
  int i = 3;
  while(i < 7 && hex[(j + i++)] > 0x80);
  if(checkNextEvent(hex, (j + i))){
    *place = j;
    if(hex[j] == 0x90){
      fillNote(hex[j + 1], &noteAr[*n]);
      *n += 1;
    }   
  } 
}

void insertPlacement2(int hex[], int *place, int j){
  int i = 2;
  while(i < 6 && hex[(j + i++)] > 0x80);
  if(checkNextEvent(hex, (j + i)))
    *place = j;
}

int checkNextEvent(int hex[], int j){
  switch (hex[j]){
    case 0x90:
    case 0x80:
    case 0xA0:
    case 0xB0:
    case 0xC0:
    case 0xD0:
    case 0xE0: return 1; break;
    default  : return 0; break;
  }
}

void findTicks(int numbersInText, int hex[], eventPlacement placement[], note noteAr[], int ticks[]){
  int tickCounter = 0, deltaCounter1 = 3, deltaCounter2 = 2;
  
  for(int j = 0; j < noteOn; j++){
    for(int i = placement[j].noteOn; i < numbersInText; i++){
      if(hex[i] == 0x80){
        if(hex[i + 1] == noteAr[j])
          break;
        else{
          countTicks1(hex, &i, deltaCounter1, ticks[], tickCounter);
        }
      }
      else if(hex[i] == 0xA0){
        if(hex[i + 1] == noteAr[j] && hex[i + 2] == 0x00)
          break;
        else{
          countTicks1(hex, &i, deltaCounter1, ticks[], tickCounter);
        }
      }
      else if(hex[i] == 0xD0){
        if(hex[i + 1] == 0x00)
          break;
        else{
          countTicks2(hex, &i, deltaCounter2, ticks[], tickCounter);
        }
      }
      else if(hex[start] == 0xC0){
        countTicks2(hex, &i, deltaCounter2, ticks[], tickCounter);
      }
      else{
        countTicks1(hex, &i, deltaCounter1, ticks[], tickCounter);
      }     
    }
  }
}

void countTicks1(int hex[], int *i, int deltaCounter, int ticks[], int *tickCounter){
  while(deltaCounter < 7 && hex[(i + deltaCounter)] > 0x80)
    ticks[tickCounter] += ((hex[(i + deltaCounter++)] - 0x80) * 128);
  ticks[tickCounter++] += hex[(i + deltaCounter++)];
  i += deltaCounter;
}

void countTicks2(int hex[], int *i, int deltaCounter, int ticks[], int *tickCounter){
  while(deltaCounter < 6 && hex[(i + deltaCounter)] > 0x80)
    ticks[tickCounter] += ((hex[(i + deltaCounter++)] - 0x80) * 128);
  ticks[tickCounter++] += hex[(i + deltaCounter++)];
  i += deltaCounter;
}