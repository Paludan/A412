typedef struct{
  int noteOn;
  int noteOff;
  int afterTouch;
  int controlChange;
  int programChange;
  int channelPressure;
  int pitchWheel;
} eventPlacement;

void findEvents(int numbersInText, int hex[], eventPlacement placement[], note noteAr[]){
  int noteOff = 0, noteOn = 0, afterTouch = 0, controlChange = 0,
      programChange = 0, channelPressure = 0, pitchWheel = 0, i = 0;

  for(int j = 0; j < numbersInText; j++){
    switch (hex[j]){
      case 0x90: insertPlacement1(hex, &placement[noteOn++].noteOn, j);                   break;
      case 0x80: insertPlacement1(hex, &placement[noteOff++].noteOff, j);                 break;
      case 0xA0: insertPlacement1(hex, &placement[afterTouch++].afterTouch, j);           break;
      case 0xB0: insertPlacement1(hex, &placement[controlChange++].controlChange, j);     break;
      case 0xC0: insertPlacement2(hex, &placement[programChange++].programChange, j);     break;
      case 0xD0: insertPlacement2(hex, &placement[channelPressure++].channelPressure, j); break;
      case 0xE0: insertPlacement1(hex, &placement[pitchWheel++].pitchWheel, j);           break;
      default  :                                                                          break;
    }
  }
}

void insertPlacement1(int hex[], int *place, int j){
  int i = 3;
  while(i < 7 && hex[(j + i++)] > 0x80);
  if(checkNextEvent(hex, (j + i))){
    *place = j;
    if(hex[j] == 0x90)
      fillNote(hex[j + 1], &noteAr[i++]);
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

void checkNoteOff(){
  for()
}

void checkVelocity(int hex[], eventPlacement placement[], int noteOn,
                   int afterTouch, int channelPressure, int noNoteOff[]){

  for(int i = 0; i < noteOn; i++)
    if(hex[(placement[i].noteOn + 2)] == 0x00)
      noNoteOff[i] = 1;

  for(int i = 0; i < afterTouch; i++)
    if(hex[(placement[i].afterTouch + 2)] == 0x00)
      for(int j = 0; j < noteOn; j++)
        if(hex[(placement[j].noteOn + 1)] == hex[(placement[i].afterTouch + 1)] && notEnded[j])
          noNoteOff[j] = 1;

  for(int i = 0; i < channelPressure; i++)
    if(hex[(placement[i].channelPressure + 1)] == 0x00)
      for(int j = 0; j < noteOn; j++)
        if(notEnded[j])
          noNoteOff[j] = 1;
}


checkNoteOff();
  int noNoteOff[noteOn] = {0};
  checkVelocity(hex, placement, noteOn, afterTouch, channelPressure, noNoteOff);