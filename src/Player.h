//! @file
//! @author Loïc HAMOT

#ifndef __DRONEWARS_PLAYER__
#define __DRONEWARS_PLAYER__

#include "stdafx.h"

//! Nombre de skill
static size_t const SkillCount = 13;

//! Coordonée entière tridimentionelle
struct Coord
{
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& X& Y& Z;
	}

	typedef int8_t Value; //!< Type pour une Coordonée monodimensionnelle
	Value X = 0;              //!< Coordonée en X
	Value Y = 0;              //!< Coordonée en Y
	Value Z = 0;              //!< Coordonée en Z

	//! Constructeur par defaut
	Coord() {}

	//! Constructeur prenant 3 valeurs X, Y et Z
	Coord(Value x, Value y, Value z):
		X(x),
		Y(y),
		Z(z)
	{
	}
};

static Coord const UndefinedCoord(-1, -1, -1);

//! Test d'égalité de deux Coord
inline bool operator == (Coord const& a, Coord const& b)
{
	return (a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z);
}


//! Test d'inégalité de deux Coord
inline bool operator != (Coord const& a, Coord const& b)
{
	return (a == b) == false;
}

inline std::ostream& operator << (std::ostream& os, Coord const& c)
{
	os << "(" << int(c.X) << "," << int(c.Y) << "," << int(c.Z) << ")" << std::endl;
	return os;
}

//! Direction tridimentionelle en valeur entière
struct Direction
{
	//! Serialize l'objet
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& X& Y& Z;
	}

	typedef int8_t Value; //!< Type pour une C
	Value X = 0;		  //!< Composante X
	Value Y = 0;		  //!< Composante Y
	Value Z = 0;		  //!< Composante Z

	//! Constructeur par defaut : Vecteur nulle
	Direction() {}

	//! Constructeur
	Direction(Value x, Value y, Value z):
		X(x),
		Y(y),
		Z(z)
	{
	}
};


//! Test d'égalité de deux Direction
inline bool operator == (Direction const& a, Direction const& b)
{
	return (a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z);
}


//! Test d'inégalité de deux Direction
inline bool operator != (Direction const& a, Direction const& b)
{
	return (a == b) == false;
}


static size_t const MaxStringSize = 256; //!< Taille max des string courte


//! Script de planète ou flotte d'un joueur
struct CodeData
{
	//! @brief Taile max du code lua
	static size_t const MaxCodeSize = 32768;
	//! @brief Taille max du code blockly
	static size_t const MaxBlocklySize = MaxCodeSize * 8;

	//! Planète ou Flotte
	enum Target : uint8_t
	{
		Planet,
		Fleet,
		Undefined
	};

	size_t   id = 0;         //!< ID de la version du code source lua
	uint32_t playerId = 0;   //!< Propriétaire
	Target   target = Undefined;  //!< Planète ou Flotte
	std::string code;        //!< Code lua
	std::string blocklyCode; //!< Code blockly (XML)
	std::string lastError;   //!< ernière erreur, si applicable
	time_t blocklyCodeDate = 0;
	time_t codeDate = 0;
};

struct Player;

//! Description d'une alliance
struct Alliance
{
	typedef uint32_t ID;      //!< Type d'identifiant
	static ID const NoId = 0; //!< Valeur indiquant l'absence d'alliance
	ID id;                    //!< Identifiant unique
	uint32_t masterID;        //!< Identifiant du propriétaire
	std::string name;         //!< Nom
	std::string description;  //!< Description
	std::string masterLogin;  //!< Login du propriétaire (Pas stoké dans SGBD)
	Player* master = nullptr; //!< Pour les script lua uniquement

	//! Constructeur
	Alliance(Alliance::ID id,
	         uint32_t playerID,
	         std::string const& name_,
	         std::string const& description_,
	         std::string const& masterLogin_) :
		id(id),
		masterID(playerID),
		name(name_),
		description(description_),
		masterLogin(masterLogin_)
	{
	}
};


//! Donnée d'un joueur
struct Player
{
	//! Un uint8_t pour chaque Skill
	typedef boost::array<uint8_t, SkillCount> SkillTab;
	typedef uint32_t ID;           //!< Type d'identifiant
	static ID const NoId = 0;      //!< Valeur indiquant l'absence de joueur
	ID id;                         //!< Identifiant unique
	std::string login;             //!< Login
	Coord mainPlanet;              //!< Coordonées de la planète principale
	uint64_t score = 0;            //!< Score
	Alliance::ID allianceID = 0;   //!< ID de l'alliance, ou Alliance::NoID
	std::string allianceName;      //!< Nom de l'alliance (Pas stoké dans le SGBD)
	uint32_t experience = 0;       //!< Experience
	uint32_t skillpoints = 0;      //!< Points de competances
	SkillTab skilltab;             //!< Niveau du joueur dans chaque competance
	size_t unreadMessageCount = 0; //!< Nombre de message en attente de lecture
	Alliance* alliance = nullptr;  //!< Pour les script lua uniquement
	size_t planetCount = 0;        //!< Pour les script lua uniquement
	size_t fleetCount = 0;         //!< Pour les script lua uniquement
	bool isAI = false;             //!< Si une IA

	//! Constructeur
	Player(ID i, std::string const& lg):
		id(i),
		login(lg)
	{
		skilltab.assign(0);
	}
};


//! tag utiliser pour identifier le niveau dans le tutoriel avec blockly
static char const* const CoddingLevelTag = "BlocklyCodding";

#endif //__DRONEWARS_PLAYER__