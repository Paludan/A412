/*@file
 * Gruppe A412
 * Lee Paludan
 * Simon Madsen
 * Jonas Stolberg
 * Jacob Mortensen
 * Esben Kirkegaard Da Scrub
 * Arne Rasmussen Da Champ
 * Tobias Morell
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>

int AMOUNT_OF_MOODS;
#define CHARS 1000
#define SCALESIZE 7
#define META_EVENT 0xff
#define TEMPO_EVENT_BYTE_1 0x51
#define TEMPO_EVENT_BYTE_2 0x03
#define BIT_SHIFT_16 16
#define BIT_SHIFT_8 8
#define BIT_SHIFT_7 7
#define MICRO_SECONDS_PER_MINUTE 60000000
#define NOTE_ON 0x90
#define NOTE_OFF 0x80
#define AFTER_TOUCH 0xA0
#define CONTROL_CHANGE 0xB0
#define PROGRAM_CHANGE 0xC0
#define CHANNEL_PRESSURE 0xD0
#define PITCH_WHEEL 0xE0
#define ZERO 0x00
#define HEX_80 0x80
#define LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME 3
#define LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME 2
#define MAX_LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME_END 7
#define MAX_LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME_END 6

/*Enums and structs*/
typedef enum mode {major, minor} mode;
typedef enum tone {C, Csharp, D, Dsharp, E, F, Fsharp, G, Gsharp, A, Asharp, B} tone;
typedef enum mood {glad, sad} mood;

/**A struct cotaining data about a single note
  *@param tone the tone stored as an integer (C = 0)
  *@param octave which octave, on a piano, the note is in (1 is the deepest, C4 is middle C)
  *@param length the notes length in standard musical notation
  *@param noteValue used in calculating the average tone
  *@param ticks the notes length in ticks
  */
typedef struct{
  int tone;
  int octave;
  int length;
  int noteValue;
  int ticks;
} note;

/**A struct containing general data pertaining to the song
  *@param tempo the tempo in beats-per-minute
  *@param ppqn ticks-per-quarter-note contains the number of ticks per quarter note
  *@param mode an enumerated value representing the mode (major/minor)
  */
typedef struct{
  unsigned int tempo;
  int ppqn;
  mode mode;
} globalMelodyInfo;

/**A struct containing a single moods name and weighting
  *@param name the name of the mood
  *@param mode a value -5 to 5 representing this parameters impact on the mood
  *@param tempo a value -5 to 5 representing this parameters impact on the mood
  *@param toneLength a value -5 to 5 representing this parameters impact on the mood
  *@param pitch a value -5 to 5 representing this parameters impact on the mood
  */
typedef struct{
  char name[15];
  int mode;
  int tempo;
  int toneLength;
  int pitch;
} moodWeighting;

/**A struct containing placements of midi events, stored as their placement in file
  *@param noteOn signals when a note starts playing
  *@param noteOff signals when a note stops playing
  *@param afterTouch changes velocity for a single note on a single channel
  *@param controlChange used for a large number of effects, none of which are used in this project (stored to find deltatimes)
  *@param programChange signals instrument change (not used; stored to find deltatimes)
  *@param channelPressure changes velocity for all notes on a specific channel (akin to a global afterTouch)
  *@param pitchWheel fine tuning of pitch for all notes on a specific channel (similar to channelPressure, but for pitch) 
  */
typedef struct{
  int noteOn;
  int noteOff;
  int afterTouch;
  int controlChange;
  int programChange;
  int channelPressure;
  int pitchWheel;
} eventPlacement;

/*Prototypes*/
void checkDirectory(char*, DIR*);
void printNote(note);
int getHex(FILE*, int[]);
void fillSongData(globalMelodyInfo*, int[], int);
int countPotentialNotes(int[], int);
void fillNote(int, note*);
void settingPoints(int*, int*, int*, int*, globalMelodyInfo, int, note [], int*);
void modePoints(globalMelodyInfo, int*);
void tempoPoints(globalMelodyInfo, int*);
void lengthPoints(int*, note [], int);
void notePoints(int*, note [], int);
void insertMoods(moodWeighting [], FILE*);
void weightingMatrix(moodWeighting [], int, int, int, int, int*);
void findEvents(int, int [], eventPlacement [], note [], int*, int*);
void insertPlacement1(int [], int*, int, note [], int*, int []);
void insertPlacement2(int [], int*, int);
int isNextHexAnEvent(int [], int);
void findTicks(int, int [], eventPlacement [], note [], int, int*, int []);
void countTicks1(int [], int*, int, note [], int*);
void countTicks2(int [], int*, int, note [], int*);
void deltaTimeToNoteLength(int, int, note*);
int isInScale(int, int[], int);
int isInMinor(int);
int isInMajor(int);
int sortToner(const void*, const void*);
void findMode(note*, int, globalMelodyInfo*);
int FindMoodAmount(FILE*);
void printResults(int, int, int, int, moodWeighting [], int []);
void printParameterVector(int, int, int, int);
void printWeightingMatrix(moodWeighting []);
void printVectorMatrixProduct(moodWeighting [], int, int, int, int, int []);
void printHappySadScale(moodWeighting [], int, int []);
void printMoodOfMelody(moodWeighting [], int);

