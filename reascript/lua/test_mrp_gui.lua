thewindow=nil

function iterate_envelope(envname)
  local numpoints = reaper.MRP_GetControlIntNumber(thewindow,envname,100)
  for i=0,numpoints-1 do
    local pt_x = reaper.MRP_GetControlFloatNumber(thewindow,envname,i)
    local pt_y = reaper.MRP_GetControlFloatNumber(thewindow,envname,1000+i)
    reaper.ShowConsoleMsg(i.." "..pt_x.." "..pt_y.."\n")
  end
end

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
    reaper.MRP_SetControlBounds(thewindow,"SlideR 1",5,5,wid-10,20)
    reaper.MRP_SetControlBounds(thewindow,"Slider 2",5,30,wid-10,20)
    reaper.MRP_SetControlBounds(thewindow,"envelope 1",5,hei-150,wid/2-10,145)
    reaper.MRP_SetControlBounds(thewindow,"Envelope 2",wid/2+5,hei-150,wid/2-10,145)
    reaper.MRP_SetWindowDirty(thewindow,0,false)
  end
  if reaper.MRP_WindowIsDirtyControl(thewindow,"But 1") then
    reaper.ShowConsoleMsg("But 1 was pressed\n")
  end
  if reaper.MRP_WindowIsDirtyControl(thewindow,"But 2") then
    reaper.MRP_SendCommandString(thewindow,"Envelope 2", "ADDPT 0.5 0.5\nADDPT 0.4 0.2\nADDPT 0.05 1.0")
  end
  if reaper.MRP_WindowIsDirtyControl(thewindow,"Slider 1") then
      local val = reaper.MRP_GetControlFloatNumber(thewindow,"slider 1",0)
      -- Change point 1 x position of envelope 1
      reaper.MRP_SetControlFloatNumber(thewindow,"Envelope 1",1,val)
      -- Change point 1 y position of envelope 2
      reaper.MRP_SetControlFloatNumber(thewindow,"Envelope 2",1001,val)
  end
  if reaper.MRP_WindowIsDirtyControl(thewindow,"Slider 2") then
        reaper.ShowConsoleMsg("Slider 2 was moved\n")
  end
  if reaper.MRP_WindowIsDirtyControl(thewindow,"Envelope 1") then
        iterate_envelope("Envelope 1")
  end
  if reaper.MRP_WindowIsDirtyControl(thewindow,"Envelope 2") then
        reaper.ShowConsoleMsg("Envelope 2 was changed\n")
  end
  reaper.MRP_WindowClearDirtyControls(thewindow)
  reaper.defer(tick)
end

thewindow=reaper.MRP_CreateWindow("Test Window")
reaper.MRP_WindowAddControl(thewindow,"Button","But 1")
reaper.MRP_SetControlBounds(thewindow,"But 1",5,100,50,20)
reaper.MRP_WindowAddControl(thewindow,"Button","But 2")
reaper.MRP_SetControlBounds(thewindow,"But 2",60,100,50,20)
reaper.MRP_WindowAddControl(thewindow,"Slider","Slider 1")
reaper.MRP_SetControlBounds(thewindow,"Slider 1",5,5,250,20)
reaper.MRP_WindowAddControl(thewindow,"Slider","Slider 2")
reaper.MRP_SetControlBounds(thewindow,"Slider 2",5,30,250,20)
reaper.MRP_WindowAddControl(thewindow,"BreakpointEnvelope","Envelope 1")
reaper.MRP_SetControlString(thewindow,"Envelope 1",0,"Spin amount")
reaper.MRP_WindowAddControl(thewindow,"BreakpointEnvelope","Envelope 2")
-- Make points of envelope 2 red
local color = reaper.ColorToNative(255,0,0)
reaper.MRP_SetControlIntNumber(thewindow,"Envelope 2",200,color)
reaper.MRP_SetControlString(thewindow,"Envelope 2",0,"Size of the universe")
tick()
