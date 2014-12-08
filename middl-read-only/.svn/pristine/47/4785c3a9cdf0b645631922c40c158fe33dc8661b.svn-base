/*
**  (C) 2008 by Remo Dentato (rdentato@users.sourceforge.net)
**
** Permission to use, copy, modify and distribute this code and
** its documentation for any purpose is hereby granted without
** fmakee, provided that the above copyright notice, or equivalent
** attribution acknowledgement, appears in all copies and
** supporting documentation.
**
** Copyright holder makes no representations about the suitability
** of this software for any purpose. It is provided "as is" without
** express or implied warranty.
*/


#define UTL_UNITTEST

#include "msq_write.c"

int mywritetrack1(int trknum)
{
   mf_midi_event(0, st_note_on, 1, 60,100);
   mf_midi_event(96, st_note_on, 1, 62,100);
   return 480;
}

int mywritetrack2(int trknum)
{
  dbgmsg("Write track2\n");
   mf_note(mf_curtick, 1, d_quarter, n_C(5), 100);
   mf_note(mf_curtick, 1, d_("q3"), n_B(4), 100);
   mf_program_change(mf_curtick, 1, mf_instrbyname("ebassfinger"));
   mf_note(mf_curtick, 1, d_quarter, n_C(3), 100);
   mf_note(mf_curtick, 1, d_("q3"), n_B(2), 100);
   return d_whole;
}

int myerror(int err, char *msg)
{
  fprintf(stderr,msg); fflush(stderr);
  return err;
}

int main(int argc, char *argv[])
{
  int status;
  unsigned char *p;
  int k;

  TSTTITLE("TESTING WRITE MIDIFILE");
  TSTSECTION("Buffer write function");

  TSTGROUP("Buffer sizing");

  status = chk_evt_buf(16);
  TST("Room allocated", (status == 0 && (evt_buf_sz - evt_buf_wm) >= 1024));

  TSTGROUP("Buffer writing");
  p=evt_buf;
  b_write8(0xAB);
  b_write7(0xCD);
  
  #if 0
  b_write16(0xFEDA); /* 1111 1110 1101 1010*/
  b_write14(0xFEDA);
  b_write32(0xFEEDBAC0);
  #endif

  TST("Write  8 bits",(p[0] == 0xAB));
  TST("Write  7 bits",(p[1] == (0xCD & 0x7F)));
  #if 0
  TST("Write 16 bits",(p[2] == 0xFE && p[3] == 0xDA)); /*1111 1110 1101 1010*/
  TST("Write 14 bits",(p[4] == 0x7D && p[5] == 0x5A)); /*0111 1101 0101 1010*/
  TSTONFAIL("Bytes written: %02x %02x",p[4],p[5]);
  #endif

  TSTGROUP("Raw write track");
  h_writetrack = (t_writetrack) mywritetrack1;
  h_error = (t_error) mf_null_handler;

  status = write_track(1);
  TST("write_track() returned unsuccessfully",(status == 309));
  TSTONFAIL("return code: %d\n",status);
  midi_file = fopen("test_file.mid","wb");
  status = write_track(1);
  TST("write_track() returned successfully",(status == 0));
  fclose(midi_file);

  midi_file = fopen("test_file.mid","rb");
  TST("File reopened for read",(midi_file != NULL));

  if (midi_file) {
    /* Now, if everything went well the file should contain:
    *   MTrk 00 00 00 0C 00 90 3C 64 60 3E 64 83 60 FF 2F 00
    */
    TSTWRITE("#");
    p = "MTrk\0\0\0\x0C\0\x90\x3C\x64\x60\x3E\x64\x83\x60\xFF\x2F\0";
    for (k=0; k<20; k++) {
      if (p[k] != fgetc(midi_file)) break;
      TSTWRITE(" %02X",p[k]);
    }
    TSTWRITE("\n");
    TST("File written correctly",(k == 20));

    fclose(midi_file);
    midi_file = NULL;
  }

  h_writetrack = (t_writetrack) mywritetrack2;
  status = mf_write("test_file.mid",0,1,96);
  TST("mf_write succeeded",(status == 0));

  TSTDONE();

  if (midi_file) fclose(midi_file);

  return (0);
}

