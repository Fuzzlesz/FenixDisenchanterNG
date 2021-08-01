Scriptname f314FD_DisenchanterScript extends ObjectReference

Message         property f314FD_Msg         auto
GlobalVariable  property f314FD_ShowMessage auto

ObjectReference Property f314FD_Stuff       auto
ActorBase       Property f314FD_Fenix       auto

form addeditem = none

event onActivate(ObjectReference akActionRef)  
  int should_show = f314FD_ShowMessage.GetValue() as int
  if should_show >= 1
    f314FD_Msg.Show()
    f314FD_ShowMessage.setvalue((should_show - 1) as float)
  endif
  
endevent

function handleItem(Form akBaseItem)
  form toAdd = f314FD_Utils.Disenchant(akBaseItem)
  
  self.RemoveItem(akBaseItem, 1, true, f314FD_Stuff)
  ObjectReference item = f314FD_Stuff.DropObject(akBaseItem, 1)
  ObjectReference toAddObject = self.placeatme(toAdd, 1, false, true)
  if item.IsOffLimits()
    toAddObject.SetActorOwner(f314FD_Fenix)
  endIf
  f314FD_Stuff.AddItem(item)
  f314FD_Stuff.RemoveItem(akBaseItem, 1)
  
  ;toAddObject.enable()
  AddItem(toAddObject, 1, true)
endfunction

event onitemadded(Form akBaseItem, int aiItemCount, ObjectReference akItemReference, ObjectReference akSourceContainer)
  addeditem = akBaseItem
  if aiItemCount == 1
    gotostate("ItemAdded")
  endif
endevent

state ItemAdded
  event onitemadded(Form akBaseItem, int aiItemCount, ObjectReference akItemReference, ObjectReference akSourceContainer)
  endevent
  
  event onbeginstate()
    handleItem(addeditem)
    addeditem = none
    gotostate("")
  endevent
endstate
