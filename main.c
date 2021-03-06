/*Mood analysing of MIDI-files
 * Gruppe A412
 * Lee Paludan
 * Simon Madsen
 * Jonas Stolberg
 * Jacob Mortensen
 * Esben Kirkegaard
 * Arne Rasmussen
 * Tobias Morell
*/

/* Headers */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>

/* Defines */
#define CHARS 1000
#define SCALESIZE 7
#define META_EVENT 0xff
#define TEMPO_EVENT_BYTE_1 0x51
#define TEMPO_EVENT_BYTE_2 0x03
#define CONVERT_THIRD_BYTE 16
#define CONVERT_SECOND_BYTE 8
#define CONVERT_TO_TICKS 7
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
#define HEX_90 0x90
#define LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME 3
#define LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME 2
#define MAX_LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME_END 7
#define MAX_LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME_END 6

/*Enums and structs*/
typedef enum mode {major, minor} mode;
typedef enum tone {C, Csharp, D, Dsharp, E, F, Fsharp, G, Gsharp, A, Asharp, B} tone;
typedef enum mood {glad, sad} mood;

/**A struct containing data about a single note
  *@param tone the tone stored as an integer (C = 0)
  *@param octave which octave, on a piano, the note is in (0 is the deepest, C4 is middle C)
  *@param length the note length in standard musical notation
  *@param noteValue used in calculating the average tone
  *@param ticks the note length in ticks
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
  *@param ppqn pulses-per-quarter-note contains the number of ticks per quarter note
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
  *@param controlChange used for a large number of effects, none of which are used in this project,
  *but stored to find deltatimes
  *@param programChange signals instrument change (not used; stored to find deltatimes)
  *@param channelPressure changes velocity for all notes on a specific channel
  *akin to a global afterTouch
  *@param pitchWheel fine tuning of pitch for all notes on a specific channel
  *similar to channelPressure, but for pitch
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
void checkForError(void *);
void printDirectory(DIR*);
void chooseTrack(char*, DIR*);
int readAndInsertMIDIFile(FILE*, int[]);
void fillSongData(globalMelodyInfo*, int[], int);
int countPotentialNotes(int[], int);
void fillNote(int, note*);
void settingPoints(int*, int*, int*, int*, globalMelodyInfo, int, note []);
void modePoints(globalMelodyInfo, int*);
void tempoPoints(globalMelodyInfo, int*);
void lengthPoints(int*, note [], int);
void pitchPoints(int*, note [], int);
void insertMoods(moodWeighting [], FILE*, int);
void weightingMatrix(moodWeighting [], int, int, int, int, int*, int);
void findEvents(int, int [], eventPlacement [], note [], int*);
void insertPlacementWhenTwoParameters(int [], int*, int, note [], int*);
void insertPlacementWhenOneParameter(int [], int*, int);
int isNextByteAnEvent(int [], int);
void findTicks(int, int [], eventPlacement [], note [], int);
void countTicksWhenTwoParameters(int [], int*, note [], int*);
void countTicksWhenOneParameter(int [], int*, note [], int*);
void deltaTimeToNoteLength(int, int, note*);
void findMode(note*, int, globalMelodyInfo*);
void checkScale(int[], int, int);
int isInScale(int, int[], int);
int isInMinor(int);
int isInMajor(int);
int sortToner(const void*, const void*);
void checkScalesForToneleaps(int [], int [], int, note[]);
void checkMelodyScale(int [], int [], int, note [], int*);
void returnToStruct(int, globalMelodyInfo *);
int findMoodAmount(FILE*);
void printResults(int, int, int, int, moodWeighting [], int [], int);
void printParameterVector(int, int, int, int);
void printWeightingMatrix(moodWeighting [], int);
void printVectorMatrixProduct(moodWeighting [], int, int, int, int, int [], int);
void printHappySadScale(moodWeighting [], int, int [], int);
void printMoodOfMelody(moodWeighting [], int);

int main(int argc, const char *argv[]){
  /* Variables */
  int numbersInText = 0, potentialNotes = 0, mode = 0, tempo = 0, toneLength = 0, pitch = 0,
      amountOfNotes = 0, amountOfMoods = 0;
  FILE* moods = fopen("moods.txt", "r");
  checkForError((void*) moods);
  char MIDIfile[25];
  DIR *dir = 0;
  FILE *f;
  eventPlacement *placement;
  amountOfMoods = findMoodAmount(moods);
  moodWeighting moodArray[amountOfMoods];
  globalMelodyInfo info;
  int *byteAr = (int *) malloc(CHARS * sizeof(int));
  checkForError((void*) byteAr);
  
  /* User input and error check */
  if (argv[1] == NULL){
    printDirectory(dir);
    chooseTrack(MIDIfile, dir);
    f = fopen(MIDIfile, "r");  
    checkForError((void*) f);
  }
  else if(argv[1] != NULL){
    f = fopen(argv[1],"r");
    checkForError((void*) f);
  }
    
  /* Reading the data from the file */
  numbersInText = readAndInsertMIDIFile(f, byteAr);
  fillSongData(&info, byteAr, numbersInText);
  potentialNotes = countPotentialNotes(byteAr, numbersInText);
  
  /* Arrays */
  note *noteAr = (note*) malloc(potentialNotes * sizeof(note));
  checkForError((void*) noteAr);
  placement = (eventPlacement*) malloc(potentialNotes * sizeof(eventPlacement));
  if(placement == NULL){
    printf("Error in allocating space for events\n");
    exit(EXIT_FAILURE);
  }
  int result[amountOfMoods];

  /* Load files, calculate points and processes points */
  findEvents(numbersInText, byteAr, placement, noteAr, &amountOfNotes);
  deltaTimeToNoteLength(info.ppqn, amountOfNotes, noteAr);
  insertMoods(moodArray, moods, amountOfMoods);
  findMode(noteAr, amountOfNotes, &info);
  settingPoints(&mode, &tempo, &toneLength, &pitch, info, amountOfNotes, noteAr);
  weightingMatrix(moodArray, mode, tempo, toneLength, pitch, result, amountOfMoods);

  /* Print results */
  printResults(mode, tempo, toneLength, pitch, moodArray, result, amountOfMoods);

  /*Clean up and close*/
  fclose(f);
  fclose(moods);
  closedir(dir);
  free(placement);
  free(byteAr);
  free(noteAr);

  return 0;
}

