Scriptname f314FD_Utils hidden

form function DisenchantNative(form a)  global native

form function Disenchant(form a) global
  form ans = a
  form mb_disenchanted = f314FD_Utils.DisenchantNative(ans)
  if mb_disenchanted
    ans = mb_disenchanted
  endif
  return ans
endfunction
