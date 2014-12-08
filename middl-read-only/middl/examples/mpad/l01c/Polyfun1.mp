# Polyfun1

tempo60 globaloose20,2 velvar10,2

|0 iCC o/16 | iOH o/16 v80 | iCH o/16 | iSD o/16 | tomson o/16 v120 | iBD o/16
|10 ch1 i26 oC4/16 guiton v80 | ch2 i34 oC2/16 guiton v120| ch3 i01 oC5/16| ch4 i01 oC6/16

m$drums1 ( 
|0 x - - - - - - - - - - 
|1 - - - - x - - - - - x
|2 - - x - - - x - x - -
|3 - - - x - - - - - x -
|4 6 3 4 6 3 4 6 3 4 2 5 
|5 x - - x - - x - - x -
)

m$drums2 ( 
|0 - - - - - - - - - - - 
|1 - - - - x - - - - - x
|2 x - x - - - x - x - -
|3 - - - x - - - - - x -
|4 6 3 4 6 3 4 6 3 4 2 5 
|5 x - - x - - x - - x -
)

m$guit1a  (|10 0 3 10 0 3 10 0 3 10 9 5 )
m$guit1   (|10 strum250 [0,3,10]== [0,3,10]== [0,3,10]== [9,5]= )
m$bass1   (|11 12 7 10 0 7 10 12 7 10 3 5 )
m$piano1a (|12 x == nt+2 == nt+3 == nt+5 =)
m$piano1  (|12 x = nt+2 = nt+3 = nt+5 =)
m$piano2a (|13 x == nt-2 == nt+2 == nt-3 =)
m$piano2  (|13 x == nt-2 == nt+2 == nt-3 ==)
m$piano2b (|13 nt-1 == nt-3 == nt+2 == nt+3 ==)
m$piano2b2(|13 x == nt-2 == nt+2 == nt+3 ==)
m$piano2c (|13 x == nt-2 == nt+2 == nt+8 ==)
m$piano2c2(|13 x == nt-2 == nt+2 == nt+7 ==)
m$piano2c3(|13 x == nt-2 == nt+2 == nt+7 nt+3 nt-1)

m$pianopause ( |12 -*11 |13 -*11 )

$drums1 $drums2*3 ($guit1 $bass1 $pianopause)*4
$drums1 $drums2*7 ($guit1 $bass1 $piano1a $piano2a)*8
$drums1 $drums2*11 ($guit1 $bass1 $piano1a )*12 $piano2*11
$drums1 $drums2*3 ($guit1 $bass1 $piano1a $piano2a)*4
|10 t+1 | t+1 | t+1 | t+1 
$drums1 $drums2*7 ($guit1 $bass1 $piano2a)*8 $piano1*11
|10 t0 | t0 | t0 | t0 
$drums1 $drums2*3 ($guit1 $bass1 $piano1a $piano2a)*4
|10 t+2 | t+2 | t+2 | t+2 
($drums1 $drums2*7) ($guit1 $bass1)*8  $piano1*11 $piano2*7
|10 t+3 | t+3 | t+3 | t+3 
($drums1 $drums2*7) ($guit1 $bass1)*8  $piano1*11 $piano2b $piano2b2*6 $piano2c
|10 t+4 | t+4 | t+4 | t+4 
($drums1 $drums2*7) ($guit1 $bass1)*8  $piano1*11 $piano2c2*6 $piano2c3
|10 t0 | t0 | t0 | t0 
($drums1 $guit1 $bass1 $piano1a $piano2a)*16
