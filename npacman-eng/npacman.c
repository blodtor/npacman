/** Super Turbo NES Pac-Man 1.0 для NES, Famicom и Dendy

FCEUX - emulator для Nintendo Entertainment System (NES), Famicom, Famicom Disk System (FDS), 
		и Dendy 
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

#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "sprites.h"
#include "npacman.h"


/**
 * Достаем значение из карты в val
 * i - строка в массиве карты
 * j - столбец в массиве карты
 */
void getValFromMap(void) {
	if (j < MAP_SIZE_Y8) {
		val = map1[j][i];
	} else if (j >= MAP_SIZE_Y8 && j < MAP_SIZE_Y16) {
		val = map2[j - MAP_SIZE_Y8][i];
	} else if (j >= MAP_SIZE_Y16 && j < MAP_SIZE_Y) {
		val = map3[j - MAP_SIZE_Y16][i];
	}
}

/**
 * Сохраняем значение val в карту
 * i - строка в массиве карты
 * j - столбец в массиве карты
 */
void setValToMap(void) {
	if (j < MAP_SIZE_Y8) {
		map1[j][i] = val;
	} else if (j >= MAP_SIZE_Y8 && j < MAP_SIZE_Y16) {
		map2[j-MAP_SIZE_Y8][i] = val;
	} else if (j >= MAP_SIZE_Y16 && j < MAP_SIZE_Y) {
		map3[j-MAP_SIZE_Y16][i] = val;
	}
}


/**
 * Клетка по заданным координатам не стена (WALL)
 * i - строка в массиве карты
 * j - столбец в массиве карты
 * return val = 1 - не стена, 0 - стена
 */
void isNotWall(void) {
	getValFromMap();
	if (val == PACMAN || val == PACGIRL || val == RED
			|| val == CHERRY || val == FOOD
			|| val == POWER_FOOD || val == EMPTY
			|| val == SHADOW) {
		val = 1;

	} else {
		val = 0;
	}
}

/**
 * 
 * Клетка по заданным координатам не стена и не дверь (WALL, DOOR)
 * y - координата Y на карте (map[][])
 * x - координата X на карте (map[][])
 * return val = 1 - не стена и не дверь, 0 - стена или дверь
 */
void isNotWallOrDoor(void) {
	getValFromMap();
	if ((val == PACMAN || val == PACGIRL || val == RED
			|| val == CHERRY || val == FOOD
			|| val == POWER_FOOD || val == EMPTY
			|| val == SHADOW) && val != DOOR) {
		val = 1;

	} else {
		val = 0;
	}
}

/**
 * Открыть двери к вишне и дому призраков
 */
void openDoors(void) {
	i = doorX;
	j = doorY;
	val = EMPTY;
	setValToMap();

	i = cherryX;
	j = cherryY;
	val = CHERRY;
	setValToMap();
	
	cherryFlag = 1;
	refreshCherry = 1;
}


/**
 * Закрыть двери к дому призраков
 * если вишню не съел PACMAN она появится еще
 */
void closeDoors(void) {
	i = doorX;
	j = doorY;
	val = DOOR;
	setValToMap();

	cherryFlag = 0;
	refreshDoor = 1;
	refreshCherry = 0;
}


/**
 * Съедена еда
 * пересчитать значения счетчиков 
 * food001 food010 food100
 * 
 * т.к. у 8 битной консоли максимальное число 256
 * то проще считать очки в 3х отдельных переменных
 * так можно запомнить число от 000 до 999
 * ну и выводить результат так проще
 */
void incFood(void) {
	sfx_play(0, 0);
	++food001;
	if (food001 >= 10) {
		food001 -= 10;
		++food010;
	}
	if (food010 >= 10) {
		food010 -= 10;
		++food100;
	}
}

/**
 *  Подсчет отчков с учетом всех бонусов
 */
void calcScore(void) {
		score100 = food100;
		score010 = food010;
		score001 = food001;

		if (cherryBonus) {
			score100 += 2;			
		}

		for (i = 0; i < powerBonus; i++) {
			score001 += 5;

			if (score001 >= 10) {
				score001 -= 10;
				++score010;
			}

			score010 += 2;

			if (score010 >=10) {
				score010 -= 10;
				++score100;
			}

		}


		for (i = 0; i < redBonus; i++) {
			score010 += 5;

			if (score010 >=10) {
				score010 -= 10;
				++score100;
			}
		}
}


/**
 * Сбрасываем все на начальные настройки по карте:
 * начальные значения счетчиков циклов
 * начальное положение персонажей
 * где будет еда и поверапы
 */
