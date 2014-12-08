
hh = {}
 
function hh.header(ty, nt, dv)
  print("HEADER:", ty, nt, dv)
  return 0
end
 
function hh.track( et, nt, tl)
  print("TRACK:", et, nt, tl)
  return 0
end
 
function hh.midi_event(tick, status, chan, data1, data2)
  print("",tick,status,chan,data1,data2)
end

function hh.sys_event(tick, status, aux, data)
  local d = {}
  
  for k = 1,string.len(data) do
    d[#d+1] = string.format("%02X",string.byte(string.sub(data,k,k)))
  end
  print("",tick,status,aux,table.concat(d))
end
 
function hh.error( ec, em)
  print("ERROR:",ec,em);
end

msq.read(arg[1] or "untitled.mid",hh);


