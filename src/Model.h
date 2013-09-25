//! @file
//! @author Loïc HAMOT

#ifndef _BTA_MODEL_
#define _BTA_MODEL_

#include "stdafx.h"

#include "serialize_unordered_map.h"
#include "Player.h"
#include "Tools.h"
#include "TypedPtree.h"


//! Pour éviter un warning bidon de doxygen
typedef boost::serialization::access boost_serialization_access;

namespace std
{
//! Traits de calcul de valeur de hachage d'une coordonée Coord
template<>
struct hash<Coord>
{
	//! Calcul la valeur de hachage d'une coordonée Coord
	size_t operator()(Coord const& p) const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, p.X);
		boost::hash_combine(seed, p.Y);
		boost::hash_combine(seed, p.Z);
		return seed;
	}
};
}

//! Foncteur de comparaison de Coord
struct CompCoord
{
	//! Compare deux Coord
	bool operator()(Coord a, Coord b) const
	{
		if(a.X < b.X)
			return true;
		else if(a.X > b.X)
			return false;
		else if(a.Y < b.Y)
			return true;
		else if(a.Y > b.Y)
			return false;
		else
			return a.Z < b.Z;
	};
};


//! Les types de Ressource
struct Ressource
{
	typedef uint32_t Value; //!< Type contenant une quantité d'une ressource
	//! Les types de Ressource
	enum Enum : uint8_t
	{
	  Metal,
	  Carbon,
	  Loicium,
	  Count
	};
};


//! Contient une quantité pour chaque type de Ressource
struct RessourceSet
{
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& tab;
	}

	//! Tableau contenant une quantité pour chaque type de ressource
	typedef boost::array<Ressource::Value, Ressource::Count> Tab;
	Tab tab; //!< Tableau contenant une quantité pour chaque type de ressource

	//! Constructeur
	RessourceSet(Tab const& t): tab(t) {}
	//! Constructeur par defaut
	RessourceSet() {tab.fill(0);}
	//! Constructeur
	RessourceSet(Ressource::Value a, Ressource::Value b, Ressource::Value c)
	{
		tab[0] = a; tab[1] = b; tab[2] = c;
	}

	//! operateur ==
	inline bool operator == (RessourceSet const& b) const
	{
		return boost::range::equal(tab, b.tab);
	}
	//! operateur !=
	inline bool operator != (RessourceSet const& b) const
	{
		return !(*this == b);
	}
};


//! Tache éxcecutable par une planète
struct PlanetTask
{
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& type& value& value2& lauchTime& duration& startCost& expired;
	}

	//! Types possibles de la tache
	enum Enum : uint8_t
	{
	  UpgradeBuilding,
	  MakeShip,
	  MakeCannon,
	  Count
	};

	uint32_t value;         //!< Valeur utilisée en fonction du type Enum
	uint32_t value2;        //!< Valeur utilisée en fonction du type Enum
	uint32_t lauchTime;     //!< Round de lancement
	uint32_t duration;      //!< Durré total d'excecution
	//! @todo: A quoi sert le cout de lancement?
	RessourceSet startCost; //!< Cout de lancement
	bool expired;           //!< true si la tache a déja été excecuté
	Enum type;              //!< Type de la tache

	//! Constructeur par defaut
	PlanetTask() {}
	//! Constructeur
	PlanetTask(Enum t, uint32_t lauch, uint32_t dur):
		value(0), value2(0), lauchTime(lauch), duration(dur), expired(false), type(t)
	{
	}
};


//! Tache éxcecutable par une flotte
struct FleetTask
{
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& type& lauchTime& duration& position& expired;
	}

	//! Types possibles de la flotte
	enum Enum : uint8_t
	{
	  Move,
	  Harvest,
	  Colonize,
	  Count
	};

	uint32_t lauchTime; //!< Round de lancement
	uint32_t duration;  //!< Durré total d'excecution
	Coord position;     //!< Localisation utilisé par le type Enum::Move
	bool expired;       //!< true si la tache a déja été excecuté
	Enum type;          //!< Type de la tache

	//! Constructeur par defaut
	FleetTask() {}
	//! Constructeur
	FleetTask(Enum t, uint32_t lauch, uint32_t dur):
		lauchTime(lauch), duration(dur), expired(false), type(t)
	{
	}
};