void init(void) {	
	// счетчики циклов начинаются с разных значений
	// чтоб рендеринг каждого персонажа был в разном глобальном цикле
	pacmanLastUpdateTime = PACMAN_SPEED;	
	redLastUpdateTime = 4;     //RED_SPEED;
	pacGirlLastUpdateTime = 6; //PACGIRL_SPEED;
	cherryTime = CHERRY_TIME

	cherryBonus = 0;
	powerBonus = 0;
	redBonus = 0;

	food001 = 1;
	food010 = 0;
	food100 = 0;


	pacmanX = 15;
	pacmanY = 17;

	pacGirlX = 15;
	pacGirlY = 3;

	oldX = 15;
	oldY = 17;

	pacmanSprite = 1;
	pacGirlSprite = 1;
	redSprite = 1;

	dx = 0;
	dy = 0;

	dxPacGirl = 0;
	dyPacGirl = 0;

	oldPacGirlX = 15;
	oldPacGirlY = 3;

	dxRed = 1;
	dyRed = 0;

	redX = 22;
	redY = 10;

	oldXRed = 22;
	oldYRed = 10;
	
	redFlag = 1;

	redTime = 0;

	cherryFlag = 0;
	refreshCherry = 0;
	refreshDoor = 1;


	oldRedVal = '.';
	oldPacGirlVal = '.';

	// расстовляем еду по карте (серые точки)
	for (i = 0; i < MAP_SIZE_X; i++) {
		for (j = 0; j < MAP_SIZE_Y; j++) {
			getValFromMap();
			if (val == EMPTY || val == PACGIRL || val == PACMAN || val == RED) {
				val = FOOD;
				setValToMap();
			}
			
		}
	}

	// расставляем поверапы
	val = POWER_FOOD;
	i = 1;
	j = 2;	
	setValToMap();

	i = 29;
	j = 2;	
	setValToMap();
	
	i = 1;
	j = 17;	
	setValToMap();

	i = 29;
	j = 17;	
	setValToMap();

	// Pac-Man на начальную позицию
	i = pacmanX;
	j = pacmanY;
	val = PACMAN;
	setValToMap();

	// Red на начальную позицию
	i = redX;
	j = redY;
	val = RED;
	setValToMap();

    // Pac-Girl на начальную позицию
	i = pacGirlX;
	j = pacGirlY;
	val = PACGIRL;
	setValToMap();

	// дверь в дом призраков
	// черешня и клетки вокруг 
	// в начальное состояние на карте	
	val = DOOR;	
	i = doorX;
	j = doorY;
	setValToMap();
	
	val = EMPTY;
	j = doorY + 1;
    setValToMap();

    j = cherryY;
    i = cherryX - 2;
    setValToMap();

    i = cherryX - 1;
    setValToMap();

	i = cherryX;
    setValToMap();    

    i = cherryX + 1;
    setValToMap();

    i = cherryX + 2;
    setValToMap();    
}

/**
 * Проиграл ли PACMAN или он мог съесть призрака
 * и что съел на месте призрака
 */
int pacmanLooser(void) {
	// Если RED и PACMAN на одной клетке поля
	if (redY == pacmanY && redX == pacmanX) {
		// RED не съедобен
		if (redFlag) {
			// Конец игры - PACMAN съеден

		  i = pacmanX;
	      j = pacmanY;
	      val = RED;
	      setValToMap();


	      calcScore();

			return 1;
		} else {
			sfx_play(2, 0);
			// RED съедобен в данный момент
			// Отправляем его в дом Приведений
			redY = 10;
			redX = 15;
			// бездвиживаем
			dyRed = 0;
			dxRed = 0;
			// закрываем дверь в дом привидений
			closeDoors();

			// отображаем RED на карте как съедобного
			i = redX;
	        j = redY;
	        val = RED;
	        redFlag = 1;
	        setValToMap();

	       	// пусть сидит в домике дополнительное время
	        redTime = RED_TIME;

			// даем бонус за то что RED съели
			++redBonus;

			// проверяем что пакмен съел вместе с RED
			if (oldRedVal == FOOD) {
				// еду				
				incFood();
			} else if (oldRedVal == POWER_FOOD) {
				// поверап
				++powerBonus;
				sfx_play(5, 0);

				// обнавляем время когда RED стал съедобным
				redTime = RED_TIME;

			} else if (oldRedVal == CHERRY) {
				// вишню
				++cherryBonus;
				sfx_play(3, 0);
			}

			oldRedVal = EMPTY;
		}
	} else if (redY == pacGirlY && redX == pacGirlX) {
		// проверяем что Pac-Girl съела на месте RED
		if (oldRedVal == FOOD) {
			// еду
			incFood();
		} else if (oldRedVal == POWER_FOOD) {
			// поверап
			++powerBonus;
			sfx_play(5, 0);

			// обнавляем время когда RED стал съедобным
			redTime = RED_TIME;

			// RED становится съедобным
			redFlag = 0;
		} else if (oldRedVal == CHERRY) {
			// вишню
			++cherryBonus;
			sfx_play(3, 0);
		}

		i = pacGirlX;
	    j = pacGirlY;
	    val = RED;
	    setValToMap();

		oldRedVal = EMPTY;
	}
	return 0;
}