int main(int argc, const char *argv[]){
  DIR *dir = 0;
  FILE *f;
  char MIDIfile[25];
  /*Variables*/
  int numbersInText = 0, notes, size = 0, mode = 5, tempo = 5, toneLength = 5, pitch = 5, amountOfNotes = 0;
  FILE* moods = fopen("moods.txt", "r");
  
  if(moods == NULL){
    perror("Error: moods missing ");
    exit(EXIT_FAILURE);
  }
  
  AMOUNT_OF_MOODS = FindMoodAmount(moods);
  moodWeighting moodArray[AMOUNT_OF_MOODS];
  globalMelodyInfo info;
  
  if (argv[1] == NULL){
    checkDirectory(MIDIfile, dir);
    f = fopen(MIDIfile, "r");  
    
    if(f == NULL){
      perror("Error opening file");
      exit(EXIT_FAILURE);
    }
  }
  else if(argv[1] != NULL){
    f = fopen(argv[1],"r");
    
    if(f == NULL){
      perror("Error opening file");
      exit(EXIT_FAILURE);
    }
  }
  
  closedir (dir); 
  int *hex = (int *) malloc(CHARS * sizeof(int));
  
  if(hex == NULL){
    printf("Memory allocation failed, bye!");
    exit(EXIT_FAILURE);
  }
  
  /*Reading the data from the file*/
  numbersInText = getHex(f, hex);
  fillSongData(&info, hex, numbersInText);
  notes = countPotentialNotes(hex, numbersInText);
  note *noteAr = (note*) malloc(notes * sizeof(note));
  
  if(noteAr == NULL){
    printf("Memory allocation failed, bye!");
    exit(EXIT_FAILURE);
  }
  
  eventPlacement placement[numbersInText];
  int result[AMOUNT_OF_MOODS];
  findEvents(numbersInText, hex, placement, noteAr, &size, &amountOfNotes);
  deltaTimeToNoteLength(info.ppqn, size, noteAr);
  insertMoods(moodArray, moods);
  findMode(noteAr, amountOfNotes, &info);
  settingPoints(&mode, &tempo, &toneLength, &pitch, info, amountOfNotes, noteAr, &size);
  weightingMatrix(moodArray, mode, tempo, toneLength, pitch, result);

  /*Clean up and close*/
  fclose(f);
  free(hex);
  free(noteAr);

  /* Print results */
  printResults(mode, tempo, toneLength, pitch, moodArray, result);

  return 0;
}

/**A function to read music directory and prompt user to choose file
  *@param MIDIfile a pointer to a string containing the name of the chosen input file
  *@param dir a pointer to a directory*/
void checkDirectory(char *MIDIfile, DIR *dir){
  struct dirent *musicDir;
  int musicNumber = -2;

  if((dir = opendir ("./Music")) != NULL) {
    printf(" Mulige numre\n");
    while ((musicDir = readdir (dir)) != NULL){
      if(musicNumber > -1 && musicNumber < 10)
        printf (" %d.  %s\n", musicNumber++, musicDir->d_name);
      else if(musicNumber > -1)
        printf (" %d. %s\n", musicNumber++, musicDir->d_name);
      else
        musicNumber++;
    }
  } 
  else{
    perror ("Failure while opening directory");
    exit (EXIT_FAILURE);
  }

  closedir(dir);

  if((dir = opendir ("./Music")) != NULL) {
    printf("\n Indtast det valgte nummer\n ");
    scanf(" %d", &musicNumber);

    for(int i = -2; i <= musicNumber; i++)
      if((musicDir = readdir (dir)) != NULL && i == (musicNumber))
        strcpy(MIDIfile, musicDir->d_name);

    printf("\n Du valgte \n %s\n Hvilket giver disse resultater\n", MIDIfile);
  } 
  else{
    perror ("Failure while opening directory");
    exit (EXIT_FAILURE);
  }

  chdir("./Music");
}

