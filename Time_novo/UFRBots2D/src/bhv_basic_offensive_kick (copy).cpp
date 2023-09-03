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

Modificado por André Ottoni - andreottoni@ymail.com
Time UaiSoccer2D de Futebol de Robôs Simulado
UFSJ - Universidade Federal de São João del Rei

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

bool
Bhv_BasicOffensiveKick::execute( PlayerAgent * agent )
{
    
    return true;

}


int 
Bhv_BasicOffensiveKick::recebeestado(PlayerAgent * agent){

    const WorldModel & wm = agent->world();

    const PlayerPtrCont & opps = wm.opponentsFromSelf();
    const PlayerObject * nearest_opp
        = ( opps.empty()
            ? static_cast< PlayerObject * >( 0 )
            : opps.front() );
    const double nearest_opp_dist = ( nearest_opp
                                      ? nearest_opp->distFromSelf()
                                      : 1000.0 );
    const Vector2D nearest_opp_pos = ( nearest_opp
                                       ? nearest_opp->pos()
                                       : Vector2D( -1000.0, 0.0 ) );
    //Definição do Estado do Ambiente
    int estado = 1;

    //Zona A - Nova
    if((wm.self().pos().x > 0)&&(wm.self().pos().x <= 13.0)){
         if(wm.self().pos().y <= -20.0) estado = 1;
         if(wm.self().pos().y >= 20.0) estado = 3;
         if((wm.self().pos().y > -20.0)&&(wm.self().pos().y < 20.0)) estado = 2;
    }else{

    //Zona B - Nova
    if((wm.self().pos().x > 13.0)&&(wm.self().pos().x <= 26.0)){
         if(wm.self().pos().y <= -20.0) estado = 4;
         if(wm.self().pos().y >= 20.0) estado = 6;
         if((wm.self().pos().y > -20.0)&&(wm.self().pos().y < 20.0)) estado = 5;
    }else{


    //Zona C - Nova
    if((wm.self().pos().x > 26.0)&&(wm.self().pos().x <= 39.0)){
         if(wm.self().pos().y <= -20.0) estado = 7;
         if(wm.self().pos().y >= 20.0) estado = 9;
         if((wm.self().pos().y > -20.0)&&(wm.self().pos().y < 20.0)) estado = 8;
    }else{


    //Zona D - Nova
    if(wm.self().pos().x > 39.0){
         if(wm.self().pos().y <= -12.0) estado = 10;
         if(wm.self().pos().y >= 12.0) estado = 12;
         if((wm.self().pos().y > -12.0)&&(wm.self().pos().y < 12.0)) estado = 11;
    }//else{

    /*//Zona E - Nova
    if(wm.self().pos().x > 40.0){
         if(wm.self().pos().y <= -12.0) estado = 13;
         if(wm.self().pos().y >= 12.0) estado = 15;
         if((wm.self().pos().y > -12.0)&&(wm.self().pos().y < 12.0)) estado = 14;
    }*/
    
   // }
   }}}

    //Definicao se adversario esta perto ou longe

    //Valido para as Zonas A, B e C
    //  if (( nearest_opp_dist > 4 )&&(estado<10))estado = estado + 15; //Longe
    
    //Valido para as Zonas C, D e E
    //if (( nearest_opp_dist > 4 )&&(estado>6))estado = estado + 9; //Longe

    if ( nearest_opp_dist > 4 ) estado = estado + 12; //Longe

    //Caso nao entre no laco, eh perto  
     
    return estado;
}

