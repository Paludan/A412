/* MIDI analysing mega device of epic proportions
 * Gruppe A412
 * Lee Paludan
 * Simon Madsen
 * Jonas Stolberg
 * Jacob Mortensen
 * Esben Kirkegaard Da Scrub
 * Arne Rasmussen Da Champ
 * Tobias Morell
 * @file
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CHARS 1000
#define AMOUNT_OF_MOODS 2

/*Enums and structs*/
typedef enum mode {major, minor} mode;
typedef enum tone {C, Csharp, D, Dsharp, E, F = 6, Fsharp, G, Gsharp, A, Asharp, B} tone;
typedef enum mood {glad, sad} mood;

typedef struct{
  int tone;
  int octave;
  int lenght;
} note;

typedef struct{
  unsigned int tempo;
  mode mode;
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

/*Prototypes*/
void findNoteLength(double x, int *, int *);
void printNote(note);
int getHex(FILE*, int[]);
void fillSongData(data*, int[], int);
int countNotes(int[], int);
void fillNote(int, note*);
void printSongData(data);
void insertMoods(moodWeighting []);
int weightingMatrix(moodWeighting [], int, int, int, int);
void findEvents(int, int [], note []);
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
  findEvents(numbersInText, hex, noteAr);
  insertMoods(moodArray);
  for(i = 0; i < notes; i++)
    printNote(noteAr[i]);
  printSongData(data);
  moodOfMelodi = weightingMatrix(moodArray, mode, tempo, toneLength, pitch);

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
    if(hex[j] == 0xff){
      if(hex[j+1] == 0x51){
        data->tempo =  60000000/((hex[j+3] << 16) | (hex[j+4] << 8) | (hex[j+5]));
      }
    }
  }
}

void findEvents(int numbersInText, int hex[], note noteAr[]){
  int note = 0x01, eventType = 0x01, counter = 0, i = 0;
  /*Read and proces the hex array*/
  for(int j = 0; j < numbersInText; j++){
    /* Hops over any noto-on, note-off or metaevent start
       Also stores the tones read after a note-on         */
    if(hex[j] == 0x00 && (hex[j + 1] == 0x90 || hex[j + 1] == 0xff)){
      counter = 1;
      j += 4;
      if(hex[j - 3] == 0x90){
        note = hex[j - 2];
        fillNote(hex[j - 2], &noteAr[i]);
        i++;
      }
      else{
        eventType = hex[j - 2];
      }
    }
    else if(hex[j] == 0x80 && hex[j + 1] == note){
      j += 2;
      note = 0x01;
      counter = 0;
    }
    if(counter){
      /* Here you can check for parameters inside a meta-event or MIDI-event */
    }
    else{
      /* Here you can check for parameters outside a meta-event or MIDI-event
         e.g. between a note-off and the next MIDI-event or a meta-event      */
    }
  }
}

/**A function to calculate the notelenght - tba
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

void settingPoints(int* mode, int* tempo, int* length, int* octave, data data){
  int deltaTime = deltaTimeToNoteLength(480, 960);
  switch(data.mode){
    case minor: *mode = -5; break;
    case major: *mode = 5; break;
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
	else if (noteLength < 3 && noteLenght >= 1.5)
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