/**A function, that retrieves the hexadecimals from the files and also returns the number of files
  *@param *f a pointer to the file the program is reading from
  *@param hexAr[] an array of integers, that the information is stored in
  */
int getHex(FILE *f, int hexAr[]){
  int i = 0, c;
  
  while( (c = fgetc(f)) != EOF && i < CHARS){
    hexAr[i] = c;
    i++;
  }

  return i;
}

/**A function to count the number of notes in the entire song
  *@param hex[] an array with the stored information from the file
  *@param amount an integer holding the total number of characters in the array
  */
int countPotentialNotes(int hex[], int amount){
  int i = 0, res = 0;
  
  for(i = 0; i < amount; i++){
    if(hex[i] == 0x90){
      res++;
    }
  }
  
  return res;
}

/**! \fn int fillSongData(globalMelodyInfo *info, int hex[], int numbersInText) 
  *A function, that inserts ppqn and tempo to the info array
  *@param *info a pointer to a structure containing the tempo and mode of the song
  *@param hex[] the array of integers read from the file
  *@param numbersInText the total amount of integers in the array
  */
void fillSongData(globalMelodyInfo *info, int hex[], int numbersInText){
  info->ppqn = (hex[12] << 8) + hex[13];
  
  for(int j = 0; j < numbersInText; j++)
    /* finds the tempo */
    if(hex[j] == META_EVENT && hex[j+1] == TEMPO_EVENT_BYTE_1 && hex[j+2] == TEMPO_EVENT_BYTE_2)
      info->tempo =  MICRO_SECONDS_PER_MINUTE/((hex[j+3] << BIT_SHIFT_16) | (hex[j+4] << BIT_SHIFT_8) | (hex[j+5]));
}

/**Searches the file for events and stores their placement in an array of eventPlacement structs 
  */
void findEvents(int numbersInText, int hex[], eventPlacement placement[], note noteAr[], int *size, int *amountOfNotes){
  int noteOff = 0, noteOn = 0, afterTouch = 0, controlChange = 0,
      programChange = 0, channelPressure = 0, pitchWheel = 0, notes[numbersInText];
  
  for(int j = 0; j < numbersInText; j++)
    switch (hex[j]){
      case NOTE_ON         : insertPlacement1(hex, &placement[noteOn++].noteOn, j, noteAr, amountOfNotes, notes);
                             break;
      case NOTE_OFF        : insertPlacement1(hex, &placement[noteOff++].noteOff, j, noteAr, amountOfNotes, notes);
                             break;
      case AFTER_TOUCH     : insertPlacement1(hex, &placement[afterTouch++].afterTouch, j, noteAr, amountOfNotes, notes);
                             break;
      case CONTROL_CHANGE  : insertPlacement1(hex, &placement[controlChange++].controlChange, j, noteAr, amountOfNotes, notes);
                             break;
      case PROGRAM_CHANGE  : insertPlacement2(hex, &placement[programChange++].programChange, j);
                             break;
      case CHANNEL_PRESSURE: insertPlacement2(hex, &placement[channelPressure++].channelPressure, j);
                             break;
      case PITCH_WHEEL     : insertPlacement1(hex, &placement[pitchWheel++].pitchWheel, j, noteAr, amountOfNotes, notes);
                             break;
      default  :             break;
    }
  findTicks(numbersInText, hex, placement, noteAr, noteOn, size, notes);
}

/**Starts in the hex which are investigated and looks forward to find a perspective
  *It goes to an assumed deltatime and finds the length of it. Thereafter it checks the next hex after the deltatime to make sure it is an event
  *If that is the case it stores the hex which is investegated in the first place
  *Furthermore if it is a noteOn event it stores the hex which is the note, processes the note and counts amount of notes
  */
void insertPlacement1(int hex[], int *place, int j, note noteAr[], int *amountOfNotes, int notes[]){
  int i = LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME;
  
  while(i < MAX_LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME_END && hex[(j + i++)] > NOTE_OFF);
  
  if(isNextHexAnEvent(hex, (j + i))){
    *place = j;
    if(hex[j] == NOTE_ON){
      notes[*amountOfNotes] = hex[j + 1];
      fillNote(hex[j + 1], &noteAr[*amountOfNotes]);
      *amountOfNotes += 1;
    }   
  } 
}