//! Caractéristique d'un batiment
struct Building
{
	//! Types de batiment
	enum Enum
	{
	  Undefined = -1,
	  CommandCenter,
	  MetalMine,
	  CarbonMine,
	  LoiciumFilter,
	  Factory,
	  Laboratory,
	  CarbonicCentral,
	  SolarCentral,
	  GeothermicCentral,
	  Count
	};

	RessourceSet price; //!< Prix du batiment
	double coef;        //!< Coef pour l'augmentation du prix

	//! Liste contenant les caractéristiques de tout les batiments
	static Building const List[Count];
};


//! Caractéristique d'un vaisseau
struct Ship
{
	//! Les types de vaisseau
	enum Enum : int8_t
	{
	  Undefined = -1,
	  Mosquito,
	  Hornet,
	  Vulture,
	  Dragon,
	  Behemoth,
	  Azathoth,
	  Queen,
	  Cargo,
	  LargeCargo,
	  Count
	};

	RessourceSet price; //!< Prix du vaisseau
	uint16_t life;      //!< Point de vie initial du vaisseau
	uint16_t power;     //!< Point déga initial du vaisseau
	uint16_t shield;    //!< Point bouclier initial du vaisseau


	static Ship const List[Count]; //!< Liste des vaisseaux (un par type)
};


//! Caractéristique d'un Cannon
struct Cannon
{
	//! Les types de cannon
	enum Enum : int8_t
	{
	  Undefined = -1,
	  Cannon1,
	  Cannon2,
	  Cannon3,
	  Cannon4,
	  Cannon5,
	  Cannon6,
	  Count
	};

	RessourceSet price; //!< Prix du vaisseau
	uint16_t life;	    //!< Point de vie initial du vaisseau
	uint16_t power;	    //!< Point déga initial du vaisseau
	uint16_t shield;    //!< Point bouclier initial du vaisseau

	static Cannon const List[Count]; //!< Liste des canons (un par type)
};


//! Données d'une planètes
struct Planet
{
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		staticIf<Archive::is_saving::value>([&]()
		{
			if(playerId >= 100000 && playerId != Player::NoId)
			{
				using namespace log4cplus;
				Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("Planet.serialize"));
				LOG4CPLUS_ERROR(logger, "playerId : " << playerId);
				BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
			}
			if(playerId == Player::NoId && taskQueue.empty() == false)
				BOOST_THROW_EXCEPTION(std::logic_error("taskQueue shourld be empty"));
			if(buildingList.size() != Building::Count)
				BOOST_THROW_EXCEPTION(std::logic_error("buildingList.size() != Building::Count"));
		});

		ar& name;
		ar& coord;
		ar& playerId;
		ar& buildingList;
		ar& taskQueue;
		ar& ressourceSet;
		ar& cannonTab;
		if(playerId >= 100000 && playerId != Player::NoId)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
		if(playerId == Player::NoId && taskQueue.empty() == false)
			BOOST_THROW_EXCEPTION(std::logic_error("taskQueue shourld be empty"));
		if(buildingList.size() != Building::Count)
			BOOST_THROW_EXCEPTION(std::logic_error("buildingList.size() != Building::Count"));
		ar& parentCoord;
		ar& time;
		ar& memory;
	}

	//! Tableau contenant un uint16_t pour chaque type de Building
	typedef boost::array<uint16_t, Building::Count> BuildingTab;
	//! Tableau contenant un uint32_t pour chaque type de Cannon
	typedef boost::array<uint32_t, Cannon::Count> CannonTab;

	std::string name;                  //!< Nom
	Coord coord;                       //!< Coordonnées
	Player::ID playerId;               //!< Propriétaire
	BuildingTab buildingList;          //!< Niveau de chaque Building
	std::vector<PlanetTask> taskQueue; //!< Liste des taches en cours
	RessourceSet ressourceSet;         //!< Ressources présente
	CannonTab cannonTab;               //!< Nombre de canons pour chaque type
	Coord parentCoord;                 //!< Coordonées de la planète parente
	time_t time;                       //!< Date de création
	TypedPtree memory; //!< Données utilisateur

	//! Taille ocupée dans le tas (pour profiling)
	size_t heap_size() const
	{
		return
		  name.capacity() +
		  buildingList.size() * sizeof(size_t) +
		  taskQueue.capacity() * sizeof(PlanetTask);
	}
	//! Constructeur par defaut
	Planet(): playerId(55555), time(0) {}
	//! Constructeur
	Planet(Coord c): coord(c), playerId(Player::NoId), time(::time(0))
	{
		buildingList.fill(0);
		cannonTab.fill(0);
		if(playerId >= 100000 && playerId != Player::NoId)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
	}
	//! true si la planète n'as pas de propriétaire
	bool isFree() const
	{
		return playerId == Player::NoId;
	}
};


