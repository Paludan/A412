--
-- mpad v0.1
-- by Remo Dentato <rdentato@users.sf.net>
--
-- This is a rewrite in Lua of the
-- musicpad.cgi script (c) loic prot 2006
-- available at http://l01c.ouvaton.org


local  keymap = { ["cmaj"]  =   0 , ["amin"]  =   0,
                  ["cbmaj"] =  -7 , ["abmin"] =  -7, ["gmaj"]  =   1 , ["emin"]  =   1,
                  ["gbmaj"] =  -6 , ["ebmin"] =  -6, ["dmaj"]  =   2 , ["bmin"]  =   2,
                  ["dbmaj"] =  -5 , ["bbmin"] =  -5, ["amaj"]  =   3 , ["f#min"] =   3,
                  ["abmaj"] =  -4 , ["fmin"]  =  -4, ["emaj"]  =   4 , ["c#min"] =   4,
                  ["ebmaj"] =  -3 , ["cmin"]  =  -3, ["bmaj"]  =   5 , ["g#min"] =   5,
                  ["bbmaj"] =  -2 , ["gmin"]  =  -2, ["f#maj"] =   6 , ["d#min"] =   6,
                  ["fmaj"]  =  -1 , ["dmin"]  =  -1, ["c#maj"] =   7 , ["a#min"] =   7 } 
                  

local flatmap ={ ["c#"] = "db", ["d#"] = "eb", ["e#"] = "f",
                 ["f#"] = "gb", ["g#"] = "ab", ["a#"] = "bb", ["b#"] = "c" }
              
local  version = "mpad.lua v0.1, by Remo Dentato"


local errmsg

local function err(msg)
  if not msg then
    errmsg = nil
  else
    errmsg = (errmsg or "") .. msg .. '\n'
  end
end