/**Does the same as insertPlacement1, but for the two events that only takes one parameter
  */
void insertPlacement2(int hex[], int *place, int j){
  int i = LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME;
  
  while(i < MAX_LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME_END && hex[(j + i++)] > NOTE_OFF);
  
  if(isNextHexAnEvent(hex, (j + i)))
    *place = j;
}

/**Returns true if next hex is an event or false if not
  */
int isNextHexAnEvent(int hex[], int j){
  switch (hex[j]){
    case NOTE_ON         :
    case NOTE_OFF        :
    case AFTER_TOUCH     :
    case CONTROL_CHANGE  :
    case PROGRAM_CHANGE  :
    case CHANNEL_PRESSURE:
    case PITCH_WHEEL     : return 1; break;
    default              : return 0; break;
  }
}

/**Analyses ticks for every noteOn event by a for loop which begins in the noteOn events start and searches for the end of the event
  */
void findTicks(int numbersInText, int hex[], eventPlacement placement[], note noteAr[], int noteOn, int *size, int notes[]){
  int tickCounter = 0, deltaCounter1 = LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME,
                       deltaCounter2 = LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME;
  
  for(int j = 0; j < noteOn; j++){
    for(int i = placement[j].noteOn; i < numbersInText; i++){
      if(hex[i] == NOTE_OFF){
        if(hex[i + 1] == notes[j]){
          tickCounter++;
          break;
        }
        else
          countTicks1(hex, &i, deltaCounter1, noteAr, &tickCounter);
      }
      else if(hex[i] == AFTER_TOUCH){
        if(hex[i + 1] == notes[j] && hex[i + 2] == ZERO){
          tickCounter++;
          break;
        }
        else
          countTicks1(hex, &i, deltaCounter1, noteAr, &tickCounter);
      }
      else if(hex[i] == CHANNEL_PRESSURE){
        if(hex[i + 1] == ZERO){
          tickCounter++;
          break;
        }  
        else
          countTicks2(hex, &i, deltaCounter2, noteAr, &tickCounter);
      }
      else if(hex[i] == PROGRAM_CHANGE)
        countTicks2(hex, &i, deltaCounter2, noteAr, &tickCounter);
      else
        countTicks1(hex, &i, deltaCounter1, noteAr, &tickCounter);   
    }
  }
  
  *size = tickCounter;
}

/**Processes event types which take two parameters, extracting deltatime (and advancing the file pointer)
  */
void countTicks1(int hex[], int *i, int deltaCounter, note noteAr[], int *tickCounter){
  noteAr[*tickCounter].ticks = 0;
  int tick = 0;
  
  while(deltaCounter < MAX_LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME_END && hex[(*i + deltaCounter)] > HEX_80)
    tick += ((hex[(*i + deltaCounter++)] - HEX_80) << BIT_SHIFT_7);
  
  tick += hex[(*i + deltaCounter)];
  noteAr[*tickCounter].ticks += tick;
  *i += deltaCounter;
}

/**Processes event types which take one parameter, extracting deltatime (and advancing the file pointer)
  */
void countTicks2(int hex[], int *i, int deltaCounter, note noteAr[], int *tickCounter){
  noteAr[*tickCounter].ticks = 0;
  int tick = 0;
  
  while(deltaCounter < MAX_LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME_END && hex[(*i + deltaCounter)] > HEX_80)
    tick += ((hex[(*i + deltaCounter++)] - HEX_80) << BIT_SHIFT_7);
  
  tick += hex[(*i + deltaCounter)];
  noteAr[*tickCounter].ticks += tick;
  *i += deltaCounter;
}

/**A function to fill out each of the structures of type note
  *@param inputTone the value of the hexadecimal collected on the "tone"-spot
  *@param note* a pointer to a note-structure
*/
void fillNote(int inputTone, note *note){
  note->tone = inputTone % 12;
  note->noteValue = inputTone;
  note->octave = inputTone / 12;
}

/**A function to print the note
  *@param note the note structure to be printed
  */
void printNote(note note){
  printf("Tone: ");
  
  switch (note.tone){
    case C     : printf("C") ; break;
    case Csharp: printf("C#"); break;
    case D     : printf("D") ; break;
    case Dsharp: printf("D#"); break;
    case E     : printf("E") ; break;
    case F     : printf("F") ; break;
    case Fsharp: printf("F#"); break;
    case G     : printf("G") ; break;
    case Gsharp: printf("G#"); break;
    case A     : printf("A") ; break;
    case Asharp: printf("A#"); break;
    case B     : printf("B") ; break;
    default    : printf("Undefined note"); break;
  }
  
  printf(", octave: %d\n", note.octave);
}