/**
 * Алгоритм призрака гоняющегося за PACMAN
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int redState() {

	// надо ли RED перейти в режим погони
	if (redTime == 0 ) {
		redFlag = 1;
		// если не двигается, пусть идет вверх
		if (dyRed == 0 && dxRed == 0) {
			dyRed = -1;
		}
	} else if (redLastUpdateTime == 0 && dyRed == 0 && dxRed == 0){
		redLastUpdateTime = RED_SPEED;
	}

	// проверяем, у RED задоно ли направление движения
	if (dxRed != 0 || dyRed != 0) {
		if (redLastUpdateTime == 0) {
			redX = redX + dxRed;
			redY = redY + dyRed;
			
			redLastUpdateTime = RED_SPEED;

			// вышли за границы
			if (redX < 0) {
				redX = MAP_SIZE_X - 2;
			} else if (redX > MAP_SIZE_X - 2) {
				redX = 0;
			}

			if (redY < 0) {
				redY = MAP_SIZE_Y - 1;
			} else if (redY > MAP_SIZE_Y - 1) {
				redY = 0;
			}

			i = redX;
			j = redY;
			isNotWall();
			if (val) {

				i = oldXRed;
				j = oldYRed;
				val = oldRedVal;
				setValToMap();

				i = redX;
				j = redY;
				getValFromMap();
				oldRedVal = val;

				if (redX == 15 && redY >= 7 && redY <= 10) {
					dyRed = -1;
					dxRed = 0;
				} else if (dxRed != 0) {

					i = redX;
					j = redY + 1;
					isNotWallOrDoor();
					val2 = val;


					j = redY - 1;
					isNotWallOrDoor();

					if (redFlag && redY != pacmanY) {

						if (val	&& val2) {
							if (redY + 1 - pacmanY < 0) { 
								val = pacmanY - (redY + 1);
							} else { 
								val = redY + 1 - pacmanY;
							}


							if (redY - 1 - pacmanY < 0) {
								val2 = pacmanY - (redY - 1);
							} else {
                                val2 = redY - 1 - pacmanY;
							}

							if (val < val2) {
								dyRed = 1;
							} else {
								dyRed = -1;
							}
						} else if (val2) {

							if (redY + 1 - pacmanY < 0) { 
								val = pacmanY - (redY + 1);
							} else { 
								val = redY + 1 - pacmanY;
							}

							if (redY - pacmanY < 0) {
								val2 = pacmanY - redY;
							} else {
                                val2 = redY - pacmanY;
							}

							if (val < val2) {
								dyRed = 1;
							}
						} else if (val) {

							if (redY + 1 - pacmanY < 0) { 
								val = pacmanY - (redY + 1);
							} else { 
								val = redY + 1 - pacmanY;
							};


							if (redY - pacmanY < 0) {
								val2 = pacmanY - redY;
							} else {
                                val2 = redY - pacmanY;
							}

							if (val < val2) {
								dyRed = -1;
							}
						}
					} else {
						if (val2) {							
							dyRed = rand8() % 2;
						}

						if (val) {
							dyRed = -1 * (rand8() % 2);					
						}
					}

					if (dyRed != 0) {
						dxRed = 0;
					}

				} else if (dyRed != 0) {


					i = redX + 1;
					j = redY;
					isNotWallOrDoor();
					val2 = val;


					i = redX - 1;
					isNotWallOrDoor();


					if (redFlag && redX != pacmanX) {
						if (val2 && val) {	

							if (redX + 1 - pacmanX < 0) {
								val = pacmanX - (redX + 1);
							} else {
								val = redX + 1 - pacmanX;
							}

							if (redX - 1 - pacmanX < 0) {
								val2 = pacmanX - (redX - 1);
							} else {
								val2 = redX - 1 - pacmanX;
							}

							if (val < val2) {
								dxRed = 1;
							} else {
								dxRed = -1;
							}
						} else if (val2) {

							if (redX + 1 - pacmanX < 0) {
								val = pacmanX - (redX + 1);
							} else {
								val = redX + 1 - pacmanX;
							}

							if (redX - pacmanX < 0) {
								val2 = pacmanX - redX;
							} else {
								val2 = redX - pacmanX;
							}

							if (val < val2) {
								dxRed = 1;
							}
						} else if (val) {

 							if (redX - 1 - pacmanX < 0) {
 								val = pacmanX - (redX - 1);
 							} else {
 								val = redX - 1 - pacmanX;
 							}

 							if (redX - pacmanX < 0) {
 								val2 = pacmanX - redX;
 							} else {
 								val2 = redX - pacmanX;
 							}

							if (val < val2) {
								dxRed = -1;
							}

						}
					} else {

						if (val2) {
							dxRed = rand8() % 2;							
						}

						if (val) {
							dxRed = -1 * (rand8() % 2);							
						}

					}

					if (dxRed != 0) {
						dyRed = 0;
					}
				}
			} else {
				if (redX == 15 && redY >= 7 && redY <= 10) {
					dyRed = -1;
					dxRed = 0;
				} else {

					redX = oldXRed;
					redY = oldYRed;

					if (dxRed != 0) {

						i = redX;
						j = redY + 1;
						isNotWallOrDoor();
						val2 = val;

						j = redY - 1;
						isNotWallOrDoor();

						dxRed = 0;
						if (val2) {
							dyRed = 1;
						} else if (val) {
							dyRed = -1;
						}
					} else {

						i = redX + 1;
						j = redY;
						isNotWallOrDoor();
						val2 = val;


						i = redX - 1;
						isNotWallOrDoor();

						dyRed = 0;
						if (val2) {
							dxRed = 1;
						} else if (val) {
							dxRed = -1;
						}
					}
				}

			}

			// сеъеи ли PACMAN привидение (или оно нас)
			if (pacmanLooser()) {
				music_play(2);
				return 0;
			}

			oldXRed = redX;
			oldYRed = redY;

		}
	}


	i = redX;
	j = redY;
	if (redFlag) {
		val = RED;
		setValToMap();
	} else {
		val = SHADOW;
		setValToMap();
	}

	return 1;
}


/**
 * Алгоритм обработки движения PACMAN на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int pacManState() {
	// проверяем, у PACMAN задоно ли направление движения
	if (dx != 0 || dy != 0) {
		
		// должен ли PACMAN переместиться на новую клетку
		if (pacmanLastUpdateTime == 0) {
			pacmanX = pacmanX + dx;
			pacmanY = pacmanY + dy;



			// сбрасываем счетчик времени
			pacmanLastUpdateTime = PACMAN_SPEED;

			
			// корректируем координаты PACMAN если надо (чтоб не вышел с поля)
			// если вышел за поле (появление с другой стороны поля)
			if (pacmanX < 0) {
				pacmanX = MAP_SIZE_X - 2;
			} else if (pacmanX > MAP_SIZE_X - 2) {
				pacmanX = 0;
			}

			if (pacmanY < 0) {
				pacmanY = MAP_SIZE_Y - 1;
			} else if (pacmanY > MAP_SIZE_Y - 1) {
				pacmanY = 0;
			}

			// если текущая клетка с едой, увиличиваем счетчик съеденного


			i = pacmanX;
			j = pacmanY;
			getValFromMap();

			if (val == FOOD) {
				incFood();
			} else if (val == POWER_FOOD) {
				// RED становится съедобным
				redFlag = 0;
				// бежит в обратную сторону
				dxRed = -dxRed;
				dyRed = -dyRed;
				
				// RED стал съедобным
				redTime = RED_TIME;

				// и даем еще бонус
				++powerBonus;
				sfx_play(5, 0);

			} else if (val == CHERRY) {
				++cherryBonus;
				sfx_play(3, 0);
			}

			i = pacmanX;
			j = pacmanY;
			isNotWallOrDoor();

			if (val) {
				// TODO стираем то что под нами
				// если в новой клетке не дверь то в старой делаем пустую клетку
				//map[oldY][oldX] = EMPTY;
				i = oldX;
				j = oldY;
				val = EMPTY;
				setValToMap();
				drawBlackBox();
			} else {
				// если в новой клетке стена WALL или дверь DOOR
				// остаемся на прошлой клетке
				pacmanY = oldY;
				pacmanX = oldX;
				// вектор движения сбрасываем (PACMAN останавливается)
				dx = 0;
				dy = 0;
			}

			// рисуем пакмена в координатах текущей клетки карты
			i = pacmanX;
			j = pacmanY;
			val = PACMAN;
			setValToMap();

			// если съеденны все FOOD и POWER_FOOD - PACMAN выиграл
			if (food100 == 2 && food010 == 7 && food001 == 1 && powerBonus == 4) {
				music_play(1);
				calcScore();
				return 0;
			}

			// TODO без возвращаемого значения
			// сеъеи ли PACMAN привидение (или оно нас)			
			if (pacmanLooser()) {
				music_play(2);
				return 0;
			}

			oldX = pacmanX;
			oldY = pacmanY;

		 }

	}
	return 1;
}


/**
 * Алгоритм обработки движения PACGIRL на карте
 * return 0 - Конец игры
 *        1 - PACMAN еще жив
 */