//! Ordre donné par le script lua a une planète
struct PlanetAction
{
	//! Types d'ordre possibles
	enum Type : int8_t
	{
	  Undefined = -1,
	  Building,
	  StopBuilding,
	  Ship,
	  Cannon,
	  Count
	};

	Type action;             //!< Type d'ordre
	Building::Enum building; //!< Building, si utile au Type
	Ship::Enum ship;         //!< Ship, si utile au Type
	Cannon::Enum cannon;     //!< Cannon, si utile au Type
	uint32_t number;         //!< Quantité de Building, Ship ou autre

	//! Constructeur par defaut
	PlanetAction():
		action(Undefined),
		building(Building::Undefined),
		ship(Ship::Undefined),
		cannon(Cannon::Undefined),
		number(0)
	{
	}
	//! Constructeur pour les type concernant des Building
	PlanetAction(Type a, Building::Enum b):
		action(a),
		building(b),
		ship(Ship::Undefined),
		cannon(Cannon::Undefined),
		number(0)
	{
	}
	//! Constructeur pour les type concernant des vaisseaux
	PlanetAction(Type a, Ship::Enum s, uint32_t n):
		action(a),
		building(Building::Undefined),
		ship(s),
		cannon(Cannon::Undefined),
		number(n)
	{
	}
	//! Constructeur pour les type concernant des canons
	PlanetAction(Type a, Cannon::Enum c, uint32_t n):
		action(a),
		building(Building::Undefined),
		ship(Ship::Undefined),
		cannon(c),
		number(n)
	{
	}
};
typedef std::vector<PlanetAction> PlanetActionList; //!< Liste de PlanetAction


//! Données d'une flotte
struct Fleet
{
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int) //version
	{
		staticIf<Archive::is_saving::value>([&]()
		{
			if(playerId >= 100000)
				BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
		});
		ar& id;
		ar& playerId;
		ar& coord;
		ar& origin;
		ar& name;
		ar& shipList;
		ar& time;
		ar& ressourceSet;
		ar& taskQueue;
		if(playerId >= 100000)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
		ar& memory;
	}

	//! Un uint32_t par type de Ship
	typedef boost::array<uint32_t, Ship::Count> ShipTab;
	typedef uint64_t ID;      //!< Type ID
	static const ID NoId = 0; //!< Valeur d'ID signifiant "Pas de flotte"

	ID id;                    //!< Identifiant unique
	Player::ID playerId;      //!< Propriétaire
	Coord coord;              //!< Coordonées actuels
	Coord origin;             //!< Coordonées d'origines
	std::string name;         //!< Nom
	ShipTab shipList;         //!< Quantité de chaque type de vaisseaux présent
	time_t time;              //!< Date de création
	RessourceSet ressourceSet;        //!< Ressources transportés par la flotte
	std::vector<FleetTask> taskQueue; //!< Liste de taque a éxcecuter
	TypedPtree memory; //!< Données utilisateurs

	//! Taille dans le tas. (Pour profiling)
	size_t heap_size() const
	{
		return name.capacity();
	}

	//! Constructeur par defaut
	Fleet(): id(NoId), playerId(Player::NoId), time(0) {shipList.assign(0);}
	//! Constructeur
	Fleet(ID fid, Player::ID pid, Coord c):
		id(fid), playerId(pid), coord(c), origin(c), time(::time(0))
	{
		shipList.fill(0);
		if(playerId >= 100000)
			BOOST_THROW_EXCEPTION(std::logic_error("playerId >= 100000!!"));
	}

	bool empty() const
	{
		return boost::accumulate(shipList, 0) == 0;
	}
};


//! Ordre qu'un scrupt LUA peut donner a une flotte
struct FleetAction
{
	//! Types d'ordres possibles
	enum Type : uint8_t
	{
	  Nothing,
	  Move,
	  Harvest,
	  Colonize,
	  Drop
	};

