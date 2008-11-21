OUTPUT_FORMAT("coff-sh")
OUTPUT_ARCH(sh)
MEMORY
{
  ram : o = 0x80010000, l = 4M
  uram : o = 0x8c010000, l = 16M
}
SECTIONS
{
  .text :
  {
    *(.text)
    *(.strings)
     _etext = . ;
  }  > ram
  .tors :
  {
    ___ctors = . ;
    *(.ctors)
    ___ctors_end = . ;
    ___dtors = . ;
    *(.dtors)
    ___dtors_end = . ;
  } > ram
  .data :
  {
    *(.data)
     _edata = . ;
  }  > ram
  .bss :
  {
     _bss_start = . ;
    *(.bss)
    *(COMMON)
     _end = . ;
  }  > uram
  .stack   :
  {
     _stack = . ;
    *(.stack)
  }  > uram
  .stab 0 (NOLOAD) :
  {
    *(.stab)
  }
  .stabstr 0 (NOLOAD) :
  {
    *(.stabstr)
  }
}