int pacGirlState() {
	// проверяем, у pacGirl задоно ли направление движения
	if (dxPacGirl != 0 || dyPacGirl != 0) {
			
		// если подключился 2 игрок
		if (players == 1) {
			players = 2;
			i = pacGirlX;
			j = pacGirlY;
			getValFromMap();
			if (val == FOOD) {
				incFood();
			}
		}

		if (pacGirlLastUpdateTime == 0) {
			pacGirlX = pacGirlX + dxPacGirl;
			pacGirlY = pacGirlY + dyPacGirl;

			pacGirlLastUpdateTime = PACGIRL_SPEED;

			// если вышел за поле (появление с другой стороны поля)
			if (pacGirlX < 0) {
				pacGirlX = MAP_SIZE_X - 2;
			} else if (pacGirlX > MAP_SIZE_X - 2) {
				pacGirlX = 0;
			}

			if (pacGirlY < 0) {
				pacGirlY = MAP_SIZE_Y - 1;
			} else if (pacGirlY > MAP_SIZE_Y - 1) {
				pacGirlY = 0;
			}


			// если текущая клетка с едой, увиличиваем счетчик съеденного
			i = pacGirlX;
			j = pacGirlY;
			getValFromMap();

			if (val == FOOD) {
				incFood();
			} else if (val == POWER_FOOD) {
				// RED становится съедобным
				redFlag = 0;
				// бежит в обратную сторону
				dxRed = -dxRed;
				dyRed = -dyRed;

				
				// RED стал съедобным
				redTime = RED_TIME;
				
				// и даем еще бонус
				++powerBonus;
				sfx_play(5, 0);
			} else if (val == CHERRY) {
				++cherryBonus;
				sfx_play(3, 0);
			}

			i = pacGirlX;
			j = pacGirlY;
			isNotWallOrDoor();
			if (val) {
				// если в новой клетке не дверь то в старой делаем пустую клетку
				oldPacGirlVal = val;
				i = oldPacGirlX;
				j = oldPacGirlY;
				val = EMPTY;
				setValToMap();
				drawBlackBox();
			} else {
				// если в новой клетке стена WALL или дверь DOOR
				// остаемся на прошлой клетке
				pacGirlY = oldPacGirlY;
				pacGirlX = oldPacGirlX;
				// вектор движения сбрасываем (PACMAN останавливается)
				dxPacGirl = 0;
				dyPacGirl = 0;
			}

			// рисуем PAC-GIRL в координатах текущей клетки карты
			i = pacGirlX;
			j = pacGirlY;
			val = PACGIRL;
			setValToMap();			

			// если съеденны все FOOD и POWER_FOOD - PACMAN выиграл
			if (food100 == 2 && food010 == 7 && food001 == 1 && powerBonus == 4) {
				music_play(1);
				calcScore();
				return 0;
			}

			// сеъеи ли PACMAN привидение (или оно нас)
			if (pacmanLooser()) {
				music_play(2);			
				return 0;
			}


			oldPacGirlX = pacGirlX;
			oldPacGirlY = pacGirlY;
		}
	}

	return 1;
}


