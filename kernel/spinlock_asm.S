.globl acquire
.globl release

acquire:
    mov w3, #1
1:
    ldxr w1, [x0]
    cbnz w1, 2f
    stxr w2, w3, [x0]
    cbnz w2, 1b 
    dmb ish
    ret 
2:
  wfe
  b 1b


release:
    dmb ish 
    str wzr, [x0]
    sev 
    ret 

