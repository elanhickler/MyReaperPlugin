mywindow=nil

function guitick()
  if reaper.MRP_WindowWantsClose(mywindow) then
    reaper.MRP_DestroyWindow(mywindow)
    return
  end
  if reaper.MRP_GetWindowDirty(mywindow) then
    local text = reaper.MRP_GetControlText(mywindow,"Line edit 1")
    local val = reaper.MRP_GetControlFloatNumber(mywindow,"Slider 1")
    local val2 = reaper.MRP_GetControlFloatNumber(mywindow,"Slider 2")
    reaper.ShowConsoleMsg(val.." "..val2.."\n")
    reaper.MRP_WindowSetTitle(mywindow,"Window "..text)
    reaper.MRP_SetWindowDirty(mywindow,false)
  end
  reaper.defer(guitick)
end

mywindow=reaper.MRP_CreateWindow("my window")
guitick()


