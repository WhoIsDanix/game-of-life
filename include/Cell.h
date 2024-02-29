#ifndef GAMEOFLIFE_CELL_H
#define GAMEOFLIFE_CELL_H

struct Cell {
    bool state = 0;
    bool nextState = 0;

    Cell() : Cell(0) {}
    Cell(bool _state) : state(_state), nextState(_state) {}
};

#endif