	Type action;      //!< Type de l'ordre
	Direction target; //!< Direction (si applicable)

	//! Constructeur
	explicit FleetAction(Type a, Direction t = Direction()): action(a), target(t) {}
};
typedef std::vector<FleetAction> FleetActionList; //!< Liste de FleetAction


//! @brief Part dun rapport de combat(FightRepport) lié à une planète ou flotte
//! @tparam T Planet ou Fleet
template<typename T>
struct Report
{
private:
	friend boost_serialization_access; //!< Pour donner access a boost
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& isDead& hasFight& experience& enemySet& fightInfo;
		if(version > 0)
			ar& wantEscape& escapeProba;
	}
	//! Constructeur par default utilisé uniquement par boost::serialization
	Report() {}

public:
	bool isDead;         //!< True si l'armée est détruite
	bool hasFight;       //!< True si l'armée a combatue
	uint32_t experience; //!< Points d'experience gagnés
	bool wantEscape;     //!< Si la flotte voulai s'enfuire
	double escapeProba;  //!< Probabilité qu'elle avait de s'enfuire
	//! Liste des énemie combatue, par index dans le FightReport
	std::set<intptr_t> enemySet;
	//! Etat des flottes/planètes avant et apprés un combat
	struct FightInfo
	{
		//! serialize l'objet
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			ar& before& after;
		}

		T before; //!< Etat de l'armé avant le combat
		T after;  //!< Etat de l'armé aprés le combat

		//! Taille alloué sur le tas par l'objet (utile pour profiling memoire)
		size_t heap_size() const
		{
			return before.heap_size() + after.heap_size();
		}

		//! Constructeur par defaut
		FightInfo() {}
		//! Constructeur
		FightInfo(T const& bef, T const& aft): before(bef), after(aft) {}
	};
	FightInfo fightInfo; //!< Info sur le combat

	//! Fabrique un FightInfo a partie d'une armé(flotte ou planète) donnée
	static FightInfo makeFightInfo(T fighter)
	{
		return FightInfo(fighter, fighter);
	}

	//! Constructeur
	Report(T const& fighter):
		isDead(false),
		hasFight(false),
		experience(0),
		wantEscape(false),
		escapeProba(0.),
		fightInfo(makeFightInfo(fighter))
	{
	}

	//! Taille alloué sur le tas par l'objet (utile pour profiling memoire)
	size_t heap_size() const
	{
		return
		  enemySet.size() * sizeof(size_t) * 3 +
		  fightInfo.heap_size();
	}
};
BOOST_CLASS_VERSION(Report<Planet>, 1);
BOOST_CLASS_VERSION(Report<Fleet>, 1);


//! Rapport de combat
struct FightReport
{
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& fleetList& hasPlanet& planet;
	}

	std::vector<Report<Fleet> > fleetList;   //!< Liste des rapport de flotte
	bool hasPlanet;                          //!< si une planète a combatue
	boost::optional<Report<Planet> > planet; //!< Rapport de la planète

	//! Constructeur par defaut
	FightReport(): hasPlanet(false) {}

	//! Taile dans le tas. (Pour profiling)
	size_t heap_size() const
	{
		size_t res = fleetList.capacity() * sizeof(Report<Fleet>);
		for(Report<Fleet> const & report : fleetList)
			res += report.heap_size();
		if(planet)
			res += planet->heap_size();
		return res;
	}
};


//! Un evenement ayant eu lieu durant la simulation et lié à un joueur
struct Event
{
	//! Les types d'évenements
	enum Type : uint8_t
	{
	  FleetCodeError,
	  FleetCodeExecError,
	  PlanetCodeError,
	  PlanetCodeExecError,
	  Upgraded,
	  ShipMade,
	  PlanetHarvested,
	  FleetWin,
	  FleetDraw,
	  FleetsGather,
	  PlanetColonized,
	  FleetLose,
	  FleetDrop,
	  PlanetLose,
	  PlanetWin,
	  CannonMade,
	  FightAvoided,
	  Count
	};

