# Meshuggah I, theme1 solo 02:54 - 03:34

tempo97

# Basic patterns with long ends.

m$1b0( xxx= )
m$1b1( xxx= ==== )
m$1b2( xxx=*2 ==== )
m$1b3( xxx=*3 ==== )
m$1b4( xxx=*4 ==== )
m$1b5( xxx=*5 ==== )
m$1b6( xxx=*6 ==== )
m$1b7( xxx=*7 ==== )

# Third part pattern, 128/8

m$1bC( 
o/32
$1b3 $1b5 ====                             
$1b1 $1b2 $1b1 $1b1 $1b4 ====
$1b1 $1b6 $1b1 $1b1 $1b4 ====
$1b1 $1b4 $1b1 $1b2 $1b1 $1b4 ====
$1b1 $1b7 $1b5 ====
$1b1 $1b2 $1b1 $1b1 $1b4 ====
$1b1 $1b4 $1b1 $1b2 $1b1 $1b4 ====
$1b1 $1b7 $1b0                    
)    

# Third part Solo, (5-5-5-7)*4 5-5-5-3 = 128

m$1sA (
o/16
(
D# E  D# =  C# D# C# =  =  = 
D# E  D# =  C  C# C  =  =  = 
D# E  D# =  C# D# C# =  =  =
D# E  F# =  C# D# C# =  C  C# C  =  =  =
)*5
D# E  D# =  C# D# C# =  =  = 
D# E  D# =  C  C# C  =  =  =
D# E  D# =  C# D# C# =  =  =
C  C# C  =  =  =
)

m$1sB (
P/16*6 
(o/8 F#==== G==== F#====== G====)*5
     F#==== G==== F#= G==
)

# Drums
#  Cymbal     | Snare             | Kick | Tom (s)
|0 iCB        | iTA               | iRS  | iHC           #instruments def
|0 8*128      | (P/16 x)*128      | $1bC | P/1*16        #part C, solo
|0 2          | P/2               | 2    | P/2           #break


# Guitars
#  bass     |   guitar     |  solo 1        |    solo 2
|5 ch1 i124  | ch2 i124      | ch3 i124 o5 v80 | ch4 i124 v60 o5 #instruments def
|5 oC2 $1bC | oC3 $1bC     | $1sA           | $1sB           #part C, solo
|5 P/2      | A2/8 G# G F# | P/2            | P/2            #break
