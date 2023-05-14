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

Modificacao: 23/09/2013

Observacoes: - Zonas para defesa e ataque
             - Aprendizado somente para o ataque. Adaptacao da Matriz R para o caso.

*/

///////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bhv_basic_offensive_kick.h"

#include <rcsc/action/body_advance_ball.h>
#include <rcsc/action/body_dribble.h>
#include <rcsc/action/body_hold_ball.h>
#include <rcsc/action/body_pass.h>
#include <rcsc/action/neck_scan_field.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>
#include <rcsc/action/body_clear_ball.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/action/bhv_shoot.h>
#include <rcsc/action/body_smart_kick.h>
#include <rcsc/action/body_kick_one_step.h>
#include <rcsc/player/debug_client.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/sector_2d.h>

#include <time.h>
#include <iostream>
#include <fstream>
using namespace std;

using namespace rcsc;

// INÍCIO: UFRBots 2022/2023 - Kelly

void Bhv_BasicOffensiveKick::selectStrategy()
{
    const double e_alea = (float)rand() / RAND_MAX;
    int selectedStrategy;
    if (e_alea <= e)
    { // ação/estratégia aleatória
        cout << "\n----------------------ação/estratégia aleatória----------------------" << endl;
        selectedStrategy = rand() % q_size + 1;
    }
    else
    { // melhor ação/estratégia
        cout << "\n----------------------melhor ação/estratégia----------------------" << endl;
        FILE *file;
        file = fopen("arquivos/q.txt", "r");

        if (file)
        {
            float q[q_size];

            for (int i = 0; i < q_size; i++)
            {
                fscanf(file, "%f", &q[i]);
            }

            fclose(file);

            float max = q[0];
            int position = 0;
            for (int i = 0; i < q_size; i++)
            {
                if (q[i] > max)
                {
                    max = q[i];
                    position = i;
                }
            }
            selectedStrategy = position + 1;
        }
        else
        {
            printf("Arquivo não encontrado.");
        }
    }
    saveStrategy(selectedStrategy);
}

void Bhv_BasicOffensiveKick::saveStrategy(int strategy)
{
    FILE *file;
    file = fopen("arquivos/strategy.txt", "w");
    fprintf(file, "%d", strategy);
    fclose(file);
}

int Bhv_BasicOffensiveKick::readStrategy()
{
    FILE *file;
    file = fopen("arquivos/strategy.txt", "r");

    if (file)
    {
        int s;
        fscanf(file, "%d", &s);
        fclose(file);
        return s;
    }
    else
    {
        printf("Arquivo não encontrado.");
    }
    return 1; // Caso não seja possível ler o arquivo, garante que alguma estratégia seja selecionada
}

void Bhv_BasicOffensiveKick::writeFlagFile(int flagValue)
{
    FILE *fileController;
    fileController = fopen("arquivos/q_controller.txt", "w");

    if (fileController)
    {
        fprintf(fileController, "%d", flagValue);
        fclose(fileController);
    }
    else
    {
        printf("Erro ao abrir o arquivo de controle.");
    }
}

int Bhv_BasicOffensiveKick::readFlagFile()
{
    FILE *fileController;

    fileController = fopen("arquivos/q_controller.txt", "r");

    if (fileController)
    {
        int flag;

        fscanf(fileController, "%d", &flag);

        fclose(fileController);

        return flag;
    }
    else
    {
        printf("Erro ao abrir o arquivo de controle.");
    }
    return 1; // Caso não seja possível ler o arquivo, retorna 1 indicando que o arquivo não pode ser escrito
}

