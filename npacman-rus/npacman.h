/** Super Turbo NES Pac-Man 1.0 для NES, Famicom и Dendy

FCEUX - Nintendo Entertainment System (NES), Famicom, Famicom Disk System (FDS), 
		and Dendy emulator
https://fceux.com/
 
сс65 - 6502 C compiler
https://cc65.github.io/

YY-CHR - chr editor
https://www.romhacking.net/utilities/119/

NES Screen Tool - chr editor
https://shiru.untergrund.net/files/nesst.zip 

В проекте активно использовались наработки Shiru
https://shiru.untergrund.net/


Ограничения накладываемые на разработку для 8 битной приставки Dendy / NES / famicom:

Большая часть переменных должна быть типа unsigned char — 8 бит, значения 0-255

Лучше не передавать значения в функции, или делать это через директиву fastcall, 
которая передает аргументы через 3 регистра — A,X,Y

Массивы не должны быть длинее 256 байт

++i заметно быстрее, чем i++

cc65 не может ни передавать структуры по значению, ни возвращать их из функции

Глобальные переменные намного быстрее локальных, даже структуры 
*/


// задний фон стартового экрана (STATE_PLAY_SELECT)
// сгенерированно с опмащью NES Screen Tool, с включенной RLE опцией
#include "BG/1.h" 

// задний фон игры и результаа игры (STATE_GAME, STATE_GAME_RESULT)
// сгенерированно с опмащью NES Screen Tool, с включенной RLE опцией
#include "BG/2.h"

// TODO delete it
//#pragma bss-name(push, "ZEROPAGE")



/**
 * Константы 
 */


// стартовый экран - выбор количества игроков
const unsigned char STATE_SELECT = 0;

// игра - второй экран после выбора количества игроков
const unsigned char STATE_GAME  = 1;

// третий экран, идет после игры - результат игры
const unsigned char STATE_RESULT = 2;

// пауза
const unsigned char STATE_PAUSE = 3;

// Еда
const unsigned char FOOD = '.';

// Поверап позваляющий есть призраков
const unsigned char POWER_FOOD = '*';

// Дверь в комнату призраков
const unsigned char DOOR = '-';

// Ни кем не занятая клетка
const unsigned char EMPTY = ' ';

// Pac-Man
const unsigned char PACMAN = 'O';

// Pac Girl
const unsigned char PACGIRL ='Q';

// Красный призрак (BLINKY)
const unsigned char RED = '^';

// Красный призрак когда его можно съесть (SHADOW или BLINKY)
const unsigned char SHADOW = '@';

// Вишня
const unsigned char CHERRY = '%';

// палитра для tile заднего фона
const unsigned char paletteForBackground[]={
	0x0f, 0x2a, 0x11, 0x10,
	0x0f, 0x15, 0x14, 0x15, 
	0x0f, 0x10, 0x14, 0x11,
	0x0f, 0x30, 0x15, 0x27
};

// палитра для спрайтов
const unsigned char paletteForSprites[]={
	0x0f, 0x30, 0x15, 0x38, 
	0x0f, 0x30, 0x15, 0x27,  
	0x0f, 0x30, 0x15, 0x11, 
	0x0f, 0x30, 0x14, 0x11
};


// скорость перехода pacman с одной клетки на другую в милисекундах
#define PACMAN_SPEED (9);

// скорость перехода pacGirl с одной клетки на другую в милисекундах
#define PACGIRL_SPEED (9);

// скорость перехода Красного призрака с одной клетки на другую в милисекундах
#define RED_SPEED (9);

// Время через которое появляеться вишня
#define CHERRY_TIME (255);

// количество очков за поедание вишни
#define SCORE_CHERY_BONUS (200);

// количество очков за поедание POWER_FOOD
#define SCORE_POWER_BONUS (25);

// количество очков за поедание RED
#define SCORE_RED_EAT (50);

// время через которое RED перестает быть съедобным
#define RED_TIME (255);

// размер карты по x
#define MAP_SIZE_Y (23)

// размер карты по y
#define MAP_SIZE_X (32)

// размер карты по y для 3х сегментов
#define MAP_SIZE_Y8 (8)
#define MAP_SIZE_Y16 (16)


/**
 * Глобальные переменные
 */

