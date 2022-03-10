#pragma once

#include <iostream>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <fstream>
#include <random>

class Maze
{

public:
    enum eDirection
    {
        eDirection_Invalid = 0, // 0000
        eDirection_Up = 1,      // 0001
        eDirection_Right = 2,   // 0010
        eDirection_Down = 4,    // 0100
        eDirection_Left = 8     // 1000
    };

    //                   0  1  2  3  4  5  6  7  8
    //                      U  R     D           L
    int Heading_X[9] = {0, 0, +1, 0, 0, 0, 0, 0, -1};
    int Heading_Y[9] = {0, -1, 0, 0, +1, 0, 0, 0, 0};
    int Mask[9] = {
        0,
        eDirection_Down | eDirection_Down << 4,
        eDirection_Left | eDirection_Left << 4,
        0,
        eDirection_Up | eDirection_Up << 4,
        0,
        0,
        0,
        eDirection_Right | eDirection_Right << 4};

    static const int ImageSize = 512;
    static const int NumCells = 10;
    //const char* Version = "1.0.0 (27/05/2014)";
    // current traversing position
    const int CellSize = ImageSize / NumCells;
    unsigned char *g_Maze = new unsigned char[NumCells * NumCells];
     int g_PtX;
     int g_PtY;


    int CellIdx();
    int RandomInt();
    int RandomInt4();
    bool IsDirValid(eDirection Dir);
    eDirection GetDirection();
    void GenerateMaze();
    void SaveBMP(const char *FileName, const void *RawBGRImage, int Width, int Height);
    void Line(unsigned char *img, int x1, int y1, int x2, int y2);
    void RenderMaze(unsigned char *img);
};

static std::random_device rd;
static std::mt19937 gen(rd()); //
static std::uniform_int_distribution<> dis(0, Maze::NumCells - 1);
static std::uniform_int_distribution<> dis4(0, 3);