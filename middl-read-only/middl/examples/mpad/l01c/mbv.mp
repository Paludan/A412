# My Bloody Valentine, When you sleep

tempo60
|0 ch1 o6 i74 
| iBD v110
| iSD stress25
| iOH v80
| ch2 o3 i33
| ch3 o5 i50
| ch3 o5 t+4
| ch3 o5 t-5

m$flute( o/16 B = n+1 n-1 = G = E == F E = C = A =*12 G === )
m$bass( C/16*8 D*8 F*16 )
m$guit( C/2 n+2 F/1 )
m$bd1( o/16 xx-x*7 xxxx )
m$oh1( o/16 xxxx*7 ---- )
m$oh2( o/16 xxxx*8 )
m$sd1( o/16 --x---x o/32 -,x o/16 --x-*5 $sdroll )
m$sd2( o/16 --x-*3 --x o/32 -,x o/16 --x-*4 )
m$sdroll (o/32 'xx'xx'xx'xx )
m$bdroll (o/16 'x'x'x'x )
m$pause( P/4 )

m$intro ( |0 $pause | $bdroll | $sdroll | $pause | $pause | $pause | $pause | $pause )
m$bar1 ( |0 $flute | $bd1 | $sd1 | $oh1 | $bass | $guit | $guit | $guit )
m$bar2 ( |0 $flute | $bd1 | $sd2 | $oh2 | $bass | $guit | $guit | $guit )

$intro ($bar2 $bar1)*2
