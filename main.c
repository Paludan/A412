/*@file
 *@brief This file is marvelous.
 *
 * MIDI analysing mega device of epic proportions
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

#define CHARS 1000
#define AMOUNT_OF_MOODS 2

/*Enums and structs*/
typedef enum mode {major, minor} mode;
typedef enum tone {C, Csharp, D, Dsharp, E, F = 6, Fsharp, G, Gsharp, A, Asharp, B} tone;
typedef enum mood {glad, sad} mood;

typedef struct{
  int tone;
  int octave;
  int length;
  int average;
} note;

typedef struct{
  unsigned int tempo;
  mode mode;
  tone key;
} data;

typedef struct{
  char *parameter;
  int point;
} points;

typedef struct{
  int mode;
  int tempo;
  int toneLength;
  int pitch;
} moodWeighting;

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
void findNoteLength(double x, int *, int *);
void printNote(note);
int getHex(FILE*, int[]);
void fillSongData(data*, int[], int);
int countNotes(int[], int);
void fillNote(int, note*);
void printSongData(data);
void settingPoints(int*, int*, int*, int*, data, int, note []);
void insertMoods(moodWeighting []);
int weightingMatrix(moodWeighting [], int, int, int, int);
void findEvents(int, int [], eventPlacement [], note []);
void insertPlacement1(int [], int *, int, note [], int *);
void insertPlacement2(int [], int *, int);
int checkNextEvent(int [], int);
int sortResult(const void *, const void *);
int deltaTimeToNoteLength (int, int);

int main(int argc, const char *argv[]){
  /*Variables*/
  int numbersInText = 0, notes, i = 0, moodOfMelodi = 0;
  /* PLACEHOLDER FIX THIS */
  int mode = 5, tempo = 5, toneLength = 5, pitch = 5;
  moodWeighting moodArray[AMOUNT_OF_MOODS];
  data data;
  FILE *f = fopen(argv[1],"r");
  if(f == NULL){
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }
  int *hex = (int *) malloc(CHARS * sizeof(int));
  if(hex == NULL){
    printf("Memory allokation failed, bye!");
    exit(EXIT_FAILURE);
  }
  
  /*Reading the data from the file*/
  numbersInText = getHex(f, hex);
  fillSongData(&data, hex, numbersInText);
  notes = countNotes(hex, numbersInText);
  note *noteAr = (note*) malloc(notes * sizeof(note));
  if(noteAr == NULL){
    printf("Memory allocation failed, bye!");
    exit(EXIT_FAILURE);
  }
  eventPlacement placement[numbersInText];
  findEvents(numbersInText, hex, placement, noteAr);
  insertMoods(moodArray);
  settingPoints(&mode, &tempo, &toneLength, &pitch, data, notes, noteAr);
  printf("%d, %d, %d, %d\n", mode, tempo, toneLength, pitch);
  for(i = 0; i < notes; i++)
    printNote(noteAr[i]);
  printSongData(data);
  moodOfMelodi = weightingMatrix(moodArray, mode, tempo, toneLength, pitch);
  printf("%d\n", moodOfMelodi);
  /*Clean up and close*/
  fclose(f);
  free(hex);
  free(noteAr);

  return 0;
}

/**A function, that retrieves the hexadecimals from the files and also returns the number of files
  *@param[FILE*] f: a pointer to the file the program is reading from
  *@param[int] hexAr[]: an array of integers, that the information is stored in
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
  *@param[int] hex[]: an array with the stored information from the file
  *@param[int] amount: an integer holding the total number of characters in the array
 */
int countNotes(int hex[], int amount){
  int i = 0, res = 0;
  for(i = 0; i < amount; i++){
    if(hex[i] == 0x90){
      res++;
    }
  }
  return res;
}

/**A function, that fills out the song data
  *@param[data*] data: a pointer to a structure containing the tempo and mode of the song
  *@param[int] hex[]:the array of integers read from the file
  *@param[int] numbersInText: the total amount of integers in the array
  */
void fillSongData(data *data, int hex[], int numbersInText){
  int j;
  /*Find the mode of the song, initialised as minor atm*/
  data->mode = minor;
  for(j = 0; j < numbersInText; j++){
    /* finds the tempo */
    if(hex[j] == 0xff && hex[j+1] == 0x51 && hex[j+2] == 0x03){
      data->tempo =  60000000/((hex[j+3] << 16) | (hex[j+4] << 8) | (hex[j+5]));
    }
  }
}

void findEvents(int numbersInText, int hex[], eventPlacement placement[], note noteAr[]){
  int noteOff = 0, noteOn = 0, afterTouch = 0, controlChange = 0,
      programChange = 0, channelPressure = 0, pitchWheel = 0, i = 0, n = -1;

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
}