/**
 *  NES
 * 
 *  Обработка нажатых кнопок игроком
 *  передвижение персонажей во время игры
 */
void actions(void) {

	// задержка для обработки нажатия кнопок (если нужна)
	if (playersTime > 0) {
		// защита от двойного нажатия,
		// когда playersTime станет равным 0 обработчик заработает  опять
		--playersTime;
	}

	if (STATE_SELECT == gameState) {
		// стартовый экран (выбор количества игроков)
		if (((pad1 & PAD_START) || (pad2 & PAD_START)) && playersTime == 0) {
			// нажат Start на 1 или 2 джойстике
			music_stop();
			//music_play(3);
			if (players == 1) {
				// выбрана игра за 1го (1 PLAYER)

				// игрок 1 убираем с карты PAC-GIRL
				i = pacGirlX;
				j = pacGirlY;
				val = FOOD;
				setValToMap();
			} else {
				// выбрана игра на 2их (2 PLAYERS)

				// надо дать очки за точку
				// которая на месте PAC-GIRL была
				incFood();
			}

			// начинаем иру
			gameState = STATE_GAME;

			// рисуем задний фон с лабиринтом для игры
			drawBackground();

			playersTime = 30;
			return;
		}


		// 1 или 2 играка будут играть - выбор нажатием SELECT
		if (((pad1 & PAD_SELECT) || (pad2 & PAD_SELECT)) && (playersTime == 0)) {
			// нажат Select на 1 или 2 джойстике

			// на 30 циклов делаем задержку обработки этого события 
			// защита от 2го нажатия
			playersTime = 30;

			// меняем выбор
			if (players == 1) {
				players = 2;
			} else {
				players = 1;
			}

			return;
		}

		// 1 или 2 игрока будут играть - выбор стрелочками
		if (((pad1 & PAD_DOWN) || (pad2 & PAD_DOWN)) && (players == 1)) {
			// Нажата кнопка вверх на 1 или 2 джойстике
			players = 2;
			return;
		}


		if (((pad1 & PAD_UP) || (pad2 & PAD_UP)) && (players == 2)) {
			// Нажата кнопка вниз на 1 или 2 джойстике
			players = 1;
			return;
		}

	}


	if (STATE_PAUSE == gameState) {
		// игра на паузе

    	if (((pad1 & PAD_START) || (pad2 & PAD_START)) && playersTime == 0) {
    		gameState = STATE_GAME;
    		playersTime = 30;
    		return;
    	}

    	pauseY = pauseY + pauseDY;
    	pauseX = pauseX + pauseDX;

    	if (pauseY <= 0 || pauseY >= 224) {
    		pauseDY = -pauseDY;
    	}

    	if (pauseX <= 0 || pauseX >= 215) {
    		pauseDX = -pauseDX;
    	}

    	if (pauseTime <=0) {
    		// раз в 30 циклов меняем номер палитры 0 -> 1 -> 2 -> 3 -> 0 и так по кругу

    		pauseTime = 30;
			if (pausePal < 3) {
				++pausePal;
			} else {
				pausePal = 0;
			}

			// меняем палитру для всех тайлов спрайта на значение pausePal
			PAUSE[3]  = pausePal;
			PAUSE[7]  = pausePal;
			PAUSE[11] = pausePal;
			PAUSE[15] = pausePal;
			PAUSE[19] = pausePal;
    	}

    	// счетчик циклов во время паузы идущий к 0
    	pauseTime--;
	}

    if (STATE_GAME == gameState) {    	
		// идет игра

    	if (((pad1 & PAD_START) || (pad2 & PAD_START)) && playersTime == 0) {
    		gameState = STATE_PAUSE;
    		playersTime = 30;
    		return;
    	}

		if (pad1 & PAD_LEFT) {
			// нажата кнопка влеао на 1 джойстике
			dx = -1;
			dy = 0;			
		}

		if (pad1 & PAD_RIGHT) {
			// нажата кнопка вправо на 1 джойстике
			dx = 1;
			dy = 0;

		}

		if (pad1 & PAD_UP) {
			// нажата кнопка вверх на 1 джойстике
			dy = -1;
			dx = 0;
		}

		if (pad1 & PAD_DOWN) {
			// нажата кнопка вниз на 1 джойстике
			dy = 1;
			dx = 0;
		}

		if (pad2 & PAD_UP) {
			// нажата кнопка вверх на 2 джойстике
			dyPacGirl = -1;
			dxPacGirl = 0;
		}

		if (pad2 & PAD_DOWN) {
			// нажата кнопка вниз на 2 джойстике
			dyPacGirl = 1;
			dxPacGirl = 0;
		}

		if (pad2 & PAD_LEFT) {
			// нажата кнопка влево на 2 джойстике
			dxPacGirl = -1;
			dyPacGirl = 0;
		}

		if (pad2 & PAD_RIGHT) {
			// нажата кнопка вправо на 2 джойстике
			dxPacGirl = 1;
			dyPacGirl = 0;
		}


		// двигаем Pac-Man
		if (!pacManState()) {
			// игра окончена
			gameState = STATE_RESULT;
			return;
		}
		
		
		// двигаем RED
		if (!redState()) {
			// игра окончена
			gameState = STATE_RESULT;
			return;
		}
		

		// двигаем Pac-Girl
		if (!pacGirlState()) {
			// игра окончена
			gameState = STATE_RESULT;
			return;
		}

		
		if (pacmanLastUpdateTime > 0 ) {
			// счетчик для анимации Pac-Man
			--pacmanLastUpdateTime;
		}

		if (redLastUpdateTime > 0) {
			// счетчик для анимации RED и SHADOW
			--redLastUpdateTime;
		}

		if (pacGirlLastUpdateTime > 0) {
			// счетчик для анимации Pac-Girl
			--pacGirlLastUpdateTime;
		}

		if (cherryTime > 0) {
			// счетчик когда надо показать черешню
			// и отрыть к ней дверь
			--cherryTime;
		}

		if (redTime > 0) {
			// счетчик когда SHADOW вновь станет RED
			--redTime;
		}
	}


	if (STATE_RESULT == gameState && ((pad1 & PAD_START) || (pad2 & PAD_START))) {
		if (food100 == 2 && food010 == 7 && food001 == 1 && powerBonus == 4) {
			music_play(3);
		}
		// показываем результат игры  и на этом экране
		// нажат Start на 1 или 2 джойстике

		// переходим на стартовый экран выбора игроков
		gameState = STATE_SELECT;

		// защита от 2го нажатия кнопки Start
		playersTime = 30;

		// очистить спрайты из буфера спрайтов
		oam_clear();

		// дождаться начала кадра
		ppu_wait_nmi();

		// нарисовать стартовый экран выбора игроков
		drawBackground();

		// сбросить игру в стартовое состояние
		// начальное положение персонажей, обнулить очки, и т.д.
		init();
	}

	return;
		
}


