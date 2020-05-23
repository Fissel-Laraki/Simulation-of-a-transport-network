#include <iostream>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <string>

#define NB_STATION_BUS 5
#define NB_STATION_METRO 3



std::string monString(int,int,int,int,int,int);
bool transferable(int,int);

int main(int argc,char** argv)
{
    time_t debut,fin,totaltime;
    time(&debut);
    srand(time(NULL));

    int total = atoi(argv[1]);
    int tps_attente_min = total/20;
    int depart,arrivee,tps_attente,transfert,tps_max;
    std::string s;
    std::ofstream monFlux("passagers.txt");
    if (monFlux)
    {
        monFlux << total << std::endl;
        for (int i = 0; i< total; i++){
            depart = rand() % (NB_STATION_BUS+ NB_STATION_METRO);       
            arrivee =  rand() % (NB_STATION_BUS+ NB_STATION_METRO);       
            if( arrivee == depart)
                arrivee = (arrivee + 1) %(NB_STATION_BUS+ NB_STATION_METRO);
            tps_attente = rand()%(total/20);
            if (transferable(depart,arrivee))
                transfert = 1;
            else
                transfert = 0;
     //       tps_max = 799;
	        tps_max = tps_attente_min + (rand()%(total/10));
            s =  monString(i,depart,arrivee,tps_attente,transfert,tps_max);
            monFlux << s << std::endl;
        }         
    }
    else
    {
        std::cout << "Erreur lors de l'ouverture du fichier." <<std::endl;
        exit(EXIT_FAILURE);
    }
    time(&fin);
    totaltime = fin -debut;
    std::cout << totaltime << std::endl;
    

    return 0;

}

std::string monString(int id ,int depart ,int arrivee,int tps_attente,int transfert,int tps_max){

    std::string s;
    s = "# " + std::to_string(id+1) + " " + std::to_string(depart) + " " + std::to_string(arrivee) + " " + std::to_string(tps_attente) + " " + std::to_string(transfert) + " " + std::to_string(tps_max); 
    return s;
}

bool transferable(int depart,int arrivee){
    return ((depart < NB_STATION_BUS && arrivee >= NB_STATION_BUS) || (arrivee < NB_STATION_BUS && depart >= NB_STATION_BUS));
}
