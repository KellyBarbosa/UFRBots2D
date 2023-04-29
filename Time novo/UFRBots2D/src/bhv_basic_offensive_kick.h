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

/////////////////////////////////////////////////////////////////////
/*

Modificado por Andr� Ottoni - andreottoni@ymail.com
Time UaiSoccer2D de Futebol de Rob�s Simulado
UFSJ - Universidade Federal de S�o Jo�o del Rei
Robocup 2012 - M�xico

*/

///////////////////////////////////////////////////////////////////////

#ifndef TOKYOTEC_BHV_BASIC_OFFENSIVE_KICK_H
#define TOKYOTEC_BHV_BASIC_OFFENSIVE_KICK_H

#include <rcsc/player/soccer_action.h>

class Bhv_BasicOffensiveKick
    : public rcsc::SoccerBehavior
{

private:
public:
    bool execute(rcsc::PlayerAgent *agent);
    // int mostrarestado(rcsc::PlayerAgent * agent );
    // Declara��o dos novos m�todos
    int recebeestado(rcsc::PlayerAgent *agent);
    int executaacao(rcsc::PlayerAgent *agent, int acao_q, int estado);
    // float calculafuzzy(rcsc::PlayerAgent * agent);
    int *recompensa(rcsc::PlayerAgent *agent, int acao_q);
    // int novoestado(rcsc::PlayerAgent * agent);
    // INÍCIO: UFRBots 2022/2023 - Kelly
    void selectStrategy();
    int strategy;
    void saveStrategy(int strategy);
    int readStrategy();

    void writeQFile(int strategy, int gols);
    void writeFlagFile(int flagValue);
    int readFlagFile();
    // float *readQFile();
    const double alpha = 0.75;
    const double gamma = 0.15;
    const double e = 0.01;
    const int q_size = 4;
    // FIM: UFRBots 2022/2023 - Kelly
};

#endif