/**
 * NES
 * 
 * Нарисовать задний фон
 */
void drawBackground(void) {
	// отключить screen
	ppu_off(); 

	// выбираем NAMETABLE A
	vram_adr(NAMETABLE_A);

	
	if (STATE_SELECT == gameState) {
		// стартовый бекграунд
		vram_unrle(n1);
	} else {
		// карта уровня с лабиринтом
		vram_unrle(n2);		
	}
	
	// включить screen
	ppu_on_all();
}


/**
 * NES
 * 
 * Нарисовать спрайты
 */
void drawSprites(void) {
	// очистить спрайты из буфера спрайтов
	oam_clear();

	if (STATE_SELECT == gameState) {
		// если находимся на стартовом экране
		if (players == 1) {
			// если выбрана игра за одного (только Pac-Man)
			// надо нарисовать спрайт PAC-MAN перед 1 PLAYER
			// с полнотью открытым ртом в сторону надписи
			oam_meta_spr(80, 108, PACMAN_R2);
		} else {
			// если выбрана игра на 2х игроков (1 игрок за Pac-Man, 2 игрок за Pac-Girl)
			// нарисовать спрайт PAC-Girl перед 2 PLAYERS
		    // с полнотью открытым ртом в сторону надписи (направо)
			oam_meta_spr(80, 123, PACGIRL_R2);			

			// нарисовать спрайт PAC-MAN после 2 PLAYERS
			// с полнотью открытым ртом в сторону надписи (влево)
			oam_meta_spr(167, 123, PACMAN_L2);
		}
	}
	
	if (STATE_GAME == gameState || STATE_RESULT == gameState) {
		// если идет игра или показываем результаты игры
		// отрисовываем спрайты игры 
		// PAC-MAN, Pac-Girl, RED или SHADOW, дверь, черешню
		refreshGame();    	
	}

	if (STATE_PAUSE == gameState) {
		// игра на паузе

		oam_meta_spr(pauseX, pauseY, PAUSE);
	}
}


/**
 * NES
 * 
 *  Перерисовать на бекграунде tile когда съели точку
 *  рисуем черный квадрат 8x8
 */
void drawBlackBox(void) {
	// x = i * 8 и смещение на 1 tile вправо
	x = (i << 3) + 8;

	// y = j * 8
    y = ((j + 1) << 3);

	address = get_ppu_addr(0, x, y);

	// tile = 0 - черный квадрат 8x8 пикселов на фоне
	one_vram_buffer(0, address); 
}


/**
 *  NES
 * 
 *  Нарисовать бонусы, очки
 *  результат игры (GAME OVER или YOU WINNER)
 */