/**Error check for integer array allocation, exits program if an error is found
  *@param allocArray a void-pointer to the array or file the program processing
  */
void checkForError(void *allocArray){
  if(allocArray == NULL){
    printf("There was an error, exiting\n!");
    exit(EXIT_FAILURE);
  }
}

/**A function to read music directory and prompt user to choose file
  *@param dir a pointer to a directory
  */
void printDirectory(DIR *dir){
  struct dirent *musicDir;
  int musicNumber = -2;

  if((dir = opendir ("./Music")) != NULL) {
    printf(" Possible tracks\n");
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
}

/**A function that opens the Music folder found in the program directory
  *Also prompts for user input and scans for the desired song
  *@param MIDIfile the name of the chosen MIDI-file
  *@param dir a pointer to the directory in which the MIDI-files are found
  */
void chooseTrack(char *MIDIfile, DIR *dir){
  struct dirent *musicDir;
  int musicNumber = 0;

  if((dir = opendir ("./Music")) != NULL) {
    printf("\n Choose a track number\n ");
    scanf(" %d", &musicNumber);

    for(int i = -2; i <= musicNumber; i++)
      if((musicDir = readdir (dir)) != NULL && i == (musicNumber))
        strcpy(MIDIfile, musicDir->d_name);

    printf("\n You chose \n %s\n Which gives these results\n", MIDIfile);
  } 
  else{
    perror ("Failure while opening directory");
    exit (EXIT_FAILURE);
  }

  chdir("./Music");
}

/**A function, that retrieves the bytes from the MIDI-file and
  *also returns the number of bytes in the file
  *@param *f a pointer to the file the program is reading from
  *@param byteAr[] an array of integers, that the information is stored in
  *@return i the number of bytes in the file
  */
int readAndInsertMIDIFile(FILE *f, int byteAr[]){
  int i = 0, c;
  
  while( (c = fgetc(f)) != EOF && i < CHARS){
    byteAr[i] = c;
    i++;
  }

  return i;
}

/**A function to count the number of notes in the entire song
  *@param byteAr[] an array with the stored information from the file
  *@param amount an integer holding the total number of characters in the array
  *@return res the maximum number of possible notes in the file
  */
int countPotentialNotes(int byteAr[], int amount){
  int i = 0, res = 0;
  
  for(i = 0; i < amount; i++){
    if(byteAr[i] == HEX_90){
      res++;
    }
  }
  
  return res;
}

/**A function, that inserts ppqn and tempo to the info array
  *@param *info a pointer to a structure containing the tempo and mode of the song
  *@param byteAr[] the array of integers read from the file
  *@param numbersInText the total amount of integers in the array
  */
void fillSongData(globalMelodyInfo *info, int byteAr[], int numbersInText){
  info->ppqn = (byteAr[12] << CONVERT_SECOND_BYTE) + byteAr[13];
  
  for(int j = 0; j < numbersInText; j++)
    /* finds the tempo */
    if(byteAr[j] == META_EVENT && byteAr[j+1] == TEMPO_EVENT_BYTE_1 && byteAr[j+2] == TEMPO_EVENT_BYTE_2)
      info->tempo =  MICRO_SECONDS_PER_MINUTE/((byteAr[j+3] << CONVERT_THIRD_BYTE) | (byteAr[j+4] << CONVERT_SECOND_BYTE) |
                     (byteAr[j+5]));
}

/**Searches the file for events and stores their placement in an array of eventPlacement structs
  *@param numbersInText the total amount of numbers in the MIDI-file
  *@param byteAr an array containing all the data from the MIDI-file
  *@param placement an array storing the places of each event
  *@param noteAr an array containing all the notes in the song
  *@param amountOfNotes the maximum possible amount of notes in the song
  */
void findEvents(int numbersInText, int byteAr[], eventPlacement placement[], note noteAr[],
                                                               int *amountOfNotes){
  int noteOff = 0, noteOn = 0, afterTouch = 0, controlChange = 0,
      programChange = 0, channelPressure = 0, pitchWheel = 0;
  
  for(int j = 0; j < numbersInText; j++)
    switch (byteAr[j]){
      case NOTE_ON         : insertPlacementWhenTwoParameters(byteAr, &placement[noteOn++].noteOn, j,
                                              noteAr, amountOfNotes);
                             break;
      case NOTE_OFF        : insertPlacementWhenTwoParameters(byteAr, &placement[noteOff++].noteOff, j,
                                              noteAr, amountOfNotes);
                             break;
      case AFTER_TOUCH     : insertPlacementWhenTwoParameters(byteAr, &placement[afterTouch++].afterTouch, j,
                                              noteAr, amountOfNotes);
                             break;
      case CONTROL_CHANGE  : insertPlacementWhenTwoParameters(byteAr, &placement[controlChange++].controlChange, j,
                                              noteAr, amountOfNotes);
                             break;
      case PROGRAM_CHANGE  : insertPlacementWhenOneParameter(byteAr, &placement[programChange++].programChange, j);
                             break;
      case CHANNEL_PRESSURE: insertPlacementWhenOneParameter(byteAr, &placement[channelPressure++].channelPressure, j);
                             break;
      case PITCH_WHEEL     : insertPlacementWhenTwoParameters(byteAr, &placement[pitchWheel++].pitchWheel, j,
                                              noteAr, amountOfNotes);
                             break;
      default              : break;
    }
  findTicks(numbersInText, byteAr, placement, noteAr, noteOn);
}

/**Used to determine if an event with two parameters truly is an event-start
  *@param byteAr an array containing all values of the bytes in the MIDI-file
  *@param place a pointer to the index of the found event in the byte-array
  *@param j the place on which the event is found in the byte-array
  *@param noteAr the array of notes in the song is stored here
  *@param amountOfNotes an integer, that counts the amount of notes
  */
void insertPlacementWhenTwoParameters(int byteAr[], int *place, int j, note noteAr[], int *amountOfNotes){
  int i = LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME;
  
  while(i < MAX_LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME_END && byteAr[(j + i++)] > NOTE_OFF);
  
  if(isNextByteAnEvent(byteAr, (j + i))){
    *place = j;
    if(byteAr[j] == NOTE_ON){
      fillNote(byteAr[j + 1], &noteAr[*amountOfNotes]);
      *amountOfNotes += 1;
    }   
  } 
}

/**Does the same as insertPlacementWhenTwoParameters, but for the two events that only takes one parameter
  *@param byteAr the array of all the bytes in the MIDI-file
  *@param place 
  *@param j the place on which the event is found
  */
void insertPlacementWhenOneParameter(int byteAr[], int *place, int j){
  int i = LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME;
  
  while(i < MAX_LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME_END && byteAr[(j + i++)] > NOTE_OFF);
  
  if(isNextByteAnEvent(byteAr, (j + i)))
    *place = j;
}

/**Returns a boolean value, used to check if the given place is an event
  *@param byteAr an array of all decimals in the MIDI-file
  *@param j the place where the function looks for the event
  *@return returns 1 if it's an event, 0 if it's not.
  */
int isNextByteAnEvent(int byteAr[], int j){
  switch (byteAr[j]){
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

/**Analyses ticks for every noteOn event by a for loop which begins in the noteOn events start and
  *searches until the note-off event is found
  *@param numbersInText the total amount of numbers in the MIDI-file
  *@param byteAr an array containing all the decimals in the MIDI-file
  *@param placement an array of 
  *@param noteAr the array of notes in the song is stored here
  *@param noteOn the total amount of note-on events in the MIDI-file
  */
void findTicks(int numbersInText, int byteAr[], eventPlacement placement[], note noteAr[], int noteOn){
  int tickCounter = 0;
  
  for(int j = 0; j < noteOn; j++){
    for(int i = placement[j].noteOn; i < numbersInText; i++){
      if(byteAr[i] == NOTE_OFF){
        if(byteAr[i + 1] == noteAr[j].noteValue){
          tickCounter++;
          break;
        }
        else
          countTicksWhenTwoParameters(byteAr, &i, noteAr, &tickCounter);
      }
      else if(byteAr[i] == AFTER_TOUCH){
        if(byteAr[i + 1] == noteAr[j].noteValue && byteAr[i + 2] == ZERO){
          tickCounter++;
          break;
        }
        else
          countTicksWhenTwoParameters(byteAr, &i, noteAr, &tickCounter);
      }
      else if(byteAr[i] == CHANNEL_PRESSURE){
        if(byteAr[i + 1] == ZERO){
          tickCounter++;
          break;
        }  
        else
          countTicksWhenOneParameter(byteAr, &i, noteAr, &tickCounter);
      }
      else if(byteAr[i] == PROGRAM_CHANGE)
        countTicksWhenOneParameter(byteAr, &i, noteAr, &tickCounter);
      else
        countTicksWhenTwoParameters(byteAr, &i, noteAr, &tickCounter);   
    }
  }
}

/**A function to count the deltatime ticks of a given event with two parameters
  *@param byteAr an array containing all the data from the MIDI-file
  *@param i a pointer to the place of the prosessed event, increased here to the start of next event
  *@param deltaCounter the distance between eventstart and deltatime start
  *@param noteAr is an array containing all notes of the MIDI-file
  *@param tickCounter used as an index the correct note
  */
void countTicksWhenTwoParameters(int byteAr[], int *i, note noteAr[], int *tickCounter){
  noteAr[*tickCounter].ticks = 0;
  int tick = 0, deltaCounter = LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME;
  
  while(deltaCounter < MAX_LENGTH_FROM_TWO_PARAMETER_EVENT_START_TO_DELTA_TIME_END &&
        byteAr[(*i + deltaCounter)] > HEX_80)
    tick += ((byteAr[(*i + deltaCounter++)] - HEX_80) << CONVERT_TO_TICKS);
  
  tick += byteAr[(*i + deltaCounter)];
  noteAr[*tickCounter].ticks += tick;
  *i += deltaCounter;
}

/**A function to count the deltatime ticks of a given event with one parameter
  *@param byteAr an array containing all the data from the MIDI-file
  *@param i a pointer to the place of the prosessed event, increased here to the start of next event
  *@param deltaCounter the distance between eventstart and deltatime start
  *@param noteAr is an array containing all notes of the MIDI-file
  *@param tickCounter used as an index the correct note
  */
void countTicksWhenOneParameter(int byteAr[], int *i, note noteAr[], int *tickCounter){
  noteAr[*tickCounter].ticks = 0;
  int tick = 0, deltaCounter = LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME;
  
  while(deltaCounter < MAX_LENGTH_FROM_ONE_PARAMETER_EVENT_START_TO_DELTA_TIME_END &&
        byteAr[(*i + deltaCounter)] > HEX_80)
    tick += ((byteAr[(*i + deltaCounter++)] - HEX_80) << CONVERT_TO_TICKS);
  
  tick += byteAr[(*i + deltaCounter)];
  noteAr[*tickCounter].ticks += tick;
  *i += deltaCounter;
}

/**A function to fill out each of the structures of type note
  *@param inputTone the bytevalue of the collected on the "tone"-byte
  *@param note* a pointer to a note-structure
*/
void fillNote(int inputTone, note *note){
  note->tone = inputTone % 12;
  note->noteValue = inputTone;
  note->octave = inputTone / 12;
}

/**A function to insert points into integers based on the data pulled from the file
 *@param mode, along with tempo, length and octave contains the points for the Melody
 *@param info contains the song ppqn, tempo and mode for the song
 *@param notes contains the amount of notes in the song
 *@param noteAr the array of notes in the song is stored here
 */
void settingPoints(int *mode, int *tempo, int *length, int *octave,
                   globalMelodyInfo info,int notes, note noteAr[]){
  modePoints(info, mode);
  tempoPoints(info, tempo);
  lengthPoints(length, noteAr, notes);
  pitchPoints(octave, noteAr, notes);
}

/**Changes the value of the integer mode depending on the mode of the melody
  *@param info contains the song ppqn, tempo and mode for the song
  *@param mode contains the points for mode in the Melody
  */
void modePoints(globalMelodyInfo info, int *mode){
  switch(info.mode){
    case minor: *mode = -5; break;
    case major: *mode = 5;  break;
    default   : *mode = 0;  break;
  }
}

/**Changes the value of the integer tempo depending on the tempo of the melody
  *@param info contains the song ppqn, tempo and mode for the song
  *@param tempo contains the points for tempo in the Melody
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
  *@param length contains the points for length in the Melody
  *@param noteAr the array of notes in the song is stored here
  *@param notes the amount of notes in the file
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
  *@param octave is a pointer to an integer containing the octave points
  *@param noteAr the array of notes in the song is stored here
  *@param notes the amount of notes in the file
  */
void pitchPoints(int *octave, note noteAr[], int notes){
  int combined = 0;
  for (int i = 0; i < notes; i++)
    combined += noteAr[i].noteValue;
  
  int averageNotePitch = combined/notes;

  if(averageNotePitch <= 16)
    *octave = -5;
  else if(averageNotePitch >= 17 && averageNotePitch <= 23)
    *octave = -4;
  else if(averageNotePitch >= 24 && averageNotePitch <= 30)
    *octave = -3;
  else if(averageNotePitch >= 31 && averageNotePitch <= 37)
    *octave = -2;
  else if(averageNotePitch >= 38 && averageNotePitch <= 44)
    *octave = -1;
  else if(averageNotePitch >= 45 && averageNotePitch <= 51)
    *octave = 0;
  else if(averageNotePitch >= 52 && averageNotePitch <= 58)
    *octave = 1;
  else if(averageNotePitch >= 59 && averageNotePitch <= 65)
    *octave = 2;
  else if(averageNotePitch >= 66 && averageNotePitch <= 72)
    *octave = 3;
  else if(averageNotePitch >= 73 && averageNotePitch <= 79)
    *octave = 4;
  else if(averageNotePitch >=80)
    *octave = 5;
}

/**Inserts the weighting of each mood in an array of structs, as read from a designated file.
  *@param moodArray The array moods are stored in
  *@param moods the file to be read
  *@param amountOfMoods the amount of moods in the mood tekst file
  */
void insertMoods(moodWeighting moodArray[], FILE* moods, int amountOfMoods){
  int scans = 0;
  
  for(int i = 0; i < amountOfMoods; i++){
    scans = fscanf(moods, "%s %d %d %d %d", moodArray[i].name , &moodArray[i].mode, 
                                            &moodArray[i].tempo, &moodArray[i].toneLength,
                                            &moodArray[i].pitch);
   
    /* checks if moods-file is read correctly */
    if(scans < 5){
      printf("Problems reading the moods-file, closing\n");
      exit(EXIT_FAILURE);
    }
  }
}

/**Vector matrix multiplication. Receives an array of moods, the various parameters of the song and a
  *pointer to an array where the results will be stored. The song data is multiplied onto each moods
  *weighting and then stored.
  *@param moodArray an array containing the weighting for all moods
  *@param result an array for holding the songs scores as per each mood
  *@param mode along with tempo, toneLength and pitch, this variable contains a score -5 to 5
  *@param amountOfMoods the amount of moods in the mood tekst file
  */
void weightingMatrix(moodWeighting moodArray[], int mode, int tempo, int toneLength, int pitch,
                     int *result, int amountOfMoods){
  for(int i = 0; i < amountOfMoods; i++)
    result[i] = 0;
  
  for(int i = 0; i < amountOfMoods; i++){
    result[i] += (moodArray[i].mode * mode);
    result[i] += (moodArray[i].tempo * tempo);
    result[i] += (moodArray[i].toneLength * toneLength);
    result[i] += (moodArray[i].pitch * pitch);
  }
}

/**Finds the note length, converted from deltatime to standard musical notation
  *after recieving the noteLength result, we round it to the nearest 32nd note
  *@param ppqn is the PPQN value for the MIDI-file.
  *@param size is the size of the array, used to prevent a segmentation fault.
  *@param noteAr is a pointer to the beginning of the array containing all the notes.
  */
void deltaTimeToNoteLength (int ppqn, int numberOfNotes, note *noteAr){
  for (int i = 0; i < numberOfNotes; i++){
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
  *@param a a constant void pointer used here to point to the tone value
  *@param b a constant void pointer used here to point to the tone value of another tone
  */
int sortTones(const void *a, const void *b){
  return (*(int *)a - *(int *)b);
}

/**A function to find the mode of the song by first calculating the tone span over sets of notes
  *in the song and then comparing it to the definition of minor and major keys
  *@param noteAr An array of all the notes in the entire song
  *@param totalNotes The number of notes in the song
  *@param info contains ppqn, tempo and mode
  */
void findMode(note noteAr[], int totalNotes, globalMelodyInfo *info){
  int majors[12] = {1,1,1,1,1,1,1,1,1,1,1,1}, minors[12] = {0,0,0,0,0,0,0,0,0,0,0,0}, mode = 0;

  checkScalesForToneleaps(majors, minors, totalNotes, noteAr);
  checkMelodyScale(majors, minors, totalNotes, noteAr, &mode);
  returnToStruct(mode, info);
}

/**A function to check which major scales match the tones of the song and open their relative minors
  *@param majors is an array containing all possible majorscales
  *@param minors is an array containing all possible minorscales
  *@param totalNotes is a variable with the value equal to the sum of all notes 
  *@param noteAr is an array containing all notes
  */
void checkScalesForToneleaps(int majors[], int minors[], int totalNotes, note noteAr[]){
  int x = 0, y = 0, z = 0, tempNote = 0;
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

/**Goes through all notes of the song and puts them into an array, 4 at a time
  *@param majors is an array containing all possible majorscales
  *@param minors is an array containing all possible minorscales
  *@param totalNotes is a variable with the value equal to the sum of all notes
  *@param noteAr is an array containing all the notes
  *@param mode is an integer defining the scale. A positive value indicates a major scale
  *and a negative value indicates a minor scale.
  */
void checkMelodyScale(int majors[], int minors[], int totalNotes, note noteAr[], int *mode){
  int x = 0, y = 0, z = 0, bar[4], sizeBar = 4, tempSpan = 999, span = 999, keynote = 0;
    
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
      *mode += isInScale(keynote, bar, sizeBar);
      x++;
    }
  }
}

/**A function to check if a given scale in given keytone corresponds with the tones in the rest
  *of the song
  *@param keytone The keytone of the processed scale
  *@param otherTones An array of the rest of the tones, which the function compares to the keytone
  *and mode
  *@param size The number of tones in the otherTones array
  *@return a boolean value, returns 1 if the mode is major, -1 if it's minor and 0,
  *if wasn't possible to decide
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

/**A function to check if the given tone leap is in the minor scale
  *@param toneLeap An integer describing the processed tone leap
  *@return a boolean value, returns 1 if the tone leap is in the minor scale, 0 if it's not
  */
int isInMinor(int toneLeap){
  int minor[] = {0, 2, 3, 5, 7, 8, 10};

  for(int i = 0; i < SCALESIZE; i++)
    if(toneLeap == minor[i])
      return 1;
  
  return 0;
}

/**A function to check if the given tone leap is in the major scale
  *@param toneLeap An integer describing the processed tone leap
  *@return a boolean value, returns 1 if the tone leap is in the major scale, 0 if it's not
  */
int isInMajor(int toneLeap){
  int major[] = {0, 2, 4, 5, 7, 9, 11};

  for(int i = 0; i < SCALESIZE; i++)
    if(toneLeap == major[i])
      return 1;
  
  return 0;
}

/**outputs result directly to the info struct array
  *@param the mode integer contains the point value of the mode result
	*@param info is used to change the mode value
  */
void returnToStruct(int mode, globalMelodyInfo *info){
  if(mode > 0)
    info->mode = major;
  else if(mode < 0)
    info->mode = minor;
}

/**Returns the amount of moods written in the moods text file
  *@param moods is a pointer to the file containing mood data 
  *@return returns the amount of specified moods in the "moods.txt" file
  */
int findMoodAmount(FILE *moods){
  int i = 1;
  
  while(fgetc(moods) != EOF)
    if(fgetc(moods) == '\n')
      i++;
  
  rewind(moods);
  return i;
}

/**Prints relevant information about the song. 
  *@param mode integer contains the point value of the mode result
  *@param tempo integer contains the point value of the tempo result
  *@param toneLength integer contains the point value of the toneLength result
  *@param pitch integer contains the point value of the pitch result
  *@param moodArray contains the weighting matrix content
  *@param vectorMatrixResult is an array containing the results from the vector matrix product
  *@param amountOfMoods is an integer equal to the amount of moods in the moods.txt
  */
void printResults(int mode, int tempo, int toneLength, int pitch, moodWeighting moodArray[],
                  int vectorMatrixResult[], int amountOfMoods){
  int moodOfMelody = 0;
  
  for(int i = 0; i < amountOfMoods; i++)
    if(moodOfMelody < vectorMatrixResult[i])
      moodOfMelody = i;

  printParameterVector(mode, tempo, toneLength, pitch);
  printWeightingMatrix(moodArray, amountOfMoods);
  printVectorMatrixProduct(moodArray, mode, tempo, toneLength, pitch, vectorMatrixResult, amountOfMoods);
  printHappySadScale(moodArray, moodOfMelody, vectorMatrixResult, amountOfMoods);
  printMoodOfMelody(moodArray, moodOfMelody);
}

/**Prints the parameter vector into the console
  *@param mode integer contains the point value of the mode result
  *@param tempo integer contains the point value of the tempo result
  *@param toneLength integer contains the point value of the toneLength result
  *@param pitch integer contains the point value of the pitch result
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
  *@param moodArray contains the weighting matrix content
  *@param amountOfMoods is an integer equal to the amount of moods in the moods.txt
	*/
void printWeightingMatrix(moodWeighting moodArray[], int amountOfMoods){
  printf("\n\n\n                             WEIGHTINGS\n");
  printf("                 Mode | Tempo | Tone length | Pitch\n");
  
  for(int i = 0; i < amountOfMoods; i++){
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
  *@param moodArray is an array containing the moods
  *@param mode is an integer defining what mode the melody has 
  *@param tempo is an integer defining the the amount of BPM
  *@param toneLength is an integer defining the toneLengths in the song
  *@param pitch is an integer defining the octaves in the song
  *@param vectorMatrixResult is an array containing the results from the vector matrix product
  *@param amountOfMoods is an integer equal to the amount of moods in the moods.txt
  */
void printVectorMatrixProduct(moodWeighting moodArray[], int mode, int tempo, int toneLength,
                              int pitch, int vectorMatrixResult[], int amountOfMoods){
  for(int i = 0; i < amountOfMoods; i++){
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
    if(vectorMatrixResult[i] < 0)
      printf("%d\n", vectorMatrixResult[i]);
    else
      printf(" %d\n", vectorMatrixResult[i]);
  }
}

/**If there only is the two moods happy and sad it prints a scale on which there is indicated
  *if it is most happy or sad and how much
  *@param moodArray is an array that contains the moods
  *@param moodOfMelody is an integer that defines which mood the melody is
  *@param vectorMatrixResult is an array containing the results from the vector matrix product
  *@param amountOfMoods is an integer equal to the amount of moods in the moods.txt
  */
void printHappySadScale(moodWeighting moodArray[], int moodOfMelody, int vectorMatrixResult[], int amountOfMoods){
  int test = 0;
  if(!strcmp(moodArray[moodOfMelody].name, "Happy") && !strcmp(moodArray[0].name, "Happy") &&
     !strcmp(moodArray[1].name, "Sad")              && amountOfMoods == 2){
    printf("\n\n\n Sad ");
    
    while(test < 51){
      if(test == 25)
        printf("|");
      else if(test == ((vectorMatrixResult[moodOfMelody] / 2) + 26))
        printf("[]");
      else
        printf("-");
      
      test++;
    }
    
    printf(" Happy\n\n\n");
  }
  else if(!strcmp(moodArray[moodOfMelody].name, "Sad") && !strcmp(moodArray[0].name, "Happy") &&
          !strcmp(moodArray[1].name, "Sad")            && amountOfMoods == 2){
    printf("\n\n\n Sad ");
    
    while(test < 51){
      if(test == 25)
        printf("|");
      else if(test == ((int)(-((vectorMatrixResult[moodOfMelody]) / 2.4)) + 26))
        printf("[]");
      else
        printf("-");
      
      test++;
    }
    
    printf(" Happy\n\n\n");
  } 
}

/**Prints the mood of the melody into the console
  *@param moodArray is an array that contains the moods
  *@param moodOfMelody is an integer that defines what mood the melody has	
  */
void printMoodOfMelody(moodWeighting moodArray[], int moodOfMelody){
  printf("\n The mood of the melody is %s\n", moodArray[moodOfMelody].name);
}
