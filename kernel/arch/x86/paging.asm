global loadPageDirectory
loadPageDirectory:
mov eax, [esp+4]
mov cr3, eax
ret

global getPageDirectory
getPageDirectory:
mov eax, cr3
ret

global reloadPageDirectory
reloadPageDirectory:
mov eax, cr3
mov cr3, eax
ret

global enablePaging
enablePaging:
mov eax, cr0
or eax, 0x80000000
mov cr0, eax
ret
