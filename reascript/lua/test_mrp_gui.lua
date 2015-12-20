mywindow=nil

tickcount=0
tickselapsed=0.0

function guitick()
  local bench_t0=reaper.time_precise()
  if reaper.MRP_WindowIsClosed(mywindow) then
    --reaper.ShowConsoleMsg("closed\n")
    return
  end
  if reaper.MRP_WindowIsDirtyControl(mywindow,"Line edit 1") then
    reaper.MRP_WindowSetTitle(mywindow,reaper.MRP_GetControlText(mywindow,"Line edit 1"))
  end
  if reaper.MRP_WindowIsDirtyControl(mywindow,"Slider 1") then
    local val = reaper.MRP_GetControlFloatNumber(mywindow,"Slider 1",0)
    reaper.MRP_SetControlText(mywindow,"Label 1",val)
    --reaper.ShowConsoleMsg(val.." ")
  end
  if reaper.MRP_WindowIsDirtyControl(mywindow,"Slider 2") or
     reaper.MRP_WindowIsDirtyControl(mywindow,"Slider 3") then
    local t0 = 1.0/1000.0*reaper.MRP_GetControlFloatNumber(mywindow,"Slider 2",0)
    local t1 = 1.0/1000.0*reaper.MRP_GetControlFloatNumber(mywindow,"Slider 3",0)
    local srclen = reaper.MRP_GetControlFloatNumber(mywindow,"Wave 1",100)
    reaper.MRP_SetControlFloatNumber(mywindow,"Wave 1",1,t0*srclen)
    reaper.MRP_SetControlFloatNumber(mywindow,"Wave 1",2,t1*srclen)
    srclen = reaper.MRP_GetControlFloatNumber(mywindow,"Wave 2",100)
    reaper.MRP_SetControlFloatNumber(mywindow,"Wave 2",1,t0*srclen)
    reaper.MRP_SetControlFloatNumber(mywindow,"Wave 2",2,t1*srclen)
  end
  if reaper.MRP_GetWindowDirty(mywindow,1) then
    local w = reaper.MRP_GetWindowPosSizeValue(mywindow,2)
    local h = reaper.MRP_GetWindowPosSizeValue(mywindow,3)
    reaper.MRP_SetControlBounds(mywindow,"Slider 1",w/2,5,w/2-10,20)
    reaper.MRP_SetControlBounds(mywindow,"Slider 2",5,30,w-10,20)
    reaper.MRP_SetControlBounds(mywindow,"Slider 3",5,60,w-10,20)
    reaper.MRP_SetControlBounds(mywindow,"Button 1",w-65,h-20,50,19)
    reaper.MRP_SetControlBounds(mywindow,"Button 2",w-115,h-20,50,19)
    reaper.MRP_SetControlBounds(mywindow,"Line edit 1",5,h-20,w-130,19)
    reaper.MRP_SetControlBounds(mywindow,"Label 1",5,5,w/2-5,19)
    --reaper.MRP_SetControlBounds(mywindow,"XY",0,55,w,h-80)
    reaper.MRP_SetControlBounds(mywindow,"Wave 1",0,85,w,(h-105)/2-5)
    reaper.MRP_SetControlBounds(mywindow,"Wave 2",0,85+((h-105)/2),w,(h-105)/2-5)
    --reaper.ShowConsoleMsg("resized to "..w.." "..h.."\n")
    reaper.MRP_SetWindowDirty(mywindow,false,1)
  end
  if reaper.MRP_WindowIsDirtyControl(mywindow,"Button 1") then
      reaper.MRP_SetControlText(mywindow,"Button 1",math.random())
  end
  if reaper.MRP_WindowIsDirtyControl(mywindow,"Button 2") then
    reaper.ShowConsoleMsg("Cancel clicked\n")
  end
  -- REMEMBER to call this if you are not sure you don't need to
  reaper.MRP_WindowClearDirtyControls(mywindow)
  local bench_t1=reaper.time_precise()
  tickselapsed=tickselapsed+(bench_t1-bench_t0)
  tickcount=tickcount+1
  if tickcount>=100 then
    local avg_bench = tickselapsed/tickcount
    --reaper.ShowConsoleMsg((avg_bench*1000.0).." ms\n")
    tickcount=0
    tickselapsed=0.0
  end
  reaper.defer(guitick)
end

mywindow=reaper.MRP_CreateWindow("My window")
reaper.MRP_WindowAddSlider(mywindow,"Slider 1",100)
reaper.MRP_WindowAddSlider(mywindow,"Slider 2",0)
reaper.MRP_WindowAddSlider(mywindow,"Slider 3",1000)
reaper.MRP_WindowAddButton(mywindow,"Button 1","OK")
reaper.MRP_WindowAddButton(mywindow,"Button 2","Cancel")
reaper.MRP_WindowAddLineEdit(mywindow,"Line edit 1","Foofoo text")
reaper.MRP_WindowAddLabel(mywindow,"Label 1", "This is a label")
--reaper.MRP_WindowAddLiceControl(mywindow,"MultiXYControl","XY")
reaper.MRP_WindowAddLiceControl(mywindow,"WaveformControl","Wave 1")
reaper.MRP_WindowAddLiceControl(mywindow,"WaveformControl","Wave 2")
guitick()