// карта 1 часть 
// разбил на 3 части, т.к. массив не может 
// быть больше 256 байт для 8 битной консоли
unsigned char map1[MAP_SIZE_Y8][MAP_SIZE_X] = {
 "7888888888888895788888888888889",
 "4.............654.............6",
 "4*i220.i22220.l8d.i22220.i220*6",
 "4..............Q..............6",
 "4.i220.fxj.i22mxn220.fxj.i220.6",
 "4......654....654....654......6",
 "1xxxxj.65s220.l8d.222e54.fxxxx3",
 "555554.654...........654.655555"
};

// карта 2 часть
unsigned char map2[MAP_SIZE_Y8][MAP_SIZE_X] = {
 "555554.654.fxxj-fxxj.654.655555",
 "88888d.l8d.678d l894.l8d.l88888",
 "...........64  %  64..^........",
 "xxxxxj.fxj.61xxxxx34.fxj.fxxxxx",
 "555554.654.l8888888d.654.655555",
 "555554.654...........654.655555",
 "78888d.l8d.i22mxn220.l8d.l88889",
 "4.............654.............6"
 };

// карта 3 часть
unsigned char map3[MAP_SIZE_Y8][MAP_SIZE_X] = {
 "4.i2mj.i22220.l8d.i22220.fn20.6",
 "4*..64.........O.........64..*6",
 "s20.ld.fxj.i22mxn220.fxj.ld.i2e",
 "4......654....654....654......6",
 "4.i2222y8z220.l8d.i22y8z22220.6",
 "4.............................6",
 "1xxxxxxxxxxxxxxxxxxxxxxxxxxxxx3",
 "                               "
 };

// для обработки нажатия кнопок первым игроком на контроллере
unsigned char pad1;

// для обработки нажатия кнопок вторым игроком на контроллере
unsigned char pad2;

// переменная для отрисовки текстовой информации
// очки, бонусы, GAME OVER, YOU WINNER
unsigned char text;

// для получения адреса по координатам x, y на tile заднего фона
int address;

// состояние иры (на каком экране находимся)
// 0 - STATE_SELECT - стартовый экран (выбор количества игроков)
// 1 - STATE_GAME - игра - второй экран после выбора количества игроков
// 2 - STATE_GAME_RESULT - третий экран, идет после игры - результат игры
// 3 - STATE_PAUSE - пауза
unsigned char gameState = 0;


// текущие координаты PACMAN
int pacmanX = 15;
int pacmanY = 17;

// текущие координаты PACGIRL
int pacGirlX = 15;
int pacGirlY = 3;

// старые координаты PACMAN
int oldX = 15;
int oldY = 17;

// последний спрайт pacman
unsigned char pacmanSprite = 1;

// последний спрайт pacGirl
unsigned char pacGirlSprite = 1;

// направление движение PACMAN
int dx = 0;
int dy = 0;

// направление движение PACGIRL
int dxPacGirl = 0;
int dyPacGirl = 0;

// старые координаты PACGIRL
int oldPacGirlX = 15;
int oldPacGirlY = 3;

// направление движения RED (SHADOW или BLINKY)
int dxRed = 1;
int dyRed = 0;

// координаты RED (SHADOW или BLINKY)
int redX = 22;
int redY = 10;

// старые координаты RED (SHADOW или BLINKY)
int oldXRed = 22;
int oldYRed = 10;

// последний спрайт RED
unsigned char redSprite = 1;

// 1 - RED в режиме охоты
// 0 - PACMAN съел POWER_FOOD и RED сейчас съедобен
unsigned char redFlag = 1;

// время когда RED стал съедобным последний раз
unsigned char redTime = 0;

// 1 - Вишня есть
// 0 - Вишни нет
unsigned char cherryFlag = 0;

// надо перерисовать черешню
unsigned char refreshCherry = 0;

// x координата черешни
unsigned char cherryX = 15;
// y координата черешни
unsigned char cherryY = 10;

// x координата двери
unsigned char doorX = 15;
// y координата двери
unsigned char doorY = 8;

// надо перерисовать дверь
unsigned char refreshDoor = 1;

// что лежит на клетке с RED (BLINKY)
unsigned char oldRedVal = '.';

// что лежит на клетке с PACGIRL
unsigned char oldPacGirlVal = '.';

// бонус за съедание RED (BLINKY)
unsigned char redBonus = 0;

// бонус за съедание POWER_FOOD
unsigned char powerBonus = 0;

// бонус за съеденные вишни
unsigned char cherryBonus = 0;

// счетчики циклов начинаются с разных значений
// чтоб рендеринг каждого персонажа был в разном глобальном цикле
// время последнего обновления положения pacman
unsigned char pacmanLastUpdateTime = PACMAN_SPEED;

