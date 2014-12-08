#!/usr/bin/perl

#
# musicpad.cgi v2.22
# (c) loic prot 2006
# visit http://l01c.ouvaton.org
# largely inspired by polymath.cgi, (c) jens johansson 2001

use CGI::Carp qw(fatalsToBrowser);
use CGI;

$version = "made with musicpad.cgi v2.2, by loic prot - http://l01c.ouvaton.org .";

&varinit;
&initialize_guitarchords;

$string = " $string \n";
$string =~ s/\s#.*\n/ \n /g;  # strip comments
$string =~ s/\s#.*\n/ \n /g;  # strip comments again, dammit!

&globalsettings;

# $string =~ s/([\*\|\,])/ $1 /g; # add whitespace..
$string =~ s/\s/ /g; # replace whitespace
$string=~s/\(/ \( /ig; # explode (
$string=~s/\)/ \) /ig; # explode )
$string =~ s/\s+/ /g; # compact whitespace

&debug("\n<br>\n<h2>*** Initial String:</h2>\n<p>$string\n<br>");

&processmacro;  # including expandmul and macro randomization

&expandtracks;

&debug("\n<h2>*** Individual Tracks :</h2>\n<p>");

foreach (@track) {
   if (/^\s*$/){ next; } # ignore empty trk
   &add_track($_);
}

if ($debug) { 
   $debtext.="\n<br>\n<h2>*** End of Treatment</h2>";
   print "Content-type: text/html\n\n<html><head><title>Musicpad Debug</title><style>p{font-family: lucida console, courier new,courier;font-size: 12;}</style></head><body><h1>Musicpad Debug Mode</h1>\n<p>$debtext\n</body></html>\n";
   exit(1);
}

&post_out;
exit(0);

sub error {
   local $string2 = $_[0];
   if ($debug) {
      $errmessage2= "\n<p>Debug Mode: These were the messages up until when the error occured: \n<br> $debtext\n";
   } else {
      $errmessage2="";
   }
   print "Content-type: text/html\n<html><head><title>Musicpad Error</title><style>p{font-family: lucida console, courier new,courier;font-size:12;}</style></head><body><h1>!!! Musicpad Error !!!</h1>\n<p>Error Message : $string2\n$errmessage2\n</body></html>\n";
   exit(1);
}

sub debug { if ($debug) { $debtext.= $_[0] } }
sub debugmax { if ($debug==2) { $debtext.= $_[0] } }

