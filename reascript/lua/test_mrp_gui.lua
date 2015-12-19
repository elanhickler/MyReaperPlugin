mywindow=nil

function guitick()
  if reaper.MRP_WindowWantsClose(mywindow) then
    reaper.MRP_DestroyWindow(mywindow)
    return
  end
  local text = reaper.MRP_GetControlText(mywindow,"Line edit 1")
  reaper.MRP_WindowSetTitle(mywindow,"Window "..text)
  reaper.defer(guitick)
end

mywindow=reaper.MRP_CreateWindow("my window")
guitick()


