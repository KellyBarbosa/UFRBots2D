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
Robocup 2012 - México

*/

///////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sample_player.h"

#include "strategy.h"
#include "field_analyzer.h"

#include "action_chain_holder.h"
#include "sample_field_evaluator.h"

#include "soccer_role.h"

#include "sample_communication.h"
#include "keepaway_communication.h"

#include "bhv_penalty_kick.h"
#include "bhv_set_play.h"
#include "bhv_set_play_kick_in.h"
#include "bhv_set_play_indirect_free_kick.h"

#include "bhv_custom_before_kick_off.h"
#include "bhv_strict_check_shoot.h"

#include "view_tactical.h"

#include "intention_receive.h"

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/bhv_emergency.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/body_kick_one_step.h>
#include <rcsc/action/neck_scan_field.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/view_synch.h>

#include <rcsc/formation/formation.h>
#include <rcsc/action/kick_table.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/player/say_message_builder.h>
#include <rcsc/player/audio_sensor.h>
#include <rcsc/player/freeform_parser.h>

#include <rcsc/common/basic_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/audio_memory.h>
#include <rcsc/common/say_message_parser.h>
// #include <rcsc/common/free_message_parser.h>

#include <rcsc/param/param_map.h>
#include <rcsc/param/cmd_line_parser.h>

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

#include <fstream> 
#include "bhv_basic_offensive_kick.h"
#include <rcsc/player/player_agent.h>


using namespace std;
using std::stringstream;


using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!

 */
SamplePlayer::SamplePlayer()
    : PlayerAgent(),
      M_communication(),
      M_field_evaluator( createFieldEvaluator() ),
      M_action_generator( createActionGenerator() )
{
    boost::shared_ptr< AudioMemory > audio_memory( new AudioMemory );

    M_worldmodel.setAudioMemory( audio_memory );

    //
    // set communication message parser
    //
    addSayMessageParser( SayMessageParser::Ptr( new BallMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new PassMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new InterceptMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new GoalieMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new GoalieAndPlayerMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new OffsideLineMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new DefenseLineMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new WaitRequestMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new PassRequestMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new DribbleMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new BallGoalieMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new OnePlayerMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new TwoPlayerMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new ThreePlayerMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new SelfMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new TeammateMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new OpponentMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new BallPlayerMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new StaminaMessageParser( audio_memory ) ) );
    addSayMessageParser( SayMessageParser::Ptr( new RecoveryMessageParser( audio_memory ) ) );

    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 9 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 8 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 7 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 6 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 5 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 4 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 3 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 2 >( audio_memory ) ) );
    // addSayMessageParser( SayMessageParser::Ptr( new FreeMessageParser< 1 >( audio_memory ) ) );

    //
    // set freeform message parser
    //
    setFreeformParser( FreeformParser::Ptr( new FreeformParser( M_worldmodel ) ) );

    //
    // set action generators
    //
    // M_action_generators.push_back( ActionGenerator::Ptr( new PassGenerator() ) );

    //
    // set communication planner
    //
    M_communication = Communication::Ptr( new SampleCommunication() );
}

/*-------------------------------------------------------------------*/
/*!

 */