/**A function to insert points into integers based on the data pulled from the file
 *@param mode, along with tempo, length and octave contains the points for the Melody
 *@param info contains the song ppqn, tempo and mode for the song
 *@param notes contains the amount of notes in the song
 *@param note contains an array of the specific notes
 */
void settingPoints(int *mode, int *tempo, int *length, int *octave,
                   globalMelodyInfo info,int notes, note noteAr[], int *size){
  modePoints(info, mode);
  tempoPoints(info, tempo);
  lengthPoints(length, noteAr, notes);
  notePoints(octave, noteAr, notes);
}

/**Changes the value of the integer mode depending on the mode of the melody
  */
void modePoints(globalMelodyInfo info, int *mode){
  switch(info.mode){
    case minor: *mode = -5; break;
    case major: *mode = 5;  break;
    default   : *mode = 0;  break;
  }
}

/**Changes the value of the integer tempo depending on the tempo of the melody
  */
void tempoPoints(globalMelodyInfo info, int *tempo){
  if(info.tempo < 60)
    *tempo = -5;
  else if(info.tempo >= 60 && info.tempo < 70)
    *tempo = -4;
  else if(info.tempo >= 70 && info.tempo < 80)
    *tempo = -3;  
  else if(info.tempo >= 80 && info.tempo < 90)
    *tempo = -2;
  else if(info.tempo >= 90 && info.tempo < 100)
    *tempo = -1;
  else if(info.tempo >= 100 && info.tempo < 120)
    *tempo =  0;  
  else if(info.tempo >= 120 && info.tempo < 130)
    *tempo =  1;
  else if(info.tempo >= 130 && info.tempo < 140)
    *tempo =  2;
  else if(info.tempo >= 140 && info.tempo < 150)
    *tempo =  3;
  else if(info.tempo >= 150 && info.tempo < 160)
    *tempo =  4;
  else if(info.tempo >=  160)
    *tempo =  5;
}

/**Changes the value of the integer length depending on the average note length of the melody
  */
void lengthPoints(int *length, note noteAr[], int notes){
  int combined = 0;
  for(int i = 0; i < notes; i++)
    combined += noteAr[i].length;
  
  int deltaTime = combined/notes;
  
  if (deltaTime < 1.5 && deltaTime >= 0)
    *length = 5;
  else if (deltaTime < 3 && deltaTime >= 1.5)
    *length = 4;
  else if (deltaTime < 5 && deltaTime >= 4)
    *length = 3;
  else if (deltaTime < 6 && deltaTime >= 5)
    *length = 2;
  else if (deltaTime < 9 && deltaTime >= 6)
    *length = 1;
  else if (deltaTime < 12 && deltaTime >= 9)
    *length = 0;
  else if (deltaTime < 16 && deltaTime >= 12)
    *length = -1;
  else if (deltaTime < 20 && deltaTime >= 16)
    *length = -2;
  else if (deltaTime < 24 && deltaTime >= 20)
    *length = -3;
  else if (deltaTime < 28 && deltaTime >= 24)
    *length = -4;
  else
    *length = -5;
}

/**Changes the value of the integer octave depending on the average note value in the melody
  */
void notePoints(int *octave, note noteAr[], int notes){
  int combined = 0;
  for (int i = 0; i < notes; i++)
    combined += noteAr[i].noteValue;
  
  int averageNote = combined/notes;

  if(averageNote <= 16)
    *octave = -5;
  else if(averageNote >= 17 && averageNote <= 23)
    *octave = -4;
  else if(averageNote >= 24 && averageNote <= 30)
    *octave = -3;
  else if(averageNote >= 31 && averageNote <= 37)
    *octave = -2;
  else if(averageNote >= 38 && averageNote <= 44)
    *octave = -1;
  else if(averageNote >= 45 && averageNote <= 51)
    *octave = 0;
  else if(averageNote >= 52 && averageNote <= 58)
    *octave = 1;
  else if(averageNote >= 59 && averageNote <= 65)
    *octave = 2;
  else if(averageNote >= 66 && averageNote <= 72)
    *octave = 3;
  else if(averageNote >= 73 && averageNote <= 79)
    *octave = 4;
  else if(averageNote >=80)
    *octave = 5;
}

