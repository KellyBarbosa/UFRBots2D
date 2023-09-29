// -*-c++-*-

/*
 *Copyright:
 *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "math.h"
#include "bhv_basic_move.h"

#include "strategy.h"

#include "bhv_basic_tackle.h"

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/intercept_table.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

#include "neck_offensive_intercept_neck.h"

using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!
 Correr com a bola
 */
bool Bhv_BasicMove::execute(PlayerAgent *agent)
{

	// chase ball
	const WorldModel &wm = agent->world(); /// variavel world model ( wm );

	double a = Fuzzy(wm.self().pos().x);								// logica fuzzy
	const PlayerPtrCont &opps = wm.opponentsFromBall(); // jogadores diante do agent
	const PlayerObject *nearest_opp = (opps.empty()
																				 ? static_cast<PlayerObject *>(0)
																				 : opps.front()); // se o vetor tiver vazio entao vai ser igual a zero se nao pegara o primeiro jogador o mais perto
	const double nearest_opp_distFromBall = (nearest_opp
																							 ? nearest_opp->distFromBall()
																							 : 1000.0); // pega a distancia do jogador mais proximo ate voce
	const PlayerPtrCont &ams = wm.teammatesFromBall();			// jogadores diante do agent
	const PlayerObject *nearest_am = (ams.empty()
																				? static_cast<PlayerObject *>(0)
																				: ams.front()); // se o vetor tiver vazio entao vai ser igual a zero se nao pegara o primeiro jogador o mais perto
	const double nearest_am_distFromBall = (nearest_am
																							? nearest_am->distFromBall()
																							: 1000.0); // pega a distancia do jogador mais proximo ate voce
	const double nearest_am_pos_x = (nearest_am
																			 ? nearest_am->pos().x
																			 : 1000.0);								 // pega o valo da posicao do jogador mais proximo
	const int self_min = wm.interceptTable()->selfReachCycle();		 // self min recebe o valor do ciclo para pegar a bola
	const int opp_min = wm.interceptTable()->opponentReachCycle(); // opp_min recebe o valor do ciclo minimo do oponente para pegar a bola
	const int mat_min = wm.interceptTable()->teammateReachCycle(); // mat_min recebe o valor do ciclo minimo do companheiro de equipe para pegar a bola
	double Px = -53.0;
	if (agent->world().ball().pos().x > -46)
		Px = Px - 51;																													// exemplo:-45.PX=-53-51=-104
	const Vector2D ponto = Vector2D(Px, 0.0);																// ponto recebe a posição do campo( posx: PX, posy:0.0)
	const Vector2D ponto2 = agent->world().ball().pos();										// ponto2 recebe a posição do campo (posx:posballx,posy:posbally)
	const Segment2D Linha = Segment2D(ponto, ponto2);												// Segment2D faz uma linha com origem em ponto e final em ponto2;
	const Vector2D inter = Linha.nearestPoint(agent->world().self().pos()); // inter recebe a distancia minima entre posiçao do agent e o segmento Linha.
	const double dash_power = ServerParam::i().maxDashPower();							// dash_power recebe dashmaximo
	double dist_thr = 0.1;
	double dist_thr2 = 3;
	// std::cout<<"JOGARDOR:"<<wm.self().unum()<<": ";

	if (mat_min > self_min && self_min <= opp_min) // se o ciclo do agent for menor ou igual, intercepta
	{
		Body_Intercept().execute(agent);
		agent->setNeckAction(new Neck_OffensiveInterceptNeck());
		return true;
	}

	if (!wm.existKickableTeammate() && wm.ball().distFromSelf() < nearest_am_distFromBall) // se agent e o mais proximo da bola
	{
		// std::cout<<"sou o mais proximo-->";
		if (wm.self().pos().x < wm.ball().pos().x) // se o x do agent for menor que da bola
		{
			// std::cout<<"meu x e menor que o da bola--->";
			if (self_min < opp_min + a) // se o ciclo do agent for menor que a do opponente mais fuzzy
			{
				const WorldModel &wm = agent->world();
				const double dash_power = ServerParam::i().maxDashPower();
				const PlayerPtrCont &opps = wm.opponentsFromBall(); // jogadores diante do agent
				const PlayerObject *nearest_opp = (opps.empty() ? static_cast<PlayerObject *>(0) : opps.front());
				const Vector2D opp_pos = (nearest_opp ? nearest_opp->pos() : Vector2D(-1000.0, 0.0));
				const Sector2D frente_opp = Sector2D(opp_pos, 0, 4, nearest_opp->body() - 45, nearest_opp->body() + 45);
				const Vector2D ponto_opp = nearest_opp->pos() + Vector2D::polar2vector(4.0, nearest_opp->body());
				if (wm.self().isWithin(frente_opp)) // se o agent estiver no campo de visao do oponente
				{
					Body_Intercept().execute(agent);
					agent->setNeckAction(new Neck_OffensiveInterceptNeck());
					return true;
				}
				else
				{
					if (!Body_GoToPoint(ponto_opp, 0, dash_power).execute(agent)) // se nao, ir para posica do inimigo;
					{
						agent->setNeckAction(new Neck_OffensiveInterceptNeck());
						Bhv_BasicMove().ExecutaTackle(agent);
						return true;
					}
					else
					{
						agent->setNeckAction(new Neck_OffensiveInterceptNeck());
						Bhv_BasicMove().ExecutaTackle(agent);
						return true;
					}
				}
			}
			// falta fechar dois if, agent mais proximo e x do agent
			else
			{
				Bhv_BasicMove().ExecutaTackle(agent);
				if (agent->world().self().isWithin(Linha))
				{ // se estiver dentro da linha
					if (!Body_GoToPoint(agent->world().ball().pos(), dist_thr2, dash_power).execute(agent))
					{ // ira para posica do inimigo;
						agent->setNeckAction(new Neck_OffensiveInterceptNeck());
						Bhv_BasicMove().ExecutaTackle(agent);
					}
					else
					{
						agent->setNeckAction(new Neck_OffensiveInterceptNeck());
						Bhv_BasicMove().ExecutaTackle(agent);
					}
				}
				else
				{ // se nao estiver dentro da linha
					if (!Body_GoToPoint(inter, dist_thr, dash_power).execute(agent))
					{ // ira para posica do inimigo; agent->setNeckAction( new Neck_OffensiveInterceptNeck() );
						Bhv_BasicMove().ExecutaTackle(agent);
					}
					else
					{
						agent->setNeckAction(new Neck_OffensiveInterceptNeck());
						Bhv_BasicMove().ExecutaTackle(agent);
					}
				}
			}
		}
		else
		{ // se o x do agent for maior que da bola
			Bhv_BasicMove().ExecutaTackle(agent);
			if (wm.ball().distFromSelf() < nearest_opp_distFromBall) // se a distancia entre a bola ate o agent for menor que a do opponente ate a bola
			{
				Body_Intercept().execute(agent);
			}
			else if (wm.ball().distFromSelf() <= 2) // se o agent estiver de 2 metros ou menos intercptar
			{
				Body_Intercept().execute(agent);
			}
		}
	}
	if (!wm.existKickableTeammate() && nearest_am_pos_x > wm.ball().pos().x && self_min <= 5)
	{

		Bhv_BasicMove().ExecutaTackle(agent);
		if (agent->world().self().isWithin(Linha))
		{

			if (!Body_GoToPoint(agent->world().ball().pos(), dist_thr2, dash_power).execute(agent))
			{
				agent->setNeckAction(new Neck_OffensiveInterceptNeck());
				Bhv_BasicMove().ExecutaTackle(agent);
			}
			else
			{
				agent->setNeckAction(new Neck_OffensiveInterceptNeck());
				Bhv_BasicMove().ExecutaTackle(agent);
			}
		}
		else
		{

			if (!Body_GoToPoint(inter, dist_thr, dash_power).execute(agent))
			{
				agent->setNeckAction(new Neck_OffensiveInterceptNeck());
				Bhv_BasicMove().ExecutaTackle(agent);
			}
			else
			{
				agent->setNeckAction(new Neck_OffensiveInterceptNeck());
				Bhv_BasicMove().ExecutaTackle(agent);
			}
		}
	}
	//-----------------------------------------------
	// tackle
	if (Bhv_BasicTackle(0.8, 80.0).execute(agent))
	{
		return true;
	}
	const Vector2D target_point = Strategy::i().getPosition(wm.self().unum());
	// const double dash_power2 = Strategy::get_normal_dash_power( wm );

	double dist_thr3 = wm.ball().distFromSelf() * 0.1;
	if (dist_thr < 1.0)
		dist_thr3 = 1.0;

	dlog.addText(Logger::TEAM,
							 __FILE__ ": Bhv_BasicMove target=(%.1f %.1f) dist_thr=%.2f",
							 target_point.x, target_point.y,
							 dist_thr);

	agent->debugClient().addMessage("BasicMove%.0f", dash_power);
	agent->debugClient().setTarget(target_point);
	agent->debugClient().addCircle(target_point, dist_thr);

	if (!Body_GoToPoint(target_point, dist_thr3, dash_power).execute(agent))
	{
		Body_TurnToBall().execute(agent);
	}

	if (wm.existKickableOpponent() && wm.ball().distFromSelf() < 18.0)
	{
		agent->setNeckAction(new Neck_TurnToBall());
	}
	else
	{
		agent->setNeckAction(new Neck_TurnToBallOrScan());
	}
	return true;
}

