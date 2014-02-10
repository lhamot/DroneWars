//! @file
//! @author Lo�c HAMOT

#ifndef __BTA_ENGINE__
#define __BTA_ENGINE__

#include "stdafx.h"
#include "Model.h"
#include "DataBase.h"

class Simulation;

//! Informations sur l'age de l'Univers
struct TimeInfo
{
	double roundDuration; //!< Dur�e du dernier round
	double univTime;      //!< Age de l'univers, en round, avec le round actuel
};


//! @brief G�re le simulateur et permet un acces thread-safe a ces donn�es
//! @remark Cette class semble peu utile pour l'instant, mais pourrait �tre
//! le serveur HTTP du simulateur, apr�s s�paration en deux processus.
class Engine
{
public:
	//! Constructeur
	Engine(DataBase::ConnectionInfo const& connInfo);

	~Engine();

	//! Lance la simulation
	void start();

	//! Stop la simulation
	void stop();

	//********* Requetes qui ne modifient pas la base *************************

	//! @brief Extrait la liste des flotte d'un joueur donn�
	//! @remark liste vide si le joueur n'existe pas
	std::vector<Fleet> getPlayerFleets(Player::ID pid) const;

	//! @brief Extrait la liste des plan�tes d'un joueur donn�
	//! @remark liste vide si le joueur n'existe pas
	std::vector<Planet> getPlayerPlanets(Player::ID pid) const;

	//! Extrait la plan�te � une coordon�e donn�e, si elle existe
	boost::optional<Planet> getPlanet(Coord coord) const;

	//! @brief Extrait une liste de plan�tes a des coordon�es donn�es
	//! @remark Les coordon�es ne pointants aucune plan�te sont ignor�es
	//! @post retour.size() <= coords.size()
	std::vector<Planet> getPlanets(std::vector<Coord> const& coord) const;

	//! Extrait la flotte ayant l'ID donn�e, si elle existe encore.
	boost::optional<Fleet> getFleet(Fleet::ID fid) const;

	//********** Requetes qui modifient la base *******************************

	//! Ajoute un joueur dans le simulateur (lui donne une plan�te)
	void addPlayer(Player::ID pid);

	//! Extrait des info temporel de l'Univers
	TimeInfo getTimeInfo() const;

	//! @brief Signale au simulateur une modif du joueur (des scripts)
	//! @pre le pid designe un joueur qui existe
	void reloadPlayer(Player::ID pid);

private:
	//! Charge une sauvegarde
	void load(std::string const& univName, size_t version);

	Universe univ_; //!< Les donn�s manipul�es par le simulateur
	std::unique_ptr<Simulation> simulation_; //!< La Simulation
	boost::thread simulating_;               //!< Excecute la Simulation
};

#endif //__BTA_ENGINE__