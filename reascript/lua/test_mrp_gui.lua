mywindow=nil

function guitick()
  if reaper.MRP_WindowWantsClose(mywindow) then
    reaper.MRP_DestroyWindow(mywindow)
  end    
  reaper.defer(guitick)
end

mywindow=reaper.MRP_CreateWindow("my win")
guitick()