/**Inserts the weighting of each mood in an array of structs, as read from a designated file.
  *@param moodArray The array moods are stored in
  *@param moods the file to be read
  */
void insertMoods(moodWeighting moodArray[], FILE* moods){
  for(int i = 0; i < AMOUNT_OF_MOODS; i++)
    fscanf(moods, "%s %d %d %d %d", moodArray[i].name , &moodArray[i].mode, 
                                    &moodArray[i].tempo, &moodArray[i].toneLength,
                                    &moodArray[i].pitch);
}

/**Vector matrix multiplication. Receives an array of moods, the various parameters of the song and a
  *pointer to an array where the results will be stored. The song data is multiplied onto each moods
  *weighting and then stored.
  *@param moodArray an array containing the weighting for all moods
  *@param result an array for holding the songs scores as per each mood
  *@param mode along with temp, toneLength and pitch, this variable contains a score -5 to 5 for how
  *that facet of the song is.
  */
void weightingMatrix(moodWeighting moodArray[], int mode, int tempo, int toneLength, int pitch, int *result){
  for(int i = 0; i < AMOUNT_OF_MOODS; i++)
    result[i] = 0;
  
  for(int i = 0; i < AMOUNT_OF_MOODS; i++){
    result[i] += (moodArray[i].mode * mode);
    result[i] += (moodArray[i].tempo * tempo);
    result[i] += (moodArray[i].toneLength * toneLength);
    result[i] += (moodArray[i].pitch * pitch);
  }
}

/**Finds the note length, converted from deltatime to standard musical notation
  */
void deltaTimeToNoteLength (int ppqn, int size, note *noteAr){
  for (int i = 0; i < size; i++){
    double noteLength = ((double) (noteAr[i].ticks)) / ((double) (ppqn/8));
    
    if (noteLength < 1.5 && noteLength >= 0)
      noteLength = 1;
    else if (noteLength < 3 && noteLength >= 1.5)
      noteLength = 2;
    else if (noteLength < 6 && noteLength >= 3)
      noteLength = 4;
    else if (noteLength < 12 && noteLength >= 6)
      noteLength = 8;
    else if (noteLength < 24 && noteLength >= 12)
      noteLength = 16;
    else
      noteLength = 32;
    
    noteAr[i].length = noteLength;
  }
}

/**A function to sort integers in ascending order, used by qsort
  */
int sortTones(const void *a, const void *b){
  return (*(int *)a - *(int *)b);
}

/**Checks if the tone given is within the scale of the key given.
  *@param scales An array containing the scalas
  *@param tone An integer representing the tone to be checked
  *@param key Integer representing the key the note is compared to
  */
void checkScale(int scales[], int tone, int key){
  if(tone < key)
    tone += 12;
  
  scales[key] = isInMajor(tone - key);
}

/**A function to find the mode of the song by first calculating the tone span over sets of notes in the song,
  *and then comparing it to the definition of minor and major keys.
  *@param noteAr An array of all the notes in the entire song
  *@param totalNotes The number of notes in the song
  *@param info contains ppqn, tempo and mode
  */
void findMode(note noteAr[], int totalNotes, globalMelodyInfo *info){
  int majors[12] = {1}, minors[12] = {0}, x = 0, y = 0, z = 0, bar[4],
      sizeBar = 4, tempSpan = 999, span = 999, keynote = 0, mode = 0, tempNote = 0;

  for(x = 0; x < totalNotes; x++){
    tempNote = noteAr[x].tone;
    
    for(y = C; y <= B; y++)
      if(majors[y])
        checkScale(majors, tempNote, y);
  }

  for(y = 0; y < 12; y++){
    z = y;
    
    if(majors[z]){
      if((z - 3) < 0)
        z += 12;
      
      minors[z-3] = 1;
    }
  }

  z = 0;  x = 0;

  /*Goes through all notes of the song and puts them into an array, 4 at a time*/
  while(x < totalNotes){
    z = x;
    
    for(y = 0; y < sizeBar; y++, z++){
      if(z < totalNotes)
        bar[y] = noteAr[z].tone;
      else
        sizeBar = y;
    }
    
    if(y == sizeBar){
      span = 999;
      /*Sort notes in ascending order*/
      qsort(bar, sizeBar, sizeof(tone), sortTones);

      /*Finds the lowest possible tonespan over the array of 4 notes*/
      for(z = 0; z < sizeBar; z++){
	      if((z + 1) > 3)
          tempSpan = (bar[(z+1)%4]+12)-bar[z] + bar[(z+2)%4]-bar[(z+1)%4] + bar[(z+3)%4]-bar[(z+2)%4];
        else if((z + 2) > 3)
          tempSpan = bar[(z+1)]-bar[z] + (bar[(z+2)%4]+12)-bar[(z+1)%4] + bar[(z+3)%4]-bar[(z+2)%4];
	      else if((z +3) > 3)
          tempSpan = bar[(z+1)]-bar[z] + bar[(z+2)]-bar[(z+1)] + (bar[(z+3)%4]+12)-bar[z];
	      else
          tempSpan = bar[(z+1)]-bar[z] + bar[(z+2)]-bar[(z+1)] + bar[(z+3)]-bar[(z+2)];
	      if(tempSpan < span && (majors[bar[z]] || minors[bar[z]])){
          span = tempSpan;
          keynote = bar[z];
        }
      }
      
      mode += isInScale(keynote, bar, sizeBar);
      x++;
    }
  }
  
  /*outputs result directly to the info struct array*/
  if(mode > 0)
    info->mode = major;
  else if(mode < 0)
    info->mode = minor;
}

