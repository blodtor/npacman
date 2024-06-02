@echo off

rem имя рома и игры
set name="npacman"

rem генерация из кода на Си файла npacman.s на ассемблере
cc65 -Oirs %name%.c --add-source

rem получаем из кода на ассемблере объектные файлы crt0.o и npacman.o 
ca65 crt0.s
ca65 %name%.s -g

rem сборка рома игры npacman.nes из crt0.o и npacman.o
ld65 -C %name%.cfg -o %name%.nes crt0.o %name%.o nes.lib -Ln labels.txt --dbgfile dbg.txt

rem удаление объектных файлов crt0.o и npacman.o
del *.o