void Bhv_BasicOffensiveKick::writeQFile(int strategy, int gols)
{
    cout << "\n---------------------- writeQFile: " << strategy << " | " << gols << " ----------------------" << endl;
    FILE *file;
    file = fopen("arquivos/q.txt", "r");

    if (file)
    {
        float q[q_size];

        for (int i = 0; i < q_size; i++)
        {
            fscanf(file, "%f", &q[i]);
        }

        fclose(file);

        float max = q[0];
        int position = 0;
        for (int i = 0; i < q_size; i++)
        {
            if (q[i] > max)
            {
                max = q[i];
                position = i;
            }
        }

        float newQ = q[strategy] + alpha * (gols + gamma * max - q[strategy]);
        q[strategy] = newQ;

        if (readFlagFile() == 0) // Indica que o arquivo não foi escrito ainda
        {
            printf("\n ****************** Pode escrever ******************\n");
            FILE *file;
            file = fopen("arquivos/q.txt", "w");

            for (int i = 0; i < q_size; i++)
            {
                fprintf(file, "%.4f ", q[i]);
            }

            writeFlagFile(1); // Indica que o arquivo já foi escrito

            fclose(file);
        }
        else
        {
            printf("\n ****************** Não pode escrever ******************\n");
        }
    }
    else
    {
        printf("Arquivo não encontrado.");
    }
}

// FIM: UFRBots 2022/2023 - Kelly

bool Bhv_BasicOffensiveKick::execute(PlayerAgent *agent)
{

    return true;
}

int Bhv_BasicOffensiveKick::recebeestado(PlayerAgent *agent)
{

    const WorldModel &wm = agent->world();

    const PlayerPtrCont &opps = wm.opponentsFromSelf();
    const PlayerObject *nearest_opp = (opps.empty()
                                           ? static_cast<PlayerObject *>(0)
                                           : opps.front());
    const double nearest_opp_dist = (nearest_opp
                                         ? nearest_opp->distFromSelf()
                                         : 1000.0);
    const Vector2D nearest_opp_pos = (nearest_opp
                                          ? nearest_opp->pos()
                                          : Vector2D(-1000.0, 0.0));
    // Defini��o do Estado do Ambiente
    int estado = 1;

    // Zona A - Nova
    if ((wm.self().pos().x > 0) && (wm.self().pos().x <= 13.0))
    {
        if (wm.self().pos().y <= -20.0)
            estado = 1;
        if ((wm.self().pos().y > -20) && (wm.self().pos().y <= -9.0))
            estado = 2;
        if ((wm.self().pos().y > -9.0) && (wm.self().pos().y <= 9))
            estado = 3;
        if ((wm.self().pos().y) > 9 && (wm.self().pos().y < 20))
            estado = 4;
        if (wm.self().pos().y >= 20.0)
            estado = 5;
    }
    else
    {

        // Zona B - Nova
        if ((wm.self().pos().x > 13.0) && (wm.self().pos().x <= 26.0))
        {
            if (wm.self().pos().y <= -20.0)
                estado = 6;
            if ((wm.self().pos().y > -20) && (wm.self().pos().y <= -9.0))
                estado = 7;
            if ((wm.self().pos().y > -9.0) && (wm.self().pos().y <= 9))
                estado = 8;
            if ((wm.self().pos().y) > 9 && (wm.self().pos().y < 20))
                estado = 9;
            if (wm.self().pos().y >= 20.0)
                estado = 10;
        }
        else
        {

            // Zona C - Nova
            if ((wm.self().pos().x > 26.0) && (wm.self().pos().x <= 39.0))
            {
                if (wm.self().pos().y <= -20.0)
                    estado = 11;
                if ((wm.self().pos().y > -20) && (wm.self().pos().y <= -9.0))
                    estado = 12;
                if ((wm.self().pos().y > -9.0) && (wm.self().pos().y <= 9))
                    estado = 13;
                if ((wm.self().pos().y) > 9 && (wm.self().pos().y < 20))
                    estado = 14;
                if (wm.self().pos().y >= 20.0)
                    estado = 15;
            }
            else
            {

                // Zona D - Nova
                if (wm.self().pos().x > 39.0)
                {
                    if (wm.self().pos().y <= -20.0)
                        estado = 16;
                    if ((wm.self().pos().y > -20) && (wm.self().pos().y <= -9.0))
                        estado = 17;
                    if ((wm.self().pos().y > -9.0) && (wm.self().pos().y <= 9))
                        estado = 18;
                    if ((wm.self().pos().y) > 9 && (wm.self().pos().y < 20))
                        estado = 19;
                    if (wm.self().pos().y >= 20.0)
                        estado = 20;
                }
            }
        }
    }

    if (nearest_opp_dist > 4)
        estado = estado + 20; // Longe

    // Caso nao entre no laco, eh perto

    return estado;
}