/**A function to check if a given scale in given keytone corresponds with the tones in the rest of the song.
  *@param keytone The keytone of the processed scale
  *@param otherTones An array of the rest of the tones, which the function compares to the keytone and mode
  *@param size The number of tones in the otherTones array
  *@return a boolean value, returns 1 if the mode is major, -1 if it's minor and 0, if wasn't possible to decide.
  */
int isInScale(int keytone, int otherTones[], int size){
  int toneLeap, isMinor = 1, isMajor = 1;

  for(int i = 0; i < size; i++){
    if(otherTones[i] < keytone)
      otherTones[i] += 12;
    
    toneLeap = otherTones[i] - keytone;
    
    if(isMinor)
      isMinor = isInMinor(toneLeap);
    
    if(isMajor)
      isMajor = isInMajor(toneLeap);
  }
  
  if(isMinor && isMajor)
    return 0;
  else if(isMinor)
    return -1;
  else if(isMajor)
    return 1;
  
  return 0;
}

/**A function to check if the given tone leap is in the minor scale.
  *@param toneLeap An integer describing the processed tone leap
  *@return a boolean value, returns 1 if the tone leap is in the minor scale, 0 if it's not.
  */
int isInMinor(int toneLeap){
  int minor[] = {0, 2, 3, 5, 7, 8, 10};

  for(int i = 0; i < SCALESIZE; i++)
    if(toneLeap == minor[i])
      return 1;
  
  return 0;
}

/**A function to check if the given tone leap is in the major scale.
  *@param toneLeap An integer describing the processed tone leap
  *@return a boolean value, returns 1 if the tone leap is in the major scale, 0 if it's not.
  */
int isInMajor(int toneLeap){
  int major[] = {0, 2, 4, 5, 7, 9, 11};

  for(int i = 0; i < SCALESIZE; i++)
    if(toneLeap == major[i])
      return 1;
  
  return 0;
}

/**Returns the amount of moods written in the moods text file
  */
int FindMoodAmount(FILE *moods){
  int i = 1;
  
  while(fgetc(moods) != EOF)
    if(fgetc(moods) == '\n')
      i++;
  
  rewind(moods);
  return i;
}

/**Prints relevant information about the song. Finds and prints the mood with the highest score,
  *and in the case of using the default sad/happy scale, scales the values to fit on the 51
  *point sliding scale
  */
void printResults(int mode, int tempo, int toneLength, int pitch, moodWeighting moodArray[], int result[]){
  int moodOfMelody = 0;
  
  for(int i = 0; i < AMOUNT_OF_MOODS; i++)
    if(moodOfMelody < result[i])
      moodOfMelody = i;

  printParameterVector(mode, tempo, toneLength, pitch);
  printWeightingMatrix(moodArray);
  printVectorMatrixProduct(moodArray, mode, tempo, toneLength, pitch, result);
  printHappySadScale(moodArray, moodOfMelody, result);
  printMoodOfMelody(moodArray, moodOfMelody);
}

/**Prints the parameter vector into the console
  */
void printParameterVector(int mode, int tempo, int toneLength, int pitch){
  printf("\n\n\n");
  printf(" Mode:");
  printf("%10d\n", mode);
  printf(" Tempo:");
  printf("%9d\n", tempo);
  printf(" Tone length:");
  printf("%3d\n", toneLength);
  printf(" Pitch:");
  printf("%9d\n", pitch);
}

