.data
.space 11
.word 0x87654321
.word 0x1, 2,-3,077,-077
.half 65535
.half -1,2,-32768,32768
.byte 'a'
.byte 0x20,0x21,0x22,'A','B','C','\''
.text
.ascii "This is"," a C stri","ng","\0"
.ascii "\"","a\"","a\"b","\0\b\f\n\r\t\v\1\33\033\xFE\xfe\Xfe\xFE\q\%"
