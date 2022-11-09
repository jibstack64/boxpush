// a simple 2d boxpush game in c++!
// https://github.com/jibstack64/boxpush

#include <iostream>
#include <caca_conio.h>
#include <vector>
#include <random>
#include <cmath>

#include "include/pretty.hpp"

// drawing characters
// chars in strings are past U+FFFF therefore require bigger containers
// has no difference in (game) performance since done at runtime
#ifdef __unix__
const std::string BOX = pty::paint("▩", "orange");
const std::string GBX = pty::paint("✔", "green");
const std::string BGD = pty::paint("□", "grey");
const std::string PLR = pty::paint("☻", "yellow");
const std::string TRG = pty::paint("X", "red");
const std::string WLL = pty::paint("#", "bold"); // automatically added when drawing
const std::string ERR = pty::paint("?", "lightred");
#else
const std::string BOX = pty::paint("[]", "orange");
const std::string GBX = pty::paint("**", "green");
const std::string BGD = pty::paint("[]", "grey");
const std::string PLR = pty::paint(":)", "yellow");
const std::string TRG = pty::paint("xx", "red");
const std::string WLL = pty::paint("##", "bold"); // automatically added when drawing
const std::string ERR = pty::paint("??", "lightred");
#endif

enum class drawing {
    FullBox, CheckMark, EmptyBox, Smiley, Cross, Hashtag, Error
};

// returns the string matching d.
const std::string drawToString(drawing d) {
    switch (d) {
        case drawing::FullBox:
        return BOX;
        case drawing::CheckMark:
        return GBX;
        case drawing::EmptyBox:
        return BGD;
        case drawing::Smiley:
        return PLR;
        case drawing::Cross:
        return TRG;
        case drawing::Hashtag:
        return WLL;
        case drawing::Error:
        default:
        return ERR;
    }
}

// returns a string containing newline x times, essentially clearing the console.
const std::string clearConsole(int x = 50) {
    std::string clr = "";
    for (int i = 0; i < x; i++) {
        clr += "\n";
    }
    return clr;
}

// the class used by all objects in the game, such as the players, boxes and targets.
// hosts basic data such as the drawing character and the (x, y) values.
struct object {
    public:
    std::string dchar;
    int x, y;
    // does this object stop the player from moving through?
    bool obstructs = false;
    // is this object a capture box?
    bool captureBox = false;
    // is this object a capture point?
    bool capturePoint = false;
    // should this object be in the background?
    bool background = false;
    // should this object render?
    bool render = true;

    // sets dchar to the character matching dc.
    void setDrawing(drawing dc) {
        dchar = drawToString(dc);
    }

    // swaps with the contents of obj.
    void swap(object* obj) {
        std::string dc = dchar;
        bool obs = obstructs;
        bool cptBox = captureBox;
        bool cptPoint = capturePoint;
        bool bgrd = background;
        bool rend = render;
        // copy dchar
        dchar = obj->dchar;
        obj->dchar = dc;
        // copy obstructs
        obstructs = obj->obstructs; 
        obj->obstructs = obs;
        // copy capturebox
        captureBox = obj->captureBox;
        obj->captureBox = cptBox;
        // copy capturepoint
        capturePoint = obj->capturePoint;
        obj->capturePoint = cptPoint;
        // copy background
        background = obj->background;
        obj->background = bgrd;
        // copy renders
        render = obj->render;
        obj->render = rend;
    }

    explicit object(int sX = 0, int sY = 0, drawing dc = drawing::Error) :
            x(sX), y(sY), dchar(drawToString(dc)) {}
};

struct map {
    private:
    std::vector<object> _originalObjects;

    public:
    const int width, height;
    std::vector<object> objects;
    int player; // the index that the player object is at in objects
    int score = 0; // score for the level
    int totalScore;

    // resets the map to its starting state.
    void reset() {
        objects = _originalObjects;
        score = 0;
    }

    // draws the map  to a string. a border, represented by the drawing::Hashtag character, is automically placed in around the map.
    const std::string draw() {
        std::string end; // the string to be returned

        // add first (top) wall
        for (int i = 0; i < width+2; i++) {
            end += WLL + "  ";
            if (i == width+1) {
                end += "\n";
            }
        }
        for (int y = height; y > 0; y--) {
            // add walls
            end += WLL + "  ";
            for (int x = 0; x < width; x++) {
                // attempt to find an object at these coordinates
                object* o = find(x, y);
                if (o != nullptr) {
                    // if there is an object
                    if (o->render) {
                        end += o->dchar;
                    } else {
                        end += BGD;
                    }
                } else {
                    end += BGD;
                }
                end += "  ";
                // deallocate pointer!!
                // delete o;
            }
            end += WLL + "\n";
        }
        // add second (bottom) wall
        for (int i = 0; i < width+2; i++) {
            end += WLL + "  ";
            if (i == width+1) {
                end += "\n";
            }
        }
    
        // return drawn string
        return end;
    }

    // increments obj's x and y by the values provided. returns true if this was successful, false otherwise.
    // if false is returned, obj is against a wall, or trying to push a box that has already been captured.
    bool move(object* obj, int x = 0, int y = 0) {
        // check if the new coordinates are out of bounds
        int nX = obj->x + x;
        int nY = obj->y + y;
        if (nY < 1 || nY > height || nX >= width || nX < 0) {
            return false;
        }

        // get the object in front of the object
        object* adjacentObject = find(obj->x+x, obj->y+y);

        // if the object exists, try to move it
        if (adjacentObject != nullptr) {
            // if it does not like to be moved, return false
            if (adjacentObject->obstructs) {
                return false;
            // if it is a background object, ignore
            } else if (adjacentObject->background) {
                goto end;
            // if the adjacent object is a capture point
            } else if (adjacentObject->capturePoint) {
                // capture that point!
                if (obj->captureBox) {
                    score++;
                    obj->obstructs = true;
                    obj->setDrawing(drawing::CheckMark); // change to differentiate
                    remove(adjacentObject); // get rid of the checkpoint
                }
            // otherwise, try to move the object
            } else {
                bool success = move(adjacentObject, x, y);
                // if the object could not be moved, don't move with it!
                if (!success) {
                    return false;
                }
            }
        }

        // if all goes swell, increment the obj's coordinates
        end:
        obj->x += x;
        obj->y += y;
        return true;
    }

