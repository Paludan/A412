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
#include <dirent.h>

#define CHARS 1000
int AMOUNT_OF_MOODS;
#define SCALESIZE 7

/*Enums and structs*/
typedef enum mode {major, minor} mode;
typedef enum tone {C, Csharp, D, Dsharp, E, F, Fsharp, G, Gsharp, A, Asharp, B} tone;
typedef enum mood {glad, sad} mood;

typedef struct{
  int tone;
  int octave;
  int length;
  int average;
  int ticks;
} note;

typedef struct{
  unsigned int tempo;
  int ppqn;
  mode mode;
  tone key;
} data;

typedef struct{
  char *parameter;
  int point;
} points;

typedef struct{
  char name[25];
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
void checkDirectory(char*, DIR*);
void findNoteLength(double x, int *, int *);
void printNote(note);
int getHex(FILE*, int[]);
void fillSongData(data*, int[], int);
int countPotentialNotes(int[], int);
void fillNote(int, note*);
void settingPoints(int*, int*, int*, int*, data, int, note [], int *);
void insertMoods(moodWeighting [], FILE*);
void weightingMatrix(moodWeighting [], int, int, int, int, int *);
void findEvents(int, int [], eventPlacement [], note [], int *);
void insertPlacement1(int [], int *, int, note [], int *, int []);
void insertPlacement2(int [], int *, int);
int checkNextEvent(int [], int);
void findTicks(int, int [], eventPlacement [], note [], int, int *, int []);
void countTicks1(int [], int *, int, note [], int *);
void countTicks2(int [], int *, int, note [], int *);
int sortResult(const void *, const void *);
void deltaTimeToNoteLength(int, int, note *);
int isInScale(int, int[], int);
int isInMinor(int);
int isInMajor(int);
int sortToner(const void*, const void*);
void findMode(note*, int, data*);
int FindMoodAmount(FILE*);
void printResults(int, int, int, int, moodWeighting [], int []);

int main(int argc, const char *argv[]){
  DIR *dir = 0;
  FILE *f;
  char MIDIfile[25];
  /*Variables*/
  int numbersInText = 0, notes, size = 0, mode = 5, tempo = 5, toneLength = 5, pitch = 5;
  FILE* moods = fopen("moods.txt", "r");
  if(moods == NULL){
    perror("Error: moods missing ");
    exit(EXIT_FAILURE);
  }
  AMOUNT_OF_MOODS = FindMoodAmount(moods);
  moodWeighting moodArray[AMOUNT_OF_MOODS];
  data data = {0, major, D};
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
  fillSongData(&data, hex, numbersInText);
  notes = countPotentialNotes(hex, numbersInText);
  note *noteAr = (note*) malloc(notes * sizeof(note));
  if(noteAr == NULL){
    printf("Memory allocation failed, bye!");
    exit(EXIT_FAILURE);
  }
  eventPlacement placement[numbersInText];
  findEvents(numbersInText, hex, placement, noteAr, &size);
  deltaTimeToNoteLength(data.ppqn, size, noteAr);
  insertMoods(moodArray, moods);
  findMode(noteAr, notes, &data);
  settingPoints(&mode, &tempo, &toneLength, &pitch, data, notes, noteAr, &size);
  int result[AMOUNT_OF_MOODS];
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
  if ((dir = opendir ("./Music")) != NULL) {
    printf("Mulige numre\n");
      while ((musicDir = readdir (dir)) != NULL) {
        printf ("%s\n", musicDir->d_name);
      }
  } 
  else {
    perror ("Failure while opening directory");
    exit (EXIT_FAILURE);
  }
  printf("Indtast det valgte nummer\n");
  scanf("%s", MIDIfile);
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

/**! \fn int fillSongData(data *data, int hex[], int numbersInText) 
  *A function, that fills out the song data
  *@param *data a pointer to a structure containing the tempo and mode of the song
  *@param hex[] the array of integers read from the file
  *@param numbersInText the total amount of integers in the array
  */
void fillSongData(data *data, int hex[], int numbersInText){
  data->ppqn = hex[12]*256+hex[13];
  /*Find the mode of the song, initialised as minor atm*/
  for(int j = 0; j < numbersInText; j++){
    /* finds the tempo */
    if(hex[j] == 0xff && hex[j+1] == 0x51 && hex[j+2] == 0x03){
      data->tempo =  60000000/((hex[j+3] << 16) | (hex[j+4] << 8) | (hex[j+5]));
    }
  }
}

void findEvents(int numbersInText, int hex[], eventPlacement placement[], note noteAr[], int *size){
  int noteOff = 0, noteOn = 0, afterTouch = 0, controlChange = 0,
      programChange = 0, channelPressure = 0, pitchWheel = 0, n = 0, notes[numbersInText];

  for(int j = 0; j < numbersInText; j++){
    switch (hex[j]){
      case 0x90: insertPlacement1(hex, &placement[noteOn++].noteOn, j, noteAr, &n, notes);               break;
      case 0x80: insertPlacement1(hex, &placement[noteOff++].noteOff, j, noteAr, &n, notes);             break;
      case 0xA0: insertPlacement1(hex, &placement[afterTouch++].afterTouch, j, noteAr, &n, notes);       break;
      case 0xB0: insertPlacement1(hex, &placement[controlChange++].controlChange, j, noteAr, &n, notes); break;
      case 0xC0: insertPlacement2(hex, &placement[programChange++].programChange, j);                    break;
      case 0xD0: insertPlacement2(hex, &placement[channelPressure++].channelPressure, j);                break;
      case 0xE0: insertPlacement1(hex, &placement[pitchWheel++].pitchWheel, j, noteAr, &n, notes);       break;
      default  :                                                                                         break;
    }
  }
  findTicks(numbersInText, hex, placement, noteAr, noteOn, size, notes);
}

void insertPlacement1(int hex[], int *place, int j, note noteAr[], int *n, int notes[]){
  int i = 3;
  while(i < 7 && hex[(j + i++)] > 0x80);
  if(checkNextEvent(hex, (j + i))){
    *place = j;
    if(hex[j] == 0x90){
      notes[*n] = hex[j + 1];
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

void findTicks(int numbersInText, int hex[], eventPlacement placement[], note noteAr[], int noteOn, int *size, int notes[]){
  int tickCounter = 0, deltaCounter1 = 3, deltaCounter2 = 2;
  
  for(int j = 0; j < noteOn; j++){
    for(int i = placement[j].noteOn; i < numbersInText; i++){
      if(hex[i] == 0x80){
        if(hex[i + 1] == notes[j])
          break;
        else
          countTicks1(hex, &i, deltaCounter1, noteAr, &tickCounter);
      }
      else if(hex[i] == 0xA0){
        if(hex[i + 1] == notes[j] && hex[i + 2] == 0x00)
          break;
        else
          countTicks1(hex, &i, deltaCounter1, noteAr, &tickCounter);
      }
      else if(hex[i] == 0xD0){
        if(hex[i + 1] == 0x00)
          break;
        else
          countTicks2(hex, &i, deltaCounter2, noteAr, &tickCounter);
      }
      else if(hex[i] == 0xC0)
        countTicks2(hex, &i, deltaCounter2, noteAr, &tickCounter);
      else
        countTicks1(hex, &i, deltaCounter1, noteAr, &tickCounter);   
    }
  }
  *size = tickCounter;
}

void countTicks1(int hex[], int *i, int deltaCounter, note noteAr[], int *tickCounter){
  noteAr[*tickCounter].ticks = 0;
  int tick = 0;
  while(deltaCounter < 7 && hex[(*i + deltaCounter)] > 0x80)
    tick += ((hex[(*i + deltaCounter++)] - 0x80) * 128);
  tick += hex[(*i + deltaCounter)];
  noteAr[*tickCounter].ticks += tick;
  *tickCounter += 1;
  *i += deltaCounter;
}

void countTicks2(int hex[], int *i, int deltaCounter, note noteAr[], int *tickCounter){
  noteAr[*tickCounter].ticks = 0;
  int tick = 0;
  while(deltaCounter < 6 && hex[(*i + deltaCounter)] > 0x80)
    tick += ((hex[(*i + deltaCounter++)] - 0x80) * 128);
  tick += hex[(*i + deltaCounter)];
  noteAr[*tickCounter].ticks += tick;
  *tickCounter += 1;
  *i += deltaCounter;
}

/**A function to fill out each of the structures of type note
  *@param inputTone the value of the hexadecimal collected on the "tone"-spot
  *@param note* a pointer to a note-structure
*/
void fillNote(int inputTone, note *note){
  note->tone = inputTone % 12;
  note->average = inputTone;
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

void settingPoints(int* mode, int* tempo, int* length, int* octave, data data, int notes, note noteAr[], int *size){
  int deltaTime = 0, combined = 0, averageNote = 0;
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

  for(int i = 0; i < notes; i++){
    combined += noteAr[i].length;
  }
  deltaTime = combined/notes;
  
  if (deltaTime < 1.5 && deltaTime >= 0)
    *length = -5;
  else if (deltaTime < 3 && deltaTime >= 1.5)
    *length = -4;
  else if (deltaTime < 6 && deltaTime >= 3)
    *length = -2;
  else if (deltaTime < 12 && deltaTime >= 6)
    *length = -0;
  else if (deltaTime < 24 && deltaTime >= 12)
    *length = 3;
  else
    *length = 5;

  switch(deltaTime){
    case 1: *length = -5; break;
    case 2: *length = -4; break;
    case 4: *length = -2; break;
    case 8: *length =  0; break;
    case 16: *length = 3; break;
    case 32: *length = 5; break;
  }
  combined = 0;
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
void insertMoods(moodWeighting moodArray[], FILE* moods){
  for(int i = 0; i < AMOUNT_OF_MOODS; i++){
    fscanf(moods, "%s %d %d %d %d", moodArray[i].name , &moodArray[i].mode, 
                                    &moodArray[i].tempo, &moodArray[i].toneLength,
                                    &moodArray[i].pitch);
  }
}

/* Vector matrix multiplication. Mood vector and weghting matrix. Return the row with the highest value */
void weightingMatrix(moodWeighting moodArray[], int mode, int tempo, int toneLength, int pitch, int *result){
  for(int i = 0; i < AMOUNT_OF_MOODS; i++){
    result[i] = 0;
  }
  
  for(int i = 0; i < AMOUNT_OF_MOODS; i++){
    result[i] += (moodArray[i].mode * mode);
    result[i] += (moodArray[i].tempo * tempo);
    result[i] += (moodArray[i].toneLength * toneLength);
    result[i] += (moodArray[i].pitch * pitch);
  }
  
  for(int i = 0; i < AMOUNT_OF_MOODS; i++){
    printf("%s: %d\n", moodArray[i].name, result[i]);
  }
}

/* Sort rows highest first */
int sortResult(const void *pa, const void *pb){
  int a = *(const int*)pa;
  int b = *(const int*)pb;
  return (b-a);
}

/* Find note length */
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
  int *i1 = (int*) a, *i2 = (int*) b;

  return *i1 - *i2;
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

/**A function to find the mode of the song by first calculating the tone span over sets of notes in the song, and then comparing it to the definition of minor and major keys.
  *@param noteAr An array of all the notes in the entire song
  *@param totalNotes The number of notes in the song
  *@param data The song data
  */
void findMode(note noteAr[], int totalNotes, data *data){
  int majors[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, minors[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int x = 0, y = 0, z = 0, bar[4], sizeBar = 4, tempSpan = 999, span = 999, keynote = 0, mode = 0, tempNote = 0;

  for(x = 0; x < totalNotes; x++){
    tempNote = noteAr[x].tone;
 
    for(y = C; y <= B; y++){
      if(majors[y])
        checkScale(majors, tempNote, y);
    }
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
  /*outputs result directly to the data struct*/
  if(mode > 0)
    data->mode = major;
  else if(mode < 0)
    data->mode = minor;
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
    if(isMajor)
      return 1;

    return 0;
}

/**A function to check if the given tone leap is in the minor scale.
  *@param toneLeap An integer describing the processed tone leap
  *@return a boolean value, returns 1 if the tone leap is in the minor scale, 0 if it's not.
  */
int isInMinor(int toneLeap){
  int minor[] = {0, 2, 3, 5, 7, 8, 10};

  for(int i = 0; i < SCALESIZE; i++){
    if(toneLeap == minor[i])
      return 1;
  }
  return 0;
}

/**A function to check if the given tone leap is in the major scale.
  *@param toneLeap An integer describing the processed tone leap
  *@return a boolean value, returns 1 if the tone leap is in the major scale, 0 if it's not.
  */
int isInMajor(int toneLeap){
  int major[] = {0, 2, 4, 5, 7, 9, 11};

  for(int i = 0; i < SCALESIZE; i++){
    if(toneLeap == major[i])
      return 1;
  }
  return 0;
}


int FindMoodAmount(FILE *moods){
  int i = 1;
  while(fgetc(moods) != EOF){
    if(fgetc(moods) == '\n')
      i++;
  }
  rewind(moods);
  return i;
}

void printResults(int mode, int tempo, int toneLength, int pitch, moodWeighting moodArray[], int result[]){
  printf("\n\n\n");
  printf(" Mode:");
    printf("%10d\n", mode);
  printf(" Tempo:");
    printf("%9d\n", tempo);
  printf(" Tone length:");
    printf("%3d\n", toneLength);
  printf(" Pitch:");
    printf("%9d\n", pitch);
  printf("\n\n\n                                       WEIGHTINGS            \n");
  printf("                           Mode | Tempo | Tone length | Pitch\n");
  
  for(int i = 0; i < AMOUNT_OF_MOODS; i++){
    printf(" %s", moodArray[i].name);
    for(int j = strlen(moodArray[i].name); j < 26; j++)
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

  for(int i = 0; i < AMOUNT_OF_MOODS; i++){
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
  int moodOfMelodi = 0;
  for(int i = 0; i < AMOUNT_OF_MOODS; i++){
    if(moodOfMelodi < result[i])
      moodOfMelodi = i;
  }
  int test = 0;
  
  if(!strcmp(moodArray[moodOfMelodi].name, "Happy")){
    printf("\n\n\n Sad ");
    while(test < 51){
      if(test == 25)
        printf("|");
      else if(test == (((result[moodOfMelodi] + 100) / 4)))
        printf("[");
      else if(test == (((result[moodOfMelodi] + 100) / 4) + 2))
        printf("]");
      else
        printf("-");
      test++;
    }
    printf(" Happy\n\n\n");
  }
  else if(!strcmp(moodArray[moodOfMelodi].name, "Sad")){
    printf("\n\n\n Sad ");
    while(test < 51){
      if(test == 25)
        printf("|");
      else if(test == (((-(result[moodOfMelodi]) + 100) / 4)))
        printf("[");
      else if(test == (((-(result[moodOfMelodi]) + 100) / 4) + 2))
        printf("]");
      else
        printf("-");
      test++;
    }
    printf(" Happy\n\n\n");
  }

  printf("\n The mood of the melodi is %s\n", moodArray[moodOfMelodi].name);
}
