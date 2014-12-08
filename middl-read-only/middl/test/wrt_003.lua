hh = {}

function hh.writetrack(trknum)
  print("TICK:",msq.tick())
  msq.note(msq.tick(), 1, "q", "tom6",100)
  print("TICK:",msq.tick())
  return 480;
end 

function hh.error(err,errmsg)
  print("Error:",err,errmsg)
end

msq.write(arg[1] or "untitled.mid", 0, 1, 480, hh)

