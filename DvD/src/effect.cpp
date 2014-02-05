#include "effect.h"

#include <algorithm>
#include <iostream>

#include "error.h"
#include "graphics.h"
#include "util.h"

#define EFFECT_MAX 64

namespace effect {
    //VARIABLES
    int nEffectAnims = 0;
    EffectAnimation* effectAnims = NULL;

    Effect effects[EFFECT_MAX];

    //EFFECT ANIMATION
    EffectAnimation::EffectAnimation() {
        name = "";
        nFrames = 0;
        frames = NULL;
    }

    void EffectAnimation::create(std::string name) {
        this->name = name;

        //Get the list of files
        std::vector<std::string> files = util::listDirectory("effects/" + name, true);

        nFrames = 0;
        frames = NULL;

        if(files.empty()) {
            return;
        }

        //See if all of our images exist first, and count them.
        //Thumbs.db, .DS_Store, etc could screw the list up.
        for(int i = 0, last = -1; i < files.size(); i++) {
            if(std::find(files.begin(), files.end(), util::toString(i+1) + ".png") != files.end()) {
                if(last != i - 1) {
                    error("Missing frames in effect animation \"" + name + "\".");
                    return;
                }
                nFrames++;
                last++;
            }
        }

        if(!nFrames) {
            return;
        }

        //Allocate array and try to populate
        frames = new Image[nFrames];

        for(int i = 0; i < nFrames; i++) {
            frames[i].createFromFile("effects/" + name + "/" + util::toString(i+1) + ".png");
        }
    }

    EffectAnimation::~EffectAnimation() {
        delete [] frames;
    }

    std::string EffectAnimation::getName() {
        return name;
    }

    Image* EffectAnimation::getFrame(int frame) {
        if(nFrames == 0)
            return NULL;

        return &frames[frame % nFrames];
    }

    int EffectAnimation::getNumFrames() {
        return nFrames;
    }

    //EFFECT
    Effect::Effect() {
        speed = frameStart = frameEnd = x = y = 0;
    }

    Effect::Effect(std::string name, int x, int y, bool moveWithCamera, bool mirror, int speed, int nLoops) {
        anim = NULL;

        //Look up the animation
        int i = 0;
        for(; i < nEffectAnims; i++) {
            if(name == effectAnims[i].getName()) {
                anim = effectAnims + i;
                break;
            }
        }

        //We failed to find the animation
        if(i == nEffectAnims) {
            error("Could not find the effect animation \"" + name + "\".");
            this->x = this->y = this->frameEnd = this->frameStart = 0;
        } else {
            //Create object
            this->x = x;
            this->y = y;
            this->moveWithCamera = moveWithCamera;
            this->mirror = mirror;
            this->speed = speed;

            //Calculate the start and end frames
            this->frameStart = os::frame;
            this->frameEnd = this->frameStart + (unsigned int)(anim->getNumFrames() * nLoops);
        }
    }

    Effect::~Effect() {
    }

    unsigned int Effect::getCreationFrame() {
        return frameStart;
    }

    bool Effect::exists() {
        return frameEnd > os::frame;
    }

    void Effect::draw() {
        if(!exists())
            return;

        Image* frame = anim->getFrame((os::frame - frameStart) / speed);

        graphics::setRender(RENDER_ADDITIVE);
        if(moveWithCamera)
            frame->drawSprite(x - frame->w / 2, y - frame->h / 2, mirror);
        else
            frame->draw(x - frame->w / 2, y - frame->h / 2, mirror);
    }

    //MISC FUNCS
    void init() {
        //See what's in the effects directory
        std::vector<std::string> dirs = util::listDirectory("effects", false);

        nEffectAnims = dirs.size();
        if(nEffectAnims <= 0)
            return;

        effectAnims = new EffectAnimation[nEffectAnims];
        for(int i = 0; i < nEffectAnims; i++) {
            effectAnims[i].create(dirs[i]);
        }
    }

    void deinit() {
        delete [] effectAnims;
    }

    void newEffect(std::string name, int x, int y, bool moveWithCamera, bool mirror, int speed, int nLoops) {
        for(int i = 0; i < EFFECT_MAX; i++) {
            if(!effects[i].exists()) {
                effects[i] = Effect(name, x, y, moveWithCamera, mirror, speed, nLoops);
                break;
            }
        }
    }

    void draw() {
        //Be sure to draw all the effects in the correct order
        bool drawn[EFFECT_MAX] = {false};

        for(;;) {
            int iEarliest = -1;
            unsigned int earliest = UINT_MAX;
            for(int i = 0; i < EFFECT_MAX; i++) {
                if(!drawn[i] && effects[i].exists() && effects[i].getCreationFrame() < earliest) {
                    iEarliest = i;
                    earliest = effects[i].getCreationFrame();
                }
            }
            if(iEarliest == -1)
                break;
            drawn[iEarliest] = true;

            effects[iEarliest].draw();
        }
    }
}