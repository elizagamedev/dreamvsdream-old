#include "globals.h"
#include "player.h"
#include "fighter.h"
#include "graphics.h"
#include "error.h"
#include "menu.h"
#include "parser.h"
#include "stage.h"
#include "network.h"
#include "atlas.h"
#include "effect.h"
#include "animation.h"

game::Player madotsuki;
game::Player poniko;
util::Vector cameraPos(0,0);
util::Vector idealCameraPos(0,0);
util::Vector cameraShake(0,0);

extern game::Fighter fighters[];
extern Stage stages[];

int framePauseTimer = 0;
void pause(int frames) {
	framePauseTimer += frames;
}

int frameShakeTimer = 0;
void shake(int frames) {
	frameShakeTimer += frames;
}

extern void parseArgs(int, char**);

static inline void mainLoop() {
	os::refresh();

	if(menu == MENU_FIGHT) {
		if(frameShakeTimer) {
			cameraShake.x = (util::roll(frameShakeTimer * 2)) - frameShakeTimer;
			cameraShake.y = (util::roll(frameShakeTimer * 2)) - frameShakeTimer;
			frameShakeTimer--;
		} else {
			cameraShake.x = cameraShake.y = 0;
		}

		idealCameraPos.x = (madotsuki.pos.x + poniko.pos.x) / 2;
		idealCameraPos.y = (madotsuki.pos.y + poniko.pos.y) / 3 - 30;

		if(idealCameraPos.y < 0) {
			idealCameraPos.y = 0;
		}
		if(idealCameraPos.y > STAGE.heightAbs - WINDOW_HEIGHT) {
			idealCameraPos.y = STAGE.heightAbs - WINDOW_HEIGHT;
		}

		if(idealCameraPos.x < STAGE.widthAbs / -2 + WINDOW_WIDTH / 2) {
			idealCameraPos.x = STAGE.widthAbs / -2 + WINDOW_WIDTH / 2;
		} else if(idealCameraPos.x > STAGE.widthAbs / 2 - WINDOW_WIDTH / 2) {
			idealCameraPos.x = STAGE.widthAbs / 2 - WINDOW_WIDTH / 2;
		}

		cameraPos.x = (cameraPos.x * 0.8 + idealCameraPos.x * 0.2);
		cameraPos.y = (cameraPos.y * 0.8 + idealCameraPos.y * 0.2);

		cameraPos.x += cameraShake.x;
		cameraPos.y += cameraShake.y;

		if(madotsuki.pos.x < poniko.pos.x) {
			//if(madotsuki.inStandardState(STATE_STAND))
			if(madotsuki.isIdle()) {
				madotsuki.setDir(RIGHT);
			}
			//if(poniko.inStandardState(STATE_STAND))
			if(poniko.isIdle()) {
				poniko.setDir(LEFT);
			}
		} else if(madotsuki.pos.x > poniko.pos.x) {
			//if(madotsuki.inStandardState(STATE_STAND))
			if(madotsuki.isIdle()) {
				madotsuki.setDir(LEFT);
			}
			//if(poniko.inStandardState(STATE_STAND))
			if(poniko.isIdle()) {
				poniko.setDir(RIGHT);
			}
		}

		STAGE.think();

		madotsuki.think();
		poniko.think();

		if(!framePauseTimer) {

			madotsuki.interact(&poniko);
			poniko.interact(&madotsuki);

			for(int i = 0; i < MAX_PROJECTILES; i++) {
				if(madotsuki.projectiles[i].state != STATE_NONE) {
					madotsuki.projectiles[i].interact(&poniko);
				}
				if(poniko.projectiles[i].state != STATE_NONE) {
					poniko.projectiles[i].interact(&madotsuki);
				}
			}
		} else {
			framePauseTimer--;
		}

		//DRAW

		STAGE.draw(false);

		madotsuki.drawSpecial();
		poniko.drawSpecial();

		if(stage != 3) {
			madotsuki.draw(true);
			poniko.draw(true);
		}

		//Which order do we draw these in?
		if(madotsuki.drawPriorityFrame < poniko.drawPriorityFrame) {
			madotsuki.draw(false);
			poniko.draw(false);
		} else {
			poniko.draw(false);
			madotsuki.draw(false);
		}

		//Draw projectiles
		for(int i = 0; i < MAX_PROJECTILES; i++) {
			if(madotsuki.projectiles[i].state != STATE_NONE) {
				madotsuki.projectiles[i].draw();
			}
			if(poniko.projectiles[i].state != STATE_NONE) {
				poniko.projectiles[i].draw();
			}
		}

		STAGE.draw(true);

		effect::draw();

		MENU->think();
		MENU->draw();
		Menu::drawFade();

		((MenuSelect*)menus[MENU_SELECT])->drawEffect(0, madotsuki.fighter->group, madotsuki.pos.x, madotsuki.pos.y + madotsuki.fighter->height, true);
	} else {
		MENU->think();
		MENU->draw();
		Menu::drawFade();
	}
}

#ifdef _WIN32
int main(int foo, char** bar)
#else
int main(int argc, char** argv)
#endif
{
#ifdef _WIN32
	int argc;
	wchar_t** argv16 = CommandLineToArgvW(GetCommandLineW(), &argc);
	char** argv = (char**)malloc(argc * sizeof(char*));
	for(int i = 0; i < argc; i++) {
		argv[i] = util::utf16to8(argv16[i]);
	}
	LocalFree(argv16);
#endif

	parseArgs(argc, (char**)argv);

#ifdef _WIN32
	for(int i = 0; i < argc; i++) {
		free(argv[i]);
	}
	free(argv);
#endif
	extern void init();
	init();

	madotsuki.playerNum = 0;
	madotsuki.speaker.init();

	poniko.playerNum = 1;
	poniko.speaker.init();

	for(;;) {
		mainLoop();
	}

	return 0;
}