	typedef size_t ID;   //!< Type d'indentifiant d'évenement
	ID id;               //!< Indentifiant unique
	time_t time;         //!< Date de l'évenement
	Type type;           //!< Type d'évenement
	std::string comment; //!< Commentaire
	intptr_t value;      //!< Valeur numerique utile pour certain Event::Type
	intptr_t value2;     //!< 2dn valeur numerique utile pour certain Event::Type
	bool viewed;         //!< true si l'évenement a été vu par son propriétaire
	Player::ID playerID; //!< Propriétaire
	Fleet::ID fleetID;   //!< Flotte lié, si applicable, sinon Fleet::NoID
	Coord planetCoord;   //!< Planète lié, si applicable, sinon (0; 0; 0)

	//! Taile dans le tas (Pour profiling memoire)
	size_t heap_size() const
	{
		return comment.capacity();
	}

	//! Constructeur
	//! @pre pid != Player::NoId
	Event(Player::ID pid, time_t ti, Type ty):
		id(0), time(ti), type(ty), value(-1), value2(-1), viewed(false),
		playerID(pid), fleetID(Fleet::NoId)
	{
		if(playerID == Player::NoId)
			BOOST_THROW_EXCEPTION(std::logic_error("playerID == Player::NoId"));
	}

	//! Modifie Event::value
	Event& setValue(intptr_t val)       {value = val; return *this;}
	//! Modifie Event::value
	Event& setValue2(intptr_t val)      {value2 = val; return *this;}
	//! Modifie Event::comment
	Event& setComment(std::string const& comm)  {comment = comm; return *this;}
	//! Modifie Event::fleetID
	Event& setFleetID(Fleet::ID fid)    {fleetID = fid; return *this;}
	//! Modifie Event::planetCoord
	Event& setPlanetCoord(Coord pcoord) {planetCoord = pcoord; return *this;}
};


//! Toute les données manipulées par le simulateur (mais pas dans le SGBD)
struct Universe
{
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& planetMap;
		ar& fleetMap;
		ar& nextFleetID;
		ar& roundCount;
		ar& roundDuration;
	}

	static const unsigned short MapSizeX = 100; //!< Nombre de cases, en X
	static const unsigned short MapSizeY = 100; //!< Nombre de cases, en Y
	static const unsigned short MapSizeZ = 100; //!< Nombre de cases, en Z

	//! Tableau associatif [Coord] => Planète
	typedef std::unordered_map<Coord, Planet> PlanetMap;
	//! Tableau associatif [Fleet::ID] => Fleet
	typedef std::map<Fleet::ID, Fleet> FleetMap;

	PlanetMap planetMap;   //!< Tab des planètes, avec leurs coordonées en clé
	FleetMap fleetMap;     //!< Tableau des flottes, avec le Fleet::ID pour clé
	Fleet::ID nextFleetID; //!< ID de la prochaine flotte (autoincrément)
	uint32_t roundCount;   //!< Nombre de round ecoulés
	double roundDuration;  //!< Durrée du dernier round

	//! Calcul la taille approximative dans pile, pour du profiling memoire
	size_t heap_size() const
	{
		size_t res = 0;
		for(auto const & planetKV : planetMap)
			res += sizeof(planetKV) +
			       planetKV.second.heap_size() +
			       2 * sizeof(size_t);
		for(auto const & fleetKV : fleetMap)
			res += sizeof(fleetKV) +
			       fleetKV.second.heap_size() +
			       2 * sizeof(size_t);
		return res;
	}

	typedef boost::shared_mutex Mutex;       //!< Type mutex pour Universe
	mutable Mutex mutex; //!< Mutex des donées de Universe

	//! Constructeur par defaut
	Universe():
		nextFleetID(1),
		roundCount(0),
		roundDuration(0.)
	{
	}

	//! Crée un Universe par copie
	Universe(Universe const& other):
		planetMap(other.planetMap),
		fleetMap(other.fleetMap),
		nextFleetID(other.nextFleetID),
		roundCount(other.roundCount),
		roundDuration(other.roundDuration)
	{
	}

	//! Copie un Universe
	Universe& operator=(Universe other)
	{
		swap(other);
		return *this;
	}

	//! Echange deux Universe sans risque d'éxception
	void swap(Universe& other) throw()
	{
		planetMap.swap(other.planetMap);
		fleetMap.swap(other.fleetMap);
		std::swap(nextFleetID, other.nextFleetID);
		std::swap(roundCount, other.roundCount);
		std::swap(roundDuration, other.roundDuration);
	}
};


#endif //_BTA_MODEL_