void insertPlacement1(int hex[], int *place, int j, note noteAr[], int *n){
  int i = 3;
  while(i < 7 && hex[(j + i++)] > 0x80);
  if(checkNextEvent(hex, (j + i))){
    *place = j;
    if(hex[j] == 0x90)
      fillNote(hex[j + 1], &noteAr[*n += 1]); 
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

/**A function to calculate the notelength - tba
*/
void findNoteLength (double x, int *high, int *low){
  double func = 16*((x*x)*(0.0000676318287050830)+(0.0128675448628599*x)-2.7216713227147);
  double temp = func;
  double temp2 = (int) temp;

  if (!(temp - (double) temp2 < 0.5)){
    func += 1;
  }

  printf("x: %f og func: %f\n", x, func);
  *high = (int) func;
  *low = 16;
}

/**A function to fill out each of the structures of type note
  *@param[int] inputTone: the value of the hexadecimal collected on the "tone"-spot
  *@param[note*] note: a pointer to a note-structure
*/
void fillNote(int inputTone, note *note){
  note->tone = inputTone % 12;
  note->average = inputTone;
  note->octave = inputTone / 12;
}

/**A function to print the note
  *@param[note] note: the note structure to be printed
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

/**A function to print out the overall data of the song, tempo and mode
  *@param[data] data: the data to be printed
  */
void printSongData(data data){
  printf("Tempo: %d\nMode: ", data.tempo);
  switch(data.mode){
    case minor: printf("minor"); break;
    case major: printf("major"); break;
    default: printf("unknown mode"); break;
  }
  putchar('\n');
}

void settingPoints(int* mode, int* tempo, int* length, int* octave, data data, int notes, note noteAr[]){
  int deltaTime = deltaTimeToNoteLength(480, 960), combined = 0, averageNote = 0;
  switch(data.mode){
    case minor: *mode = -5; break;
    case major: *mode = 5; break;
    default: *mode = 0; break;
  }
  if(data.tempo < 60)
    *tempo = -5;
  else if(data.tempo >= 60 && data.tempo < 70)
    *tempo = -4;
  else if(data.tempo >= 70 && data.tempo < 80)
    *tempo = -3;  
  else if(data.tempo >= 80 && data.tempo < 90)
    *tempo = -2;
  else if(data.tempo >= 90 && data.tempo < 100)
    *tempo = -1;
  else if(data.tempo >= 100 && data.tempo < 120)
    *tempo =  0;  
  else if(data.tempo >= 120 && data.tempo < 130)
    *tempo =  1;
  else if(data.tempo >= 130 && data.tempo < 140)
    *tempo =  2;
  else if(data.tempo >= 140 && data.tempo < 150)
    *tempo =  3;
  else if(data.tempo >= 150 && data.tempo < 160)
    *tempo =  4;
  else if(data.tempo >=  160)
    *tempo =  5;

  switch(deltaTime){
    case 1: *length = -5; break;
    case 2: *length = -4; break;
    case 4: *length = -2; break;
    case 8: *length =  0; break;
    case 16: *length = 3; break;
    case 32: *length = 5; break;
  }
  for (int i = 0; i < notes; i++){
    combined += noteAr[i].average;
  }
  averageNote = combined/notes;

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


/* Inserts the weighting of each mood in the weighting matrix 0 = happy 1 = sad*/
void insertMoods(moodWeighting moodArray[]){
  moodArray[glad].mode           = 3;
  moodArray[glad].tempo          = 4;
  moodArray[glad].toneLength     = 2;
  moodArray[glad].pitch          = 1;

  moodArray[sad].mode            = -4;
  moodArray[sad].tempo           = -5;
  moodArray[sad].toneLength      = -3;
  moodArray[sad].pitch           = 0;
}

/* Vector matrix multiplication. Mood vector and weghting matrix. Return the row with the highest value */
int weightingMatrix(moodWeighting moodArray[], int mode, int tempo, int toneLength, int pitch){
  int result[AMOUNT_OF_MOODS] = {0};
  for(int i = 0; i < AMOUNT_OF_MOODS; i++){
    result[i] += (moodArray[i].mode * mode);
    result[i] += (moodArray[i].tempo * tempo);
    result[i] += (moodArray[i].toneLength * toneLength);
    result[i] += (moodArray[i].pitch * pitch);
  }
  if (result[0] > result[1])
    printf("Happy\n");
  else if (result[1] > result[0])
    printf("Sad\n");
  
  qsort(result, AMOUNT_OF_MOODS, sizeof(int), sortResult);
  return result[0];
}

/* Sort rows highest first */
int sortResult(const void *pa, const void *pb){
  int a = *(const int*)pa;
  int b = *(const int*)pb;
  return (b-a);
}

/* Find note length */
int deltaTimeToNoteLength (int ticks, int ppqn){
  double noteLength= ((double) (ticks)) / ((double) (ppqn/8));

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
  return noteLength;
}