int Bhv_BasicOffensiveKick::executaacao(PlayerAgent *agent, int acao_q, int estado)
{

    const WorldModel &wm = agent->world();

    // INÍCIO: UFRBots 2022/2023 - Kelly

    int our_score = (wm.ourSide() == LEFT
                         ? wm.gameMode().scoreLeft()
                         : wm.gameMode().scoreRight());
    int opp_score = (wm.ourSide() == LEFT
                         ? wm.gameMode().scoreRight()
                         : wm.gameMode().scoreLeft());

    if (wm.time().cycle() >= 0 && wm.seeTime().cycle() <= 100)
    {
        strategy = 1;
        selectStrategy();
        writeFlagFile(0);
    }
    else
    {
        strategy = readStrategy();
    }
    if (wm.time().cycle() >= 5900 && wm.seeTime().cycle() <= 6000)
    {
        writeQFile(strategy - 1, our_score);
    }

    cout << "\n---------------------- executaacao() - Strategy: " << strategy << " ----------------------" << endl;

    //  FIM: UFRBots 2022/2023 - Kelly

    Vector2D ball = wm.ball().pos();
    Vector2D me = wm.self().pos();

    int num = wm.self().unum();

    const PlayerPtrCont &opps = wm.opponentsFromSelf();
    const PlayerObject *nearest_opp = (opps.empty()
                                           ? static_cast<PlayerObject *>(0)
                                           : opps.front());
    const double nearest_opp_dist = (nearest_opp
                                         ? nearest_opp->distFromSelf()
                                         : 1000.0);
    const Vector2D nearest_opp_pos = (nearest_opp
                                          ? nearest_opp->pos()
                                          : Vector2D(-1000.0, 0.0));

    Vector2D pass_point;

    /// setting shouldAvoid Opponents ///////////////////

    bool shouldAvoid = true;

    if (ball.x < 32.0 && ball.x > -36.0)
        shouldAvoid = true;

    if (ball.x > 25.0 && ball.absY() < 22.0)
        shouldAvoid = false;

    if ((num == 8 || num == 7) && ball.x < 35)
        shouldAvoid = true;

    //       shouldAvoid = false;

    //       if( me.absY() > 29.5 && num > 9 )
    //            shouldAvoid = false;

    /// setting minDist /////////////////////////////////

    float minDist = 9.0;

    if (ball.x > 30.0)
        minDist = 8.0;

    if (ball.x > 36.0 && ball.absY() < 20.0)
        minDist = 7.0;

    if (ball.x > 36.0 && ball.absY() < 10.0)
        minDist = 6.0;

    if ((num == 9 || num == 10) && ball.absY() > 25.0)
        minDist = 6.0;

    if (num > 6 && ball.absY() < 12.0)
        minDist = 4.5;

    /// setting Dribble Target ////////////////////////////////////

    Vector2D drib_target(51.0, wm.self().pos().absY());
    if (drib_target.y < 20.0)
        drib_target.y = 20.0;
    if (drib_target.y > 32.0)
        drib_target.y = 31.0;
    if (wm.self().pos().y < 0.0)
        drib_target.y *= -1.0;

    if ((num == 7 || num == 8) && ball.x < 25.0 && ball.x > wm.offsideLineX() - 8.0)
        drib_target = Vector2D(me.x + 10.0, 22.0);

    //       if( num < 9 && ball.x < 36.0 && ball.absY() < 15.0 )
    //             drib_target = Vector2D( me.x + 10.0, 20.0 );

    if (num == 6 && ball.x > 28.0)
        drib_target = Vector2D(40.0, 0.0);

    if (ball.x > 25.0 && ball.x < 33.0)
        drib_target.y = me.y;

    if (num > 9 && me.absY() > 25.0)
        drib_target = Vector2D(me.x + 13.0, 31.2);

    // in attacking situation ///////////////////////////////////////

    Vector2D goal = Vector2D(51.0, 0.0);

    //      if( (wm.self().unum() == 7 || wm.self().unum() == 8) && ball.x > 35.0 )
    //          goal = Vector2D(41.0, 0.0);

    if (wm.self().unum() == 11 && (ball.x > 25.0 || ball.x > wm.offsideLineX() - 10.0))
        goal = Vector2D(47.0, 0.0);

    if (me.x > 51.0)
        goal.x -= 3.0;

    if (wm.self().unum() == 11 && ball.x > wm.offsideLineX() - 10.0 &&
        wm.countOpponentsIn(Circle2D(Vector2D(me.x + 5.0, me.y), 5.0),
                            3, false) > 1 &&
        ball.x < 30.0)
        drib_target = me + Vector2D::polar2vector(20.0,
                                                  (Vector2D(me.x + 4.0, me.y + sign(me.y) * 12.0) - me).dir());

    if (ball.x > 40.0)
        drib_target = me + Vector2D::polar2vector(20.0, (goal - me).dir());

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    //---------Zonas de Defesa-----------------------------///

    // if (( nearest_opp_dist > 4 )&&(wm.self().pos().x <= 0)) acao_q = 2;
    // Zona A
    if (wm.self().pos().x <= -36.0)
    {
        if (wm.self().pos().y <= -20.0)
            acao_q = 3;
        if (wm.self().pos().y >= 20.0)
            acao_q = 3;
        if ((wm.self().pos().y > -20.0) && (wm.self().pos().y < 20.0))
            acao_q = 3;
    }
    else
    {

        // Zona B
        if ((wm.self().pos().x > -36.0) && (wm.self().pos().x <= -26.0))
        {
            if (wm.self().pos().y <= -20.0)
                acao_q = 3;
            if (wm.self().pos().y >= 20.0)
                acao_q = 3;
            if ((wm.self().pos().y > -20.0) && (wm.self().pos().y < 20.0))
                acao_q = 3;
        }
        else
        {

            // Zona C
            if ((wm.self().pos().x > -26.0) && (wm.self().pos().x <= 0))
            {
                if (wm.self().pos().y <= -20.0)
                    acao_q = 1;
                if (wm.self().pos().y >= 20.0)
                    acao_q = 2;
                if ((wm.self().pos().y > -20.0) && (wm.self().pos().y < 20.0))
                    acao_q = 2;
            }
        }
    }

    // INÍCIO: UFRBots 2022/2023 - Kelly
    int actions[40][q_size] = {
        {2, 2, 1, 5}, // 1
        {2, 2, 1, 5}, // 2
        {5, 5, 1, 5}, // 3
        {2, 2, 1, 5}, // 4
        {2, 2, 1, 5}, // 5
        {2, 2, 2, 3}, // 6
        {2, 2, 2, 3}, // 7
        {3, 3, 3, 3}, // 8
        {2, 2, 2, 3}, // 9
        {2, 2, 2, 3}, // 10
        {2, 5, 2, 3}, // 11
        {2, 3, 2, 3}, // 12
        {5, 5, 5, 3}, // 13
        {2, 3, 2, 3}, // 14
        {2, 5, 2, 3}, // 15
        {3, 3, 3, 3}, // 16
        {3, 3, 3, 3}, // 17
        {6, 6, 6, 6}, // 18
        {3, 3, 3, 3}, // 19
        {3, 3, 3, 3}, // 20
        {1, 1, 1, 5}, // 21
        {1, 1, 1, 5}, // 22
        {5, 5, 1, 5}, // 23
        {1, 1, 1, 5}, // 24
        {1, 1, 1, 5}, // 25
        {1, 1, 1, 1}, // 26
        {1, 1, 1, 1}, // 27
        {3, 3, 1, 1}, // 28
        {1, 1, 1, 1}, // 29
        {1, 1, 1, 1}, // 30
        {1, 1, 1, 1}, // 31
        {1, 3, 1, 3}, // 32
        {5, 3, 1, 3}, // 33
        {1, 3, 1, 3}, // 34
        {1, 1, 1, 1}, // 35
        {1, 2, 1, 1}, // 36
        {1, 2, 1, 3}, // 37
        {6, 6, 6, 6}, // 38
        {1, 2, 1, 3}, // 39
        {1, 2, 1, 1}, // 40
    };

    switch (estado)
    {
    case 1:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 2:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 3:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 4:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 5:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 6:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 7:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 8:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 9:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 10:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 11:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 12:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 13:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 14:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 15:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 16:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 17:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 18:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 19:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 20:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 21:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 22:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 23:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 24:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 25:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 26:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 27:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 28:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 29:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 30:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 31:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 32:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 33:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 34:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 35:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 36:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 37:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 38:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 39:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    case 40:
        acao_q = actions[estado - 1][strategy - 1];
        break;
    }
    // FIM: UFRBots 2022/2023 - Kelly

    const AngleDeg drib_angle = (drib_target - wm.self().pos()).th();

    const int max_dash_step = wm.self().playerType().cyclesToReachDistance(wm.self().pos().dist(drib_target));

    // check opponents
    // 15m, +-30 degree
    const Sector2D sector(wm.self().pos(),
                          0.5, 15.0,
                          drib_angle - 30.0,
                          drib_angle + 30.0);

    const rcsc::ServerParam &param = rcsc::ServerParam::i();

    const rcsc::Vector2D target = param.theirTeamGoalPos();

    const Vector2D M_target_point; //!< trapped ball position

    Vector2D first_vel = M_target_point - wm.ball().pos();
    // first_vel.setLength( action.firstBallSpeed() );

    const Vector2D kick_accel = first_vel - wm.ball().vel();
    const double kick_power = kick_accel.r() / wm.self().kickRate();
    const AngleDeg kick_angle = kick_accel.th() - wm.self().body();

    // agent->debugClient().setTarget( target );

    int acao = 0;
    acao = acao_q;
    /* int q = 0;
     const float e = 0.16666667*100;


     //initialize random seed:
     srand ( time(NULL) );

     //////Politica e-gulosa/////////////////////
     // generate secret number:
     q = rand() % 100 + 1;
     if(q<=e){
          // generate secret number:
          acao = rand() % 6 + 1;
     }else{
          acao = acao_q;
     }  */

    ///////////////////////////////////////////

    ///////Politica aleatoria//////////////////////////////

    // acao = rand() % 6 + 1;

    /////////////////////////////////////////////////////////

    // Executa a a��o selecionada

    switch (acao)
    {

    case 1: // Drible 1

        if (nearest_opp_dist < 5.0 && nearest_opp_dist > (ServerParam::i().tackleDist() + ServerParam::i().defaultPlayerSpeedMax() * 1.5) && wm.self().body().abs() < 70.0)
        {
            const Vector2D body_dir_drib_target = wm.self().pos() + Vector2D::polar2vector(5.0, wm.self().body());
            int max_dir_count = 0;
            wm.dirRangeCount(wm.self().body(), 20.0, &max_dir_count, NULL, NULL);

            if (body_dir_drib_target.x < ServerParam::i().pitchHalfLength() - 1.0 && body_dir_drib_target.absY() < ServerParam::i().pitchHalfWidth() - 1.0 && max_dir_count < 3)
            {
                // check opponents
                // 10m, +-30 degree
                const Sector2D sector(wm.self().pos(),
                                      0.5, 10.0,
                                      wm.self().body() - 30.0,
                                      wm.self().body() + 30.0);
                // opponent check with goalie
                if (!wm.existOpponentIn(sector, 10, true))
                {
                    dlog.addText(Logger::TEAM,
                                 __FILE__ ": (execute) dribble to my body dir");
                    agent->debugClient().addMessage("OffKickDrib(1)");
                    Body_Dribble(body_dir_drib_target,
                                 1.0,
                                 ServerParam::i().maxDashPower(),
                                 2)
                        .execute(agent);
                    agent->setNeckAction(new Neck_TurnToLowConfTeammate());
                    return true;
                }
            }
        }
        // opponent is behind of me
        if (nearest_opp_pos.x < wm.self().pos().x + 1.0)
        {
            // check opponents
            // 15m, +-30 degree
            const Sector2D sector(wm.self().pos(),
                                  0.5, 15.0,
                                  drib_angle - 30.0,
                                  drib_angle + 30.0);
            // opponent check with goalie
            if (!wm.existOpponentIn(sector, 10, true))
            {
                const int max_dash_step = wm.self().playerType().cyclesToReachDistance(wm.self().pos().dist(drib_target));
                if (wm.self().pos().x > 35.0)
                {
                    drib_target.y *= (10.0 / drib_target.absY());
                }
                dlog.addText(Logger::TEAM,
                             __FILE__ ": (execute) fast dribble to (%.1f, %.1f) max_step=%d",
                             drib_target.x, drib_target.y,
                             max_dash_step);
                agent->debugClient().addMessage("OffKickDrib(2)");
                Body_Dribble(drib_target,
                             1.0,
                             ServerParam::i().maxDashPower(),
                             std::min(5, max_dash_step))
                    .execute(agent);
            }
            else
            {
                dlog.addText(Logger::TEAM,
                             __FILE__ ": (execute) slow dribble to (%.1f, %.1f)",
                             drib_target.x, drib_target.y);
                agent->debugClient().addMessage("OffKickDrib(3)");
                Body_Dribble(drib_target,
                             1.0,
                             ServerParam::i().maxDashPower(),
                             2)
                    .execute(agent);
            }
            agent->setNeckAction(new Neck_TurnToLowConfTeammate());

            return true;
        }
        // opp is far from me
        if (nearest_opp_dist > 5.0)
        {
            dlog.addText(Logger::TEAM,
                         __FILE__ ": opp far. dribble(%.1f, %.1f)",
                         drib_target.x, drib_target.y);
            agent->debugClient().addMessage("OffKickDrib(4)");
            Body_Dribble(drib_target,
                         1.0,
                         ServerParam::i().maxDashPower() * 0.4,
                         1)
                .execute(agent);
            agent->setNeckAction(new Neck_TurnToLowConfTeammate());
            return true;
        }

        // opp is far from me
        if (nearest_opp_dist > 3.0)
        {
            dlog.addText(Logger::TEAM,
                         __FILE__ ": (execute) opp far. dribble(%f, %f)",
                         drib_target.x, drib_target.y);
            agent->debugClient().addMessage("OffKickDrib(5)");
            Body_Dribble(drib_target,
                         1.0,
                         ServerParam::i().maxDashPower() * 0.2,
                         1)
                .execute(agent);
            agent->setNeckAction(new Neck_TurnToLowConfTeammate());
            return true;
        }

        if (nearest_opp_dist > 2.5)
        {
            dlog.addText(Logger::TEAM,
                         __FILE__ ": hold");
            agent->debugClient().addMessage("OffKickHold");
            Body_HoldBall().execute(agent);
            agent->setNeckAction(new Neck_TurnToLowConfTeammate());
            return true;
        }

        break; // Fim Case 1

    case 2: // Drible 2

        // opponent check with goalie
        if (!wm.existOpponentIn(sector, 10, true))
        {
            const int max_dash_step = wm.self().playerType().cyclesToReachDistance(wm.self().pos().dist(drib_target));
            if (wm.self().pos().x > 35.0)
            {
                drib_target.y *= (10.0 / drib_target.absY());
            }
        }
        dlog.addText(Logger::TEAM,
                     __FILE__ ": (execute) slow dribble to (%.1f, %.1f)",
                     drib_target.x, drib_target.y);
        agent->debugClient().addMessage("OffKickDrib(3)");
        Body_Dribble(drib_target,
                     1.0,
                     ServerParam::i().maxDashPower(),
                     std::min(5, max_dash_step))
            .execute(agent);
        agent->setNeckAction(new Neck_TurnToLowConfTeammate());
        break; // Fim case 2

    case 3:

        if (Body_Pass::get_best_pass(wm, &pass_point, NULL, NULL))
        {
            if (((pass_point.x > wm.self().pos().x - 1.0)) || ((wm.self().pos().x > 37) && (pass_point.x > wm.self().pos().x - 6.0)))
            {
                bool safety = true;
                const PlayerPtrCont::const_iterator opps_end = opps.end();
                for (PlayerPtrCont::const_iterator it = opps.begin(); it != opps_end; ++it)
                {
                    if ((*it)->pos().dist(pass_point) < 1.0)
                    {
                        safety = false;
                    }
                }

                if (safety)
                {
                    dlog.addText(Logger::TEAM,
                                 __FILE__ ": (execute) do best pass");
                    agent->debugClient().addMessage("OffKickPass(1)");
                    Body_Pass().execute(agent);
                    agent->setNeckAction(new Neck_TurnToLowConfTeammate());
                    return true;
                }
                else
                {
                    dlog.addText(Logger::TEAM,
                                 __FILE__ ": hold");
                    agent->debugClient().addMessage("OffKickHold");
                    Body_HoldBall().execute(agent);
                    agent->setNeckAction(new Neck_TurnToLowConfTeammate());
                }
            }
            else
            {
                dlog.addText(Logger::TEAM,
                             __FILE__ ": hold");
                agent->debugClient().addMessage("OffKickHold");
                Body_HoldBall().execute(agent);
                agent->setNeckAction(new Neck_TurnToLowConfTeammate());
            }
        }

        break; // Fim Case 3

    case 4:

        if (Body_Pass::get_best_pass(wm, &pass_point, NULL, NULL))
        {
            if (pass_point.x > wm.self().pos().x - 1.0)
            {
                bool safety = true;
                const PlayerPtrCont::const_iterator opps_end = opps.end();
                for (PlayerPtrCont::const_iterator it = opps.begin(); it != opps_end; ++it)
                {
                    if ((*it)->pos().dist(pass_point) < 4.0)
                    {
                        safety = false;
                    }
                }
                if (safety)
                {
                    dlog.addText(Logger::TEAM,
                                 __FILE__ ": (execute) do best pass");
                    agent->debugClient().addMessage("OffKickPass(1)");
                    Body_Pass().execute(agent);
                    agent->setNeckAction(new Neck_TurnToLowConfTeammate());
                    return true;
                }
                else
                {
                    dlog.addText(Logger::TEAM,
                                 __FILE__ ": clear");
                    agent->debugClient().addMessage("OffKickAdvance");
                    Body_AdvanceBall().execute(agent);
                    agent->setNeckAction(new Neck_ScanField());
                }
            }
            else
            {
                dlog.addText(Logger::TEAM,
                             __FILE__ ": clear");
                agent->debugClient().addMessage("OffKickAdvance");
                Body_AdvanceBall().execute(agent);
                agent->setNeckAction(new Neck_ScanField());
            }
        }
        break; // Fim Case 4

    case 5: // Passe 3

        if (Body_Pass::get_best_pass(wm, &pass_point, NULL, NULL))
        {
            if (pass_point.x > wm.self().pos().x - 18.0)
            {
                bool safety = true;
                const PlayerPtrCont::const_iterator opps_end = opps.end();
                for (PlayerPtrCont::const_iterator it = opps.begin(); it != opps_end; ++it)
                {
                    if ((*it)->pos().dist(pass_point) < 2.0)
                    {
                        safety = false;
                    }
                }
                if ((safety) || ((wm.self().pos().x > 37) && (((wm.self().pos().y > -17.0) && (wm.self().pos().y < -10.0)) || ((wm.self().pos().y > 10.0) && (wm.self().pos().y < 17.0)))))
                {
                    dlog.addText(Logger::TEAM,
                                 __FILE__ ": (execute) do best pass");
                    agent->debugClient().addMessage("OffKickPass(1)");
                    Body_Pass().execute(agent);
                    agent->setNeckAction(new Neck_TurnToLowConfTeammate());
                    return true;
                }
                else
                {
                    dlog.addText(Logger::TEAM,
                                 __FILE__ ": hold");
                    agent->debugClient().addMessage("OffKickHold");
                    Body_HoldBall().execute(agent);
                    agent->setNeckAction(new Neck_TurnToLowConfTeammate());
                }
            }
            else
            {
                dlog.addText(Logger::TEAM,
                             __FILE__ ": hold");
                agent->debugClient().addMessage("OffKickHold");
                Body_HoldBall().execute(agent);
                agent->setNeckAction(new Neck_TurnToLowConfTeammate());
            }
        }

        break; // Fim Case 5

    // Chutar
    case 6:
        Body_SmartKick(target,
                       param.ballSpeedMax(),
                       param.ballSpeedMax() * 0.96,
                       3)
            .execute(agent);
        agent->setNeckAction(new Neck_ScanField());
        break;
    }
    return acao;
}

