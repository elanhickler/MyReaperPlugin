mywindow=nil

function guitick()
  if reaper.MRP_WindowIsClosed(mywindow) then
    --reaper.ShowConsoleMsg("closed\n")
    return
  end
  if reaper.MRP_WindowIsDirtyControl(mywindow,"Line edit 1") then
    reaper.MRP_WindowSetTitle(mywindow,reaper.MRP_GetControlText(mywindow,"Line edit 1"))
  end
  if reaper.MRP_WindowIsDirtyControl(mywindow,"Slider 1") then
    local val = reaper.MRP_GetControlFloatNumber(mywindow,"Slider 1")
    reaper.MRP_SetControlText(mywindow,"Label 1",val)
    --reaper.ShowConsoleMsg(val.." ")
  end
  if reaper.MRP_WindowIsDirtyControl(mywindow,"Slider 2") then
    local playrate = 0.25+0.75/1000.0*reaper.MRP_GetControlFloatNumber(mywindow,"Slider 2")
    reaper.CSurf_OnPlayRateChange(playrate)
  end
  if reaper.MRP_GetWindowDirty(mywindow,1) then
    local w = reaper.MRP_GetWindowPosSizeValue(mywindow,2)
    local h = reaper.MRP_GetWindowPosSizeValue(mywindow,3)
    reaper.MRP_SetControlBounds(mywindow,"Slider 1",w/2,5,w/2-10,20)
    reaper.MRP_SetControlBounds(mywindow,"Slider 2",5,30,w-10,h-60)
    reaper.MRP_SetControlBounds(mywindow,"Button 1",w-45,h-20,35,19)
    reaper.MRP_SetControlBounds(mywindow,"Button 2",w-95,h-20,50,19)
    reaper.MRP_SetControlBounds(mywindow,"Line edit 1",5,h-20,w-110,19)
    reaper.MRP_SetControlBounds(mywindow,"Label 1",5,5,w/2-5,19)
    --reaper.ShowConsoleMsg("resized to "..w.." "..h.."\n")
    reaper.MRP_SetWindowDirty(mywindow,false,1)
  end
  if reaper.MRP_WindowIsDirtyControl(mywindow,"Button 2") then
    reaper.ShowConsoleMsg("Cancel clicked\n")
  end
  -- REMEMBER to call this if you are not sure you don't need to
  reaper.MRP_WindowClearDirtyControls(mywindow)
  reaper.defer(guitick)
end

mywindow=reaper.MRP_CreateWindow("My window")
reaper.MRP_WindowAddSlider(mywindow,"Slider 1",100)
reaper.MRP_WindowAddSlider(mywindow,"Slider 2",900)
reaper.MRP_WindowAddButton(mywindow,"Button 1","OK")
reaper.MRP_WindowAddButton(mywindow,"Button 2","Cancel")
reaper.MRP_WindowAddLineEdit(mywindow,"Line edit 1","Foofoo text")
reaper.MRP_WindowAddLabel(mywindow,"Label 1", "This is a label")
guitick()


