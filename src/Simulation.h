//! @file
//! @author Lo�c HAMOT

#ifndef __DRONEWARS_SIMULATION__
#define __DRONEWARS_SIMULATION__

#include <queue>
#include <boost/thread/locks.hpp>
#include "Model.h"
#include "DataBase.h"
#include <boost/chrono.hpp>
#include "PlayerCodes.h"


//! G�re la simulation
class Simulation
{
	Simulation(Simulation const&);              //!< copy ctor non impl�ment�
	Simulation& operator = (Simulation const&); //!< operator = non impl�ment�
public:
	//! Constructeur
	Simulation(Universe& univ,
	           DataBase::ConnectionInfo const& connInfo);

	//! Informe qu'un joueur � �t� modifi�(g�n�ralement les scripts)
	void reloadPlayer(Player::ID pid);

	//! Boucle principale
	void loop();

	//! Donne l'age total de l'univers en rounds, y comprie le round actuel
	double getUnivTime();

	//! Demande la pr�paration d'une planete pour le nouveau joueur donn�e
	void createMainPlanet(Player::ID pid);

private:

	//! Excecute un round
	void round(ScriptTools::Engine&,
	           PlayerCodeMap& codesMap,
	           std::vector<Event>& events);

	//! Recharge les script des joueur dans la base de donn�e
	void updatePlayersCode(ScriptTools::Engine& luaEngine,
	                       PlayerCodeMap& codesMap,
	                       std::vector<Event>& events);

	//! Lance une sauvegarde dans le fichier univName (dans un autre thread)
	void save(std::string const& univName) const;

	//! Supprime les sauvegardes consid�r�es comme trop vielles
	void removeOldSaves() const;

	//! Pr�pare des plan�tes pour les nouveaux joueurs
	void createNewPlayersPlanets(Universe& univCopy);

	//! Mutex de playerToReload_
	mutable boost::shared_mutex reloadPlayerMutex_;
	//! Liste des joueurs a recharger au prochain tour
	std::set<Player::ID> playerToReload_;
	//! Date du d�but du round
	boost::chrono::system_clock::time_point roundStart;
	Universe& univ_;                     //!< Universe
	DataBase database_;                  //!< Acces a la base de donn�e
	//! Mutex de planetToCreate_
	mutable boost::shared_mutex planetToCreateMutex_;
	//! Queue de plan�tes a pr�parer pour des nouveaux joueurs
	std::queue<Player::ID> planetToCreate_;
	mutable boost::thread savingThread_; //!< thread de sauvegarde
};


#endif //__DRONEWARS_SIMULATION__