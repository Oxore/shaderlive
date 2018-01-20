if (&ft=='c' || &ft=='h' || &ft=='cpp' || &ft=='vert' || &ft=='frag' )
  setlocal shiftwidth=4
  setlocal tabstop=4
  setlocal expandtab
endif

" Neomake c include
let g:neomake_c_enabled_makers = ['gcc']
let g:neomake_c_gcc_maker = {'args': ['-fms-extentions', '-fsyntax-only', '-Wall', '-Wextra', '-I../include/', '-I./include/']}