// Retorna a Recompensa de Acordo com Par Estado/A��o
int *Bhv_BasicOffensiveKick::recompensa(PlayerAgent *agent, int acao_q)
{

    int R_M[24][6] = {{-1, 0, -1, -1, -1, -1},
                      {-1, -1, 0, -1, -1, -1},
                      {-1, 0, -1, -1, -1, -1},
                      {1, 0, 0, 0, 0, 0},
                      {0, 0, 1, 0, 0, 0},
                      {1, 0, 0, 0, 0, 0},
                      {1, 10, 1, 1, 1, 1},
                      {1, 1, 10, 1, 1, 1},
                      {1, 10, 1, 1, 1, 1},
                      {20, 10, 10, 10, 10, 10},
                      {10, 10, 10, 10, 10, 40},
                      {10, 20, 10, 10, 10, 10},
                      {-1, 0, -1, -1, -1, -1},
                      {-1, -1, 0, -1, -1, -1},
                      {-1, 0, -1, -1, -1, -1},
                      {1, 0, 0, 0, 0, 0},
                      {0, 0, 1, 0, 0, 0},
                      {1, 0, 0, 0, 0, 0},
                      {1, 10, 1, 1, 1, 1},
                      {1, 1, 10, 1, 1, 1},
                      {1, 10, 1, 1, 1, 1},
                      {20, 10, 10, 10, 10, 10},
                      {10, 10, 10, 10, 10, 40},
                      {10, 20, 10, 10, 10, 10}}; // Linhas = Estados, Colunas = Acoes

    int *A_E_R; // Matriz de Acao(A)/Estado(S)/Recompensa(R)/Novo Estado(S')
    A_E_R = (int *)malloc(4 * sizeof(int));
    int estado = Bhv_BasicOffensiveKick().recebeestado(agent);
    int acao = Bhv_BasicOffensiveKick().executaacao(agent, acao_q, estado);
    int novo_estado = Bhv_BasicOffensiveKick().recebeestado(agent);
    int r = R_M[estado - 1][acao - 1];
    A_E_R[0] = acao;
    A_E_R[1] = estado;
    A_E_R[2] = r;
    A_E_R[3] = novo_estado;
    return A_E_R;
}
