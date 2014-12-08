



quarter = 384

m = msq.open("xx.mid")
 
tick = 0;

m:settrack(0)
m:sysevent(0,msq.meta_event,msq.copyright_notice,8,"Author!!")


m:settrack(1)
m:midievent(tick,msq.note_on,0,60,80)
tick = tick + quarter
m:midievent(tick,msq.note_off,0,60,80)

m:midievent(tick,msq.note_on,0,62,80)
tick = tick + quarter
m:midievent(tick,msq.note_off,0,62,80)

m:midievent(tick,msq.note_on,0,64,80)
tick = tick + quarter
m:midievent(tick,msq.note_off,0,64,80)

m:midievent(tick,msq.note_on,0,65,80)
tick = tick + quarter
m:midievent(tick,msq.note_off,0,65,80)

m:close(m)