sub varinit {


#   if ($ENV{'REQUEST_METHOD'} eq "GET") {
#      $buffer = $ENV{'QUERY_STRING'};
#   } else {
#      read(STDIN, $buffer, $ENV{'CONTENT_LENGTH'});
#   }
#   foreach (split(/&/, $buffer)) {
#      ($name, $value) = split(/=/); $value =~ tr/+/ /;
#      $value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg; $F{$name} = $value;
#   }

$cgi = new CGI;

for $key ( $cgi->param() ) {
	$F{$key} = $cgi->param($key);
}

   $string = $F{'string'};
   $mediatype = $F{'mediatype'};
   $tempo = $F{'tempo'};
   $ppqn = $F{'ppqn'};
   $gduty = $F{'duty'};
   $gvel = $F{'vel'};

   defined($mediatype) || ($mediatype = "audio/x-midi");
   defined($tempo) || ($tempo = 60);
   defined($ppqn) || ($ppqn = 192);
   defined($gduty) || ($gduty = 100);
   defined($gvel) || ($gvel = 100);

   defined($string) || ($string = 'c e g g+');

   # test1 : ch1 i1 D5/16 D4/8 P/16 E4/4 D E3 8 16 n/32 xxoxxox | iBD n/8 xxoxxoxxo
   # test repetitions: (4 4 4 4)*2
   # test patterns: m$test(4 4 4 4) $test
   # test ioh:   ch10 m$test(8*4) iOH $test iCH $test
   # test simple notes: c c+ d d+ e f f+ g g+ a a+ b

   $midi = ($mediatype !~ /text/i);       # generate midi data rather than text
   $midi = 1;

   %drummap = qw (BD 36 B2 35 SD 38 S2 40 RS 37 HH 44 OH 46 CH 42 TA 54 T1 50 T2 48 T3 47 T4 45 T5 43 T6 41 CC 49 C2 57 TC 52 RC 51 R2 59 RB 53 SC 55 CB 56 HC 39);
   %notemap = qw (C- -1 CB -1 C 0 C+ 1 C# 1 D- 1 DB 1 D 2 D# 3 D+ 3 EB 3 E- 3 E 4 E+ 5 E# 5 F- 4 FB 4 F 5 F+ 6 F# 6 G- 6 GB 6 G 7 G+ 8 G# 8 A- 8 AB 8 A 9 A+ 10 A# 10 B- 10 BB 10 B 11 B+ 12 B# 12);

   $abspos = 0;
   $abspos_next = 0;

}

sub globalsettings {
   $debug=0;
   if ($string=~s/dEbUgMaX//) { $debug=2; }       # debugmax hidden mode
   if ($string=~s/dEbUg//) { $debug=1; }    # debug mode
   if ($string=~s/tempo(\d+)//i) { $tempo=$1; debug("\n<br>global tempo $tempo"); }   # tempo
   if ($string=~s/resolution(\d+)//i) {           # resolution
      if ($1==0) {$ppqn = 96}
      elsif ($1==1) {$ppqn = 192}
      elsif ($1==2) {$ppqn = 384}
      elsif ($1==3) {$ppqn = 1536}
      else {$ppqn=$1}
      debug("\n<br>resolution mode $1 ($ppqn ppqn)");
      }
   if ($string=~s/duty(\d+)//i) { $gduty=$1; debug("\n<br>global duty $gduty");}    # duty
   if ($string=~s/velocity(\d+)//i) { $gvel=$1; debug("\n<br>global velocity $gvel"); } # velocity
   if ($string=~s/globaloose(\d+),(\d+|G)//i) { $gloosew=$1; $glooseq=$2; debug("\n<br>global loose $gloosew,$glooseq"); } else { $gloosew=0; $glooseq=1; }   # globaloosew,q
   if ($string=~s/globalvelvar(\d+),(\d+|G)//i) { $gvelvarw=$1; $gvelvarq=$2; debug("\n<br>global velvar $gvelvarw,$gvelvarq"); } else { $gvelvarw=0; $gvelvarq=1; }   # globalvelvarw,q
   if ($string=~s/globalguiton//i) { $globalguitmode=1; debug("\n<br>global guitar mode on");} else { $globalguitmode=0;}   # guitmode
}   
        


sub getbound {
   local ($stringa, $startp, $opendelim, $closedelim) = @_ ;
   local ($loc, $locopen, $locclose, $level, $notover) ; 
   $loc = index($stringa, $opendelim, $startp);
   if ($loc == -1) { return -1; }
   $level = 1 ;
   $notover = 1 ;
   while ( $level && $notover) {
      $locopen = index($stringa, $opendelim, $loc+1);
      $locclose = index($stringa, $closedelim, $loc+1);
      if (($locopen+$locclose) == -2) { $notover=0; next; }
      if (($locopen<$locclose) && ($locopen!=-1)) { $loc=$locopen; $level++; } else { $loc=$locclose; $level--; }
   }
    if ($level) {
      &error("matching ".$opendelim."/".$closedelim." error"); 
   } else { 
      return $loc 
   };
}


sub getboundrev { # beware of exchanging $opendelim and $closedelim signs, and $startp is usually length($stringa)
   local ($stringa, $startp, $opendelim, $closedelim) = @_ ;
   local ($loc, $locopen, $locclose, $level, $notover) ; 
   $loc = rindex($stringa, $opendelim, $startp);
   if ($loc == -1) { return -1; }
   $level = 1 ;
   $notover = 1 ;
   while ( $level && $notover) {
      $locopen = rindex($stringa, $opendelim, $loc-1);
      $locclose = rindex($stringa, $closedelim, $loc-1);
      if (($locopen+$locclose) == -2) { $notover=0; next; }
      if ($locopen>$locclose) { $loc=$locopen; $level++; } else { $loc=$locclose; $level--; }
   }
    if ($level) {
      &error("matching ".$opendelim."/".$closedelim." error"); 
   } else {
      return $loc;
   }
}

sub processmacro { 
# macro is stored with ()
   local ($macpos,$foundmacro, $macstart, $macend, $mac_key, $exp_something);

debug("\n<h2>*** Storing Macros:</h2>\n<p>");

   while (($macpos = index($string, "m\$")) != -1) {
      $foundmacro=1;
      $macstart = index($string, "(",$macpos);
      $mac_key = substr($string, $macpos+2, $macstart-$macpos-2);
      $macend = getbound($string, $macstart, '(', ')' );
      $mac_key =~ s/\s+//g;
      $macro{$mac_key} = ' '.substr($string, $macstart, $macend-$macstart+1).' ';
      substr($string, $macpos, $macend-$macpos+1)="";
   }
   $string =~ s/\s+/ /g; # compact whitespace

   while (($macpos = index($string, "mrnd\$")) != -1) {
      $foundmacro=1;
      $macstart = index($string, "(",$macpos);
      $mac_key = substr($string, $macpos+5, $macstart-$macpos-5);
      $macend = getbound($string, $macstart, '(', ')' );
      $mac_key =~ s/\s+//g;
      $macrornd{$mac_key}=substr($string, $macstart+1, $macend-$macstart-1);
      $macrornd{$mac_key}=~ s/^\s+//;
      $macrornd{$mac_key}=~ s/\s+$//;
      substr($string, $macpos, $macend-$macpos+1)="";
   }
   $string =~ s/\s+/ /g; # compact whitespace

   foreach $mactemp (keys %macro) { debug("Macro $mactemp:\n<br>$macro{$mactemp}\n<br>") }

   foreach $mactemp (keys %macrornd) { debug("Random Macro $mactemp:\n<br>$macrornd{$mactemp}\n<br>") }

debug("\n<br>String after storing macros:\n<br>\n<br>$string\n<br>");

# replacing macros

   $exp_something = 1;
   while ($exp_something) {
      $exp_something = 0;
      if (500 < $amokcount++) {&error("macro expansion ran amok. self-reference?"); }      
      &expand_mul;
      $string =~ s/\s+/ /g; # compact whitespace
      if ($foundmacro) {
         $exp_something += ($string =~s/\$([a-z0-9\-_]+)/(((! defined($macro{$1})) && (! defined($macrornd{$1}))) ? &error("macro $1 not defined"):&writemac($1))/egi);
      }
   $string =~ s/\s+/ /g; # compact whitespace
   debug("\n<h2>*** Macros Replace step $amokcount:</h2>\n");
   if ($exp_something) { debug("<p>$string\n<br>"); } else { debug("<p>found no macro to replace\n<br>"); }
   }
}

sub writemac {
   local ($whichmac) = @_;
   if (defined($macro{$whichmac})) {
      return $macro{$whichmac};
   } elsif (defined($macrornd{$whichmac})) {
      local @macchoices = split(/\s+/, $macrornd{$whichmac});
      return $macchoices[ rand @macchoices ];
   } else {
   &error("strange, I can't find macro $whichmac !!!") 
   }
}	

sub expand_mul {
   local ($pre, $what, $whatstart, $whatend, $rpt, $post, $starpos, $foundonemul);
   $foundonemul = index($string, "*");
   while (($starpos = index($string, "*")) != -1) {
      $pre = substr($string, 0, $starpos);
      $post = substr($string, $starpos+1, length($string)-$starpos);
      unless ($post =~ /\s*([\d\.\*]+)(.*)\s*$/) {&error("illegal repeat: can't find factor"); }
      $rpt = $1; $post = $2;     
      if ($pre =~ /(.*)\)\s*$/i) {
         $pre = $1 . ")" ;
         $whatstart = length($pre);
         $whatend = getboundrev($pre, $whatstart, ')', '(' );
         $what = substr($pre, $whatend, $whatstart-$whatend+1);
         $pre = substr($pre, 0, $whatend-1);
      } else {
         if ($pre =~ /(.*)\s+(\S+)\s*$/i) {
            $pre = $1; $what = $2;
         } else {
             &error("illegal repeat: can't find what to repeat");
         }
      }
      $what = (" $what " x $rpt);
      $string = $pre . $what . $post;
   }
   debug("\n<h2>*** Multipliers Expansion step $amokcount:</h2>\n");
   if ($foundonemul!=-1) { debug("<p>$string\n<br>"); } else { debug("<p>found no multipliers to expand\n<br>") }
}

sub expandtracks {
   local $curtrack=0;
   local $temptrack;
   $string.=" |";
   while ($string=~s/(.*?)\|//) {
      $temptrack=$1;
      if ($temptrack=~s/^(\d+)//) {
         $curtrack=$1;
      } else {
         $curtrack+=1;
      }
   $track[$curtrack].= $temptrack;
   }
}


sub guitarchord {
   local ($subchord) = @_;
   local @chordtemp = split(/,/, $subchord);
   @chord=();
   $chordindex=0;
   for ($c=0; $c<6; $c++) {
      if (($chordtemp[$c]!='-')||(!$chordtemp[$c])) {
         $chord[$chordindex]=$tuning[$c]+$chordtemp[$c];
         $chordindex++;
      }
   }
   debugmax("\n<br>guitar chord: $subchord (".join(',',@chord).")");
}

sub add_track {
   local ($stringx) = @_;
   local ($length, $ticks_on, $ticks_off, $delta_on, $delta_off);
   local $seqtime = 0;
   local $abstime = 0;
   local $nlength = 4;
   local $octave = 4;
   local $note = 65;
   local @chord=($note);
   local $chan = 0;
   local $nratio = 1;
   local $nduty = ($gduty)/100;
   local $vel = $gvel;
   local $trans=0;
   local $nivstress=50;
   local $nivsoft=25;
   local $loosew=&timetotick($gloosew);
   local $looseq=$glooseq;
   local $velvarw=$gvelvarw;
   local $velvarq=$gvelvarq;
   local $guitmode=$globalguitmode;
   local $strumdelay=0;
   local $strumup=0;
   local $strumhitdown=0;
   local $strumupvel=100;
   local @tuning=qw(40 45 50 55 59 64);
   local $tommode=0;
   $track = "";

   $stringx=~s/[\(\)]//g;             # delete ( )
   $stringx=~s/x/ x /ig;              # explode x-= patterns: x
   $stringx=~s/' x/'x/ig;             # re-compact 'x
   $stringx=~s/, x/,x/ig;             # re-compact ,x
   $stringx=~s/syse x /sysex/ig;      # re-compact sysex
   $stringx=~s/-/ - /ig;              # explode x-= patterns: - 
   $stringx=~s/([A-GNTH]) - /$1-/ig;  # re-compact A- style notes and T- (transpose, nt) and N- (note) and H- (pitch).
   $stringx=~s/, - /,-/ig;            # re-compact ,- (inside chord)
   $stringx=~s/\[ - /\[-/ig;          # re-compact [- (first negative note in chord)
   $stringx=~s/g: - ,/g:-,/ig;        # re-compact g:-, (first not-played guitar string)
   $stringx=~s/, - ,/,-,/ig;          # re-compact ,-, (not-played guitar string)
   $stringx=~s/, - \]/,-\]/ig;        # re-compact ,-] (last not-played guitar string)
   $stringx=~s/=/ = /ig;              # explode x-= patterns: =
   $stringx=~s/\s+=/=/ig;             # re-compact = to the left
   $stringx=~ s/\s+/ /g;             # compact whitespace

debug("\n<br>\n<h2>--:: Track ::--</h2>\n<p>$stringx\n<br>");
debugmax("\n<br>loose +/-$gloosewms (+/-$loosew ticks), q=$looseq ");
debugmax("\n<br>velvar +/-$gvelvarw%, q=$gvelvarq");

   local @values = split(/\s+/, $stringx); 

   foreach $command (@values) {
debugmax("\n<br>--- command: $command");
      $pause=0;
      $stress=0;
      $soft=0;
      $hold=1;
      $deltanote=0;
      $temptrans=0;

# before everything: ignore empty commands

      if ($command=~/^\s*$/) {next; }

# first: chords

      if ($command=~s/tuning\[(.*)\]//i) {        # guitar tuning
      	 $tuningcommand=$1;
         @tuning = split(/,/, $tuningcommand);
         foreach $ch (@tuning) {
            ($tunnote, $tunoct) = $ch=~m/([A-G][b#\+-]?)(\d)/i;
            if (defined($notemap{uc($tunnote)})) {
               $ch=$notemap{uc($tunnote)}+ 12*$tunoct;
            } else {
               error('tuning definition problem in '.$tuningcommand.' : I don\'t understand '.$ch);
            }
         }
debugmax("\n<br>guitar tuning: $1 (".join(',',@tuning).")");
      next;
      }   

      if ($command=~s/\[(\-?\d+,.*)\]//i) {       # first form: [-5,0,3,7,...] relative to actual note (begins with a figure, then a comma)
         @chord = split(/,/, $1);
         foreach $ch (@chord) { $ch+=$note; }
debugmax("\n<br>relative chord: $1 (".join(',',@chord).")");
      }   

      if ($command=~s/\[g:((-|\d+),.*)\]//i) {    # second form: [g:-,3,5,...], guitar chord relative to actual guitar tuning 
         &guitarchord($1);                        # (begins with g: and a figure or a -, then a comma)
      }   

      if ($command=~s/\[g:(.*)\]//i) {            # third form: [g:Amaj], standard guitar chord (begins with a g: then anything)
         $keycommand=$1;
         if (!defined($guitchords{uc($keycommand)})) {
            $keycommand=~s/:.*//i;                      # get rid of :version
            if (!defined($guitchords{uc($keycommand)})) {
               error('I don\'t know guitar chord '.$1);
            }
         }
         &guitarchord($guitchords{uc($keycommand)});
      }   

      if ($command=~s/\[(.*)\]//i) {              # fourth form: [Am], Standard chord (anything in brackets)
         $keycommand=$1;
         $keycommand2=$1;
         if (!defined($keychords{uc($keycommand)})) {           # unknown chord...
            if ($keycommand=~s/^([A-G][b#\+-]?)//i) {           # but there seem to be a note first
               $keynote=$1;
               if (!defined($keychords{uc($keycommand)})) {
                  $keycommand=~s/:.*//i;                      # last chance, get rid of :version
                  if (!defined($keychords{uc($keycommand)})) {
                     error('I don\'t know chord '.$keycommand.' in '.$keycommand2);  # no, really, unknown chord...
                  }
               }
               if (!defined($notemap{uc($keynote)})) {
                  error('I don\'t know note '.$keynote.' in '.$keycommand2);      # no, unknown note
               }
               $note=$notemap{uc($keynote)}+ 12*$octave;                          # yes, I change the note
            } else {
               $keycommand=~s/:.*//i;                      # last chance, get rid of :version
               if (!defined($keychords{uc($keycommand)})) {
                  error('I don\'t know chord '.$keycommand2);      # no, unknown chord...
               }
            }
         }
         @chord = split(/,/, $keychords{uc($keycommand)} );
         foreach $ch (@chord) { $ch+=$note; }
debugmax("\n<br>chord: keycommand2 (".join(',',@chord).")");
      }   


# then the multi-letters commands

      if ($command=~s/strum(\d+),(\d+),(\d+)//i) {                     # strum definition (delay and upstrike min delay and velocity%)
      	 $strumdelay=&timetotick($1);
      	 $strumup=&timetotick($2);
      	 $strumupvel=$3;
      	 debugmax("\n<br>strum delay $1ms ($strumdelay ticks), upstrike delay $2ms ($strumup ticks), strumpup velocity $strumupvel%");
      	 next;
      }    
      if ($command=~s/strum(\d+),(\d+)//i) {                           # strum definition (delay and upstrike min delay)
      	 $strumdelay=&timetotick($1);
      	 $strumup=&timetotick($2);
      	 debugmax("\n<br>strum delay $1ms ($strumdelay ticks), upstrike delay $2ms ($strumup ticks)");
      	 next;
      }    
      if ($command=~s/strum(\d+)//i) {                                 # strum definition (delay only)
      	 $strumdelay=&timetotick($1);
      	 debugmax("\n<br>strum delay $1ms ($strumdelay ticks)");
      	 next; 
      }
      if ($command=~s/tomson//i) {                                     # toms mode on
         $tommode=1;
         $chan=10-1;
         $chan &= 0xF;     
         $note = $drummap{'T4'};
         @chord=($note); 
    	 debugmax("\n<br>midi time $abstime: toms mode on");
      	 next; 
      }
      if ($command=~s/tomsoff//i) {                                     # toms mode on
         $tommode=0;
    	 debugmax("\n<br>midi time $abstime: toms mode off");
      	 next; 
      }
      if ($command=~s/guiton//i) { $guitmode=1; debugmax("\n<br>guitar mode on"); next; }             # guitar mode on
      if ($command=~s/guitoff//i) { $guitmode=0; debugmax("\n<br>guitar mode off"); next; }           # guitar mode off
      if ($command=~s/stress(\d+)//i) { $nivstress=$1; debugmax("\n<br>stress $nivstress"); next; }   # stress definition
      if ($command=~s/soft(\d+)//i) { $nivsoft=$1; debugmax("\n<br>soft $nivsoft"); next; }           # soft definition

      if ($command=~s/loose(\d+),(\d+|g)//i) {     # loosea,b
      	 $loosew=&timetotick($1); 
      	 $looseq=$2; 
      	 debugmax("\n<br>loose +/-$1ms (+/-$loosew ticks), q=$looseq "); 
      	 next; 
      }
      if ($command=~s/velvar(\d+),(\d+|g)//i) {    # velvara,b
      	 $velvarw=$1; 
      	 $velvarq=$2; 
      	 debugmax("\n<br>velvar +/-$1%, q=$2"); 
      	 next; 
      }

      if ($command=~s/ctrl(\d+),(\d+)//i) {      # ctrla,b : midi controler
      	 $track .= pack ('w C3', round($abstime-$seqtime), 0xB0 | $chan, $1, $2);
      	 $seqtime=$abstime;
      	 next; 
      }
      if ($command=~s/sysex(.*)//i) {            # sysexa,b,c,d,... : midi sysex message
      	 debugmax("\n<br>midi time $abstime: sysex: $1");
         local @sysex = split(/,/, $1);
      	 $track .= pack ('w C w', round($abstime-$seqtime), 240, ($#sysex+1));
      	 foreach $sys (@sysex) { $track .= pack ('C', $sys ); }
      	 $track .= pack ('C', 247);
      	 $seqtime=$abstime;
       	 next; 
      }
      if ($command=~s/pitch\+(\d+)//i) {       # pitch+nb: positive pitch
      	 debugmax("\n<br>midi time $abstime: pitch: +$1");
      	 $pitch = (8192*$1/100)+8192;
      	 if ($pitch>16383) { $pitch=16383; }
      	 if ($pitch<0) { $pitch=0; }
      	 $track .= pack ('w C3', round($abstime-$seqtime), 0xE0 | $chan, ($pitch & 0x7F), (($pitch>>7)& 0x7F) );
      	 $seqtime=$abstime;
      	 next; 
      }
      if ($command=~s/pitch-(\d+)//i) {        # pitch-nb: negative pitch
      	 debugmax("\n<br>midi time $abstime: pitch: -$1");
      	 $pitch = 8192-(8192*$1/100);
      	 if ($pitch>16383) { $pitch=16383; }
      	 if ($pitch<0) { $pitch=0; }
      	 $track .= pack ('w C3', round($abstime-$seqtime), 0xE0 | $chan, ($pitch & 0x7F), (($pitch>>7)& 0x7F) );
      	 $seqtime=$abstime;
      	 next; 
      }
      if ($command=~s/pitch0//i) {            # pitch0: zero pitch
      	 debugmax("\n<br>midi time $abstime: pitch: 0");
      	 $pitch = 8192;
      	 $track .= pack ('w C3', round($abstime-$seqtime), 0xE0 | $chan, ($pitch & 0x7F), (($pitch>>7)& 0x7F) );
      	 $seqtime=$abstime;
      	 next; 
      }


# Then the dual/tri-letters commands

      if ($command=~s/ch(\d+)//i) {              # ch : channel definition
      	 $chan=$1-1;
         $chan &= 0xF;     
      	 next; 
      }
      if ($command=~s/i(\d+)//i) {               # i + nb: instrument definition
      	 $track .= pack ('w C2', 0, 0xC0 | $chan, ($1-1));
  	 debugmax("\n<br>midi time $abstime: defined instrument $1");
      	 next; 
        
      }
      if ($command=~s/i([A-Z][A-Z0-9])//i) {     # i + 2 letters/digits: drum instrument definition
         if (defined($drummap{uc($1)})) { 
            $note = $drummap{uc($1)};
            $chan=10-1;
            $chan &= 0xF;     
            @chord=($note); 
      	 debugmax("\n<br>midi time $abstime: defined drum instrument $1");
         }
      	 next; 
      }
      if ($command=~s/nt\+(\d)//i) { $temptrans=$1; }  # nt+ : temporary relative note
      if ($command=~s/nt-(\d)//i) { $temptrans=-$1; }  # nt- : temporary relative note

# Then the uni-letters commands

      if ($command=~s/r(\d+)\/(\d+)//i) { $nratio=$1/$2; debugmax("\n<br>ratio $nratio ($1/$2)"); next; }   # ratio Ra/b
      if ($command=~s/r1//i) { $nratio=1; debugmax("\n<br>ratio $nratio"); next; }                          # ratio R1
      if ($command=~s/u(\d+)//i) { $nduty=($1)/100; debugmax("\n<br>duty $nduty"); next; }                  # u duty
      if ($command=~s/v(\d+)//i) { $vel=$1; debugmax("\n<br>vel $vel"); next; }                             # v velocity
      if ($command=~s/t\+(\d+)//i) { $trans=$1; debugmax("\n<br>transpose +$trans"); next; }                # t+ transpose up
      if ($command=~s/t0//i) { $trans=0; debugmax("\n<br>transpose $trans"); next; }                        # t0 transpose 0
      if ($command=~s/t\-(\d+)//i) { $trans=-$1; debugmax("\n<br>transpose $trans"); next; }                # t- transpose down

      if ($command=~s/^\s*\/\s*$//i) { $octave+=1; debugmax("\n<br>octave +1 ($octave)"); next; }           # / octave +1
      if ($command=~s/^\s*\\\s*$//i) { $octave-=1; debugmax("\n<br>octave -1 ($octave)"); next; }           # \ octave -1

      if ($command=~s/\/(\d+)//i) { $nlength=$1; }                                # length after /
      if ($command=~s/([A-GO][b#\+-]?)(\d)/$1/i) { $octave=$2; }                  # octave after A..F+ or O
      if ($command=~s/([A-G][b#\+-]?)//i) {                                       # note
         if (defined($notemap{uc($1)})) { 
            $note=$notemap{uc($1)}+ 12*$octave;
            @chord=($note); 
         }
      }
      if ($command=~s/N\+(\d)//i) { $deltanote=$1; }                              # n+ : relative note
      if ($command=~s/N-(\d)//i) { $deltanote=-$1; }                              # n- : relative note
      if ($command=~s/N(\d+)//i) { $note=$1; @chord=($note); }                    # n : absolute note

      if ($command=~s/o//i) { debugmax(" not played"); next; }                    # o => next value

      $command=~s/=/$hold++;''/eg ;                                               # counts '=' in $hold
      if ($command=~s/P//i) { $pause=1; }                                         # p => pause
      if ($command=~s/-//i) { $pause=1; }                                         # - => pause (after A- notes)
      if ($command=~s/'//i) { $stress=1; }                                        # ' => stress
      if ($command=~s/,//i) { $soft=1; }                                          # , => soft
      
      if ($command=~s/(\d+)//i) {                                                 # if there is a number left, it's either:
      	 if ($guitmode) {
      	    $temptrans=$1;                                                  # - temporary transpose if guitmode on
         } elsif ($tommode) {                                                          # - or a tom indication if tommode on
            $tom='T'.$1;
            if (defined($drummap{$tom})) { 
               $note = $drummap{$tom};
               @chord=($note); 
            }
         } else {
            $nlength=$1;                                                    # - or length alone if guitmode off
         }
      }

#      unless ($command=~ /[x]*/i) { next; } # if doesn't match x, it's not something we want to play

   if (!$nlength) { error('You seem to be trying to play a note of length 1/0 ... Haven\'t you mixed the guitar mode with normal mode ?'); }
      $length = (( 1 / $nlength ) *$nratio)*$hold;
      $length *= $ppqn * 4;
      if ($deltanote) { $note+=$deltanote; @chord=($note); }
      if ($pause) {
         $abstime += $length;
      } else {

debugmax("\n<br>midi time $abstime:");

         if (($strumup) && (($abstime-$previousnotetime)<$strumup) && ($strumhitdown)) {
            @chordtemp=reverse @chord;
            $strumhitdown=0;
debugmax(" upstrike")
         } else {
            @chordtemp=@chord;
            $strumhitdown=1;
         }

         $lvel=$vel*(1+$velvarw*&rndq($velvarq)/100)*($strumhitdown?1:$strumupvel/100);
         if ($stress) { $lvel*=(1+$nivstress/100) }
         if ($soft) { $lvel*=(1-$nivsoft/100) }
         $lvel=round($lvel);
         if ($lvel<0) {$lvel=0}
         if ($lvel>127) {$lvel=127}

         $ticks_on = $nduty * $length;
         $ticks_off = (1-$nduty) * $length;
         $firstnote = 1;

         $previousnotetime=$abstime;
         
         foreach $chnote (@chordtemp) {
            $fnote=$chnote+$trans+$temptrans;
            if ($fnote<0) {$fnote=0}
            if ($fnote>127) {$fnote=127}
            if ($firstnote) {
               $abstimetemp=$abstime+$loosew*&rndq($looseq);
               $delta_on = &round($abstimetemp) - $seqtime;
               if ($delta_on<=(round($loosew)*2)) {$abstimetemp-=$delta_on; $delta_on=0;}
               $firstnote=0;
            } else {
               $abstimetemp+=$strumdelay;
               $delta_on = &round($abstimetemp) - $seqtime;
            }
            if (!$debug) { $track .= pack ('w C3', $delta_on, 0x90 | $chan, $fnote, $lvel); }
debugmax("\n<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Delta $delta_on NoteOn chan $chan note $fnote vel $lvel");
            $seqtime += $delta_on;
         }

         $abstime += $ticks_on;
         $firstnote = 1;
    
         foreach $chnote (@chordtemp) {
            $fnote=$chnote+$trans+$temptrans;
            if ($fnote<0) {$fnote=0}
            if ($fnote>127) {$fnote=127}
            if ($firstnote) {
               $delta_off = &round($abstime+$loosew*&rndq($looseq)) - $seqtime;
               if ($delta_off<0) {$delta_off=0}
               $firstnote=0;
            } else {
               $delta_off = 0;
            }
            if (!$debug) { $track .= pack ('w C3', $delta_off, 0x80 | $chan, $fnote, 0); }
debugmax("\n<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Delta $delta_off NoteOff chan $chan note $fnote");
            $seqtime += $delta_off;
         }
         
         $abstime += $ticks_off;
  
      }
   }
   $aaa=&round($abstime) - $seqtime;
   if ($aaa<0) { $aaa=0 }
   $track .= pack('w C3', $aaa , 0xFF, 0x2f, 0); 
   push(@mtrack, $track); 
}

sub rndq {
   local ($q)=@_;
   local $r=rand;
   if (uc($q)=='G') {
      our ($phase, $U1, $V1, $S);
      my $X;
      if ($phase == 0) {
         while (1) {
            $U1 = rand; $U2 = rand;
            $V1 = 2 * $U1 - 1; $V2 = 2 * $U2 - 1;
            $S = $V1 * $V1 + $V2 * $V2;
            unless ($S >= 1 || $S == 0) {last; }
         }
         $X = $V1 * sqrt(-2 * log($S) / $S);
      } else {
         $X = $V2 * sqrt(-2 * log($S) / $S);
      }
      $phase = 1 - $phase;
      return $X/2;
   } else {
      return ((abs($r-.5)*2)**$q)*($r>.5?1:-1)
   }
}

sub post_out {
   local $pretrack;
   local $pretrack_output = 0;
   local $mtrack0;
   local $wholetrack = "";
   if ($midi) {
      local $t = 1000000 * 60 / $tempo;
      local $ntrks = @mtrack;
      $pretrack = "MThd" . pack('Nn3', 6, 1, $ntrks, $ppqn);
      $mtrack0 = pack('C3 C', 0, 0xFF, 1, length($version)) . $version .
         pack('C7', 0, 0xFF, 0x51, 3, ($t>>16)&0xFF, ($t>>8)&0xFF, $t&0xFF);

      foreach(@mtrack) {
         if (! $pretrack_output) {
            $mtrack0 .= $_;
            $wholetrack = $pretrack . "MTrk" . pack('N', length($mtrack0)) . $mtrack0;
            $pretrack_output = 1;
         } else {
            $wholetrack .= "MTrk" . pack('N', length($_)) . $_;
         }
      }
      binmode (STDOUT);
   }
   print "Content-Type:application/x-download\n";  
   print "Content-Disposition:attachment;filename=newmusic.mid\n\n"; 
   print "$wholetrack";

}

sub round {
   int($_[0] + 0.5 );
}

sub timetotick {
   ($_[0]/1000)*($tempo/60)*$ppqn
}

sub initialize_guitarchords {

%guitchords = (
'A/AB' => '-,0,2,1,2,0',
'A/B' => '0,0,2,4,2,0',
'A/B:1' => '-,0,7,6,0,0',
'A/D' => '-,0,0,2,2,0',
'A/D:1' => '-,-,0,2,2,0',
'A/D:2' => '-,-,0,6,5,5',
'A/D:3' => '-,-,0,9,10,9',
'A/G' => '3,-,2,2,2,0',
'A/G:1' => '-,0,2,0,2,0',
'A/G:2' => '-,0,2,2,2,3',
'A/GB' => '0,0,2,2,2,2',
'A/GB:1' => '0,-,4,2,2,0',
'A/GB:2' => '2,-,2,2,2,0',
'A/GB:3' => '-,0,4,2,2,0',
'A/GB:4' => '-,-,2,2,2,2',
'A5' => '5,7,7,-,-,5',
'A5:1' => '-,0,2,2,-,0',
'A5:2' => '5,7,7,-,-,0',
'A6' => '0,0,2,2,2,2',
'A6:1' => '0,-,4,2,2,0',
'A6:2' => '2,-,2,2,2,0',
'A6:3' => '-,0,4,2,2,0',
'A6:4' => '-,-,2,2,2,2',
'A6/7' => '0,0,2,0,2,2',
'A6/7:1' => '5,5,4,0,3,0',
'A6/7:2' => '-,0,2,0,3,2',
'A7' => '3,-,2,2,2,0',
'A7:1' => '-,0,2,0,2,0',
'A7:2' => '-,0,2,2,2,3',
'A7{#5}' => '1,0,3,0,2,1',
'A7/ADD11' => '-,0,0,0,2,0',
'A7SUS4' => '-,0,2,0,3,0',
'A7SUS4:1' => '-,0,2,0,3,3',
'A7SUS4:2' => '-,0,2,2,3,3',
'A7SUS4:3' => '5,-,0,0,3,0',
'A7SUS4:4' => '-,0,0,0,-,0',
'AADD9' => '0,0,2,4,2,0',
'AADD9:1' => '-,0,7,6,0,0',
'AAUG' => '-,0,3,2,2,1',
'AAUG:1' => '-,0,-,2,2,1',
'AAUG/D' => '-,-,0,2,2,1',
'AAUG/G' => '1,0,3,0,2,1',
'ADIM/AB' => '-,-,1,2,1,4',
'ADIM/E' => '0,3,-,2,4,0',
'ADIM/F' => '-,-,1,2,1,1',
'ADIM/F:1' => '-,-,3,5,4,5',
'ADIM/G' => '-,-,1,2,1,3',
'ADIM/GB' => '-,-,1,2,1,2',
'ADIMIN7' => '-,-,1,2,1,2',
'AMIN' => '-,0,2,2,1,0',
'AMIN:1' => '-,0,7,5,5,5',
'AMIN:2' => '-,3,2,2,1,0',
'AMIN:3' => '8,12,-,-,-,0',
'AMIN/B' => '0,0,7,5,0,0',
'AMIN/B:1' => '-,3,2,2,0,0',
'AMIN/D' => '-,-,0,2,1,0',
'AMIN/D:1' => '-,-,0,5,5,5',
'AMIN/EB' => '0,3,-,2,4,0',
'AMIN/F' => '0,0,3,2,1,0',
'AMIN/F:1' => '1,3,3,2,1,0',
'AMIN/F:2' => '1,-,2,2,1,0',
'AMIN/F:3' => '-,-,2,2,1,1',
'AMIN/F:4' => '-,-,3,2,1,0',
'AMIN/G' => '0,0,2,0,1,3',
'AMIN/G:1' => '-,0,2,0,1,0',
'AMIN/G:2' => '-,0,2,2,1,3',
'AMIN/G:3' => '-,0,5,5,5,8',
'AMIN/GB' => '-,0,2,2,1,2',
'AMIN/GB:1' => '-,-,2,2,1,2',
'AM6' => '-,0,2,2,1,2',
'AM6:1' => '-,-,2,2,1,2',
'AMIN7' => '0,0,2,0,1,3',
'AMIN7:1' => '-,0,2,0,1,0',
'AMIN7:2' => '-,0,2,2,1,3',
'AMIN7:3' => '-,0,5,5,5,8',
'AMIN7{B5}' => '-,-,1,2,1,3',
'AMIN7/ADD11' => '-,5,7,5,8,0',
'AMAJ' => '0,0,2,2,2,0',
'AMAJ:1' => '0,4,-,2,5,0',
'AMAJ:2' => '5,7,7,6,5,5',
'AMAJ:3' => '-,0,2,2,2,0',
'AMAJ:4' => '-,4,7,-,-,5',
'AMAJ7' => '-,0,2,1,2,0',
'AMIN/MAJ9' => '-,0,6,5,5,7',
'ASUS' => '0,0,2,2,3,0',
'ASUS:1' => '-,0,2,2,3,0',
'ASUS:2' => '5,5,7,7,-,0',
'ASUS:3' => '-,0,0,2,3,0',
'ASUS2' => '0,0,2,2,0,0',
'ASUS2:1' => '0,0,2,4,0,0',
'ASUS2:2' => '0,2,2,2,0,0',
'ASUS2:3' => '-,0,2,2,0,0',
'ASUS2:4' => '-,-,2,2,0,0',
'ASUS2/AB' => '-,0,2,1,0,0',
'ASUS2/C' => '0,0,7,5,0,0',
'ASUS2/C:1' => '-,3,2,2,0,0',
'ASUS2/D' => '0,2,0,2,0,0',
'ASUS2/D:1' => '-,2,0,2,3,0',
'ASUS2/DB' => '0,0,2,4,2,0',
'ASUS2/DB:1' => '-,0,7,6,0,0',
'ASUS2/EB' => '-,2,1,2,0,0',
'ASUS2/F' => '0,0,3,2,0,0',
'ASUS2/G' => '3,-,2,2,0,0',
'ASUS2/G:1' => '-,0,2,0,0,0',
'ASUS2/G:2' => '-,0,5,4,5,0',
'ASUS2/GB' => '-,0,4,4,0,0',
'ASUS2/GB:1' => '-,2,4,2,5,2',
'ASUS4/AB' => '4,-,0,2,3,0',
'ASUS4/B' => '0,2,0,2,0,0',
'ASUS4/BB' => '0,1,-,2,3,0',
'ASUS4/C' => '-,-,0,2,1,0',
'ASUS4/C:1' => '-,-,0,5,5,5',
'ASUS4/DB' => '-,0,0,2,2,0',
'ASUS4/DB:1' => '-,-,0,2,2,0',
'ASUS4/DB:2' => '-,-,0,6,5,5',
'ASUS4/DB:3' => '-,-,0,9,10,9',
'ASUS4/F' => '-,-,7,7,6,0',
'ASUS4/G' => '-,0,2,0,3,0',
'ASUS4/G:1' => '-,0,2,0,3,3',
'ASUS4/G:2' => '-,0,2,2,3,3',
'ASUS4/G:3' => '-,0,0,0,-,0',
'ASUS4/GB' => '0,0,0,2,3,2',
'ASUS4/GB:1' => '0,0,4,2,3,0',
'ASUS4/GB:2' => '2,-,0,2,3,0',
'ASUS4/GB:3' => '-,0,2,2,3,2',
'ASUS4/GB:4' => '-,-,2,2,3,2',
'ASUS4/GB:5' => '-,5,4,2,3,0',
'ASUS4/GB:6' => '-,9,7,7,-,0',
'AB/A' => '-,-,1,2,1,4',
'AB/F' => '-,8,10,8,9,8',
'AB/F:1' => '-,-,1,1,1,1',
'AB/GB' => '-,-,1,1,1,2',
'AB/GB:1' => '-,-,4,5,4,4',
'AB5' => '4,6,6,-,-,4',
'AB6' => '-,8,10,8,9,8',
'AB6:1' => '-,-,1,1,1,1',
'AB7' => '-,-,1,1,1,2',
'AB7:1' => '-,-,4,5,4,4',
'ABAUG' => '-,3,2,1,1,0',
'ABDIM/E' => '0,2,0,1,0,0',
'ABDIM/E:1' => '0,2,2,1,3,0',
'ABDIM/E:2' => '-,2,0,1,3,0',
'ABDIM/E:3' => '-,-,0,1,0,0',
'ABDIM/EB' => '-,-,0,4,4,4',
'ABDIM/F' => '-,2,0,1,0,1',
'ABDIM/F:1' => '-,-,0,1,0,1',
'ABDIM/F:2' => '-,-,3,4,3,4',
'ABDIMIN7' => '-,2,0,1,0,1',
'ABDIMIN7:1' => '-,-,0,1,0,1',
'ABDIMIN7:2' => '-,-,3,4,3,4',
'ABMIN' => '-,-,6,4,4,4',
'ABMIN/D' => '-,-,0,4,4,4',
'ABMIN/E' => '0,2,1,1,0,0',
'ABMIN/E:1' => '0,-,6,4,4,0',
'ABMIN/E:2' => '-,-,1,1,0,0',
'ABMIN/GB' => '-,-,4,4,4,4',
'ABMIN7' => '-,-,4,4,4,4',
'ABMAJ' => '4,6,6,5,4,4',
'ABSUS' => '-,-,6,6,4,4',
'ABSUS2/F' => '-,1,3,1,4,1',
'B/A' => '2,-,1,2,0,2',
'B/A:1' => '-,0,1,2,0,2',
'B/A:2' => '-,2,1,2,0,2',
'B/A:3' => '-,2,4,2,4,2',
'B/AB' => '-,-,4,4,4,4',
'B/E' => '-,2,2,4,4,2',
'B/E:1' => '-,-,4,4,4,0',
'B5' => '7,9,9,-,-,2',
'B5:1' => '-,2,4,4,-,2',
'B6' => '-,-,4,4,4,4',
'B7' => '2,-,1,2,0,2',
'B7:1' => '-,0,1,2,0,2',
'B7:2' => '-,2,1,2,0,2',
'B7:3' => '-,2,4,2,4,2',
'B7/ADD11' => '0,0,4,4,4,0',
'B7/ADD11:1' => '0,2,1,2,0,2',
'B7SUS4' => '-,0,4,4,0,0',
'B7SUS4:1' => '-,2,4,2,5,2',
'BAUG' => '3,2,1,0,0,3',
'BAUG:1' => '3,-,1,0,0,3',
'BAUG/E' => '3,-,1,0,0,0',
'BAUG/E:1' => '-,-,1,0,0,0',
'BB' => '-,-,0,3,-,0',
'BBAUG' => '-,-,0,3,3,2',
'BBMAJ' => '1,1,3,3,3,1',
'BBMAJ:1' => '-,1,3,3,3,1',
'BBMAJ:2' => '-,-,0,3,3,1',
'BDIM/A' => '1,2,3,2,3,1',
'BDIM/A:1' => '-,2,0,2,0,1',
'BDIM/A:2' => '-,-,0,2,0,1',
'BDIM/AB' => '-,2,0,1,0,1',
'BDIM/AB:1' => '-,-,0,1,0,1',
'BDIM/AB:2' => '-,-,3,4,3,4',
'BDIM/G' => '1,-,0,0,0,3',
'BDIM/G:1' => '3,2,0,0,0,1',
'BDIM/G:2' => '-,-,0,0,0,1',
'BDIMIN7' => '-,2,0,1,0,1',
'BDIMIN7:1' => '-,-,0,1,0,1',
'BDIMIN7:2' => '-,-,3,4,3,4',
'BMIN' => '2,2,4,4,3,2',
'BMIN:1' => '-,2,4,4,3,2',
'BMIN:2' => '-,-,0,4,3,2',
'BMIN/A' => '-,0,4,4,3,2',
'BMIN/A:1' => '-,2,0,2,0,2',
'BMIN/A:2' => '-,2,0,2,3,2',
'BMIN/A:3' => '-,2,4,2,3,2',
'BMIN/A:4' => '-,-,0,2,0,2',
'BMIN/G' => '2,2,0,0,0,3',
'BMIN/G:1' => '2,2,0,0,3,3',
'BMIN/G:2' => '3,2,0,0,0,2',
'BMIN/G:3' => '-,-,4,4,3,3',
'BMIN7' => '-,0,4,4,3,2',
'BMIN7:1' => '-,2,0,2,0,2',
'BMIN7:2' => '-,2,0,2,3,2',
'BMIN7:3' => '-,2,4,2,3,2',
'BMIN7:4' => '-,-,0,2,0,2',
'BMIN7{B5}' => '1,2,3,2,3,1',
'BMIN7{B5}:1' => '-,2,0,2,0,1',
'BMIN7{B5}:2' => '-,-,0,2,0,1',
'BMIN7/ADD11' => '0,0,2,4,3,2',
'BMIN7/ADD11:1' => '0,2,0,2,0,2',
'BMAJ' => '-,2,4,4,4,2',
'BMAJ7/#11' => '-,2,3,3,4,2',
'BSUS' => '7,9,9,-,-,0',
'BSUS:1' => '-,2,4,4,-,0',
'BSUS2' => '-,4,4,4,-,2',
'BSUS2:1' => '-,-,4,4,2,2',
'BSUS2/E' => '-,4,4,4,-,0',
'BSUS4/A' => '-,0,4,4,0,0',
'BSUS4/A:1' => '-,2,4,2,5,2',
'BSUS4/AB' => '0,2,2,1,0,2',
'BSUS4/AB:1' => '0,-,4,1,0,0',
'BSUS4/AB:2' => '2,2,2,1,0,0',
'BSUS4/DB' => '-,4,4,4,-,0',
'BSUS4/EB' => '-,2,2,4,4,2',
'BSUS4/EB:1' => '-,-,4,4,4,0',
'BSUS4/G' => '0,2,2,0,0,2',
'BSUS4/G:1' => '0,2,4,0,0,0',
'BSUS4/G:2' => '0,-,4,0,0,0',
'BSUS4/G:3' => '2,2,2,0,0,0',
'BB/A' => '1,1,3,2,3,1',
'BB/AB' => '-,1,3,1,3,1',
'BB/AB:1' => '-,-,3,3,3,4',
'BB/DB' => '-,-,0,6,6,6',
'BB/E' => '-,1,3,3,3,0',
'BB/G' => '3,5,3,3,3,3',
'BB/G:1' => '-,-,3,3,3,3',
'BB5' => '6,8,8,-,-,6',
'BB5:1' => '-,1,3,3,-,6',
'BB6' => '3,5,3,3,3,3',
'BB6:1' => '-,-,3,3,3,3',
'BB6/ADD9' => '-,3,3,3,3,3',
'BB7' => '-,1,3,1,3,1',
'BB7:1' => '-,-,3,3,3,4',
'BB7SUS4' => '-,1,3,1,4,1',
'BBADD#11' => '-,1,3,3,3,0',
'BBAUG/E' => '2,-,4,3,3,0',
'BBDIM/C' => '-,3,-,3,2,0',
'BBDIM/D' => '-,-,0,3,2,0',
'BBDIM/G' => '-,1,2,0,2,0',
'BBDIM/G:1' => '-,-,2,3,2,3',
'BBDIM/GB' => '2,4,2,3,2,2',
'BBDIM/GB:1' => '-,-,4,3,2,0',
'BBDIMIN7' => '-,1,2,0,2,0',
'BBDIMIN7:1' => '-,-,2,3,2,3',
'BBMIN' => '1,1,3,3,2,1',
'BBMIN/AB' => '-,1,3,1,2,1',
'BBMIN/D' => '-,-,0,6,6,6',
'BBMIN/GB' => '-,-,3,3,2,2',
'BBMIN7' => '-,1,3,1,2,1',
'BBMAJ7' => '1,1,3,2,3,1',
'BBMAJ9' => '-,3,3,3,3,5',
'BBSUS2' => '-,-,3,3,1,1',
'BBSUS2/G' => '-,3,5,3,6,3',
'BBSUS4/AB' => '-,1,3,1,4,1',
'C/A' => '0,0,2,0,1,3',
'C/A:1' => '-,0,2,0,1,0',
'C/A:2' => '-,0,2,2,1,3',
'C/A:3' => '-,0,5,5,5,8',
'C/B' => '0,3,2,0,0,0',
'C/B:1' => '-,2,2,0,1,0',
'C/B:2' => '-,3,5,4,5,3',
'C/BB' => '-,3,5,3,5,3',
'C/D' => '3,-,0,0,1,0',
'C/D:1' => '-,3,0,0,1,0',
'C/D:2' => '-,3,2,0,3,0',
'C/D:3' => '-,3,2,0,3,3',
'C/D:4' => '-,-,0,0,1,0',
'C/D:5' => '-,-,0,5,5,3',
'C/D:6' => '-,10,12,12,13,0',
'C/D:7' => '-,5,5,5,-,0',
'C/F' => '-,3,3,0,1,0',
'C/F:1' => '-,-,3,0,1,0',
'C5' => '-,3,5,5,-,3',
'C6' => '0,0,2,0,1,3',
'C6:1' => '-,0,2,0,1,0',
'C6:2' => '-,0,2,2,1,3',
'C6:3' => '-,0,5,5,5,8',
'C6/ADD9' => '-,5,7,5,8,0',
'C7' => '-,3,5,3,5,3',
'C7SUS4' => '-,3,5,3,6,3',
'C9(B5)' => '0,3,-,3,3,2',
'CADD9' => '3,-,0,0,1,0',
'CADD9:1' => '-,3,0,0,1,0',
'CADD9:2' => '-,3,2,0,3,0',
'CADD9:3' => '-,3,2,0,3,3',
'CADD9:4' => '-,-,0,0,1,0',
'CADD9:5' => '-,-,0,5,5,3',
'CADD9:6' => '-,10,12,12,13,0',
'CADD9:7' => '-,3,2,0,3,0',
'CADD9:8' => '-,5,5,5,-,0',
'CAUG' => '-,-,4,5,-,0',
'CAUG:1' => '-,3,2,1,1,0',
'CDIM/A' => '-,-,1,2,1,2',
'CDIM/AB' => '-,-,1,1,1,2',
'CDIM/AB:1' => '-,-,4,5,4,4',
'CDIM/D' => '-,5,4,5,4,2',
'CDIMIN7' => '-,-,1,2,1,2',
'CMIN' => '-,3,5,5,4,3',
'CMIN:1' => '-,-,5,5,4,3',
'CMIN/A' => '-,-,1,2,1,3',
'CMIN/BB' => '-,3,5,3,4,3',
'CM6' => '-,-,1,2,1,3',
'CMIN7' => '-,3,5,3,4,3',
'CMAJ' => '0,3,2,0,1,0',
'CMAJ:1' => '0,3,5,5,5,3',
'CMAJ:2' => '3,3,2,0,1,0',
'CMAJ:3' => '3,-,2,0,1,0',
'CMAJ:4' => '-,3,2,0,1,0',
'CMAJ:5' => '-,3,5,5,5,0',
'CMAJ7' => '0,3,2,0,0,0',
'CMAJ7:1' => '-,2,2,0,1,0',
'CMAJ7:2' => '-,3,5,4,5,3',
'CMAJ9' => '-,3,0,0,0,0',
'CSUS' => '-,3,3,0,1,1',
'CSUS:1' => '-,-,3,0,1,1',
'CSUS2' => '-,10,12,12,13,3',
'CSUS2:1' => '-,5,5,5,-,3',
'CSUS2:2' => '-,3,0,0,3,3',
'CSUS2:3' => '-,3,5,5,3,3',
'CSUS2/A' => '-,5,7,5,8,3',
'CSUS2/A:1' => '-,-,0,2,1,3',
'CSUS2/B' => '3,3,0,0,0,3',
'CSUS2/B:1' => '-,3,0,0,0,3',
'CSUS2/E' => '3,-,0,0,1,0',
'CSUS2/E:1' => '-,3,0,0,1,0',
'CSUS2/E:2' => '-,3,2,0,3,0',
'CSUS2/E:3' => '-,3,2,0,3,3',
'CSUS2/E:4' => '-,-,0,0,1,0',
'CSUS2/E:5' => '-,-,0,5,5,3',
'CSUS2/E:6' => '-,10,12,12,13,0',
'CSUS2/E:7' => '-,5,5,5,-,0',
'CSUS2/F' => '3,3,0,0,1,1',
'CSUS4/A' => '3,-,3,2,1,1',
'CSUS4/A:1' => '-,-,3,2,1,3',
'CSUS4/B' => '-,3,3,0,0,3',
'CSUS4/BB' => '-,3,5,3,6,3',
'CSUS4/D' => '3,3,0,0,1,1',
'CSUS4/E' => '-,3,3,0,1,0',
'CSUS4/E:1' => '-,-,3,0,1,0',
'D/B' => '-,0,4,4,3,2',
'D/B:1' => '-,2,0,2,0,2',
'D/B:2' => '-,2,0,2,3,2',
'D/B:3' => '-,2,4,2,3,2',
'D/B:4' => '-,-,0,2,0,2',
'D/C' => '-,5,7,5,7,2',
'D/C:1' => '-,0,0,2,1,2',
'D/C:2' => '-,3,-,2,3,2',
'D/C:3' => '-,5,7,5,7,5',
'D/DB' => '-,-,0,14,14,14',
'D/DB:1' => '-,-,0,2,2,2',
'D/E' => '0,0,0,2,3,2',
'D/E:1' => '0,0,4,2,3,0',
'D/E:2' => '2,-,0,2,3,0',
'D/E:3' => '-,0,2,2,3,2',
'D/E:4' => '-,-,2,2,3,2',
'D/E:5' => '-,5,4,2,3,0',
'D/E:6' => '-,9,7,7,-,0',
'D/G' => '5,-,4,0,3,5',
'D/G:1' => '3,-,0,2,3,2',
'D5' => '5,5,7,7,-,5',
'D5:1' => '-,0,0,2,3,5',
'D6' => '-,0,4,4,3,2',
'D6:1' => '-,2,0,2,0,2',
'D6:2' => '-,2,0,2,3,2',
'D6:3' => '-,2,4,2,3,2',
'D6:4' => '-,-,0,2,0,2',
'D6/ADD9' => '0,0,2,4,3,2',
'D6/ADD9:1' => '0,2,0,2,0,2',
'D7' => '-,5,7,5,7,2',
'D7:1' => '-,0,0,2,1,2',
'D7:2' => '-,3,-,2,3,2',
'D7:3' => '-,5,7,5,7,5',
'D7SUS4' => '-,5,7,5,8,3',
'D7SUS4:1' => '-,-,0,2,1,3',
'D9' => '0,0,0,2,1,2',
'D9:1' => '2,-,0,2,1,0',
'D9:2' => '-,5,7,5,7,0',
'D9(#5)' => '0,3,-,3,3,2',
'DADD9' => '0,0,0,2,3,2',
'DADD9:1' => '0,0,4,2,3,0',
'DADD9:2' => '2,-,0,2,3,0',
'DADD9:3' => '-,0,2,2,3,2',
'DADD9:4' => '-,-,2,2,3,2',
'DADD9:5' => '-,5,4,2,3,0',
'DADD9:6' => '-,9,7,7,-,0',
'DAUG' => '-,-,0,3,3,2',
'DAUG/E' => '2,-,4,3,3,0',
'DDIM/B' => '-,2,0,1,0,1',
'DDIM/B:1' => '-,-,0,1,0,1',
'DDIM/B:2' => '-,-,3,4,3,4',
'DDIM/BB' => '-,1,3,1,3,1',
'DDIM/BB:1' => '-,-,3,3,3,4',
'DDIM/C' => '-,-,0,1,1,1',
'DDIMIN7' => '-,2,0,1,0,1',
'DDIMIN7:1' => '-,-,0,1,0,1',
'DDIMIN7:2' => '-,-,3,4,3,4',
'DMIN' => '-,0,0,2,3,1',
'DMIN/B' => '1,2,3,2,3,1',
'DMIN/B:1' => '-,2,0,2,0,1',
'DMIN/B:2' => '-,-,0,2,0,1',
'DMIN/BB' => '1,1,3,2,3,1',
'DMIN/C' => '-,5,7,5,6,5',
'DMIN/C:1' => '-,-,0,2,1,1',
'DMIN/C:2' => '-,-,0,5,6,5',
'DMIN/DB' => '-,-,0,2,2,1',
'DMIN/E' => '-,-,7,7,6,0',
'DM6' => '1,2,3,2,3,1',
'DM6:1' => '-,2,0,2,0,1',
'DM6:2' => '-,-,0,2,0,1',
'DMIN7' => '-,5,7,5,6,5',
'DMIN7:1' => '-,-,0,2,1,1',
'DMIN7:2' => '-,-,0,5,6,5',
'DMIN7(B5)' => '-,-,0,1,1,1',
'DMIN7/ADD11' => '3,-,0,2,1,1',
'DMAJ' => '-,5,4,2,3,2',
'DMAJ:1' => '-,9,7,7,-,2',
'DMAJ:2' => '2,0,0,2,3,2',
'DMAJ:3' => '-,0,0,2,3,2',
'DMAJ:4' => '-,0,4,2,3,2',
'DMAJ:5' => '-,-,0,2,3,2',
'DMAJ:6' => '-,-,0,7,7,5',
'DMAJ7' => '-,-,0,14,14,14',
'DMAJ7:1' => '-,-,0,2,2,2',
'DMIN/MAJ7' => '-,-,0,2,2,1',
'DSUS' => '5,-,0,0,3,5',
'DSUS:1' => '3,0,0,0,3,3',
'DSUS:2' => '-,0,0,0,3,3',
'DSUS:3' => '-,-,0,2,3,3',
'DSUS2' => '5,5,7,7,-,0',
'DSUS2:1' => '-,0,0,2,3,0',
'DSUS2:2' => '0,0,2,2,3,0',
'DSUS2:3' => '-,0,2,2,3,0',
'DSUS2:4' => '-,-,0,2,3,0',
'DSUS2/AB' => '4,-,0,2,3,0',
'DSUS2/B' => '0,2,0,2,0,0',
'DSUS2/B:1' => '-,2,0,2,3,0',
'DSUS2/BB' => '0,1,-,2,3,0',
'DSUS2/C' => '-,-,0,2,1,0',
'DSUS2/C:1' => '-,-,0,5,5,5',
'DSUS2/DB' => '-,0,0,2,2,0',
'DSUS2/DB:1' => '-,-,0,2,2,0',
'DSUS2/DB:2' => '-,-,0,6,5,5',
'DSUS2/DB:3' => '-,-,0,9,10,9',
'DSUS2/F' => '-,-,7,7,6,0',
'DSUS2/G' => '-,0,2,0,3,0',
'DSUS2/G:1' => '-,0,2,0,3,3',
'DSUS2/G:2' => '-,0,2,2,3,3',
'DSUS2/G:3' => '5,-,0,0,3,0',
'DSUS2/G:4' => '-,0,0,0,-,0',
'DSUS2/GB' => '0,0,0,2,3,2',
'DSUS2/GB:1' => '0,0,4,2,3,0',
'DSUS2/GB:2' => '2,-,0,2,3,0',
'DSUS2/GB:3' => '-,0,2,2,3,2',
'DSUS2/GB:4' => '-,-,2,2,3,2',
'DSUS2/GB:5' => '-,5,4,2,3,0',
'DSUS2/GB:6' => '-,9,7,7,-,0',
'DSUS4/B' => '3,0,0,0,0,3',
'DSUS4/B:1' => '3,2,0,2,0,3',
'DSUS4/C' => '-,5,7,5,8,3',
'DSUS4/C:1' => '-,-,0,2,1,3',
'DSUS4/E' => '-,0,2,0,3,0',
'DSUS4/E:1' => '-,0,2,0,3,3',
'DSUS4/E:2' => '-,0,2,2,3,3',
'DSUS4/E:3' => '5,-,0,0,3,0',
'DSUS4/E:4' => '-,0,0,0,-,0',
'DSUS4/GB' => '5,-,4,0,3,5',
'DSUS4/GB:1' => '3,-,0,2,3,2',
'DB/B' => '-,4,3,4,0,4',
'DB/BB' => '-,1,3,1,2,1',
'DB/C' => '-,3,3,1,2,1',
'DB/C:1' => '-,4,6,5,6,4',
'DB5' => '-,4,6,6,-,4',
'DB6' => '-,1,3,1,2,1',
'DB7' => '-,4,3,4,0,4',
'DBAUG' => '-,-,3,0,2,1',
'DBAUG:1' => '-,0,3,2,2,1',
'DBAUG:2' => '-,0,-,2,2,1',
'DBAUG/D' => '-,-,0,2,2,1',
'DBAUG/G' => '1,0,3,0,2,1',
'DBDIM/A' => '3,-,2,2,2,0',
'DBDIM/A:1' => '-,0,2,0,2,0',
'DBDIM/A:2' => '-,0,2,2,2,3',
'DBDIM/B' => '0,2,2,0,2,0',
'DBDIM/BB' => '-,1,2,0,2,0',
'DBDIM/BB:1' => '-,-,2,3,2,3',
'DBDIM/D' => '3,-,0,0,2,0',
'DBDIM/D:1' => '-,-,0,0,2,0',
'DBDIMIN7' => '-,1,2,0,2,0',
'DBDIMIN7:1' => '-,-,2,3,2,3',
'DBMIN' => '-,4,6,6,5,4',
'DBMIN:1' => '-,-,2,1,2,0',
'DBMIN:2' => '-,4,6,6,-,0',
'DBMIN/A' => '-,0,2,1,2,0',
'DBMIN/B' => '0,2,2,1,2,0',
'DBMIN/B:1' => '-,4,6,4,5,4',
'DBMIN7' => '0,2,2,1,2,0',
'DBMIN7:1' => '-,4,6,4,5,4',
'DBMIN7(B5)' => '0,2,2,0,2,0',
'DBMAJ' => '4,4,6,6,6,4',
'DBMAJ:1' => '-,4,3,1,2,1',
'DBMAJ:2' => '-,4,6,6,6,4',
'DBMAJ:3' => '-,-,3,1,2,1',
'DBMAJ:4' => '-,-,6,6,6,4',
'DBMAJ7' => '-,3,3,1,2,1',
'DBMAJ7:1' => '-,4,6,5,6,4',
'DBSUS2' => '-,-,6,6,4,4',
'DBSUS4/BB' => '-,-,4,3,2,4',
'E/A' => '-,0,2,1,0,0',
'E/D' => '0,2,0,1,0,0',
'E/D:1' => '0,2,2,1,3,0',
'E/D:2' => '-,2,0,1,3,0',
'E/D:3' => '-,-,0,1,0,0',
'E/DB' => '0,2,2,1,2,0',
'E/DB:1' => '-,4,6,4,5,4',
'E/EB' => '0,2,1,1,0,0',
'E/EB:1' => '0,-,6,4,4,0',
'E/EB:2' => '-,-,1,1,0,0',
'E/GB' => '0,2,2,1,0,2',
'E/GB:1' => '0,-,4,1,0,0',
'E/GB:2' => '2,2,2,1,0,0',
'E11/B9' => '0,0,3,4,3,4',
'E5' => '0,2,-,-,-,0',
'E5:1' => '-,7,9,9,-,0',
'E6' => '0,2,2,1,2,0',
'E6:1' => '-,4,6,4,5,4',
'E7' => '0,2,0,1,0,0',
'E7:1' => '0,2,2,1,3,0',
'E7:2' => '-,2,0,1,3,0',
'E7:3' => '-,-,0,1,0,0',
'E7/ADD11' => '-,0,0,1,0,0',
'E7/B9(B5)' => '0,1,3,1,3,1',
'E7SUS4' => '0,2,0,2,0,0',
'E9' => '0,2,0,1,0,2',
'E9:1' => '2,2,0,1,0,0',
'EADD9' => '0,2,2,1,0,2',
'EADD9:1' => '0,-,4,1,0,0',
'EADD9:2' => '2,2,2,1,0,0',
'EAUG' => '-,3,2,1,1,0',
'EDIM/C' => '-,3,5,3,5,3',
'EDIM/D' => '3,-,0,3,3,0',
'EDIM/DB' => '-,1,2,0,2,0',
'EDIM/DB:1' => '-,-,2,3,2,3',
'EDIM/EB' => '-,-,5,3,4,0',
'EDIMIN7' => '-,1,2,0,2,0',
'EDIMIN7:1' => '-,-,2,3,2,3',
'EMIN' => '0,2,2,0,0,0',
'EMIN:1' => '3,-,2,0,0,0',
'EMIN:2' => '-,2,5,-,-,0',
'EMIN/A' => '3,-,2,2,0,0',
'EMIN/A:1' => '-,0,2,0,0,0',
'EMIN/A:2' => '-,0,5,4,5,0',
'EMIN/C' => '0,3,2,0,0,0',
'EMIN/C:1' => '-,2,2,0,1,0',
'EMIN/C:2' => '-,3,5,4,5,3',
'EMIN/D' => '0,2,0,0,0,0',
'EMIN/D:1' => '0,2,0,0,3,0',
'EMIN/D:2' => '0,2,2,0,3,0',
'EMIN/D:3' => '0,2,2,0,3,3',
'EMIN/D:4' => '-,-,0,12,12,12',
'EMIN/D:5' => '-,-,0,9,8,7',
'EMIN/D:6' => '-,-,2,4,3,3',
'EMIN/D:7' => '0,-,0,0,0,0',
'EMIN/D:8' => '-,10,12,12,12,0',
'EMIN/DB' => '0,2,2,0,2,0',
'EMIN/EB' => '3,-,1,0,0,0',
'EMIN/EB:1' => '-,-,1,0,0,0',
'EMIN/GB' => '0,2,2,0,0,2',
'EMIN/GB:1' => '0,2,4,0,0,0',
'EMIN/GB:2' => '0,-,4,0,0,0',
'EMIN/GB:3' => '2,2,2,0,0,0',
'EM6' => '0,2,2,0,2,0',
'EMIN7' => '0,2,0,0,0,0',
'EMIN7:1' => '0,2,0,0,3,0',
'EMIN7:2' => '0,2,2,0,3,0',
'EMIN7:3' => '0,2,2,0,3,3',
'EMIN7:4' => '-,-,0,0,0,0',
'EMIN7:5' => '-,-,0,12,12,12',
'EMIN7:6' => '-,-,0,9,8,7',
'EMIN7:7' => '-,-,2,4,3,3',
'EMIN7:8' => '0,-,0,0,0,0',
'EMIN7:9' => '-,10,12,12,12,0',
'EMIN7(B5)' => '3,-,0,3,3,0',
'EMIN7/ADD11' => '0,0,0,0,0,0',
'EMIN7/ADD11:1' => '0,0,0,0,0,3',
'EMIN7/ADD11:2' => '3,-,0,2,0,0',
'EM9' => '0,2,0,0,0,2',
'EM9:1' => '0,2,0,0,3,2',
'EM9:2' => '2,2,0,0,0,0',
'EMAJ' => '0,2,2,1,0,0',
'EMAJ:1' => '-,7,6,4,5,0',
'EMAJ7' => '0,2,1,1,0,0',
'EMAJ7:1' => '0,-,6,4,4,0',
'EMAJ7:2' => '-,-,1,1,0,0',
'EMAJ9' => '0,2,1,1,0,2',
'EMAJ9:1' => '4,-,4,4,4,0',
'EMIN/MAJ7' => '3,-,1,0,0,0',
'EMIN/MAJ7:1' => '-,-,1,0,0,0',
'EMIN/MAJ9' => '0,6,4,0,0,0',
'ESUS' => '0,0,2,2,0,0',
'ESUS:1' => '0,0,2,4,0,0',
'ESUS:2' => '0,2,2,2,0,0',
'ESUS:3' => '-,0,2,2,0,0',
'ESUS:4' => '-,-,2,2,0,0',
'ESUS2' => '7,9,9,-,-,0',
'ESUS2:1' => '-,2,4,4,-,0',
'ESUS2/A' => '-,0,4,4,0,0',
'ESUS2/A:1' => '-,2,4,2,5,2',
'ESUS2/AB' => '0,2,2,1,0,2',
'ESUS2/AB:1' => '0,-,4,1,0,0',
'ESUS2/AB:2' => '2,2,2,1,0,0',
'ESUS2/DB' => '-,4,4,4,-,0',
'ESUS2/EB' => '-,2,2,4,4,2',
'ESUS2/EB:1' => '-,-,4,4,4,0',
'ESUS2/G' => '0,2,2,0,0,2',
'ESUS2/G:1' => '0,2,4,0,0,0',
'ESUS2/G:2' => '0,-,4,0,0,0',
'ESUS2/G:3' => '2,2,2,0,0,0',
'ESUS4/AB' => '-,0,2,1,0,0',
'ESUS4/C' => '0,0,7,5,0,0',
'ESUS4/C:1' => '-,3,2,2,0,0',
'ESUS4/D' => '0,2,0,2,0,0',
'ESUS4/D:1' => '-,2,0,2,3,0',
'ESUS4/DB' => '0,0,2,4,2,0',
'ESUS4/DB:1' => '-,0,7,6,0,0',
'ESUS4/EB' => '-,2,1,2,0,0',
'ESUS4/F' => '0,0,3,2,0,0',
'ESUS4/G' => '3,-,2,2,0,0',
'ESUS4/G:1' => '-,0,2,0,0,0',
'ESUS4/G:2' => '-,0,5,4,5,0',
'ESUS4/GB' => '-,0,4,4,0,0',
'ESUS4/GB:1' => '-,2,4,2,5,2',
'EB/C' => '-,3,5,3,4,3',
'EB/D' => '-,6,8,7,8,6',
'EB/DB' => '-,1,1,3,2,3',
'EB/DB:1' => '-,6,8,6,8,6',
'EB/DB:2' => '-,-,1,3,2,3',
'EB/E' => '-,-,5,3,4,0',
'EB5' => '-,6,8,8,-,6',
'EB6' => '-,3,5,3,4,3',
'EB7' => '-,1,1,3,2,3',
'EB7:1' => '-,6,8,6,8,6',
'EB7:2' => '-,-,1,3,2,3',
'EBAUG' => '3,2,1,0,0,3',
'EBAUG:1' => '3,-,1,0,0,3',
'EBAUG/E' => '3,-,1,0,0,0',
'EBAUG/E:1' => '-,-,1,0,0,0',
'EBDIM/B' => '2,-,1,2,0,2',
'EBDIM/B:1' => '-,0,1,2,0,2',
'EBDIM/B:2' => '-,2,1,2,0,2',
'EBDIM/B:3' => '-,2,4,2,4,2',
'EBDIM/C' => '-,-,1,2,1,2',
'EBDIMIN7' => '-,-,1,2,1,2',
'EBMIN' => '-,-,4,3,4,2',
'EBMIN/DB' => '-,-,1,3,2,2',
'EBMIN7' => '-,-,1,3,2,2',
'EBMAJ' => '-,1,1,3,4,3',
'EBMAJ:1' => '-,-,1,3,4,3',
'EBMAJ:2' => '-,-,5,3,4,3',
'EBMAJ7' => '-,6,8,7,8,6',
'EBSUS2/AB' => '-,1,3,1,4,1',
'EBSUS4/F' => '-,1,3,1,4,1',
'F/D' => '-,5,7,5,6,5',
'F/D:1' => '-,-,0,2,1,1',
'F/D:2' => '-,-,0,5,6,5',
'F/E' => '0,0,3,2,1,0',
'F/E:1' => '1,3,3,2,1,0',
'F/E:2' => '1,-,2,2,1,0',
'F/E:3' => '-,-,2,2,1,1',
'F/E:4' => '-,-,3,2,1,0',
'F/EB' => '-,-,1,2,1,1',
'F/EB:1' => '-,-,3,5,4,5',
'F/G' => '3,-,3,2,1,1',
'F/G:1' => '-,-,3,2,1,3',
'F5' => '1,3,3,-,-,1',
'F5:1' => '-,8,10,-,-,1',
'F6' => '-,5,7,5,6,5',
'F6:1' => '-,-,0,2,1,1',
'F6:2' => '-,-,0,5,6,5',
'F6/ADD9' => '3,-,0,2,1,1',
'F7' => '-,-,1,2,1,1',
'F7:1' => '-,-,3,5,4,5',
'FADD9' => '3,-,3,2,1,1',
'FADD9:1' => '-,-,3,2,1,3',
'FAUG' => '-,0,3,2,2,1',
'FAUG:1' => '-,0,-,2,2,1',
'FAUG/D' => '-,-,0,2,2,1',
'FAUG/G' => '1,0,3,0,2,1',
'FDIM/D' => '-,2,0,1,0,1',
'FDIM/D:1' => '-,-,0,1,0,1',
'FDIM/D:2' => '-,-,3,4,3,4',
'FDIM/DB' => '-,4,3,4,0,4',
'FDIMIN7' => '-,2,0,1,0,1',
'FDIMIN7:1' => '-,-,0,1,0,1',
'FDIMIN7:2' => '-,-,3,4,3,4',
'FMIN' => '-,3,3,1,1,1',
'FMIN:1' => '-,-,3,1,1,1',
'FMIN/D' => '-,-,0,1,1,1',
'FMIN/DB' => '-,3,3,1,2,1',
'FMIN/DB:1' => '-,4,6,5,6,4',
'FMIN/EB' => '-,8,10,8,9,8',
'FMIN/EB:1' => '-,-,1,1,1,1',
'FM6' => '-,-,0,1,1,1',
'FMIN7' => '-,8,10,8,9,8',
'FMIN7:1' => '-,-,1,1,1,1',
'FMAJ' => '1,3,3,2,1,1',
'FMAJ:1' => '-,0,3,2,1,1',
'FMAJ:2' => '-,3,3,2,1,1',
'FMAJ:3' => '-,-,3,2,1,1',
'FMAJ7' => '0,0,3,2,1,0',
'FMAJ7:1' => '1,3,3,2,1,0',
'FMAJ7:2' => '1,-,2,2,1,0',
'FMAJ7:3' => '-,-,2,2,1,1',
'FMAJ7:4' => '-,-,3,2,1,0',
'FMAJ7/#11' => '0,2,3,2,1,0',
'FMAJ7/#11:1' => '1,3,3,2,0,0',
'FMAJ9' => '0,0,3,0,1,3',
'FSUS' => '-,-,3,3,1,1',
'FSUS2' => '-,3,3,0,1,1',
'FSUS2:1' => '-,-,3,0,1,1',
'FSUS2/A' => '3,-,3,2,1,1',
'FSUS2/A:1' => '-,-,3,2,1,3',
'FSUS2/B' => '-,3,3,0,0,3',
'FSUS2/BB' => '-,3,5,3,6,3',
'FSUS2/D' => '3,3,0,0,1,1',
'FSUS2/E' => '-,3,3,0,1,0',
'FSUS2/E:1' => '-,-,3,0,1,0',
'FSUS4/G' => '-,3,5,3,6,3',
'G/A' => '3,0,0,0,0,3',
'G/A:1' => '3,2,0,2,0,3',
'G/C' => '3,3,0,0,0,3',
'G/C:1' => '-,3,0,0,0,3',
'G/E' => '0,2,0,0,0,0',
'G/E:1' => '0,2,0,0,3,0',
'G/E:2' => '0,2,2,0,3,0',
'G/E:3' => '0,2,2,0,3,3',
'G/E:4' => '-,-,0,12,12,12',
'G/E:5' => '-,-,0,9,8,7',
'G/E:6' => '-,-,2,4,3,3',
'G/E:7' => '0,-,0,0,0,0',
'G/E:8' => '-,10,12,12,12,0',
'G/F' => '1,-,0,0,0,3',
'G/F:1' => '3,2,0,0,0,1',
'G/F:2' => '-,-,0,0,0,1',
'G/GB' => '2,2,0,0,0,3',
'G/GB:1' => '2,2,0,0,3,3',
'G/GB:2' => '3,2,0,0,0,2',
'G/GB:3' => '-,-,4,4,3,3',
'G5' => '3,5,5,-,-,3',
'G5:1' => '3,-,0,0,3,3',
'G6' => '0,2,0,0,0,0',
'G6:1' => '0,2,0,0,3,0',
'G6:2' => '0,2,2,0,3,0',
'G6:3' => '0,2,2,0,3,3',
'G6:4' => '-,-,0,12,12,12',
'G6:5' => '-,-,0,9,8,7',
'G6:6' => '-,-,2,4,3,3',
'G6:7' => '0,-,0,0,0,0',
'G6:8' => '-,10,12,12,12,0',
'G6/ADD9' => '0,0,0,0,0,0',
'G6/ADD9:1' => '0,0,0,0,0,3',
'G6/ADD9:2' => '3,-,0,2,0,0',
'G7' => '1,-,0,0,0,3',
'G7:1' => '3,2,0,0,0,1',
'G7:2' => '-,-,0,0,0,1',
'G7/ADD11' => '-,3,0,0,0,1',
'G7SUS4' => '3,3,0,0,1,1',
'G9' => '-,0,0,0,0,1',
'G9:1' => '-,2,3,2,3,3',
'GADD9' => '3,0,0,0,0,3',
'GADD9:1' => '3,2,0,2,0,3',
'GAUG' => '3,2,1,0,0,3',
'GAUG:1' => '3,-,1,0,0,3',
'GAUG/E' => '3,-,1,0,0,0',
'GAUG/E:1' => '-,-,1,0,0,0',
'GDIM/E' => '-,1,2,0,2,0',
'GDIM/E:1' => '-,-,2,3,2,3',
'GDIM/EB' => '-,1,1,3,2,3',
'GDIM/EB:1' => '-,6,8,6,8,6',
'GDIM/EB:2' => '-,-,1,3,2,3',
'GDIMIN7' => '-,1,2,0,2,0',
'GDIMIN7:1' => '-,-,2,3,2,3',
'GMIN' => '3,5,5,3,3,3',
'GMIN:1' => '-,-,0,3,3,3',
'GMIN/E' => '3,-,0,3,3,0',
'GMIN/EB' => '-,6,8,7,8,6',
'GMIN/F' => '3,5,3,3,3,3',
'GMIN/F:1' => '-,-,3,3,3,3',
'GM13' => '0,0,3,3,3,3',
'GM6' => '3,-,0,3,3,0',
'GMIN7' => '3,5,3,3,3,3',
'GMIN7:1' => '-,-,3,3,3,3',
'GMIN7/ADD11' => '-,3,3,3,3,3',
'GM9' => '3,5,3,3,3,5',
'GMAJ' => '-,10,12,12,12,10',
'GMAJ:1' => '3,2,0,0,0,3',
'GMAJ:2' => '3,2,0,0,3,3',
'GMAJ:3' => '3,5,5,4,3,3',
'GMAJ:4' => '3,-,0,0,0,3',
'GMAJ:5' => '-,5,5,4,3,3',
'GMAJ:6' => '-,-,0,4,3,3',
'GMAJ:7' => '-,-,0,7,8,7',
'GMAJ7' => '2,2,0,0,0,3',
'GMAJ7:1' => '2,2,0,0,3,3',
'GMAJ7:2' => '3,2,0,0,0,2',
'GMAJ7:3' => '-,-,4,4,3,3',
'GSUS' => '-,10,12,12,13,3',
'GSUS:1' => '-,3,0,0,3,3',
'GSUS:2' => '-,3,5,5,3,3',
'GSUS:3' => '-,5,5,5,3,3',
'GSUS2' => '5,-,0,0,3,5',
'GSUS2:1' => '3,0,0,0,3,3',
'GSUS2:2' => '-,0,0,0,3,3',
'GSUS2:3' => '-,-,0,2,3,3',
'GSUS2/B' => '3,0,0,0,0,3',
'GSUS2/B:1' => '3,2,0,2,0,3',
'GSUS2/C' => '-,5,7,5,8,3',
'GSUS2/C:1' => '-,-,0,2,1,3',
'GSUS2/E' => '-,0,2,0,3,0',
'GSUS2/E:1' => '-,0,2,0,3,3',
'GSUS2/E:2' => '-,0,2,2,3,3',
'GSUS2/E:3' => '5,0,0,0,3,0',
'GSUS2/GB' => '5,-,4,0,3,5',
'GSUS2/GB:1' => '3,-,0,2,3,2',
'GSUS4/A' => '-,5,7,5,8,3',
'GSUS4/A:1' => '-,-,0,2,1,3',
'GSUS4/B' => '3,3,0,0,0,3',
'GSUS4/B:1' => '-,3,0,0,0,3',
'GSUS4/E' => '3,-,0,0,1,0',
'GSUS4/E:1' => '-,3,0,0,1,0',
'GSUS4/E:2' => '-,3,2,0,3,0',
'GSUS4/E:3' => '-,3,2,0,3,3',
'GSUS4/E:4' => '-,-,0,0,1,0',
'GSUS4/E:5' => '-,-,0,5,5,3',
'GSUS4/E:6' => '-,10,12,12,13,0',
'GSUS4/E:7' => '-,5,5,5,-,0',
'GSUS4/F' => '3,3,0,0,1,1',
'GB/AB' => '-,-,4,3,2,4',
'GB/E' => '2,4,2,3,2,2',
'GB/E:1' => '-,-,4,3,2,0',
'GB/EB' => '-,-,1,3,2,2',
'GB/F' => '-,-,3,3,2,2',
'GB6' => '-,-,1,3,2,2',
'GB7' => '2,4,2,3,2,2',
'GB7:1' => '-,-,4,3,2,0',
'GB7(#5)' => '2,-,4,3,3,0',
'GB7/#9' => '-,0,4,3,2,0',
'GB7SUS4' => '-,4,4,4,-,0',
'GBADD9' => '-,-,4,3,2,4',
'GBAUG' => '-,-,0,3,3,2',
'GBAUG/E' => '2,-,4,3,3,0',
'GBDIM/D' => '-,5,7,5,7,2',
'GBDIM/D:1' => '-,0,0,2,1,2',
'GBDIM/D:2' => '-,3,-,2,3,2',
'GBDIM/D:3' => '-,5,7,5,7,5',
'GBDIM/E' => '-,0,2,2,1,2',
'GBDIM/E:1' => '-,-,2,2,1,2',
'GBDIM/EB' => '-,-,1,2,1,2',
'GBDIMIN7' => '-,-,1,2,1,2',
'GBMIN' => '2,4,4,2,2,2',
'GBMIN:1' => '-,4,4,2,2,2',
'GBMIN:2' => '-,-,4,2,2,2',
'GBMIN/D' => '-,-,0,14,14,14',
'GBMIN/D:1' => '-,-,0,2,2,2',
'GBMIN/E' => '0,0,2,2,2,2',
'GBMIN/E:1' => '0,-,4,2,2,0',
'GBMIN/E:2' => '2,-,2,2,2,0',
'GBMIN/E:3' => '-,0,4,2,2,0',
'GBMIN/E:4' => '-,-,2,2,2,2',
'GBMIN7' => '0,0,2,2,2,2',
'GBMIN7:1' => '0,-,4,2,2,0',
'GBMIN7:2' => '2,-,2,2,2,0',
'GBMIN7:3' => '-,0,4,2,2,0',
'GBMIN7:4' => '-,-,2,2,2,2',
'GBMIN7(B5)' => '-,0,2,2,1,2',
'GBMIN7(B5):1' => '-,-,2,2,1,2',
'GBMIN7/B9' => '0,0,2,0,2,2',
'GBMAJ' => '2,4,4,3,2,2',
'GBMAJ:1' => '-,4,4,3,2,2',
'GBMAJ:2' => '-,-,4,3,2,2',
'GBMAJ7' => '-,-,3,3,2,2',
'GBSUS' => '-,4,4,4,2,2',
'GBSUS2/BB' => '-,-,4,3,2,4',
'GBSUS4/E' => '-,4,4,4,-,0'
);

%keychords = (
'MAJ' => '0,4,7',
'MIN' => '0,3,7',
'7' => '0,4,7,10',
'MIN7' => '0,3,7,10',
'MAJ7' => '0,3,7,11',
'6' => '0,4,7,9',
'MIN6' => '0,3,7,9',
'AUG' => '0,3,8',
'AUG7' => '0,4,8,11',
'DIM' => '0,3,6',
'DIM7' => '0,3,6,9',
'7{5B}' => '0,4,6,10',
'MIN7{5B}' => '0,3,6,10',
'9' => '0,4,7,10,14',
'MIN9' => '0,3,7,10,14',
'MAJ9' => '0,4,7,11,14',
'11' => '0,4,7,10,14,17',
'DIM9' => '0,4,7,10,13',
'{9}' => '0,4,7,14',
'{4}' => '0,4,7,17',
'SUS' => '0,5,7',
'SUS9' => '0,7,14',
'7SUS' => '0,5,7,10',
'7SUS9' => '0,2,7,10',
'5' => '0,7',
'MAJ:1' => '0,4,-5',
'MIN:1' => '0,3,-5',
'7:1' => '0,4,7,-2',
'MIN7:1' => '0,3,7,-2',
'MAJ7:1' => '0,3,7,-1',
'6:1' => '0,4,7,-3',
'MIN6:1' => '0,3,7,-3',
'AUG:1' => '0,3,-4',
'AUG7:1' => '0,4,8,-1',
'DIM:1' => '0,3,-6',
'DIM7:1' => '0,3,6,-3',
'7{5B}:1' => '0,4,6,-2',
'MIN7{5B}:1' => '0,3,6,-2',
'9:1' => '0,4,7,10,2',
'MIN9:1' => '0,3,7,10,2',
'MAJ9:1' => '0,4,7,11,2',
'11:1' => '0,4,7,10,14,5',
'DIM9:1' => '0,4,7,10,1',
'{9}:1' => '0,4,7,2',
'{4}:1' => '0,4,7,5',
'SUS:1' => '0,5,-5',
'SUS9:1' => '0,7,2',
'7SUS:1' => '0,5,7,-2',
'7SUS9:1' => '0,2,7,-2',
'5:1' => '0,-5',
'MAJ:2' => '12,4,7',
'MIN:2' => '12,3,7',
'7:2' => '12,4,7,10',
'MIN7:2' => '12,3,7,10',
'MAJ7:2' => '12,3,7,11',
'6:2' => '12,4,7,9',
'MIN6:2' => '12,3,7,9',
'AUG:2' => '12,3,8',
'AUG7:2' => '12,4,8,11',
'DIM:2' => '12,3,6',
'DIM7:2' => '12,3,6,9',
'7{5B}:2' => '12,4,6,10',
'MIN7{5B}:2' => '12,3,6,10',
'9:2' => '12,4,7,10,14',
'MIN9:2' => '12,3,7,10,14',
'MAJ9:2' => '12,4,7,11,14',
'11:2' => '12,4,7,10,14,17',
'DIM9:2' => '12,4,7,10,13',
'{9}:2' => '12,4,7,14',
'{4}:2' => '12,4,7,17',
'SUS:2' => '12,5,7',
'SUS9:2' => '12,7,14',
'7SUS:2' => '12,5,7,10',
'7SUS9:2' => '12,2,7,10',
'5:2' => '12,7',
'7:3' => '0,4,-5,-2',
'MIN7:3' => '0,3,-5,-2',
'MAJ7:3' => '0,3,-5,-1',
'6:3' => '0,4,-5,-3',
'MIN6:3' => '0,3,-5,-3',
'AUG7:3' => '0,4,-4,-1',
'DIM7:3' => '0,3,-6,-3',
'7{5B}:3' => '0,4,-6,-2',
'MIN7{5B}:3' => '0,3,-6,-2',
'9:3' => '0,4,7,-2,2',
'MIN9:3' => '0,3,7,-2,2',
'MAJ9:3' => '0,4,7,-1,2',
'11:3' => '0,4,7,10,2,5',
'DIM9:3' => '0,4,7,-2,1',
'{9}:3' => '0,4,-5,2',
'{4}:3' => '0,4,-5,5',
'7SUS:3' => '0,5,-5,-2',
'7SUS9:3' => '0,2,-5,-2',
'9:4' => '0,4,-5,-2,2',
'MIN9:4' => '0,3,-5,-2,2',
'MAJ9:4' => '0,4,-5,-1,2',
'11:4' => '0,4,7,-2,2,5',
'DIM9:4' => '0,4,-5,-2,1',
'11:5' => '0,4,-5,-2,2,5'
);

}