SamplePlayer::~SamplePlayer()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SamplePlayer::initImpl( CmdLineParser & cmd_parser )
{
    bool result = PlayerAgent::initImpl( cmd_parser );

    // read additional options
    result &= Strategy::instance().init( cmd_parser );

    rcsc::ParamMap my_params( "Additional options" );
#if 0
    std::string param_file_path = "params";
    param_map.add()
        ( "param-file", "", &param_file_path, "specified parameter file" );
#endif

    cmd_parser.parse( my_params );

    if ( cmd_parser.count( "help" ) > 0 )
    {
        my_params.printHelp( std::cout );
        return false;
    }

    if ( cmd_parser.failed() )
    {
        std::cerr << "player: ***WARNING*** detected unsuppprted options: ";
        cmd_parser.print( std::cerr );
        std::cerr << std::endl;
    }

    if ( ! result )
    {
        return false;
    }

    if ( ! Strategy::instance().read( config().configDir() ) )
    {
        std::cerr << "***ERROR*** Failed to read team strategy." << std::endl;
        return false;
    }

    if ( KickTable::instance().read( config().configDir() + "/kick-table" ) )
    {
        std::cerr << "Loaded the kick table: ["
                  << config().configDir() << "/kick-table]"
                  << std::endl;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!
  main decision
  virtual method in super class
*/
void
SamplePlayer::actionImpl()
{
    //
    // update strategy and analyzer
    //
    Strategy::instance().update( world() );
    FieldAnalyzer::instance().update( world() );
    
    //Verifica se está com posse de bola
    bool kickable = this->world().self().isKickable();
    
    if ( this->world().existKickableTeammate()
         && this->world().teammatesFromBall().front()->distFromBall()
         < this->world().ball().distFromSelf() )
    {
        kickable = false;
    }
   
    //const WorldModel & wm = this->world();
    //int num = wm.self().unum();
  
   // int num = WorldModel().self().unum();

    //Se estiver com posse de bola executa o Aprendizado Por Reforço - Q-Learning
    if ( kickable )
    {        
       int *A_E_R;
       float  Matriz_Q[30][6];
       //A_E_R =  Bhv_BasicOffensiveKick().recompensa(this);
       ofstream arq;// cria objeto de leitura e gravaÃ§Ã£o
       int i =0, j=0, k=0, col_Q = 0, acao_q = 0, estado = 0, a = 0, s = 0, sinal = 1;
       float r = 0, fuzzy = 0; 
       int linhas = 24, colunas = 6;
       //const float gamma = 0.9, alfa = 0.125;
       const float gamma = 0.9, alfa = 0;
       int Q_t_1 = 0;
       float Q_t = 0, maior = -1000000;
       float Q_f;
       int lin = 0, const_linha = 0;
       string l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11, l12, l13, l14, l15, l16, l17, l18, l19, l20, l21; 
       string l22, l23, l24, l25, l26, l27, l28, l29, l30, l31, la, lb, lc, ld, le, lf, lg, lh, lj, li;//strings do arquivo
       estado =  Bhv_BasicOffensiveKick().recebeestado(this);

       for (i = 0; i < linhas; i++) {//Inicilizando a Matriz Q
	     for (j = 0; j < colunas; j++){
                Matriz_Q[i][j] = 0;
         }
       } 
           /////////////Lendo do Arquivo//////////////////
           ifstream fin1("q.txt");
           ifstream fin2("q1.txt");
           ifstream fin3("q2.txt");
           ifstream fin4("q3.txt");
           //ifstream fin5("q4.txt");
           //ifstream fin5("q1.txt");

           getline(fin1, l1);//Linha 1: Numero de Linha e Colunas
           //getline(fin, l1b); 
           getline(fin1, l2);//Linha 2: Estado 1 
           getline(fin1, l3);//Linha 3: Estado 2 
           getline(fin1, l4);//Linha 4: Estado 3
           getline(fin1, l5);//Linha 5: Estado 4
           getline(fin1, l6);//Linha 6: Estado 5
           getline(fin1, l7);//Linha 7: Estado 6
           getline(fin2, l8);//Linha 8: Estado 7 
           getline(fin2, l9);//Linha 9: Estado 8 
           getline(fin2, l10);//Linha 10: Estado 9
           getline(fin2, l11);//Linha 11: Estado 10
           getline(fin2, l12);//Linha 12: Estado 11
           getline(fin2, l13);//Linha 13: Estado 12
           getline(fin3, l14);//Linha 14: Estado 13 
           getline(fin3, l15);//Linha 15: Estado 14 
           getline(fin3, l16);//Linha 16: Estado 15
           getline(fin3, l17);//Linha 17: Estado 16
           getline(fin3, l18);//Linha 18: Estado 17
           getline(fin3, l19);//Linha 19: Estado 18
           getline(fin4, l20);//Linha 20: Estado 19 
           getline(fin4, l21);//Linha 21: Estado 20 
           getline(fin4, l22);//Linha 22: Estado 21
           getline(fin4, l23);//Linha 23: Estado 22
           getline(fin4, l24);//Linha 24: Estado 23
           getline(fin4, l25);//Linha 25: Estado 24
           /*getline(fin5, l26);//Linha 26: Estado 25 
           getline(fin5, l27);//Linha 27: Estado 26 
           getline(fin5, l28);//Linha 28: Estado 27
           getline(fin5, l29);//Linha 29: Estado 28
           getline(fin5, l30);//Linha 30: Estado 29
           getline(fin5, l31);//Linha 31: Estado 30*/

           ///Lendo Parametros gravados no instante anterior no arquivo///
           
           la = l1[0]; //Acao

           stringstream sa (la);
           sa >> a;

           lc = l1[2]; //Estado
           ld = l1[3]; 
           lc = lc + ld;
 
           stringstream ss (lc);
           ss >> s;

           le = l1[4]; //Reforco
           lf = l1[5];
           lg = l1[6];
           lh = l1[7];
           li = l1[8];
           lj = l1[9];  
           le = le + lf + lg + lh + li + lj; 

           stringstream sr (le);
           sr >> r;

           //if((s == 1)||(s == 2)||(s == 3)||(s == 4)||(s == 5)||(s == 6)||(estado==1)){
           for (k=0; k<42; k++){ //Estado 1
                la = l2[k]; 
                lb = l2[k+1];
                lc = l2[k+2];
                ld = l2[k+3];
	        le = l2[k+4];
		lf = l2[k+5]; 
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }           
                    stringstream s1 (la);
                    s1 >> Matriz_Q[0][col_Q];
 		    col_Q++;
                }
           }    
	   col_Q = 0;
           //}
   
    
           //if((s == 1)||(s == 2)||(s == 3)||(s == 4)||(s == 5)||(s == 6)||(estado==2)){
           for (k=0; k<42; k++){ //Estado 2
                la = l3[k];       
                lb = l3[k+1]; 
		lc = l3[k+2];
		ld = l3[k+3];   
	        le = l3[k+4];
		lf = l3[k+5];
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s2 (la);
                    s2 >> Matriz_Q[1][col_Q];
		    col_Q++;
                }
           } 
           col_Q = 0;
           //}

           //if((s == 1)||(s == 2)||(s == 3)||(s == 4)||(s == 5)||(s == 6)||(estado==3)){
           for (k=0; k<42; k++){ //Estado 3
                la = l4[k];
                lb = l4[k+1];
	        lc = l4[k+2];
		ld = l4[k+3];      
	        le = l4[k+4];
		lf = l4[k+5];   
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s3 (la);
                    s3 >> Matriz_Q[2][col_Q];
		    col_Q++;
                }
           } 
	   col_Q = 0;//}

           //if((s == 1)||(s == 2)||(s == 3)||(s == 4)||(s == 5)||(s == 6)||(estado==4)){
           for (k=0; k<42; k++){ //Estado 4
                la = l5[k];
                lb = l5[k+1];    
		lc = l5[k+2];
		ld = l5[k+3];    
	        le = l5[k+4];
		lf = l5[k+5];   
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s4 (la);
                    s4 >> Matriz_Q[3][col_Q];
		    col_Q++;
                }
           } 
           col_Q = 0;//}

          // if((s == 1)||(s == 2)||(s == 3)||(s == 4)||(s == 5)||(s == 6)||(estado==5)){
           for (k=0; k<42; k++){ //Estado 5
                la = l6[k];
                lb = l6[k+1];
	        lc = l6[k+2];
		ld = l6[k+3];  
	        le = l6[k+4];
		lf = l6[k+5];          
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s5 (la);
                    s5 >> Matriz_Q[4][col_Q];
		    col_Q++;
                }
           }
	   col_Q = 0;//} 

           //if((s == 1)||(s == 2)||(s == 3)||(s == 4)||(s == 5)||(s == 6)||(estado==6)){
           for (k=0; k<42; k++){ //Estado 6
                la = l7[k];
                lb = l7[k+1];
	        lc = l7[k+2];
		ld = l7[k+3];
	        le = l7[k+4];
		lf = l7[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s6 (la);
                    s6 >> Matriz_Q[5][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;//}

           if((s == 7)||(s == 8)||(s == 9)||(s == 10)||(s == 11)||(s == 12)||(estado==7)){
           for (k=0; k<42; k++){ //Estado 7
                la = l8[k]; 
                lb = l8[k+1];
                lc = l8[k+2];
                ld = l8[k+3];
	        le = l8[k+4];
		lf = l8[k+5]; 
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }          
                    stringstream s7 (la);
                    s7 >> Matriz_Q[6][col_Q];
 		    col_Q++;
                }
           }    
	   col_Q = 0;}

           if((s == 7)||(s == 8)||(s == 9)||(s == 10)||(s == 11)||(s == 12)||(estado==8)){
           for (k=0; k<42; k++){ //Estado 8
                la = l9[k];       
                lb = l9[k+1]; 
		lc = l9[k+2];
		ld = l9[k+3];   
	        le = l9[k+4];
		lf = l9[k+5];
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s8 (la);
                    s8 >> Matriz_Q[7][col_Q];
		    col_Q++;
                }
           } 
           col_Q = 0;}

           if((s == 7)||(s == 8)||(s == 9)||(s == 10)||(s == 11)||(s == 12)||(estado==9)){
           for (k=0; k<42; k++){ //Estado 9
                la = l10[k];
                lb = l10[k+1];
	        lc = l10[k+2];
		ld = l10[k+3];      
	        le = l10[k+4];
		lf = l10[k+5];   
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s9 (la);
                    s9 >> Matriz_Q[8][col_Q];
		    col_Q++;
                }
           } 
	   col_Q = 0;}

 
           if((s == 7)||(s == 8)||(s == 9)||(s == 10)||(s == 11)||(s == 12)||(estado==10)){
           for (k=0; k<42; k++){ //Estado 10
                la = l11[k];
                lb = l11[k+1];    
		lc = l11[k+2];
		ld = l11[k+3];    
	        le = l11[k+4];
		lf = l11[k+5];   
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s10 (la);
                    s10 >> Matriz_Q[9][col_Q];
		    col_Q++;
                }
           } 
           col_Q = 0;}

           if((s == 7)||(s == 8)||(s == 9)||(s == 10)||(s == 11)||(s == 12)||(estado==11)){
           for (k=0; k<42; k++){ //Estado 11
                la = l12[k];
                lb = l12[k+1];
	        lc = l12[k+2];
		ld = l12[k+3];  
	        le = l12[k+4];
		lf = l12[k+5];          
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s11 (la);
                    s11 >> Matriz_Q[10][col_Q];
		    col_Q++;
                }
           }
	   col_Q = 0;} 

           if((s == 7)||(s == 8)||(s == 9)||(s == 10)||(s == 11)||(s == 12)||(estado==12)){
           for (k=0; k<42; k++){ //Estado 12
                la = l13[k];
                lb = l13[k+1];
	        lc = l13[k+2];
		ld = l13[k+3];
	        le = l13[k+4];
		lf = l13[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s12 (la);
                    s12 >> Matriz_Q[11][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 13)||(s == 14)||(s == 15)||(s == 16)||(s == 17)||(s == 18)||(estado==13)){
           for (k=0; k<42; k++){ //Estado 13
                la = l14[k];
                lb = l14[k+1];
	        lc = l14[k+2];
		ld = l14[k+3];
	        le = l14[k+4];
		lf = l14[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s13 (la);
                    s13 >> Matriz_Q[12][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 13)||(s == 14)||(s == 15)||(s == 16)||(s == 17)||(s == 18)||(estado==14)){
           for (k=0; k<42; k++){ //Estado 14
                la = l15[k];
                lb = l15[k+1];
	        lc = l15[k+2];
		ld = l15[k+3];
	        le = l15[k+4];
		lf = l15[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s14 (la);
                    s14 >> Matriz_Q[13][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 13)||(s == 14)||(s == 15)||(s == 16)||(s == 17)||(s == 18)||(estado==15)){
           for (k=0; k<42; k++){ //Estado 15
                la = l16[k];
                lb = l16[k+1];
	        lc = l16[k+2];
		ld = l16[k+3];
	        le = l16[k+4];
		lf = l16[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s15 (la);
                    s15 >> Matriz_Q[14][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 13)||(s == 14)||(s == 15)||(s == 16)||(s == 17)||(s == 18)||(estado==16)){
           for (k=0; k<42; k++){ //Estado 16
                la = l17[k];
                lb = l17[k+1];
	        lc = l17[k+2];
		ld = l17[k+3];
	        le = l17[k+4];
		lf = l17[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s16 (la);
                    s16 >> Matriz_Q[15][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 13)||(s == 14)||(s == 15)||(s == 16)||(s == 17)||(s == 18)||(estado==17)){
           for (k=0; k<42; k++){ //Estado 17
                la = l18[k];
                lb = l18[k+1];
	        lc = l18[k+2];
		ld = l18[k+3];
	        le = l18[k+4];
		lf = l18[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s17 (la);
                    s17 >> Matriz_Q[16][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 13)||(s == 14)||(s == 15)||(s == 16)||(s == 17)||(s == 18)||(estado==18)){
           for (k=0; k<42; k++){ //Estado 18
                la = l19[k];
                lb = l19[k+1];
	        lc = l19[k+2];
		ld = l19[k+3];
	        le = l19[k+4];
		lf = l19[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s18 (la);
                    s18 >> Matriz_Q[17][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 19)||(s == 20)||(s == 21)||(s == 22)||(s == 23)||(s == 24)||(estado==19)){
           for (k=0; k<42; k++){ //Estado 19
                la = l20[k];
                lb = l20[k+1];
	        lc = l20[k+2];
		ld = l20[k+3];
	        le = l20[k+4];
		lf = l20[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s19 (la);
                    s19 >> Matriz_Q[18][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 19)||(s == 20)||(s == 21)||(s == 22)||(s == 23)||(s == 24)||(estado==20)){
           for (k=0; k<42; k++){ //Estado 20
                la = l21[k];
                lb = l21[k+1];
	        lc = l21[k+2];
		ld = l21[k+3];
	        le = l21[k+4];
		lf = l21[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s20 (la);
                    s20 >> Matriz_Q[19][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 19)||(s == 20)||(s == 21)||(s == 22)||(s == 23)||(s == 24)||(estado==21)){
           for (k=0; k<42; k++){ //Estado 21
                la = l22[k];
                lb = l22[k+1];
	        lc = l22[k+2];
		ld = l22[k+3];
	        le = l22[k+4];
		lf = l22[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s21 (la);
                    s21 >> Matriz_Q[20][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 19)||(s == 20)||(s == 21)||(s == 22)||(s == 23)||(s == 24)||(estado==22)){
           for (k=0; k<42; k++){ //Estado 22
                la = l23[k];
                lb = l23[k+1];
	        lc = l23[k+2];
		ld = l23[k+3];
	        le = l23[k+4];
		lf = l23[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s22 (la);
                    s22 >> Matriz_Q[21][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 19)||(s == 20)||(s == 21)||(s == 22)||(s == 23)||(s == 24)||(estado==23)){
           for (k=0; k<42; k++){ //Estado 23
                la = l24[k];
                lb = l24[k+1];
	        lc = l24[k+2];
		ld = l24[k+3];
	        le = l24[k+4];
		lf = l24[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s23 (la);
                    s23 >> Matriz_Q[22][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}

           if((s == 19)||(s == 20)||(s == 21)||(s == 22)||(s == 23)||(s == 24)||(estado==24)){
           for (k=0; k<42; k++){ //Estado 24
                la = l25[k];
                lb = l25[k+1];
	        lc = l25[k+2];
		ld = l25[k+3];
	        le = l25[k+4];
		lf = l25[k+5];           
	        if((la.compare("0") == 0)||(la.compare("1") == 0)||(la.compare("2") == 0)||(la.compare("3") == 0)||(la.compare("4") == 0)||(la.compare("5") == 0)||(la.compare("6") == 0)||(la.compare("7") == 0)||(la.compare("8") == 0)||(la.compare("9") == 0)||(la.compare("-") == 0)){                 
	            if((lb.compare("0") == 0)||(lb.compare("1") == 0)||(lb.compare("2") == 0)||(lb.compare("3") == 0)||(lb.compare("4") == 0)||(lb.compare("5") == 0)||(lb.compare("6") == 0)||(lb.compare("7") == 0)||(lb.compare("8") == 0)||(lb.compare("9") == 0)||(lb.compare(".") == 0)||(lb.compare("-") == 0)){
                        la = la + lb;
                        k++;
                        if((lc.compare("0") == 0)||(lc.compare("1") == 0)||(lc.compare("2") == 0)||(lc.compare("3") == 0)||(lc.compare("4") == 0)||(lc.compare("5") == 0)||(lc.compare("6") == 0)||(lc.compare("7") == 0)||(lc.compare("8") == 0)||(lc.compare("9") == 0)||(lc.compare(".") == 0)||(lc.compare("-") == 0)){
                           la = la + lc;
                           k++;
                           if((ld.compare("0") == 0)||(ld.compare("1") == 0)||(ld.compare("2") == 0)||(ld.compare("3") == 0)||(ld.compare("4") == 0)||(ld.compare("5") == 0)||(ld.compare("6") == 0)||(ld.compare("7") == 0)||(ld.compare("8") == 0)||(ld.compare("9") == 0)||(ld.compare(".") == 0)||(ld.compare("-") == 0)){
                                la = la + ld;
                                k++;
                                if((le.compare("0") == 0)||(le.compare("1") == 0)||(le.compare("2") == 0)||(le.compare("3") == 0)||(le.compare("4") == 0)||(le.compare("5") == 0)||(le.compare("6") == 0)||(le.compare("7") == 0)||(le.compare("8") == 0)||(le.compare("9") == 0)||(le.compare(".") == 0)||(le.compare("-") == 0)){
                                      la = la + le;
                                      k++;
                                      if((lf.compare("0") == 0)||(lf.compare("1") == 0)||(lf.compare("2") == 0)||(lf.compare("3") == 0)||(lf.compare("4") == 0)||(lf.compare("5") == 0)||(lf.compare("6") == 0)||(lf.compare("7") == 0)||(lf.compare("8") == 0)||(lf.compare("9") == 0)||(lf.compare("-") == 0)){
                                           la = la + lf;
                                           k++;
                                      }
                                 }
                             }
                         }  
                    }
                    stringstream s24 (la);
                    s24 >> Matriz_Q[23][col_Q];
		    col_Q++;
                }
           }   
           col_Q = 0;}
   
          fin1.close();  //Fecha o arquivo  
          fin2.close();  //Fecha o arquivo  
          fin3.close();  //Fecha o arquivo  
          fin4.close();  //Fecha o arquivo  
          //fin5.close();  //Fecha o arquivo        

         /*///MOSTRA MATRIZ Q NA TELA/////////////////////
         for (i = 0; i < linhas; i++) {
	     for (j = 0; j < colunas; j++){
                cout<<Matriz_Q[i][j]<<" ";
                if(j==(colunas - 1)){
                       cout<<" "<<endl;//pula linha do arquivo
                }
             }
          } */
          //cout<<"==============="<<endl;
           //////////////////////////////////////////////

          //////VERIFICA QUAL A ACAO TEM O MAIOR VALOR DE Q O ESTADO (Tomada de Decisao)///////////////////////
          for (i = 0; i < linhas; i++) {
	     for (j = 0; j < colunas; j++){
                if(i == (s-1)){ //Se i igual ao estado s
                     if(Matriz_Q[i][j]>maior){
                         acao_q = j+1;
                         maior = Matriz_Q[i][j];
                     }  
                }
             }
           }
           maior = -1000000;
           //cout<<"Estado:"<<s<<" Acao:"<<a<<" Recompensa:"<<r<<endl;
          /////////////////////////////////////////////////////////////////////

          ///////////////////////////////////////////////////////////////////////////////////
          A_E_R =  Bhv_BasicOffensiveKick().recompensa(this, acao_q);
          A_E_R[1] = estado;
          /*if(A_E_R[2] == -100){ 
             fuzzy = Bhv_BasicOffensiveKick().calculafuzzy(this); 
          }*/
          /////////////////////////////////////////////////////////////////////////////

          //////VERIFICA QUAL O MAIOR VALOR DE ACAO PARA O ESTADO (Para Calculo de Qts+1)///////////////////////
          for (i = 0; i < linhas; i++) {
	     for (j = 0; j < colunas; j++){
                if(i == (A_E_R[1]-1)){ //Se i igual a novo estado s'
                     if(Matriz_Q[i][j]>maior){
                         maior = Matriz_Q[i][j];
                     }  
                }
             }
           }

           //cout<<"S:"<<s<<"A:"<<A_E_R[0]<<"Acao Q:"<<acao_q<<endl;
	   //cout<<"Novo Estado:"<<A_E_R[1]<<"-----"<<"Maior Valor:"<<maior<<endl; 	

           //cout<<"S:"<<s<<"|| A:"<<a<<"|| R(S,A):"<<r<<endl;
	   //cout<<"S':"<<A_E_R[1]<<"|| A':"<<A_E_R[0]<<"|| R'(S',A'):"<<A_E_R[2]<<endl;
           //cout<<"Acao Q:"<<acao_q<<"|| Maior Valor:"<<maior<<"|| Q(S',A'):"<<Matriz_Q[A_E_R[1]-1][A_E_R[0]-1]<<endl; 	

  
          //////////////////////////////////////////////////////////////////////////          

          //////////CALCULO DO VALOR DE Qt+1////////////////////////////////////////

          // Qt+1 (st , at ) = Qt (st , at ) + Î±[rt + Î³Vt (st+1 ) âˆ’ Qt (st , at )]
          //V(s) Ã© o valor mÃ¡ximo de Q(s,a) de um estado s, para todas as aÃ§Ãµes possÃ­veis de serem executadas nesse estado (Celiberto Jr, 2006).

          Q_t = Matriz_Q[s-1][a-1];
          
          
          //Q-learning
          Q_t_1 = Q_t + alfa*(r+gamma*maior-Q_t); //Valor inteiro
          Q_f = Q_t + alfa*(r+gamma*maior-Q_t);  //Valor float


          //SARSA

          //Q_t_1 = Q_t + alfa*(r+gamma*Matriz_Q[A_E_R[1]-1][A_E_R[0]-1]-Q_t); //Valor inteiro
          //Q_f = Q_t + alfa*(r+gamma*Matriz_Q[A_E_R[1]-1][A_E_R[0]-1]-Q_t);  //Valor float

          //
             //cout<<"Qt+1:"<<Q_t_1<<"Q_f:"<<Q_f<<endl;
          Q_f *= 10;
         Q_f = (int) Q_f;
    

          /*if(Q_f<0){
            sinal = -1;
          }else{
            sinal = 1;
          }
          
        //  modulo = (((Q_f- Q_t_1)^2))^(0.5);

          if ((Q_f - Q_t_1)>(0.95*sinal)){ //Arrendodamento
             Q_f = Q_t_1 +(1*sinal);   
          }else{
             if ((Q_f - Q_t_1)>(0.85*sinal)){ //Arrendodamento
               Q_f = Q_t_1 +(0.9*sinal);   
             }else{
                if ((Q_f - Q_t_1)>(0.75*sinal)){ 
                  Q_f = Q_t_1 +(0.8*sinal);   
                }else{
                   if ((Q_f - Q_t_1)>(0.65*sinal)){ 
                      Q_f = Q_t_1 +(0.7*sinal);
                   }else{    
                      if ((Q_f - Q_t_1)>(0.55*sinal)){ 
                        Q_f = Q_t_1 +(0.6*sinal);
                      }else{
                         if ((Q_f - Q_t_1)>(0.45*sinal)){ 
                           Q_f = Q_t_1 +(0.5*sinal);   
                         }else{
                            if ((Q_f - Q_t_1)>(0.35*sinal)){ 
                              Q_f = Q_t_1 +(0.4*sinal);
                            }else{   
                                if ((Q_f - Q_t_1)>(0.25*sinal)){ 
                                  Q_f = Q_t_1 +(0.3*sinal);   
                                }else{
                                   if ((Q_f - Q_t_1)>(0.15*sinal)){ 
                                      Q_f = Q_t_1 +(0.2*sinal);   
                                   }else{
                                      if ((Q_f - Q_t_1)>(0.05*sinal)){                                
                                         Q_f = Q_t_1 +(0.1*sinal);   
                                      }else{
                                         Q_f = Q_t_1;
                                      }
                                   }
                                }
                             }
                          }
                       }
                    }
                  }
               } 
           }    */
	  // cout<<"Q_t:"<<Q_t<<"Q_t_1:"<<Q_f<<endl;
          // cout<<"================================="<<endl;
         ///////////////////////////////////////////////////////////////////
 
         //////GRAVA OS VALORES DA MATRIZ Q NO ARQUIVO///////////////////////////////////////////////////// 
	 arq.open("q.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo

         if(A_E_R[2] == -100){
            if(estado<10){
               arq << A_E_R[0] << " " << A_E_R[1] << " " << fuzzy <<endl;
            }else{
               arq << A_E_R[0] << " " << A_E_R[1] << fuzzy <<endl;
            }
         }else{
            if(estado<10){
               //arq << A_E_R[0] << " " << A_E_R[1] << " " << A_E_R[2] <<endl;
               arq << 0 << " " << 0 << " " << 0 <<endl;
            }else{
              // arq << A_E_R[0] << " " << A_E_R[1] << A_E_R[2] <<endl;
               arq << 0 << " " << 0 << " " << 0 <<endl;
            }
	 }
	 for (i = 0; i < 6; i++) {
		for (j = 0; j < colunas; j++){
                   if ((i == (s-1))&&(j == (a-1))){
                      arq <<Q_f<<" ";
                   }else{
                      arq <<Matriz_Q[i][j]<<" ";//Aqui deve ficar o valor do SxA pegado na abertura do arquivo fin
                   } 
                   if(j==(colunas - 1)){
                       arq<<" "<<endl;//pula linha do arquivo
                   }
                }
         }         

         arq.close();

         switch(s){
            case 1:
               //arq.open("q.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 1; 
            break;
            case 2:
               //arq.open("q.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 2;
            break;
            case 3:
               //arq.open("q.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 3;
            break;
            case 4:
               //arq.open("q.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 4;
            break;
            case 5:
               //arq.open("q.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 5;
            break;
            case 6:
               //arq.open("q.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 6;
            break;
            case 7:
               arq.open("q1.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 1;
               const_linha = 6;
            break;
            case 8:
               arq.open("q1.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 2;
               const_linha = 6;
            break;
            case 9:
               arq.open("q1.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 3;
               const_linha = 6;
            break;
            case 10:
               arq.open("q1.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 4;
               const_linha = 6;
            break;
            case 11:
               arq.open("q1.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo
               lin = 5;
               const_linha = 6;
            break;
            case 12:
               arq.open("q1.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 6;
               const_linha = 6;
            break;
            case 13:
               arq.open("q2.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 1;
               const_linha = 12;
            break;
            case 14:
               arq.open("q2.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 2;
               const_linha = 12;
            break;
            case 15:
               arq.open("q2.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 3;
               const_linha = 12;
            break;
            case 16:
               arq.open("q2.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 4;
               const_linha = 12;
            break;
            case 17:
               arq.open("q2.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo
               lin = 5; 
               const_linha = 12;
            break;
            case 18:
               arq.open("q2.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 6;
               const_linha = 12;
            break;
            case 19:
               arq.open("q3.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 1;
               const_linha = 18;
            break;
            case 20:
               arq.open("q3.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 2;
               const_linha = 18;
            break;
            case 21:
               arq.open("q3.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 3;
               const_linha = 18;
            break;
            case 22:
               arq.open("q3.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 4;
               const_linha = 18;
            break;
            case 23:
               arq.open("q3.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo
               lin = 5; 
               const_linha = 18;
            break;
            case 24:
               arq.open("q3.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo
               lin = 6; 
               const_linha = 18;
            break;
            /*case 25:
               arq.open("q4.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 1;
               const_linha = 24;
            break;
            case 26:
               arq.open("q4.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 2;
               const_linha = 24;
            break;
            case 27:
               arq.open("q4.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 3;
               const_linha = 24;
            break;
            case 28:
               arq.open("q4.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 4;
               const_linha = 24;
            break;
            case 29:
               arq.open("q4.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 5;
               const_linha = 24;
            break;
            case 30:
               arq.open("q4.txt", ios::ate | ios::out | ios::in | ios::trunc);// abre o arquivo 
               lin = 6; 
               const_linha = 24;
            break;*/
 
         } 
         
         linhas = const_linha + 6;
	 for (i = (const_linha); i < linhas; i++) {
		for (j = 0; j < colunas; j++){
                   if ((i == (s-1))&&(j == (a-1))){
                      arq <<Q_f<<" ";
                   }else{
                      arq <<Matriz_Q[i][j]<<" ";//Aqui deve ficar o valor do SxA pegado na abertura do arquivo fin
                   } 
                   if(j==(colunas - 1)){
                       arq<<" "<<endl;//pula linha do arquivo
                   }
                }
          }
        

          arq.close();
         //////////////////////////////////////////////////////////////////////////
    }
 

    //
    // prepare action chain
    //
    M_field_evaluator = createFieldEvaluator();
    M_action_generator = createActionGenerator();

    ActionChainHolder::instance().setFieldEvaluator( M_field_evaluator );
    ActionChainHolder::instance().setActionGenerator( M_action_generator );

    //
    // special situations (tackle, objects accuracy, intention...)
    //
    if ( doPreprocess() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": preprocess done" );
        return;
    }

    //
    // update action chain
    //
    ActionChainHolder::instance().update( world() );


    //
    // create current role
    //
    SoccerRole::Ptr role_ptr;
    {
        role_ptr = Strategy::i().createRole( world().self().unum(), world() );

        if ( ! role_ptr )
        {
            std::cerr << config().teamName() << ": "
                      << world().self().unum()
                      << " Error. Role is not registerd.\nExit ..."
                      << std::endl;
            M_client->setServerAlive( false );
            return;
        }
    }


    //
    // override execute if role accept
    //
    if ( role_ptr->acceptExecution( world() ) )
    {
        role_ptr->execute( this );
        return;
    }


    //
    // play_on mode
    //
    if ( world().gameMode().type() == GameMode::PlayOn )
    {
        role_ptr->execute( this );
        return;
    }


    //
    // penalty kick mode
    //
    if ( world().gameMode().isPenaltyKickMode() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": penalty kick" );
        Bhv_PenaltyKick().execute( this );
        return;
    }

    //
    // other set play mode
    //
    Bhv_SetPlay().execute( this );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handleActionStart()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handleActionEnd()
{
    if ( world().self().posValid() )
    {
#if 0
        const ServerParam & SP = ServerParam::i();
        //
        // inside of pitch
        //

        // top,lower
        debugClient().addLine( Vector2D( world().ourOffenseLineX(),
                                         -SP.pitchHalfWidth() ),
                               Vector2D( world().ourOffenseLineX(),
                                         -SP.pitchHalfWidth() + 3.0 ) );
        // top,lower
        debugClient().addLine( Vector2D( world().ourDefenseLineX(),
                                         -SP.pitchHalfWidth() ),
                               Vector2D( world().ourDefenseLineX(),
                                         -SP.pitchHalfWidth() + 3.0 ) );

        // bottom,upper
        debugClient().addLine( Vector2D( world().theirOffenseLineX(),
                                         +SP.pitchHalfWidth() - 3.0 ),
                               Vector2D( world().theirOffenseLineX(),
                                         +SP.pitchHalfWidth() ) );
        //
        debugClient().addLine( Vector2D( world().offsideLineX(),
                                         world().self().pos().y - 15.0 ),
                               Vector2D( world().offsideLineX(),
                                         world().self().pos().y + 15.0 ) );

        // outside of pitch

        // top,upper
        debugClient().addLine( Vector2D( world().ourOffensePlayerLineX(),
                                         -SP.pitchHalfWidth() - 3.0 ),
                               Vector2D( world().ourOffensePlayerLineX(),
                                         -SP.pitchHalfWidth() ) );
        // top,upper
        debugClient().addLine( Vector2D( world().ourDefensePlayerLineX(),
                                         -SP.pitchHalfWidth() - 3.0 ),
                               Vector2D( world().ourDefensePlayerLineX(),
                                         -SP.pitchHalfWidth() ) );
        // bottom,lower
        debugClient().addLine( Vector2D( world().theirOffensePlayerLineX(),
                                         +SP.pitchHalfWidth() ),
                               Vector2D( world().theirOffensePlayerLineX(),
                                         +SP.pitchHalfWidth() + 3.0 ) );
        // bottom,lower
        debugClient().addLine( Vector2D( world().theirDefensePlayerLineX(),
                                         +SP.pitchHalfWidth() ),
                               Vector2D( world().theirDefensePlayerLineX(),
                                         +SP.pitchHalfWidth() + 3.0 ) );
#else
        // top,lower
        debugClient().addLine( Vector2D( world().ourDefenseLineX(),
                                         world().self().pos().y - 2.0 ),
                               Vector2D( world().ourDefenseLineX(),
                                         world().self().pos().y + 2.0 ) );

        //
        debugClient().addLine( Vector2D( world().offsideLineX(),
                                         world().self().pos().y - 15.0 ),
                               Vector2D( world().offsideLineX(),
                                         world().self().pos().y + 15.0 ) );
#endif
    }

    //
    // ball position & velocity
    //
    dlog.addText( Logger::WORLD,
                  "WM: BALL pos=(%lf, %lf), vel=(%lf, %lf, r=%lf, ang=%lf)",
                  world().ball().pos().x,
                  world().ball().pos().y,
                  world().ball().vel().x,
                  world().ball().vel().y,
                  world().ball().vel().r(),
                  world().ball().vel().th().degree() );


    dlog.addText( Logger::WORLD,
                  "WM: SELF move=(%lf, %lf, r=%lf, th=%lf)",
                  world().self().lastMove().x,
                  world().self().lastMove().y,
                  world().self().lastMove().r(),
                  world().self().lastMove().th().degree() );

    Vector2D diff = world().ball().rpos() - world().ball().rposPrev();
    dlog.addText( Logger::WORLD,
                  "WM: BALL rpos=(%lf %lf) prev_rpos=(%lf %lf) diff=(%lf %lf)",
                  world().ball().rpos().x,
                  world().ball().rpos().y,
                  world().ball().rposPrev().x,
                  world().ball().rposPrev().y,
                  diff.x,
                  diff.y );

    Vector2D ball_move = diff + world().self().lastMove();
    Vector2D diff_vel = ball_move * ServerParam::i().ballDecay();
    dlog.addText( Logger::WORLD,
                  "---> ball_move=(%lf %lf) vel=(%lf, %lf, r=%lf, th=%lf)",
                  ball_move.x,
                  ball_move.y,
                  diff_vel.x,
                  diff_vel.y,
                  diff_vel.r(),
                  diff_vel.th().degree() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handleServerParam()
{
    if ( KickTable::instance().createTables() )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << " KickTable created."
                  << std::endl;
    }
    else
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << " KickTable failed..."
                  << std::endl;
        M_client->setServerAlive( false );
    }


    if ( ServerParam::i().keepawayMode() )
    {
        std::cerr << "set Keepaway mode communication." << std::endl;
        M_communication = Communication::Ptr( new KeepawayCommunication() );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handlePlayerParam()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
SamplePlayer::handlePlayerType()
{

}

/*-------------------------------------------------------------------*/
/*!
  communication decision.
  virtual method in super class
*/
void
SamplePlayer::communicationImpl()
{
    if ( M_communication )
    {
        M_communication->execute( this );
    }
}

/*-------------------------------------------------------------------*/
/*!
*/
bool
SamplePlayer::doPreprocess()
{
    // check tackle expires
    // check self position accuracy
    // ball search
    // check queued intention
    // check simultaneous kick

    const WorldModel & wm = this->world();

    dlog.addText( Logger::TEAM,
                  __FILE__": (doPreProcess)" );

    //
    // freezed by tackle effect
    //
    if ( wm.self().isFrozen() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": tackle wait. expires= %d",
                      wm.self().tackleExpires() );
        // face neck to ball
        this->setViewAction( new View_Tactical() );
        this->setNeckAction( new Neck_TurnToBallOrScan() );
        return true;
    }

    //
    // BeforeKickOff or AfterGoal. jump to the initial position
    //
    if ( wm.gameMode().type() == GameMode::BeforeKickOff
         || wm.gameMode().type() == GameMode::AfterGoal_ )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": before_kick_off" );
        Vector2D move_point =  Strategy::i().getPosition( wm.self().unum() );
        Bhv_CustomBeforeKickOff( move_point ).execute( this );
        this->setViewAction( new View_Tactical() );
        return true;
    }

    //
    // self localization error
    //
    if ( ! wm.self().posValid() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": invalid my pos" );
        Bhv_Emergency().execute( this ); // includes change view
        return true;
    }

    //
    // ball localization error
    //
    const int count_thr = ( wm.self().goalie()
                            ? 10
                            : 5 );
    if ( wm.ball().posCount() > count_thr
         || ( wm.gameMode().type() != GameMode::PlayOn
              && wm.ball().seenPosCount() > count_thr + 10 ) )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": search ball" );
        this->setViewAction( new View_Tactical() );
        Bhv_NeckBodyToBall().execute( this );
        return true;
    }

    //
    // set default change view
    //

    this->setViewAction( new View_Tactical() );

    //
    // check shoot chance
    //
    if ( doShoot() )
    {
        return true;
    }

    //
    // check queued action
    //
    if ( this->doIntention() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": do queued intention" );
        return true;
    }

    //
    // check simultaneous kick
    //
    if ( doForceKick() )
    {
        return true;
    }

    //
    // check pass message
    //
    if ( doHeardPassReceive() )
    {
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SamplePlayer::doShoot()
{
    const WorldModel & wm = this->world();

    if ( wm.gameMode().type() != GameMode::IndFreeKick_
         && wm.time().stopped() == 0
         && wm.self().isKickable()
         && Bhv_StrictCheckShoot().execute( this ) )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": shooted" );

        // reset intention
        this->setIntention( static_cast< SoccerIntention * >( 0 ) );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SamplePlayer::doForceKick()
{
    const WorldModel & wm = this->world();

    if ( wm.gameMode().type() == GameMode::PlayOn
         && ! wm.self().goalie()
         && wm.self().isKickable()
         && wm.existKickableOpponent() )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": simultaneous kick" );
        this->debugClient().addMessage( "SimultaneousKick" );
        Vector2D goal_pos( ServerParam::i().pitchHalfLength(), 0.0 );

        if ( wm.self().pos().x > 36.0
             && wm.self().pos().absY() > 10.0 )
        {
            goal_pos.x = 45.0;
            dlog.addText( Logger::TEAM,
                          __FILE__": simultaneous kick cross type" );
        }
        Body_KickOneStep( goal_pos,
                          ServerParam::i().ballSpeedMax()
                          ).execute( this );
        this->setNeckAction( new Neck_ScanField() );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SamplePlayer::doHeardPassReceive()
{
    const WorldModel & wm = this->world();

    if ( wm.audioMemory().passTime() != wm.time()
         || wm.audioMemory().pass().empty()
         || wm.audioMemory().pass().front().receiver_ != wm.self().unum() )
    {

        return false;
    }

    int self_min = wm.interceptTable()->selfReachCycle();
    Vector2D intercept_pos = wm.ball().inertiaPoint( self_min );
    Vector2D heard_pos = wm.audioMemory().pass().front().receive_pos_;

    dlog.addText( Logger::TEAM,
                  __FILE__":  (doHeardPassReceive) heard_pos(%.2f %.2f) intercept_pos(%.2f %.2f)",
                  heard_pos.x, heard_pos.y,
                  intercept_pos.x, intercept_pos.y );

    if ( ! wm.existKickableTeammate()
         && wm.ball().posCount() <= 1
         && wm.ball().velCount() <= 1
         && self_min < 20
         //&& intercept_pos.dist( heard_pos ) < 3.0 ) //5.0 )
         )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": (doHeardPassReceive) intercept cycle=%d. intercept",
                      self_min );
        this->debugClient().addMessage( "Comm:Receive:Intercept" );
        Body_Intercept().execute( this );
        this->setNeckAction( new Neck_TurnToBall() );
    }
    else
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": (doHeardPassReceive) intercept cycle=%d. go to receive point",
                      self_min );
        this->debugClient().setTarget( heard_pos );
        this->debugClient().addMessage( "Comm:Receive:GoTo" );
        Body_GoToPoint( heard_pos,
                    0.5,
                        ServerParam::i().maxDashPower()
                        ).execute( this );
        this->setNeckAction( new Neck_TurnToBall() );
    }

    this->setIntention( new IntentionReceive( heard_pos,
                                              ServerParam::i().maxDashPower(),
                                              0.9,
                                              5,
                                              wm.time() ) );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
FieldEvaluator::ConstPtr
SamplePlayer::getFieldEvaluator() const
{
    return M_field_evaluator;
}

/*-------------------------------------------------------------------*/
/*!

*/
FieldEvaluator::ConstPtr
SamplePlayer::createFieldEvaluator() const
{
    return FieldEvaluator::ConstPtr( new SampleFieldEvaluator );
}


/*-------------------------------------------------------------------*/
/*!
*/
#include "actgen_cross.h"
#include "actgen_direct_pass.h"
#include "actgen_self_pass.h"
#include "actgen_strict_check_pass.h"
#include "actgen_short_dribble.h"
#include "actgen_simple_dribble.h"
#include "actgen_shoot.h"
#include "actgen_action_chain_length_filter.h"

ActionGenerator::ConstPtr
SamplePlayer::createActionGenerator() const
{
    CompositeActionGenerator * g = new CompositeActionGenerator();

    //
    // shoot
    //
    g->addGenerator( new ActGen_RangeActionChainLengthFilter
                     ( new ActGen_Shoot(),
                       2, ActGen_RangeActionChainLengthFilter::MAX ) );

    //
    // strict check pass
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_StrictCheckPass(), 1 ) );

    //
    // cross
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_Cross(), 1 ) );

    //
    // direct pass
    //
    // g->addGenerator( new ActGen_RangeActionChainLengthFilter
    //                  ( new ActGen_DirectPass(),
    //                    2, ActGen_RangeActionChainLengthFilter::MAX ) );

    //
    // short dribble
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_ShortDribble(), 1 ) );

    //
    // self pass (long dribble)
    //
    g->addGenerator( new ActGen_MaxActionChainLengthFilter
                     ( new ActGen_SelfPass(), 1 ) );

    //
    // simple dribble
    //
    // g->addGenerator( new ActGen_RangeActionChainLengthFilter
    //                  ( new ActGen_SimpleDribble(),
    //                    2, ActGen_RangeActionChainLengthFilter::MAX ) );

    return ActionGenerator::ConstPtr( g );
}
