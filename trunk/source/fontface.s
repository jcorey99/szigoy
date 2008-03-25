# Fonts

.rodata
.balign 32
.globl fontsize
.globl fontface
fontsize:       .long fontfaceend - fontface

fontface:	
.incbin "../source/ttf/font.ttf"
fontfaceend:

