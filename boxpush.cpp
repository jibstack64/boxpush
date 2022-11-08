// a simple 2d boxpush game in c++!
// https://github.com/jibstack64/boxpush

#include <iostream>
#include <vector>
#include <random>
#include <cmath>

#include "include/pretty.hpp"

// drawing characters
// chars in strings are past U+FFFF therefore require bigger containers
// has no difference in (game) performance since done at runtime
const std::string BOX = pty::paint("▩", "green");
const std::string BGD = pty::paint("□", "grey");
const std::string PLR = pty::paint("☻", "yellow");
const std::string TRG = pty::paint("X", "red");
const std::string WLL = pty::paint("#", "bold"); // automatically added when drawing
const std::string ERR = pty::paint("?", "lightred");

enum class drawing {
    FullBox, EmptyBox, Smiley, Cross, Hashtag, Error
};

// returns the string matching d.
const std::string drawToString(drawing d) {
    switch (d) {
        case drawing::FullBox:
        return BOX;
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
    const std::string dchar;
    int x, y;
    // does this object stop the player from moving through?
    bool obstructs = false;
    // is this object a capture box?
    bool captureBox = false;
    // is this object a capture point?
    bool capturePoint = false;
    // should this object render?
    bool render = true;

    explicit object(int sX = 0, int sY = 0, drawing dc = drawing::Error) :
            x(sX), y(sY), dchar(drawToString(dc)) {}
};

struct map {
    const int width, height;
    std::vector<object> objects;
    int player; // the index that the player object is at in objects
    int score = 0; // score for the level

    // draws the map to a string. a border, represented by the drawing::Hashtag character, is automically placed in around the map.
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
        // get incremented coordinates and object at them
        int nextX = obj->x + x;
        int nextY = obj->y + y;
        object* nextObj = find(nextX, nextY);

        // check if against going over wall, etc
        if (nextX < 0 || nextX >= width || nextY > height || nextY < 1) {
            return false;
        } else if (nextObj != nullptr) {
            // if it is obstructed
            if (nextObj->obstructs) {
                return false;
            // if it does not
            } else if (nextObj->captureBox) {
                // discover what is in front of the nextObj
                object* nextNextObj = find(nextX+x, nextY+y);
                if (nextNextObj != nullptr) {
                    if (nextNextObj->capturePoint) {
                        nextObj->obstructs = true; // nextObj now obstructs
                        nextNextObj->render = false; // stop nextNextObj from rendering
                        score++;
                    }
                }
                // move the object with the object pushing it
                // this is... probably where bugs will arise, NOTED!
                move(nextObj, x, y);
            }
        }
        
        // if all is valid, increment and return true
        obj->x = nextX;
        obj->y = nextY;
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

    // generates an instance of an object, assigning x and y values where they are available and not used by another object.
    object autoObject(drawing dc) {
        // generate x and y values
        int x, y;
        x = rand() % width;
        y = rand() % height;

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
        int boxPairs = (w*h)/5;
        // auto-generate boxes and targets
        for (int i = 0; i < boxPairs; i++) {
            objects.push_back(autoObject(drawing::Cross)); // target
            objects[i].capturePoint = true;
        }
        for (int i = boxPairs; i < boxPairs*2; i++) {
            objects.push_back(autoObject(drawing::FullBox)); // box
            objects[i].captureBox = true;
        }
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
        std::cout << pty::paint("> Score : ", {"grey", "bold"}) << pty::paint(cm.score, cm.score == 0 ? "red" : "green");
        std::cout << pty::paint(" | Player co-ordinates : ", {"grey", "bold"}) << player.x << " , " << player.y << std::endl;
        std::cout << drn;

        // await user input
        retake: // for retaking key input
        std::cout << pty::paint("\nWhich way do you wish to move?", "bold") << pty::paint(" (w⬆,a⬅,s⬇,d➡)", "grey") << ": ";
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
            default:
            goto retake;
        }
    }

    return 0;
}