/**Prints the weighting matrix into the console
  */
void printWeightingMatrix(moodWeighting moodArray[]){
  printf("\n\n\n                             WEIGHTINGS\n");
  printf("                 Mode | Tempo | Tone length | Pitch\n");
  
  for(int i = 0; i < AMOUNT_OF_MOODS; i++){
    printf(" %s", moodArray[i].name);
    for(int j = strlen(moodArray[i].name); j < 16; j++)
      printf(" ");
    if(moodArray[i].mode > -1)
      printf(" ");
    printf(" %d", moodArray[i].mode);
    for(int j = 0; j < 2; j++)
      printf(" ");
    printf("| ");
    if(moodArray[i].tempo > -1)
      printf(" ");
    printf(" %d", moodArray[i].tempo);
    for(int j = 0; j < 3; j++)
      printf(" ");
    printf("|    ");
    if(moodArray[i].toneLength > -1)
      printf(" ");
    printf(" %d", moodArray[i].toneLength);
    for(int j = 0; j < 6; j++)
      printf(" ");
    printf("|  ");
    if(moodArray[i].pitch > -1)
      printf(" ");
    printf(" %d\n", moodArray[i].pitch);
  }
  printf("\n\n\n");
}

/**Prints the matrix vector product calculation for each mood and their results
  */
void printVectorMatrixProduct(moodWeighting moodArray[], int mode, int tempo, int toneLength, int pitch, int result[]){
  for(int i = 0; i < AMOUNT_OF_MOODS; i++){
    printf(" %s", moodArray[i].name);
    for(int j = strlen(moodArray[i].name); j < 16; j++)
      printf(" ");
    if(mode < 0)
      printf(" %d * ", mode);
    else
      printf(" %d * ", mode);
    if(moodArray[i].mode < 0)
      printf("%d + ", moodArray[i].mode);
    else
      printf(" %d + ", moodArray[i].mode);
    if(tempo < 0)
      printf("%d * ", tempo);
    else
      printf(" %d * ", tempo);
    if(moodArray[i].tempo < 0)
      printf("%d + ", moodArray[i].tempo);
    else
      printf(" %d + ", moodArray[i].tempo);
    if(toneLength < 0)
      printf("%d * ", toneLength);
    else
      printf(" %d * ", toneLength);
    if(moodArray[i].toneLength < 0)
      printf("%d + ", moodArray[i].toneLength);
    else
      printf(" %d + ", moodArray[i].toneLength);
    if(pitch < 0)
      printf("%d * ", pitch);
    else
      printf(" %d * ", pitch);
    if(moodArray[i].pitch < 0)
      printf("%d = ", moodArray[i].pitch);
    else
      printf(" %d = ", moodArray[i].pitch);
    if(result[i] < 0)
      printf("%d\n", result[i]);
    else
      printf(" %d\n", result[i]);
  }
}

/**If there only is the two moods happy and sad it prints a scale on which there is indicated
  *if it is most happy or sad and how much
  */
void printHappySadScale(moodWeighting moodArray[], int moodOfMelody, int result[]){
  int test = 0;
  if(!strcmp(moodArray[moodOfMelody].name, "Happy") && !strcmp(moodArray[0].name, "Happy") &&
     !strcmp(moodArray[1].name, "Sad")              && AMOUNT_OF_MOODS == 2){
    printf("\n\n\n Sad ");
    
    while(test < 51){
      if(test == 25)
        printf("|");
      else if(test == ((result[moodOfMelody] / 2) + 26))
        printf("[]");
      else
        printf("-");
      
      test++;
    }
    
    printf(" Happy\n\n\n");
  }
  else if(!strcmp(moodArray[moodOfMelody].name, "Sad") && !strcmp(moodArray[0].name, "Happy") &&
          !strcmp(moodArray[1].name, "Sad")            && AMOUNT_OF_MOODS == 2){
    printf("\n\n\n Sad ");
    
    while(test < 51){
      if(test == 25)
        printf("|");
      else if(test == ((int)(-((result[moodOfMelody]) / 2.4)) + 26))
        printf("[]");
      else
        printf("-");
      
      test++;
    }
    
    printf(" Happy\n\n\n");
  } 
}

/**Prints the mood of the melody into the console
  */
void printMoodOfMelody(moodWeighting moodArray[], int moodOfMelody){
  printf("\n The mood of the melody is %s\n", moodArray[moodOfMelody].name);
}