    // finds and returns a pointer to the object with the matching x and y values.
    // if the object cannot be found, a nullptr is returned.
    object* find(int x, int y) {
        for (auto& obj : objects) {
            if (obj.x == x && obj.y == y) {
                return &obj;
            }
        }
        return nullptr;
    }

    // moves the object to the back of the stack, but always above the score flags.
    void push(object* obj) {
        int firstBox = -1;
        int moveObj;
        for (int i = 0; i < objects.size(); i++) {
            if (objects[i].captureBox && firstBox == -1) {
                firstBox = i;
            }
            if (&(objects[i]) == obj) {
                moveObj = i;
            }
        }
        if (firstBox == moveObj) {
            return;
        } else {
            // swap values
            objects[firstBox].swap(obj);
        }
    }

    // finds the index of and removes the provided object.
    // not actually "removing" the object, just setting its x and ys out of proper map range.
    bool remove(object* obj) {
        for (int i = 0; i < objects.size(); i++) {
            if (&(objects[i]) == obj) {
                // move the object out of bounds forcefully
                objects[i].render = false;
                objects[i].x = width*4;
                objects[i].y = height*4;
                return true;
            }
        }
        return false;
    }

    // generates an instance of an object, assigning x and y values where they are available and not used by another object.
    object autoObject(drawing dc) {
        // generate x and y values
        int x, y;
        x = rand() % width;
        y = rand() % height;
        
        // if x or y are out of bounds
        if (x < 1 || x >= width-1 || y > height-1 || y < 2) {
            return autoObject(dc);
        }

        // check if the x and y values are taken
        // THIS CAN CAUSE A RECURSION LOOP IF THE MAP IS NOT LARGE ENOUGH!
        for (const auto& obj : objects) {
            if (obj.x == x && obj.y == y) {
                return autoObject(dc);
            }
        }

        // allocate and return game object
        return object(x, y, dc);
    }

    map(int w = 10, int h = 10) : width(w), height(h) {
        // generate the player
        objects.push_back(autoObject(drawing::Smiley));
        player = 0;

        // detect the number of boxes and targets to be generated given the w and h
        totalScore = (w*h)/10;
        if (totalScore == 0) {
            totalScore = w; // last resort, anyone making a fork / pull won't be this stupid...
        }
        // auto-generate boxes and targets
        for (int i = 1; i < totalScore+1; i++) {
            objects.push_back(autoObject(drawing::Cross)); // target
            objects[i].capturePoint = true;
            std::cout << i << std::endl;
        }
        for (int i = totalScore+1; i < (totalScore*2)+1; i++) {
            objects.push_back(autoObject(drawing::FullBox)); // box
            objects[i].captureBox = true;
            std::cout << i << std::endl;
        }

        // original objects in case of reset()
        _originalObjects = objects;
    }
};

// contains all of the game data.
struct boxpush {
    std::vector<map> maps;
    int mapIndex = 0;

    // returns a reference to the map at the current map index.
    map& currentMap() {
        return maps[mapIndex];
    }

    boxpush(std::initializer_list<map> ms) : maps(ms) { srand(time(0)); }
};

int main() {
    // initialise the main game object and the maps.
    boxpush game(
        {
            map(10, 10), map(12, 8), map(10, 8),
            map(10, 12), map(12, 12), map(8, 8),
            map(10, 10), map(12, 9), map(11, 11),
            map(11, 12)
        }
    );

    // start game mainloop
    while (true) {
        // get current map
        map& cm = game.currentMap();

        // get the player object
        object& player = cm.objects[cm.player];

        // draw to terminal
        const std::string drn = cm.draw();
        std::cout << clearConsole(cm.height);
        std::cout << pty::paint("> Score : ", {"grey", "bold"}) << pty::paint(cm.score, cm.score == 0 ? "red" : "green");
        std::cout << pty::paint(" | Level : ", {"grey", "bold"}) << pty::paint(std::to_string(game.mapIndex + 1) + " / " + std::to_string(game.maps.size()), "orange");
        std::cout << pty::paint(" | Player co-ordinates : ", {"grey", "bold"}) << player.x << " , " << player.y << std::endl;
        std::cout << drn;

        // await user input
        retake: // for retaking key input
        std::cout << pty::paint("\nWhich way do you wish to move?", "bold") << pty::paint(" (w⬆,a⬅,s⬇,d➡,r⏪)", "grey") << ": ";
        char moveKey;
        std::cin >> moveKey;

        // move the player
        switch (moveKey) {
            case 'w':
            cm.move(&player, 0, 1); break;
            case 'a':
            cm.move(&player, -1, 0); break;
            case 's':
            cm.move(&player, 0, -1); break;
            case 'd':
            cm.move(&player, 1, 0); break;
            case 'r':
            cm.reset(); break; // reset the map
            default:
            goto retake;
        }

        // if the player has won, go to next level!
        if (cm.score == cm.totalScore) {
            if (game.mapIndex == game.maps.size()-1) {
                return 0;
            } else {
                game.mapIndex++;
            }
        }
    }

    return 0;
}