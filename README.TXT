Super Turbo NES Pac-Man v1.0 - Pac-Man для игры в 2м под Dendy / NES / Famicom

папки:
pacman-eng - проект с интерфейсом на английском
pacman-rus - проект с интерфейсом на русском

бинарники:
pacman-eng/npacman.nes - rom игры с английским интерфесом который можно запустить в эуляторе, например FCEUX
pacman-eng/npacman_prg.bin - для прошивки в микросжему 27C256 на 32kb (я использовал ST M27C256B-15F1)
pacman-eng/npacman_chr.bin - для прошивки в микросхему 27C64 на 8kb (я использовал ATMEL AT28C64B 15PU)

pacman-rus/npacman [RUS].nes - rom игры с русским интерфесом который можно запустить в эуляторе, например Fceux
pacman-rus/npacman [RUS]_prg.bin - для прошивки в микросжему типа 27C256 на 32kb (я использовал ST M27C256B-15F1)
pacman-rus/npacman [RUS]_chr.bin - для прошивки в микросхему типа 27C64 на 8kb (я использовал ATMEL AT28C64B 15PU)

файлы для сборки: 
makefile - для сборки из под Linux командой make
npacman.cfg - конфиг для сборки игры

ресcурсные файлы:
npacman.chr - файл содержащий Tiles для заднего фона и спрайтов игры (изменять в NES Screen Tool)
GB/1.nam - файл с NAMETABLE стартового экрана (изменять в NES Screen Tool)
GB/2.nam - файл с NAMETABLE игры и результата (изменять в NES Screen Tool)

исходный код:
npacman.h - заголовочный файл игры на C (для 6502 C compiler)
npacman.c - исходный код игры на C (для 6502 C compiler)
sprites.h - спрайты для игры (для 6502 C compiler)
GB/1.h - задний фон стартового экрана, сгенерирован из GB/1.nam (для 6502 C compiler)
GB/2.h - задний фон игры и экрана с результатом, сгенерирован из GB/2.nam (для 6502 C compiler)
LIB/neslib.h - библиотека NES hardware-dependent функции от Shiru (для 6502 C compiler)
LIB/nesdoug.h - библиотека от Doug Fraker (для 6502 C compiler)

библиотеки (исходники на ассемблере):
crt0.s - код на ассемблере ca65 необходимый для игры, на основе кода Groepaz/Hitmen и Ullrich von Bassewitz
LIB/neslib.s -  код на ассемблере ca65 необходимый для LIB/neslib.h
LIB/nesdoug.s -  код на ассемблере ca65 необходимый для LIB/nesdoug.h 

звуки и музыка:
MUSIC/famitone2.s - код на ассемблере ca65 аудио драйвера 
MUSIC/Fx.s - код на ассемблера ca65 для звуковых эфектов в игре
MUSIC/Music.s - код на ассемблера ca65 содержащий музыку для игры

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

NES ROM Maker - для получения из .nes рома prg.bin и chr.bin для прошивки в микросхемы для платы картриджа
https://vfiuchcikicshuusrch.ddns.net/?post=3


Ограничения накладываемые на разработку для 8 битной приставки Dendy / NES / famicom 
при использовании 6502 C compiler:

Большая часть переменных должна быть типа unsigned char — 8 бит, значения 0-255

Лучше не передавать значения в функции, или делать это через директиву fastcall, 
которая передает аргументы через 3 регистра — A,X,Y

Массивы не должны быть длинее 256 байт

++i заметно быстрее, чем i++

cc65 не может ни передавать структуры по значению, ни возвращать их из функции

Глобальные переменные намного быстрее локальных, даже структуры 


BlodTor 24.10.2023