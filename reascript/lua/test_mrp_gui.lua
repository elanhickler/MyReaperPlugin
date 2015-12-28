thewindow=nil
t0=reaper.time_precise()
function tick()
  if reaper.MRP_WindowIsClosed(thewindow) then
    reaper.ShowConsoleMsg("lua : window was closed\n")
    reaper.MRP_DestroyWindow(thewindow)
    --multiple destroy test
    --reaper.MRP_DestroyWindow(thewindow)
    return
  end
  if reaper.time_precise()-t0>3.0 then
    reaper.MRP_WindowSetTitle(thewindow,"Timer fired")
  end
  if reaper.MRP_GetWindowDirty(thewindow,0) then
    --reaper.ShowConsoleMsg("window was resized ")
    local wid = reaper.MRP_GetWindowPosSizeValue(thewindow,2)
    local hei = reaper.MRP_GetWindowPosSizeValue(thewindow,3)
    reaper.MRP_SetControlBounds(thewindow,"Slider 1",5,5,wid-10,20)
    reaper.MRP_SetControlBounds(thewindow,"Envelope 1",5,hei-150,wid/2-10,145)
    reaper.MRP_SetControlBounds(thewindow,"Envelope 2",wid/2+5,hei-150,wid/2-10,145)
    reaper.MRP_SetWindowDirty(thewindow,0,false)
  end
  if reaper.MRP_WindowIsDirtyControl(thewindow,"OK") then
    reaper.ShowConsoleMsg("OK was pressed\n")
  end
  if reaper.MRP_WindowIsDirtyControl(thewindow,"Slider 1") then
      local val = reaper.MRP_GetControlFloatNumber(thewindow,"Slider 1",0)
      reaper.ShowConsoleMsg(val.." ")
  end
  if reaper.MRP_WindowIsDirtyControl(thewindow,"Slider 2") then
        reaper.ShowConsoleMsg("Slider 2 was moved\n")
  end
  if reaper.MRP_WindowIsDirtyControl(thewindow,"Envelope 1") then
        reaper.ShowConsoleMsg("Envelope 1 was changed\n")
  end
  if reaper.MRP_WindowIsDirtyControl(thewindow,"Envelope 2") then
        reaper.ShowConsoleMsg("Envelope 2 was changed\n")
  end
  reaper.MRP_WindowClearDirtyControls(thewindow)
  reaper.defer(tick)
end

thewindow=reaper.MRP_CreateWindow("Test Window")
reaper.MRP_WindowAddControl(thewindow,"Button","OK")
reaper.MRP_SetControlBounds(thewindow,"OK",100,100,50,20)
reaper.MRP_WindowAddControl(thewindow,"Button","Cancel")
reaper.MRP_SetControlBounds(thewindow,"Cancel",100,150,50,20)
reaper.MRP_WindowAddControl(thewindow,"Slider","Slider 1")
reaper.MRP_SetControlBounds(thewindow,"Slider 1",5,5,250,20)
reaper.MRP_WindowAddControl(thewindow,"Slider","Slider 2")
reaper.MRP_SetControlBounds(thewindow,"Slider 2",5,30,250,20)
reaper.MRP_WindowAddControl(thewindow,"BreakpointEnvelope","Envelope 1")
reaper.MRP_SetControlString(thewindow,"Envelope 1",0,"Spin amount")
reaper.MRP_WindowAddControl(thewindow,"BreakpointEnvelope","Envelope 2")
reaper.MRP_SetControlString(thewindow,"Envelope 2",0,"Size of the universe")
tick()