local function totracks(text)
  local tracks
  local cur_track = -1
  local macros = { [""] = {}, ["rnd"] = {}, ["seq"] = {}}
  local n,cnt

  err()
  
  tracks = { tempo    =  60, ppqn        = 192, gduty =   1, gvelvarw = 0, 
             gloosew  =   0, glooseq     =   1, gvel  = 100, gvelvarq = 1,
             count    =   0, gguitarmode = nil, key   =   0, gmetern  = 4,
             gmeterm  =   4}
  
  text = string.lower(" "..text.."\n")
  text = string.gsub(text,"%s#[^\n]*\n"," ")       -- Remove comments
  text = string.gsub(text,"%s#[^\n]*\n"," ")       -- Remove comments
  text = string.gsub(text,"[%(%)]"," %0 ")
  text = string.gsub(text,"%s+"," ")
  text = string.gsub(text,"([a-g])-","%1b")  -- replace - with b to avoid confusion with pauses

  -- Global variables

  text,n = string.gsub(text, "tempo%s*(%d+)", function (val) tracks.tempo=val ; return "" end )
  
  local function setppqn (val)
        if (val == 0) then tracks.ppqn = 96
    elseif (val == 1) then tracks.ppqn = 192
    elseif (val == 2) then tracks.ppqn = 384
    elseif (val == 3) then tracks.ppqn = 1536
    else  tracks.ppqn = val
    end
    return ""
  end 
  
  text,n = string.gsub(text,"ppqn*(%d+)", setppqn)
  
  text,n = string.gsub(text,"resolution%s*(%d+)",setppqn)

  text,n = string.gsub(text,"duty%s*(%d+)", function (val) tracks.gduty = tonumber(val) /100; return "" end)
  
  text,n = string.gsub(text,"velocity%s*(%d+)", function (val) tracks.gvel = val ; return "" end)
  
  text,n = string.gsub(text,"globaloose%s*(%d+),([%dg]+)", function (val,val2)
                                                             if val2 == 'g' then val2 = -1 end
                                                             tracks.gloosew = tonumber(val)
                                                             tracks.glooseq = tonumber(val2)
                                                             return ""
                                                           end)

  text,n = string.gsub(text,"globalvelvar%s*(%d+),([%dg]+)", function (val,val2)
                                                               if val2 == 'g' then val2 = -1 end
                                                               tracks.gvelvarw = tonumber(val)
                                                               tracks.gvelvarq = tonumber(val2) or 1
                                                               return ""
                                                             end)

  text,n = string.gsub(text,"globalguiton", function () tracks.gguitarmode = true ; return "" end)

  text,n = string.gsub(text,"key%s*([%+%-]?)(%w+)(m[ia][nj])", function (val,val2) -- TOBEFIXED
                                                      tracks.key = tonumber(val2) or 0
                                                      if val == "-" then
                                                        tracks.key = -tracks.key
                                                      elseif val == "" then
                                                        tracks.key = keymap[val2] or 0
                                                      end
                                                    return "" end)

  -- Process Macros

  local function add_macro(mtype,name, body)
    --print ("**",name,body)
    if macros[""][name] or macros["rnd"][name] or macros["seq"][name] then
      err(string.format("Macro $%s(...) already defined",name))
    else
      if mtype == "" then
        macros[""][name] = "( " .. body .." )"
      elseif mtype == "rnd" or mtype == "seq" then
        local m = {}
        string.gsub(body,"%$(\w+)",function (n) m[#m+1] = n end )
        macros[mtype][name] = m
      else
        err(string.format("Syntax error: m%s$%s(...) is not a valid macro definition", mtype, name))
      end
    end

    return ""
  end

  text = string.gsub(text," m(%w*)%$(%w+)%s*(%b())",add_macro)
  if errmsg then return nil,errmsg end

  -- expand macros

  local function repl_macro(name)
    if macros[""][name] then  return macros[""][name] end
    if macros["rnd"][name] then
      local n=math.random(#macros["rnd"][name])
      return macros[rnd][name][n]
    end
    err(string.format("Unknown macro $%s", name))
    return 
  end

  n   = 0
  cnt = 0

  repeat
    text,n = string.gsub(text,"%$(%w+)",repl_macro)
    if errmsg then return nil,errmsg end
    cnt = cnt+1
  until n == 0 or cnt >= 500

  if cnt >= 500 then
    err(string.format("Too many macro expansions. Check for recursive definition."))
    return nil,errmsg
  end

  -- expand mul

  repeat
    text,n = string.gsub(text,"%s*(%b())%s*%*(%d+)", function (body,n)
                                                       return string.rep(string.sub(body,2,-2) .. " ",n)
                                                     end)
  until n == 0

  repeat
    text,n = string.gsub(text,"%s+(%S+)%s*%*(%d+)", function (body,n)
                                                       return " " .. string.rep(body .. " ",n)
                                                     end)
  until n == 0
  --

  text = string.gsub(text,"%s*[%(%)]+%s*"," ")
  text = string.gsub(text,"%s*([=%-])%s*","%1")
  
  string.gsub("|0 " .. text,"%s*|(%d*)%s*([^|]*)", function (trknum,trk)
                                                     cur_track = (tonumber(trknum) or cur_track) + 1
                                                     if cur_track > tracks.count then
                                                       tracks.count = cur_track 
                                                     end
                                                     tracks[cur_track] = tracks[cur_track] or {}
                                                     tracks[cur_track][#tracks[cur_track]+1] = trk
                                                   end)

  for n=1,tracks.count do
    if tracks[n] then 
      tracks[n][#tracks[n]+1] = " "
      tracks[n] = table.concat(tracks[n]," ")
      -- print (tracks[n])
    end
  end
  return tracks
end

local phase = 0
local U1    = 0 ; local V1    = 0
local U2    = 0 ; local V2    = 0
local S     = 0

-- Convert to MIDI

local function round(x)
   return math.floor(x + 0.5)
end

local function timetotick(tm,tempo,ppqn)
   return (tm/1000)*(tempo/60)*ppqn
end

local function synerr(str,p)
  err(string.format("Syntax error: ... %s\n",string.sub(str,p,p+40)))
end

local tracks_to_write 

local function writetempotrack()
  --print ("TRACK 0",tracks_to_write.tempo)
  smf.write_meta_event(0,smf.text_event,"Created with mpad 0.1: http://midll.googlecode.com");
  smf.write_tempo(smf.bpm_to_tempo(tracks_to_write.tempo));
end

local function writetrack(n)

  if n == 0 then return writetempotrack() end
  local track = tracks_to_write[n]
  
  if not track then return 0 end

  local pos = 1
  local i
  local j = 0
  local track = tracks_to_write[n]
  local v, v2, v3,v4
  
  local length, ticks_on, ticks_off, delta_on, delta_off,lvel
  local seqtime = 0
  local abstime = 0
  local nlength = 4
  local octave = 4
  local note = 60
  local chan = 1
  local port = 0
  local nratio = 1
  local nduty = tracks_to_write.gduty
  local vel = tracks_to_write.gvel
  local trans = 0
  local nivstress = 0.5
  local nivsoft = 0.25
  local loosew = timetotick(tracks_to_write.gloosew, tracks_to_write.tempo, tracks_to_write.ppqn)
  local looseq = tracks_to_write.glooseq
  local velvarw = tracks_to_write.gvelvarw
  local velvarq = tracks_to_write.gvelvarq
  local strumdelay = 0
  local strumup = 0
  local strumhitdown = 0
  local strumupvel = 100
  local instr = 0
  local tuning = {40, 45, 50, 55, 59, 64}

  local pause = 0;
  local stress = 0;
  local hold;
  local deltanote = 0;
  local temptrans = 0;
  local chord = nil
  local lastchord = {note}
  
  -- print(">>",track)
  
  local tomsmode
  local guitmode = tracks_to_write.gguitarmode

  -- if guitmode then 
  --   smf.write_midi_event(0, chan, smf.program_change, 25)
  -- end
  
  while track and j do
   while j do
      pause     = nil;
      stress    = 1;
      hold      = nil;
      deltanote = 0;
      temptrans = 0;
      pos       = j+1
      
      if chord then lastchord = chord end
      
      i,j = string.find(track,"^%s+",pos)
      if i then break; end
      
      i,j,v = string.find(track,"^tuning%s*%[([^%]]-)%]",pos)
      if i then         -- print("TUNING:",v,pos,i,j)
        break
      end
      
      i,j,v,v2 = string.find(track,"^%[([^%]]-)%]%s*(=*)",pos)
      if i then         -- print("CHORD: ",v,pos,i,j)
        
        break
      end

      i,j,v,v2,v3 = string.find(track,"^strum%s*(%d+),?(%d*),?(%d*)",pos)
      if i then         -- print("STRUM: ",v,pos,i,j)
         strumdelay = timetotick(tonumber(v), tracks_to_write.tempo, tracks_to_write.ppqn)
         v2 = tonumber(v2)
         if v2 then
      	   strumup = timetotick(v2 , tracks_to_write.tempo, tracks_to_write.ppqn)
      	 end
      	 strumupvel = tonumber(v3) or strumupvel;
        break
      end
      
      i,j,v = string.find(track,"^toms(o[nf]f?)",pos)
      if i then         -- print("TOMS:  ",v,pos,i,j)
        if v == "on" then 
         tomsmode = true; chan = 10
        else
         tomsmode = nil;  chan = 1
        end
        break
      end
      
      i,j,v = string.find(track,"^guit(o[nf]f?)",pos)
      if i then         -- print("GUITAR:",v,pos,i,j)
        if v == "on" then
          guitmode = true
          if instr ~= 24 then
            smf.write_midi_event(0, chan, smf.program_change, 24)
          end
        else
          guitmode = nil
          smf.write_midi_event(0, chan, smf.program_change, instr or 0)
        end
        break
      end

      i,j,v = string.find(track,"^stress%s*(%d+)",pos)
      if i then         -- print("STRESS:",v,pos,i,j)
        nivstress = tonumber(v) / 100
        break
      end
      
      i,j,v = string.find(track,"^soft%s*(%d+)",pos)
      if i then         -- print("SOFT:  ",v,pos,i,j)
        nivsoft = tonumber(v) / 100
        break
      end
      
      i,j,v,v2 = string.find(track,"^loose%s*(%d+),([%dg]+)",pos)
      if i then         -- print("LOOSE: ",v,v2,pos,i,j)
        if v2 == 'g' then v2 = -1
                     else v2 = tonumber(v2) or tracks_to_write.glooseq end
        v = tonumber(v) or tracks_to_write.gloosew
        loosew = v; looseq = v2
        break
      end
      
      i,j,v,v2 = string.find(track,"^velvar%s*(%d+),([%dg]+)",pos)
      if i then         -- print("VELVAR:",v,v2,pos,i,j)
        if v2 == 'g' then v2 = -1
                     else v2 = tonumber(v2) or tracks_to_write.gvelvarq end
        v = tonumber(v)
        velvarw = v; velvarq = v2
        break
      end
      
      i,j,v,v2 = string.find(track,"^ctrl%s*(%w+),(%d+)",pos)
      if i then         -- print("CTRL:  ",v,v2,pos,i,j)
        break
      end
      
      i,j,v = string.find(track,"^sysex%s*(%S*)",pos)
      if i then         -- print("SYSEX:",v,pos,i,j)
        break
      end
            
      i,j,v,v2 = string.find(track,"^pitch%s*([%+%-]?)(%d+)",pos)
      if i then         -- print("PITCH: ",v,v2,pos,i,j)
        v2 = tonumber(v2)
        if v == "" then 
          if v2 == 0 then v2 = 8192
          else v2 = v2 - 1
          end
        else
          v2 = (8192 * v2) / 100
          if v == "-" then v2 = -v2 end
          v2 = 8192 + v2
        end
        if v2<0 then v2 = 0
        elseif v2>16383 then v2 = 16383
        end
        v = v2 % 128; v2 = math.floor(v2 / 128)
        smf.write_midi_event(0, chan, smf.pitch_wheel, v, v2)
        break
      end

      i,j,v = string.find(track,"^ch%s*(%d+)",pos)
      if i then          -- print("CHAN:  ",v,pos,i,j)
        chan = tonumber(v); if chan < 1 or 16 < chan then chan = 1 end
        break
      end
       
      i,j,v = string.find(track,"^i%s*(%w+)",pos)
      if i then       -- print("INSTR: ",v,pos,i,j)
        if drummap[v] then
          chan = 10
          note = drummap[v]
        elseif instrmap[v] then
          v = instrmap[v] - 1
          if v ~= instr then
            instr = v
            smf.write_midi_event(0, chan, smf.program_change, instr )
          end
        else
          v = (tonumber(v) or 1) - 1
          if v < 0   then v=0 end
          if v > 127 then v=127 end
          if v ~= instr then
            instr = v
            smf.write_midi_event(0, chan, smf.program_change, instr )
          end
        end
        break
      end
      
      i,j,v,v2 = string.find(track,"^r%s*(%d+)/?(%d*)",pos)
      if i then         -- print("RATIO: ",v,v2,pos,i,j)
        nratio = tonumber(v)
        if v2 ~= "" then
          nratio = nratio/tonumber(v2)
        end
        break
      end
      
      i,j,v = string.find(track,"^u%s*(%d+)",pos)
      if i then         -- print("DUTY:  ",v,pos,i,j)
        nduty = tonumber(v) /100
        break
      end
      
      i,j,v = string.find(track,"^v%s*(%d+)",pos)
      if i then         -- print("VEL:   ",v,pos,i,j)
        v = tonumber(v)
            if v < 0   then vel = 0
        elseif v > 127 then vel = 127
        else vel = v   end
        break
      end
      
      i,j,v,v2 = string.find(track,"^t%s*([%+%-]?)(%d+)",pos)
      if i then         -- print("TRNS:  ",v,pos,i,j)
        v2 = tonumber(v2)
            if v == '-' then trans = trans - v2
        elseif v == '+' then trans = trans + v2
        else                 trans = v2 end
        break
      end

      i,j = string.find(track,"^/",pos)
      if i then 
        octave = octave + 1 ; if octave > 9 then octave = 9 end
        break
      end

      i,j = string.find(track,"^\\",pos)
      if i then 
        octave = octave - 1 ; if octave < 0 then octave = 0 end
        break
      end

      local function calcstress(str) 
        if str == "'" then stress = stress + nivstress
        else stress = stress - nivsoft end
      end
            
      i,j,v,v2,v3,v4,v5,v6 = string.find(track,"^([',]*)n(t?)%s*([%+%-]?)(%d+)/?(%d*)(=*)",pos)
      if i then          --print("NOTE N:",v,v2,v3,v4,v5,v6,pos,i,j)
        if v ~= "" then string.gsub(v,".",calcstress) end
        v4 = tonumber(v4)
        tmpnote = note
            if v3 == '-' then tmpnote = tmpnote - v4
        elseif v3 == '+' then tmpnote = tmpnote + v4
        else                  tmpnote = v4 end
        
        if tmpnote < 0   then tmpnote = 0 end
        if tmpnote > 127 then tmpnote = 127 end

        hold = 1 + string.len(v6)
        
        v5 = tonumber(v5) or nlength
                    
        if v2 == 't' then 
          hold = hold /nlength * v5
        else
          nlength = v5
          note = tmpnote 
        end
        chord = {tmpnote}
        break
      end

      i,j,v,v2,v3 = string.find(track,"^o([a-g]?[#%+b]?)(%d*)/?(%d*)",pos)
      if i then         -- print("NOPLAY:",v,v2,v3,pos,i,j)
        if v2 ~= "" then
          octave  = tonumber(v2) ; if octave > 9 then octave = 9 end
        end
        if v  ~= "" then note    = notemap[v] + 12 * octave end
        if v3 ~= "" then nlength = tonumber(v3) end
        break
      end
      
      i,j,v,v2,v3,v4,v5 = string.find(track,"^([',]*)([a-g][#%+b]?)(%d*)/?(%d*)(=*)",pos)
      if i then        --print("Note:",v,v2,v3,v4,v5,pos,i,j)
        if v ~= "" then string.gsub(v,".",calcstress) end
        if v3 ~= "" then
          octave  = tonumber(v3)
          if octave > 9 then octave = 9 end
        end
        note = notemap[v2] + 12 * octave
        if v4 ~= "" then nlength = tonumber(v4) end
        hold = 1 + string.len(v5)
        chord = {note}
        break
      end
      
      i,j,v,v2 = string.find(track,"^p/?(%d*)(=*)",pos)
      if i then         -- print("NOPLAY:",v,v2,v3,pos,i,j)
        if v  ~= "" then nlength = tonumber(v) end
        chord = nil
        hold = 1 + string.len(v2)
        break
      end
       
      i,j,v = string.find(track,"^-([%-=]*)",pos)
      if i then        -- print("pause:",v,pos,i,j)
        chord = nil
        hold = 1 + string.len(v)
        break
      end
      
      i,j,v,v2,v3 = string.find(track,"^([',]*)x/?(%d*)(=*)",pos)
      if i then         -- print("SAME :",v,v2,pos,i,j)
        if v ~= "" then string.gsub(v,".",calcstress) end
        if v2 ~= "" then nlength = tonumber(v2) end
        hold = 1 + string.len(v3)
        chord = lastchord
        break
      end

      i,j,v,v2,v3 = string.find(track,"^([',]*)(%d+)(=*)",pos)
      if i then         -- print("NUMB :",v,v2,v3,pos,i,j)
        if v ~= "" then string.gsub(v,".",calcstress) end
        hold = 1 + string.len(v3)
        if guitmode then
          chord = {note + tonumber(v2)}
        elseif tomsmode then
          chord = {drummap["t"..v2] or drummap["t1"]}
        else
          nlength = tonumber(v2)
          chord = {note}
        end
        break
      end
      
      v = string.sub(track,pos)
      if v ~= "" then
        synerr(track,pos)
        return -1
      end
    end
    
    --print ("xx",hold,j,chan)
    if hold then
      length = (nratio * 4 / nlength)* hold * tracks_to_write.ppqn
      lvel = vel
      if velvarw > 0 then 
        lvel = lvel*(1+velvarw * rndq(velvarq)/100) -- *($strumhitdown?1:$strumupvel/100);
      end
      lvel = round(lvel * stress)
      if lvel < 0   then lvel = 0   end
      if lvel > 127 then lvel = 127 end
      -- print ("vel:",vel, "lvel:",lvel, "stress:", stress )
      
      if chord then
        endnote={}
        delta_on = abstime-seqtime
        delta_off = length * nduty;
        absoff = abstime + length
        for k = 1, #chord do
          tmpnote = chord[k] + trans
          smf.write_midi_event(delta_on, chan, smf.note_on, tmpnote, lvel)
          delta_on = 0
        end
        for k = 1, #chord do
          tmpnote = chord[k] + trans
          smf.write_midi_event(delta_off, chan, smf.note_off, tmpnote, 0)
          delta_off = 0
        end
        abstime = abstime + length
        seqtime = abstime
      else
        abstime = abstime + length
      end
    end
  end
  return 0
end


local function tomidi(tracks,filename)
  local k
  -- print("**","writing tracks",tracks.count)
  tracks_to_write = tracks
  smf.write(1,tracks.count,tracks.ppqn,filename,writetrack)
  return errmsg
end

function mpad.tomidi(text,filename)
  local tracks, err
  tracks,err = totracks(text)
  if tracks and not err then 
    err = tomidi(tracks,filename)
  end
  return err
end

package.loaded["mpad_lib"] = mpad
return mpad