void drawText(void) {
	if (STATE_GAME == gameState || STATE_RESULT == gameState) {
		// идет игра или отображаем результат игры

		// количество съеденых черешень
		text = cherryBonus + '0';
		one_vram_buffer(text, NTADR_A(7,27));

		// количество съеденных призраков
		text = redBonus + '0';
		one_vram_buffer(text, NTADR_A(7,25));

        // количество съеденных поверапов
		text = powerBonus + '0';
		one_vram_buffer(text, NTADR_A(24,27));
		
		// количество съеденных серых точек (еды)
		text = food100 + '0';
		one_vram_buffer(text, NTADR_A(24,25));
		
		text = food010 + '0';
		one_vram_buffer(text, NTADR_A(25,25));
		
		text = food001 + '0';
		one_vram_buffer(text, NTADR_A(26,25));
	}

	if (STATE_RESULT == gameState) {
		// отображаем результат игры
		if (food100 == 2 && food010 == 7 && food001 == 1 && powerBonus == 4) {
			// если победили
			// пишем YOU WINNER
			one_vram_buffer('Y', NTADR_A(11,25));
			one_vram_buffer('O', NTADR_A(12,25));	
			one_vram_buffer('U', NTADR_A(13,25));	
			one_vram_buffer('W', NTADR_A(15,25));	
			one_vram_buffer('I', NTADR_A(16,25));	
			one_vram_buffer('N', NTADR_A(17,25));	
			one_vram_buffer('N', NTADR_A(18,25));				
			one_vram_buffer('E', NTADR_A(19,25));	
			one_vram_buffer('R', NTADR_A(20,25));	
		} else {
			// если проиграли
			// пишем GAME OVER
			one_vram_buffer('G', NTADR_A(11,25));
			one_vram_buffer('A', NTADR_A(12,25));	
			one_vram_buffer('M', NTADR_A(13,25));	
			one_vram_buffer('E', NTADR_A(14,25));	
			one_vram_buffer('O', NTADR_A(16,25));	
			one_vram_buffer('V', NTADR_A(17,25));	
			one_vram_buffer('E', NTADR_A(18,25));				
			one_vram_buffer('R', NTADR_A(19,25));	
		}

		// пишем SCORE
		one_vram_buffer('S', NTADR_A(11,27));
		one_vram_buffer('C', NTADR_A(12,27));	
		one_vram_buffer('O', NTADR_A(13,27));	
		one_vram_buffer('R', NTADR_A(14,27));	
		one_vram_buffer('E', NTADR_A(15,27));	

		// отображаем количество полученных очков
		// с учетом всех бонусов
		// черешня 200 очков
		// съеденный призрак 50 очков
		// поверап 25 очков
		// серая точка (еда) 1 очко
		text = score100 + '0';
		one_vram_buffer(text, NTADR_A(17,27));
		
		text = score010 + '0';
		one_vram_buffer(text, NTADR_A(18,27));
		
		text = score001 + '0';
		one_vram_buffer(text, NTADR_A(19,27));
	}

	// игра на паузе
  	if (STATE_PAUSE == gameState) {          // двоичное    = 16   = 10 ричное
		one_vram_buffer(0b11100100, 0x23c0); // 11 10 01 00 = 0xe4 = 228
											 //  3  2  1  0 - палитра

											 // в NESst и на TV так:
											 // 00 01       0  1
		                                     // 10 11  или  2  3


		// получить адрес в ATTRIBUTE TABLE
		// для NT = 0 (NAME TABLE)
		// в координате x = 255 y = 0 (разрешение экрана 256×240 для Dendy)
		//                    NT   x  y
		address = get_at_addr(0, 255, 0);


		// замменить байт в vram ppu по адресу address
		// на значение 0xe4				     // двоичное    = 16   = 10 ричное
		one_vram_buffer(0xe4, address);      // 11 10 01 00 = 0xe4 = 228
		                                     //  3  2  1  0 - палитра

		switch(pausePal) {
			case  1:
				pauseMTbyte = 0b01010101;    // 01 01 01 01 = 0x55 = 170
				break;                       //  1  1  1  1 - палитра
			case  2:
				pauseMTbyte = 0xaa;		     // 10 10 10 10 = 0xAA = 252
				break;                       //  2  2  2  2 - палитра
			case 3:
				pauseMTbyte = 255;           // 11 11 11 11 = 0xFF = 255
				break;                       //  3  3  3  3 - палитра
			default:
				pauseMTbyte = 0;		     // 00 00 00 00 = 0x0 = 0
		}

        //                    NT    row to x     col to  y
		address = get_at_addr(0, (24 << 3) + 8, ((25 + 1) << 3));
		one_vram_buffer(pauseMTbyte, address);

		//                    NT    row to x     col to  y
		address = get_at_addr(0, ROW_TO_X(28), COL_TO_Y(25));
		one_vram_buffer(pauseMTbyte, address);
	}
}


/**
 * NES
 *
 * Нарисовать только 1 объект с карты
 * i - строка в массиве карты
 * j - столбец в массиве карты
 */
