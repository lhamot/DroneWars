//! @file
//! @author Loïc HAMOT

#ifndef __BTA_ENGINE__
#define __BTA_ENGINE__

#include "stdafx.h"
#include "Model.h"
#include "DataBase.h"

class Simulation;

//! Informations sur l'age de l'Univers
struct TimeInfo
{
	double roundDuration; //!< Durée du dernier round
	double univTime;      //!< Age de l'univers, en round, avec le round actuel
};


//! @brief Gère le simulateur et permet un acces thread-safe a ces données
//! @remark Cette class semble peu utile pour l'instant, mais pourrait ètre
//! le serveur HTTP du simulateur, aprés séparation en deux processus.
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

	//! @brief Extrait la liste des flotte d'un joueur donné
	//! @remark liste vide si le joueur n'existe pas
	std::vector<Fleet> getPlayerFleets(Player::ID pid) const;

	//! @brief Extrait la liste des planètes d'un joueur donné
	//! @remark liste vide si le joueur n'existe pas
	std::vector<Planet> getPlayerPlanets(Player::ID pid) const;

	//! Extrait la planète à une coordonée donnée, si elle existe
	boost::optional<Planet> getPlanet(Coord coord) const;

	//! @brief Extrait une liste de planètes a des coordonées données
	//! @remark Les coordonées ne pointants aucune planète sont ignorées
	//! @post retour.size() <= coords.size()
	std::vector<Planet> getPlanets(std::vector<Coord> const& coord) const;

	//! Extrait la flotte ayant l'ID donnée, si elle existe encore.
	boost::optional<Fleet> getFleet(Fleet::ID fid) const;

	//********** Requetes qui modifient la base *******************************

	//! Ajoute un joueur dans le simulateur (lui donne une planète)
	void addPlayer(Player::ID pid);

	//! Extrait des info temporel de l'Univers
	TimeInfo getTimeInfo() const;

	//! @brief Signale au simulateur une modif du joueur (des scripts)
	//! @pre le pid designe un joueur qui existe
	void reloadPlayer(Player::ID pid);

private:
	//! Charge une sauvegarde
	void load(std::string const& univName, size_t version);

	Universe univ_; //!< Les donnés manipulées par le simulateur
	std::unique_ptr<Simulation> simulation_; //!< La Simulation
	boost::thread simulating_;               //!< Excecute la Simulation
};

#endif //__BTA_ENGINE__