int 
Bhv_BasicOffensiveKick::executaacao(PlayerAgent * agent, int acao_q, int estado){
  
    const WorldModel & wm = agent->world();

    const PlayerPtrCont & opps = wm.opponentsFromSelf();

    Vector2D pass_point;

    Vector2D me = wm.self().pos();

    const PlayerObject * nearest_opp
        = ( opps.empty()
            ? static_cast< PlayerObject * >( 0 )
            : opps.front() );
    const double nearest_opp_dist = ( nearest_opp
                                      ? nearest_opp->distFromSelf()
                                      : 1000.0 );
    const Vector2D nearest_opp_pos = ( nearest_opp
                                       ? nearest_opp->pos()
                                       : Vector2D( -1000.0, 0.0 ) );

    
    //---------Zonas de Defesa-----------------------------///

    //if (( nearest_opp_dist > 4 )&&(wm.self().pos().x <= 0)) acao_q = 2;
    //Zona A
    if(wm.self().pos().x <= -36.0){
         if(wm.self().pos().y <= -20.0) acao_q = 4;
         if(wm.self().pos().y >= 20.0) acao_q = 4;
         if((wm.self().pos().y > -20.0)&&(wm.self().pos().y < 20.0)) acao_q = 4;
    }else{

    //Zona B
    if((wm.self().pos().x > -36.0)&&(wm.self().pos().x <= -26.0)){
         if(wm.self().pos().y <= -20.0) acao_q = 3;
         if(wm.self().pos().y >= 20.0) acao_q = 3;
         if((wm.self().pos().y > -20.0)&&(wm.self().pos().y < 20.0)) acao_q = 3;
    }else{


    //Zona C
    if((wm.self().pos().x > -26.0)&&(wm.self().pos().x <= 0)){
         if(wm.self().pos().y <= -20.0) acao_q = 1;
         if(wm.self().pos().y >= 20.0) acao_q = 2;
         if((wm.self().pos().y > -20.0)&&(wm.self().pos().y < 20.0)) acao_q = 2;
    }}}

    switch(estado){
       case 1:
           acao_q = 3;
           break;
       case 2:
           acao_q = 3;
           break;
       case 3:
           acao_q = 2;
           break;
       case 4:
           acao_q = 1;
           break;
       case 5:
           acao_q = 1;
           break;
       case 6:
           acao_q = 2;
           break;
       case 7:
           acao_q = 2;
           break;
       case 8:
           acao_q = 3;
           break;
       case 9:
           acao_q = 2;
           break;
       case 10:
           acao_q = 3;
           break;
       case 11:
           acao_q = 6;
           break;
       case 12:
           acao_q = 3;
           break;
       case 13:
           acao_q = 2;  
           break;
       case 14:
           acao_q = 3;
           break;
       case 15:
           acao_q = 2;
           break;
       case 16:
           acao_q = 1; 
           break;
       case 17:
           acao_q = 3;
           break;
       case 18:
           acao_q = 1;
           break;
       case 19:
           acao_q = 2;
           break;
       case 20:
           acao_q = 2;
           break;
       case 21:
           acao_q = 2;
           break;
       case 22:
           acao_q = 1;
           break;
       case 23:
           acao_q = 6;
           break;
       case 24:
           acao_q = 2;
           break;
       
    }

    ////////////////////////////////////////////////////////////////////////////////

    //Rotina para verificar a possiblidade de toque para o companheiro dentro da area
    if(wm.self().pos().x > 45.0){
        agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
        if((wm.countOpponentsIn( Circle2D( me , 6.0 ) , 3 , false ) >= 1)){
           if(((wm.self().pos().y > -17.0)&&(wm.self().pos().y < -10.0)) || ((wm.self().pos().y > 10.0)&&(wm.self().pos().y < 17.0))){
               acao_q = 5;
           }
        }
        /*if ( nearest_opp_dist < 3 ){ //Distancia para o agente mais proximo menor que 3 => rand para acao (Passe ou chute)
             int acao_ataque = rand() % 2 + 1;
                if(acao_ataque == 1) acao_q = 6;
                if(acao_ataque == 2) acao_q = 5;
        }  */
    }    
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Vector2D drib_target( 50.0, wm.self().pos().absY() );
    if ( drib_target.y < 20.0 ) drib_target.y = 20.0;
    if ( drib_target.y > 29.0 ) drib_target.y = 27.0;
    if ( wm.self().pos().y < 0.0 ) drib_target.y *= -1.0;
    const AngleDeg drib_angle = ( drib_target - wm.self().pos() ).th();

    const int max_dash_step = wm.self().playerType().cyclesToReachDistance( wm.self().pos().dist( drib_target ) );

    // check opponents
    // 15m, +-30 degree
    const Sector2D sector( wm.self().pos(),
                               0.5, 15.0,
                               drib_angle - 30.0,
                               drib_angle + 30.0 );

    const rcsc::ServerParam & param = rcsc::ServerParam::i();

    const rcsc::Vector2D target = param.theirTeamGoalPos();

    const Vector2D M_target_point; //!< trapped ball position 

    Vector2D first_vel = M_target_point - wm.ball().pos();
    //first_vel.setLength( action.firstBallSpeed() );

    const Vector2D kick_accel = first_vel - wm.ball().vel();
    const double kick_power = kick_accel.r() / wm.self().kickRate();
    const AngleDeg kick_angle = kick_accel.th() - wm.self().body();

    //agent->debugClient().setTarget( target );

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
    
    //Executa a ação selecionada
 
    switch(acao){

    case 1: // Drible 1
    
        // opponent check with goalie
        if ( ! wm.existOpponentIn( sector, 10, true ) )
        {
            const int max_dash_step
                = wm.self().playerType()
                .cyclesToReachDistance( wm.self().pos().dist( drib_target ) );
            if ( wm.self().pos().x > 35.0 )
            {
                drib_target.y *= ( 10.0 / drib_target.absY() );
            }
         }
         dlog.addText( Logger::TEAM,
                          __FILE__": (execute) fast dribble to (%.1f, %.1f) max_step=%d",
                          drib_target.x, drib_target.y,
                          max_dash_step );
         agent->debugClient().addMessage( "OffKickDrib(2)" );
         Body_Dribble( drib_target,
                          1.0,
                          ServerParam::i().maxDashPower(),
                          std::min( 5, max_dash_step )
                          ).execute( agent );
          agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
    break;

    case 2: // Drible 2

        // opponent check with goalie
        if ( ! wm.existOpponentIn( sector, 10, true ) )
        {
            const int max_dash_step
                = wm.self().playerType()
                .cyclesToReachDistance( wm.self().pos().dist( drib_target ) );
            if ( wm.self().pos().x > 35.0 )
            {
                drib_target.y *= ( 10.0 / drib_target.absY() );
            }
         }
         dlog.addText( Logger::TEAM,
                          __FILE__": (execute) slow dribble to (%.1f, %.1f)",
                          drib_target.x, drib_target.y );
         agent->debugClient().addMessage( "OffKickDrib(3)" );
         Body_Dribble( drib_target,
                          1.0,
                          ServerParam::i().maxDashPower(),
                          2
                          ).execute( agent );
         agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
    break;
   

    case 3: 

 if ( Body_Pass::get_best_pass( wm, &pass_point, NULL, NULL ) )
    {
        if ( pass_point.x > wm.self().pos().x - 1.0 )
        {
            bool safety = true;
            const PlayerPtrCont::const_iterator opps_end = opps.end();
            for ( PlayerPtrCont::const_iterator it = opps.begin();
                  it != opps_end;
                  ++it )
            {
                if ( (*it)->pos().dist( pass_point ) < 2.0 )
                {
                    safety = false;
                }
            }

            if ( safety )
            {
                dlog.addText( Logger::TEAM,
                              __FILE__": (execute) do best pass" );
                agent->debugClient().addMessage( "OffKickPass(1)" );
                Body_Pass().execute( agent );
                agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
                return true;
            }else{
                dlog.addText( Logger::TEAM,
                      __FILE__": hold" );
        agent->debugClient().addMessage( "OffKickHold" );
        Body_HoldBall().execute( agent );
        agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
            }
        }else{
         dlog.addText( Logger::TEAM,
                      __FILE__": hold" );
        agent->debugClient().addMessage( "OffKickHold" );
        Body_HoldBall().execute( agent );
        agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
        }
    }

    break;

    case 4: 

       
  if ( Body_Pass::get_best_pass( wm, &pass_point, NULL, NULL ) )
    {
        if ( pass_point.x > wm.self().pos().x - 1.0 )
        {
            bool safety = true;
            const PlayerPtrCont::const_iterator opps_end = opps.end();
            for ( PlayerPtrCont::const_iterator it = opps.begin();
                  it != opps_end;
                  ++it )
            {
                if ( (*it)->pos().dist( pass_point ) < 4.0 )
                {
                    safety = false;
                }
            }

            if ( safety )
            {
                dlog.addText( Logger::TEAM,
                              __FILE__": (execute) do best pass" );
                agent->debugClient().addMessage( "OffKickPass(1)" );
                Body_Pass().execute( agent );
                agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
                return true;
            }else{

        dlog.addText( Logger::TEAM,
                      __FILE__": clear" );
        agent->debugClient().addMessage( "OffKickAdvance" );
        Body_AdvanceBall().execute( agent );
        agent->setNeckAction( new Neck_ScanField() );
            }
        }else{
        dlog.addText( Logger::TEAM,
                      __FILE__": clear" );
        agent->debugClient().addMessage( "OffKickAdvance" );
        Body_AdvanceBall().execute( agent );
        agent->setNeckAction( new Neck_ScanField() );
         
        }
    }
    break;
     

    case 5: //Passe 3

  if ( Body_Pass::get_best_pass( wm, &pass_point, NULL, NULL ) )
    {
        if ( pass_point.x > wm.self().pos().x - 18.0 )
        {
            bool safety = true;
            const PlayerPtrCont::const_iterator opps_end = opps.end();
            for ( PlayerPtrCont::const_iterator it = opps.begin();
                  it != opps_end;
                  ++it )
            {
                if ( (*it)->pos().dist( pass_point ) < 2.0 )
                {
                    safety = false;
                }
            }

            if (( safety ) || ((wm.self().pos().x>45) && (((wm.self().pos().y > -17.0)&&(wm.self().pos().y < -10.0)) || ((wm.self().pos().y > 10.0)&&(wm.self().pos().y < 17.0))) ))
            {
                dlog.addText( Logger::TEAM,
                              __FILE__": (execute) do best pass" );
                agent->debugClient().addMessage( "OffKickPass(1)" );
                Body_Pass().execute( agent );
                agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
                return true;
            }else{
                dlog.addText( Logger::TEAM,
                      __FILE__": hold" );
        agent->debugClient().addMessage( "OffKickHold" );
        Body_HoldBall().execute( agent );
        agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
            }
        }else{
         dlog.addText( Logger::TEAM,
                      __FILE__": hold" );
        agent->debugClient().addMessage( "OffKickHold" );
        Body_HoldBall().execute( agent );
        agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
        }
    }
      
    break;
     
    //Chutar
    case 6:
         Body_SmartKick( target,
                               param.ballSpeedMax(),
                               param.ballSpeedMax() * 0.96,
                               3 ).execute( agent ) ;
         agent->setNeckAction( new Neck_ScanField() );
    break;
    }

  
    return acao;
}

//Retorna a Recompensa de Acordo com Par Estado/Ação
int 
*Bhv_BasicOffensiveKick::recompensa(PlayerAgent * agent, int acao_q){

    int R_M[24][6] =   {{-1, 0, -1, -1, -1, -1},
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
			{10, 20, 10, 10, 10, 10}};//Linhas = Estados, Colunas = Acoes

    
    int *A_E_R;//Matriz de Acao(A)/Estado(S)/Recompensa(R)/Novo Estado(S')
    A_E_R = (int*) malloc(4 * sizeof(int));
    int estado = Bhv_BasicOffensiveKick().recebeestado(agent);
    int acao = Bhv_BasicOffensiveKick().executaacao(agent, acao_q, estado);
    int novo_estado = Bhv_BasicOffensiveKick().recebeestado(agent);  
    int r = R_M[estado-1][acao-1];
    A_E_R[0] = acao;
    A_E_R[1] = estado;
    A_E_R[2] = r;
    A_E_R[3] = novo_estado;
    return A_E_R;
    
}
    