// время поседнего обновления RED
unsigned char redLastUpdateTime = 4; //RED_SPEED;

// время последнего обновления положения pacGirl
unsigned char pacGirlLastUpdateTime = 6; //PACGIRL_SPEED;

// время через которое появится вишня
unsigned char cherryTime = CHERRY_TIME;

// переменные для работы с массивами map1, map2, map3 и для циклов
// // через функции setValToMap getValFromMap
unsigned char i = 0;
unsigned char j = 0;

// переменная через которую записываем и получаем значения из map1, map2, map3
// через функции setValToMap getValFromMap
unsigned char val;

// временная переменная, в нее часто сохраняем значение val
unsigned char val2;

// координаты y, y привязанные к верхнему левому углу (в пикселях)
// используются для определения где рисовать объекты
unsigned char y;
unsigned char x;

// количество выбранных игроков для игры
unsigned char players = 1;

// счетчик циклов для защиты от 2го нажатия на стартовом экране 
// кнопки Select
unsigned char playersTime = 0;


// для ывода сколько точек белых съели их больше 256 поэтому
// храню в 3х счетчиках, ну и для вывода проще так
// единицы для съеденной еды
unsigned char food001 = 1;

// десятки для съеденной еды
unsigned char food010 = 0;

// сотни для съеденной еды
unsigned char food100 = 0;


// для вывода результата игры значение от 000 до 999
// единицы для итоговых очков
unsigned char score001 = 0;

// десятки для итоговых очков
unsigned char score010 = 0;

// сотни для итоговых очков
unsigned char score100 = 0;

// координата спрайта с словом ПАУЗА по x
unsigned char pauseX = 112;

// координата спрайта с словом ПАУЗА по y
unsigned char pauseY = 122;

// скорость смещения спрайта с словом ПАУЗА по x
int pauseDX = -1;

// скорость смещения спрайта с словом ПАУЗА по y
int pauseDY = -2;

/**
 * Функции
 */


/**
 * Сохраняем значение val в карту
 * i - строка в массиве карты
 * j - столбец в массиве карты
 */
void setValToMap(void);

/**
 * Достаем значение из карты в val
 * i - строка в массиве карты
 * j - столбец в массиве карты
 */
void getValFromMap(void);

/**
 * Клетка по заданным координатам не стена (WALL)
 * i - строка в массиве карты
 * j - столбец в массиве карты
 * return val = 1 - не стена, 0 - стена
 */
void isNotWall(void);

/**
 * Клетка по заданным координатам не стена и не дверь (WALL, DOOR)
 * y - координата Y на карте (map[][])
 * x - координата X на карте (map[][])
 * return val = 1 - не стена и не дверь, 0 - стена или дверь
 */
void isNotWallOrDoor(void);

/**
 * Открыть двери к вишне и дому призраков
 */
void openDoors(void);

/**
 * Закрыть двери к дому призраков
 */
void closeDoors(void);

/**
 * Съедена еда
 * пересчитать значения счетчиков 
 * food001 food010 food100
 */
void incFood(void);

/**
 * Подсчет отчков с учетом всех бонусов
 */
void calcScore(void);

/**
 * Сбрасываем все на начальные настройки по карте:
 * начальные значения счетчиков циклов
 * начальное положение персонажей
 * где будет еда и поверапы
 */
void init(void);

/**
 * Проиграл ли PACMAN или он мог съесть призрака
 * и что съел на месте призрака
 */
int pacmanLooser(void);

/**
 * Алгоритм обработки движения PACMAN на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int pacManState();

/**
 * Алгоритм призрака гоняющегося за PACMAN
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int redState();

/**
 * Алгоритм обработки движения PACGIRL на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int pacGirlState();

/**
 *  Обработка нажатых кнопок игроком
 *  передвижение персонажей во время игры
 */
void actions(void);

/**
 * Нарисовать только 1 объект с карты
 * i - строка в массиве карты
 * j - столбец в массиве карты
 */
void draw(void);

/**
 * Нарисовать задний фон
 */
void drawBackground(void);

/**
 * Нарисовать спрайты
 */
void drawSprites(void);

/**
 *  Нарисовать бонусы, очки
 *  результат игры (GAME OVER или YOU WINNER)
 */
void drawText(void);

/**
 *  Перерисовать на заднем фоне tile когда съели точку
 *  рисуем черный квадрат 8x8
 */
void drawBlackBox(void);

/**
 * Обновить карту / персонажей, двери, черешню
 * тут только отрисовка
 */
void refreshGame(void);






