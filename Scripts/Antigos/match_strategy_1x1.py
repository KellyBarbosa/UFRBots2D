import os
import subprocess
from time import sleep
import io
import csv
import threading
import signal
import time
from datetime import date

RESULT_COMAND_SERVER = ''
PARTIDAS = 10
#DIR_MONITOR = ""
DIR_LOG = "/media/kelly/6A06C78A61BA499A/UFRBots/LOGS/logs_strategy_cbr2023"
DIR_OUR_TIME = "/media/kelly/6A06C78A61BA499A/UFRBots/Strategy_cbr2023/V1/UFRBots2D/src"
DIR_OPP_TIME = "/media/kelly/6A06C78A61BA499A/UFRBots/LOGS/OppTimes/bullrussia/src"
GAME_MODE = 2 # GAME_MODE: 1 - normal | 2 - rápido

def command(cmd, type=2):
    global RESULT_COMAND_SERVER
    if (type == 1):
        buffer = subprocess.getoutput(cmd)
        RESULT_COMAND_SERVER = str(buffer)
    else:
        os.system(cmd)

#strategies = ['1', '2', '3', '4', 'hybrid']

def startMatch():
    global RESULT_COMAND_SERVER

    # Modo de jogo
    # mode: 1 - normal
    # mode: 2 - rápido
    #mode = 2 
    # Caminho do diretório do adversário
    #path2 = f"/DIRETORIO_TIME_2" 
    # Caminho onde o monitor irá ser executado
    #DIR_MONITOR = "/DIRETORIO_ONDE_QUER_ABRIR_O_MONITOR" 
    input_ = "cd ~ && cd " + DIR_LOG + " && rcssmonitor --auto-reconnect-mode on"
    t0 = threading.Thread(target=command, args=(input_, 1))
    t0.start()
    sleep(3)

    # Loop das estratégias
    #for strategy in strategies:
    # Início da execução da estratégia
    inicio = time.time()
    # Array para armazenar os resultados dos jogos
    results = []
    # Diretório onde os logs serão armazenados
    #DIR_LOG = "/DIRETORIO_ONDE_QUER_SALVAR_O_LOG" 
    # Diretório do time
    #path1 = f"/DIRETORIO_TIME_1"  

    # Loop das partidas
    for i in range(PARTIDAS):

        # modo normal
        input_ = "cd ~ && cd " + DIR_LOG + " && rcssserver server::auto_mode = true"

        if (int(GAME_MODE) == 2):
            input_ = "cd ~ && cd " + DIR_LOG + \
                " && rcssserver server::auto_mode = true server::nr_extra_halfs = 0 server::penalty_shoot_outs = false  server::synch_mode=true"

        #  iniciando servidor
        t1 = threading.Thread(target=command, args=(input_, 1))

        # adicionando time 1
        input_ = 'cd && cd ' + str(DIR_OUR_TIME) + ' && ./start.sh'
        t2 = threading.Thread(target=command, args=(input_, 2))

        # adicionando time 2
        input_ = 'cd && cd ' + str(DIR_OPP_TIME) + ' && ./start.sh'
        t3 = threading.Thread(target=command, args=(input_, 2))

        t1.start()
        sleep(3)
        t2.start()
        sleep(1)
        t3.start()
        sleep(3)

        # All threads running in parallel, now we wait
        t1.join()
        t2.join()
        t3.join()

        # Obtendo placar
        scores = ['0', '0']
        buf = io.StringIO(RESULT_COMAND_SERVER)
        limit = 10000
        while True:
            line = buf.readline()
            limit = limit - 1
            if (limit == 0):
                break
            if "Score" in line:
                scores = line.replace(" ", "").replace("Score:", "").replace(
                    "\n", "").replace("\t", "").split('-')
                break

        results.append(tuple([int(scores[0]), int(scores[1]), (int(scores[0]) - int(scores[1]))]))
        sleep(5)
    # Fim da execução da estratégia
    fim = time.time()
    saveScore(f'{DIR_LOG}/resultados_{date.today()}.csv', results)
    saveTime(f'{DIR_LOG}/time.csv', fim-inicio)

def saveScore(path_file, results):
    #opening_type = 'a' if os.path.isfile(path_file) else opening_type = 'w'
    opening_type = 'w'
    if(os.path.isfile(path_file)):
        opening_type = 'a'
    with open(path_file, opening_type) as f:
        csv_writer = csv.writer(f)
        if opening_type == 'w': 
            csv_writer.writerow(['Our score', 'Opponent score', 'Goal balance'])
        csv_writer.writerows(results)

def saveTime(path_file, time):
    #opening_type = 'a' if os.path.isfile(path_file) else opening_type = 'w'
    opening_type = 'w'
    if(os.path.isfile(path_file)):
        opening_type = 'a'
    with open(path_file, opening_type) as f:
        csv_writer = csv.writer(f)
        if opening_type == 'w': 
            csv_writer.writerow(['time (s)'])
        csv_writer.writerow([time]) 

def main():
    startMatch()
    print("\n\n\n\n******************** All matches have been played ********************")
    sleep(600)
    os.kill(os.getppid(), signal.SIGHUP)

if __name__ == '__main__':
    main()
