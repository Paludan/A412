/*
**  (C) Remo Dentato (rdentato@gmail.com)
**
** Permission to use, copy, modify and distribute this code and
** its documentation for any purpose is hereby granted without
** fee, provided that the above copyright notice, or equivalent
** attribution acknowledgement, appears in all copies and
** supporting documentation.
**
** Copyright holder makes no representations about the suitability
** of this software for any purpose. It is provided "as is" without
** express or implied warranty.
*/

#include "mf_instr.h"

static char *instr_names[] = {
  "\244abassdrum",                  /*  (0x80 |  36) */
  "\244abassdrum1",                 /*  (0x80 |  36) */
  "\243abassdrum2",                 /*  (0x80 |  35) */
  "\025accordion",                  /*  21*/
  "\040acousticbass",               /*  32*/
  "\000acousticgrand",              /*   0*/
  "\161agogo",                      /* 113*/
  "\101altosax",                    /*  65*/
  "\176applause",                   /* 126*/
  "\246asnare",                     /*  (0x80 |  38) */
  "\155bagpipe",                    /* 109*/
  "\151banjo",                      /* 105*/
  "\103baritonesax",                /*  67*/
  "\106bassoon",                    /*  70*/
  "\173birdtweet",                  /* 123*/
  "\114blownbottle",                /*  76*/
  "\075brasssection",               /*  61*/
  "\171breathnoise",                /* 121*/
  "\001brightacoustic",             /*   1*/
  "\305cabasa",                     /*  (0x80 |  69) */
  "\010celesta",                    /*   8*/
  "\052cello",                      /*  42*/
  "\252chihat",                     /*  (0x80 |  42) */
  "\264chinesecy",                  /*  (0x80 |  52) */
  "\064choiraahs",                  /*  52*/
  "\023churchorgan",                /*  19*/
  "\107clarinet",                   /*  71*/
  "\313claves",                     /*  (0x80 |  75) */
  "\007clavinet",                   /*   7*/
  "\053contrabass",                 /*  43*/
  "\270cowbell",                    /*  (0x80 |  56) */
  "\261crashcy",                    /*  (0x80 |  49) */
  "\261crashcy1",                   /*  (0x80 |  49) */
  "\271crashcy2",                   /*  (0x80 |  57) */
  "\036distortionguitar",           /*  30*/
  "\036distguitar",                 /*  30*/
  "\020drawbarorgan",               /*  16*/
  "\017dulcimer",                   /*  15*/
  "\041ebassfinger",                /*  33*/
  "\042ebasspick",                  /*  34*/
  "\033ecleanguitar",               /*  27*/
  "\032ejazzguitar",                /*  26*/
  "\002electricgrand",              /*   2*/
  "\034emutedguitar",               /*  28*/
  "\105englishhorn",                /*  69*/
  "\004epiano1",                    /*   4*/
  "\005epiano2",                    /*   5*/
  "\250esnare",                     /*  (0x80 |  40) */
  "\156fiddle",                     /* 110*/
  "\111flute",                      /*  73*/
  "\074frenchhorn",                 /*  60*/
  "\043fretlessbass",               /*  35*/
  "\143fxatmosphere",               /*  99*/
  "\144fxbrightness",               /* 100*/
  "\142fxcrystal",                  /*  98*/
  "\146fxechoes",                   /* 102*/
  "\145fxgoblins",                  /* 101*/
  "\140fxrain",                     /*  96*/
  "\147fxscifi",                    /* 103*/
  "\141fxsoundtrack",               /*  97*/
  "\011glockenspiel",               /*   9*/
  "\170guitarfretnoise",            /* 120*/
  "\037guitarharmonics",            /*  31*/
  "\177gunshot",                    /* 127*/
  "\303hagogo",                     /*  (0x80 |  67) */
  "\247handclap",                   /*  (0x80 |  39) */
  "\026harmonica",                  /*  22*/
  "\006harpsichord",                /*   6*/
  "\274hbongo",                     /*  (0x80 |  60) */
  "\175helicopter",                 /* 125*/
  "\253hfloortom",                  /*  (0x80 |  43) */
  "\254hh",                         /*  (0x80 |  44) */
  "\254hihat",                      /*  (0x80 |  44) */
  "\260hmidtom",                    /*  (0x80 |  48) */
  "\003honkytonk",                  /*   3*/
  "\301htimbale",                   /*  (0x80 |  65) */
  "\262htom",                       /*  (0x80 |  50) */
  "\314hwoodblock",                 /*  (0x80 |  76) */
  "\154kalimba",                    /* 108*/
  "\153koto",                       /* 107*/
  "\304lagogo",                     /*  (0x80 |  68) */
  "\275lbongo",                     /*  (0x80 |  61) */
  "\300lconga",                     /*  (0x80 |  64) */
  "\127leadbasslead",               /*  87*/
  "\122leadcalliope",               /*  82*/
  "\124leadcharang",                /*  84*/
  "\123leadchiff",                  /*  83*/
  "\126leadfifths",                 /*  86*/
  "\121leadsawtooth",               /*  81*/
  "\120leadsquare",                 /*  80*/
  "\125leadvoice",                  /*  85*/
  "\251lfloortom",                  /*  (0x80 |  41) */
  "\312lguiro",                     /*  (0x80 |  74) */
  "\257lmidtom",                    /*  (0x80 |  47) */
  "\302ltimbale",                   /*  (0x80 |  66) */
  "\255ltom",                       /*  (0x80 |  45) */
  "\310lwhistle",                   /*  (0x80 |  72) */
  "\315lwoodblock",                 /*  (0x80 |  77) */
  "\306maracas",                    /*  (0x80 |  70) */
  "\014marimba",                    /*  12*/
  "\316mcuica",                     /*  (0x80 |  78) */
  "\165melodictom",                 /* 117*/
  "\276mhconga",                    /*  (0x80 |  62) */
  "\320mtriangle",                  /*  (0x80 |  80) */
  "\012musicbox",                   /*  10*/
  "\073mutedtrumpet",               /*  59*/
  "\030nylonstringguitar",          /*  24*/
  "\104oboe",                       /*  68*/
  "\117ocarina",                    /*  79*/
  "\317ocuica",                     /*  (0x80 |  79) */
  "\277ohconga",                    /*  (0x80 |  63) */
  "\256ohihat",                     /*  (0x80 |  46) */
  "\067orchestrahit",               /*  55*/
  "\056orchestralstrings",          /*  46*/
  "\321otriangle",                  /*  (0x80 |  81) */
  "\035overdrivenguitar",           /*  29*/
  "\134padbowed",                   /*  92*/
  "\133padchoir",                   /*  91*/
  "\136padhalo",                    /*  94*/
  "\135padmetallic",                /*  93*/
  "\130padnewage",                  /*  88*/
  "\132padpolysynth",               /*  90*/
  "\137padsweep",                   /*  95*/
  "\131padwarm",                    /*  89*/
  "\113panflute",                   /*  75*/
  "\254pedhihat",                   /*  (0x80 |  44) */
  "\021percussiveorgan",            /*  17*/
  "\000piano",                      /*   0*/
  "\110piccolo",                    /*  72*/
  "\055pizzicatostrings",           /*  45*/
  "\112recorder",                   /*  74*/
  "\024reedorgan",                  /*  20*/
  "\167reversecymbal",              /* 119*/
  "\265ridebell",                   /*  (0x80 |  53) */
  "\263ridecy",                     /*  (0x80 |  51) */
  "\273ridecy2",                    /*  (0x80 |  59) */
  "\022rockorgan",                  /*  18*/
  "\172seashore",                   /* 122*/
  "\311sguiro",                     /*  (0x80 |  73) */
  "\152shamisen",                   /* 106*/
  "\157shanai",                     /* 111*/
  "\245sidestick",                  /*  (0x80 |  37) */
  "\150sitar",                      /* 104*/
  "\115skakuhachi",                 /*  77*/
  "\044slapbass1",                  /*  36*/
  "\045slapbass2",                  /*  37*/
  "\246snare",                      /*  (0x80 |  38) */
  "\100sopranosax",                 /*  64*/
  "\267splashcy",                   /*  (0x80 |  55) */
  "\162steeldrums",                 /* 114*/
  "\031steelstringguitar",          /*  25*/
  "\060stringensemble1",            /*  48*/
  "\061stringensemble2",            /*  49*/
  "\307swhistle",                   /*  (0x80 |  71) */
  "\046synthbass1",                 /*  38*/
  "\047synthbass2",                 /*  39*/
  "\076synthbrass1",                /*  62*/
  "\077synthbrass2",                /*  63*/
  "\166synthdrum",                  /* 118*/
  "\062synthstrings1",              /*  50*/
  "\063synthstrings2",              /*  51*/
  "\066synthvoice",                 /*  54*/
  "\251t1",                         /*  (0x80 |  41) */
  "\253t2",                         /*  (0x80 |  43) */
  "\255t3",                         /*  (0x80 |  45) */
  "\257t4",                         /*  (0x80 |  47) */
  "\260t5",                         /*  (0x80 |  48) */
  "\262t6",                         /*  (0x80 |  50) */
  "\164taikodrum",                  /* 116*/
  "\266tambourine",                 /*  (0x80 |  54) */
  "\027tangoaccordion",             /*  23*/
  "\174telephonering",              /* 124*/
  "\102tenorsax",                   /*  66*/
  "\057timpani",                    /*  47*/
  "\160tinklebell",                 /* 112*/
  "\251tom1",                       /*  (0x80 |  41) */
  "\253tom2",                       /*  (0x80 |  43) */
  "\255tom3",                       /*  (0x80 |  45) */
  "\257tom4",                       /*  (0x80 |  47) */
  "\260tom5",                       /*  (0x80 |  48) */
  "\262tom6",                       /*  (0x80 |  50) */
  "\054tremolostrings",             /*  44*/
  "\071trombone",                   /*  57*/
  "\070trumpet",                    /*  56*/
  "\072tuba",                       /*  58*/
  "\016tubularbells",               /*  14*/
  "\013vibraphone",                 /*  11*/
  "\272vibraslap",                  /*  (0x80 |  58) */
  "\051viola",                      /*  41*/
  "\050violin",                     /*  40*/
  "\065voiceoohs",                  /*  53*/
  "\116whistle",                    /*  78*/
  "\163woodblock",                  /* 115*/
  "\015xylophone",                  /*  13*/
};

/* Shortcuts:
**
**   muted    m
**   short    s
**   long     l
**   high     h
**   low      l
**   cymbal   cy
**   acoustic a
**   electric e
**   closed   c
**   open     o
**
**  EXAMPLES:
**
**   closed-hihat  = chihat
**   muted cuica   = mcuica
**   low tom       = ltom
**   long whistle  = lwhistle
**   short whistle = swhistle
**
*/

#define lowercase(x)    tolower((int)(x))

static int mf_i_cmp(const void *k, const void *b)
{
  unsigned char *p = *(unsigned char **)k;
  unsigned char *q = (*(unsigned char **)b)+1;
  int c;

  while (1) {
    c = lowercase(*p) - lowercase(*q);
    if ( c != 0 || *p == '\0' || *q == '\0') break;
    p++; q++;
  }

  return c;
}

int mf_instrbyname(char *name)
{
  unsigned char **q;

  q = bsearch(&name, instr_names, sizeof(instr_names)/sizeof(char *),
                                                       sizeof(char *), mf_i_cmp);
  return  (q == NULL)? -1 : (int)(**q);
}

