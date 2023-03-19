// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifndef BHV_BASIC_MOVE_H
#define BHV_BASIC_MOVE_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/player/soccer_action.h>

class Bhv_BasicMove
    : public rcsc::SoccerBehavior
{
public:
    // INÍCIO: UFRBots 2022/2023 - Kelly
    void writeQFile(int strategy, int gols);
    void writeFlagFile(int flagValue);
    int readFlagFile();
    // int selectStrategy();
    float *readQFile();
    const double e = 0.01;
    const int q_size = 4;
    // FIM: UFRBots 2022/2023 - Kelly
    Bhv_BasicMove()
    {
    }

    bool execute(rcsc::PlayerAgent *agent);
    double Fuzzy(double entrada);
    bool ExecutaTackle(rcsc::PlayerAgent *agent);

private:
    double getDashPower(const rcsc::PlayerAgent *agent);
};

#endif