double
Bhv_BasicMove::Fuzzy(double entrada)
{
	double posx, saida = 0;
	double matriz[2][106] = {
			{-52.5000, -51.5000, -50.5000, -49.5000, -48.5000, -47.5000, -46.5000, -45.5000, -44.5000, -43.5000, -42.5000, -41.5000, -40.5000, -39.5000, -38.5000, -37.5000, -36.5000, -35.5000, -34.5000, -33.5000, -32.5000, -31.5000, -30.5000, -29.5000, -28.5000, -27.5000, -26.5000, -25.5000, -24.5000, -23.5000, -22.5000, -21.5000, -20.5000, -19.5000, -18.5000, -17.5000, -16.5000, -15.5000, -14.5000, -13.5000, -12.5000, -11.5000, -10.5000, -9.5000, -8.5000, -7.5000, -6.5000, -5.5000, -4.5000, -3.5000, -2.5000, -1.5000, -0.5000, 0.5000, 1.5000, 2.5000, 3.5000, 4.5000, 5.5000, 6.5000, 7.5000, 8.5000, 9.5000, 10.5000, 11.5000, 12.5000, 13.5000, 14.5000, 15.5000, 16.5000, 17.5000, 18.5000, 19.5000, 20.5000, 21.5000, 22.5000, 23.5000, 24.5000, 25.5000, 26.5000, 27.5000, 28.5000, 29.5000, 30.5000, 31.5000, 32.5000, 33.5000, 34.5000, 35.5000, 36.5000, 37.5000, 38.5000, 39.5000, 40.5000, 41.5000, 42.5000, 43.5000, 44.5000, 45.5000, 46.5000, 47.5000, 48.5000, 49.5000, 50.5000, 51.5000, 52.5000},
			{4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.4380, 4.6965, 5.1587, 5.5586, 5.9080, 6.2159, 6.4893, 6.7336, 6.9531, 7.1514, 7.3311, 7.4945, 7.6437, 7.7801, 7.9051, 8.0197, 8.1249, 8.2215, 8.3100, 8.3912, 8.4656, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.4998, 8.5621, 8.6908, 8.8255, 8.9668, 9.1151, 9.2709, 9.4350, 9.6081, 9.7910, 9.9846, 10.1901, 10.4087, 10.6419, 10.8913, 11.1589, 11.4471, 11.7586, 12.0965, 12.4648, 12.8683, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863, 13.0863}};
	double b = ceil(entrada);
	double y = ceil(entrada - 0.4);
	if ((b - entrada) < 0.5)
	{
		posx = y - 0.5;
	}
	else
	{
		posx = y + 0.5;
	}
	for (int i = 0; i < 105; i++)
	{
		if (posx == matriz[0][i])
		{
			saida = matriz[1][i];
		}
	}
	return saida;
}
bool Bhv_BasicMove::ExecutaTackle(PlayerAgent *agent)
{

	const WorldModel &wm = agent->world();	// agente
	double ln_1 = wm.ball().distFromSelf(); // ln_1-> distancia do agente ao oponente com bola
	double ln_2 = wm.self().pos().x;				// ln_2 -> posição do agente no campo em x
	double ln_2_aux = ln_2 + 52;

	// tackle
	if (!wm.existKickableTeammate())
	{
		if ((0 <= ln_1 && ln_1 <= 1 && 0 <= ln_2_aux && ln_2_aux <= (25.375)) || (0.5 < ln_1 && ln_1 <= 1.05 && 0 <= ln_2_aux && ln_2_aux <= (17.125)) || (0.5 < ln_1 && ln_1 <= 1.05 && 12.375 < ln_2_aux && ln_2_aux <= 25.375))
		{
			const ServerParam &param = ServerParam::i();
			double tackle_power = param.maxTacklePower();
			dlog.addText(Logger::TEAM,
									 __FILE__ ": Bhv_BasicTackle. (old) body dir");
			agent->debugClient().addMessage("Tackle+");
			agent->doTackle(tackle_power);
			agent->setNeckAction(new rcsc::Neck_TurnToBallOrScan());
			return true;
		}
	}
	return false;
}