void draw(void) {
	getValFromMap();
	 // x = i * 8 и на 4 пиксела вправо
    x = (i << 3) + 4;

    // y = j * 8 и на 3 пиксела вверх
    y = ((j + 1) << 3) - 3;

    if ( val == PACMAN ) {
        if (pacmanSprite == 1) {
        	if (pacmanLastUpdateTime == 0) {
            	pacmanSprite = 2;
            }

            if (dx < 0) {
            	oam_meta_spr(x, y, PACMAN_L1);                
            } else if (dx > 0) {
            	oam_meta_spr(x, y, PACMAN_R1);
            } else if (dy < 0) {
            	oam_meta_spr(x, y, PACMAN_UP1);
            } else if (dy > 0) {
            	oam_meta_spr(x, y, PACMAN_D1);
            } else {
            	oam_meta_spr(x, y, PACMAN_0);                
            }
        } else if (pacmanSprite == 2) {
        	if (pacmanLastUpdateTime == 0) {
            	pacmanSprite = 3;
            }
            if (dx < 0) {
            	oam_meta_spr(x, y, PACMAN_L2);                
            } else if (dx > 0) {
            	oam_meta_spr(x, y, PACMAN_R2);
            } else if (dy < 0) {
            	oam_meta_spr(x, y, PACMAN_UP2);
            } else if (dy > 0) {
            	oam_meta_spr(x, y, PACMAN_D2);
            } else {
            	oam_meta_spr(x, y, PACMAN_0);
            }
        } else if (pacmanSprite == 3) {
        	if (pacmanLastUpdateTime == 0) {
            	pacmanSprite = 1;
            }
            oam_meta_spr(x, y, PACMAN_0);
        }
    } else if (val == RED) {
        if (redSprite == 1) {
        	if (redLastUpdateTime == 0) {
            	redSprite = 2;
            }

            if (dxRed < 0) {
            	oam_meta_spr(x, y, RED_L1);
            } else if (dxRed > 0) {
            	oam_meta_spr(x, y, RED_R1);
            } else if (dyRed > 0) {
            	oam_meta_spr(x, y, RED_D1);
            } else {
            	oam_meta_spr(x, y, RED_UP1);
            }
        } else {
        	if (redLastUpdateTime == 0) {
            	redSprite = 1;
            }

            if (dxRed < 0) {
            	oam_meta_spr(x, y, RED_L2);
            } else if (dxRed > 0) {
            	oam_meta_spr(x, y, RED_R2);
            } else if (dyRed > 0) {
            	oam_meta_spr(x, y, RED_D2);
            } else {
            	oam_meta_spr(x, y, RED_UP2);
            }
        }
    } else if (val == PACGIRL) {
        if (pacGirlSprite == 1) {
        	if (pacGirlLastUpdateTime == 0) {
        		pacGirlSprite = 2;
        	}

            if (dxPacGirl < 0) {
            	oam_meta_spr(x, y, PACGIRL_L1);
            } else if (dxPacGirl > 0) {
            	oam_meta_spr(x, y, PACGIRL_R1);
            } else if (dyPacGirl < 0) {
            	oam_meta_spr(x, y, PACGIRL_UP1);
            } else if (dyPacGirl > 0) {
            	oam_meta_spr(x, y, PACGIRL_D1);
            } else {
            	oam_meta_spr(x, y, PACGIRL_0);
            }
        } else if (pacGirlSprite == 2) {
        	if (pacGirlLastUpdateTime == 0) {
        		pacGirlSprite = 3;
        	}

            if (dxPacGirl < 0) {
            	oam_meta_spr(x, y, PACGIRL_L2);
            } else if (dxPacGirl > 0) {
            	oam_meta_spr(x, y, PACGIRL_R2);
            } else if (dyPacGirl < 0) {
            	oam_meta_spr(x, y, PACGIRL_UP2);
            } else if (dyPacGirl > 0) {
            	oam_meta_spr(x, y, PACGIRL_D2);
            } else {
            	oam_meta_spr(x, y, PACGIRL_0);
            }
        } else if (pacGirlSprite == 3) {
        	if (pacGirlLastUpdateTime == 0) {
        		pacGirlSprite = 1;
        	}

			if (dxPacGirl != 0) {
				oam_meta_spr(x, y, PACGIRL_1);
			} else {
				oam_meta_spr(x, y, PACGIRL_0);
			}
        }
    } else if (val == SHADOW) {
        if (redSprite == 1) {
        	if (redLastUpdateTime == 0) {
            	redSprite = 2;
            	sfx_play(6, 0);
            }
            oam_meta_spr(x, y, SPIRIT1);
        } else {
        	if (redLastUpdateTime == 0) {
            	redSprite = 1;
            	sfx_play(8, 0);
            }
            oam_meta_spr(x, y, SPIRIT2);  
        }
    } else if (val == CHERRY) {
    	oam_meta_spr(x, y, CHERRY_SPR);
    } else if (val == DOOR) {
    	oam_meta_spr(x, y + 8, DOOR_SPR);
    }
}



/**
 * NES
 * 
 * Обновить карту / персонажей, двери, черешню
 */
void refreshGame(void) {
	if (!cherryFlag && redTime == 0 && !cherryBonus && cherryTime == 0) {
		// открыть двери
		openDoors();
	}

    if (refreshDoor) {    	    	
    	i = doorX;
		j = doorY;	
        getValFromMap();
		if (val != DOOR) {
			refreshDoor = 0;
		} else {
			// рисуем дверь
        	draw();
		}
    }
	
    if (refreshCherry) {	 
    	i = cherryX;
		j = cherryY;   
        getValFromMap();
		if (val != CHERRY) {
			refreshCherry = 0;
		} else {
		    // рисуем черешню
	    	draw();
		}
    }

    // рисуем призрака RED 
    i = redX;
  	j = redY;    	
   	draw();   


    // рисуем Pac-Man
	i = pacmanX;
    j = pacmanY;    	
	draw();
           
      
    // рисуем Pac-Girl
   	i = pacGirlX;
	j = pacGirlY;		
	draw();

}


/**
 * NES
 * 
 * Точка входа в программу
 */
void main (void) {
   // seed для функции случайных чисел
	set_rand(66);
	
	// выключить screen 
	ppu_off(); 
	
	// загрузить палитру для заднего фона
	pal_bg(paletteForBackground);

	// загрузить палитру для спрайтов	
	pal_spr(paletteForSprites);
	
	// используем второй набор тайлов для спрайтов (NAMETABLE B)
	bank_spr(1);
	
	// сдвинуть фон вниз на 1 пиксель
	set_scroll_y(0xff); 
	
	// нарисовать задний фон
	drawBackground();
	
	// проиграть музыку
	music_play(0);
	
	// устанавливает указатель на буфер
	set_vram_buffer(); 

	// бесконечный цикл
	while (1) {
		// дождаться начала кадра
		ppu_wait_nmi();
		
		// считываем что нажато на 1 контроллере (джойстике)
		pad1 = pad_poll(0);
		
		// считываем что нажато на 2 контроллере (джойстике)
		pad2 = pad_poll(1); 

		// нарисовать бонусы, очки или результат игры
		drawText();
				
		// обработать действия игроков (нажатие кнопок контроллеров)
		// подвинуть персонажи в зависимости от того что нажато на крате (map1, map2, map3)
		actions();					

		// нарисовать спрайты согласно расположению на карте (map1, map2, map3)
		drawSprites();
	}
}
