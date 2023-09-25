Super Turbo NES Pac-Man v1.0 - Pac-Man для игры в 2м под Dendy / NES / Famicom

pacman-eng - интерфейс на английском
pacman-rus - интерфейс на русском

makefile - для сборки из под Linux командой make
npacman.cfg - конфиг для сборки игры
npacman.chr - файл содержащий Tiles для заднего фона и спрайтов игры (изменять в NES Screen Tool)
GB/1.nam - файл с NAMETABLE стартового экрана (изменять в NES Screen Tool)
GB/2.nam - файл с NAMETABLE игры и результата (изменять в NES Screen Tool)

npacman.h - заголовочный файл игры на C (для 6502 C compiler)
npacman.c - исходный код игры на C (для 6502 C compiler)
sprites.h - спрайты для игры (для 6502 C compiler)
GB/1.h - задний фон стартового экрана, сгенерирован из GB/1.nam (для 6502 C compiler)
GB/2.h - задний фон игры и экрана с результатом, сгенерирован из GB/2.nam (для 6502 C compiler)
LIB/neslib.h - библиотека NES hardware-dependent функции от Shiru (для 6502 C compiler)
LIB/nesdoug.h - библиотека от Doug Fraker (для 6502 C compiler)

crt0.s - код на ассемблере ca65 необходимый для игры, на основе кода Groepaz/Hitmen и Ullrich von Bassewitz
LIB/neslib.s -  код на ассемблере ca65 необходимый для LIB/neslib.h
LIB/nesdoug.s -  код на ассемблере ca65 необходимый для LIB/nesdoug.h 
MUSIC/famitone2.s - код на ассемблере ca65 аудио драйвера 
MUSIC/Fx.s - код на ассемблера ca65 для звуковых эфектов в игре
MUSIC/Music.s - код на ассемблера ca65 содержащий музыку для игры

npacman.nes, npacman [RUS].nes - РОМы игры можно запустить в эмуляторе FCEUX

Описание игры:
1 игрок управляет Pac-Man за которым гоняется 1 призрак красного цвета, 
в случае если призрак догонит Pac-Man - игра закончится.
в случае если Pac-Man или Pac-Girl съедят большую зеленую точку, призрак станет 
фиолетового цвета и начнет убегать от Pac-Man т.к. теперь его самого можно съесть
2 игрок управляет Pac-Girl. Она помогает собирать точки на уровне при этом ни Pac-Man ни 
призрак не могут съесть Pac-Girl а она их (бессмертная) 
Игра закончится победой если игроки смогут собрать всю еду (маленькие и большие точки) на уровне
В доме призраков через некоторое время появляется вишня - за нее просто дают очки 
чтоб выиграть есть не обязательно ее!

FCEUX - emulator для Nintendo Entertainment System (NES), Famicom, Famicom Disk System (FDS) и Dendy 
https://fceux.com/
 
сс65 - 6502 C compiler
https://cc65.github.io/

YY-CHR - chr editor
https://www.romhacking.net/utilities/119/

NES Screen Tool - chr editor
https://shiru.untergrund.net/files/nesst.zip 

В проекте активно использовались наработки Shiru
https://shiru.untergrund.net/

В проекте активно использовались наработки Doug Fraker
https://github.com/nesdoug


Ограничения накладываемые на разработку для 8 битной приставки Dendy / NES / famicom 
при использовании 6502 C compiler:

Большая часть переменных должна быть типа unsigned char — 8 бит, значения 0-255

Лучше не передавать значения в функции, или делать это через директиву fastcall, 
которая передает аргументы через 3 регистра — A,X,Y

Массивы не должны быть длинее 256 байт

++i заметно быстрее, чем i++

cc65 не может ни передавать структуры по значению, ни возвращать их из функции

Глобальные переменные намного быстрее локальных, даже структуры 


BlodTor 25.09.2023