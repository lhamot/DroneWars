//! @file
//! @author Lo�c HAMOT

#ifndef __DRONEWARS_DATABASE__
#define __DRONEWARS_DATABASE__

#include "stdafx.h"
#include <memory>
#include "Model.h"


//! Message envoy� d'un joueur � un autre
struct Message
{
	typedef uint32_t ID;     //!< Type de l'identifiant des message
	ID id;                   //!< Identifiant unique
	Player::ID sender;       //!< Joueur qui a envoy� le message
	Player::ID recipient;    //!< Joueur qui a recue le message
	time_t time;             //!< Date de l'envoie
	std::string subject;     //!< Sujet
	std::string message;     //!< Corps du message
	std::string senderLogin; //!< Login de l'�m�teur (Pas stok� dans le SGBD)

	//! Constructeur
	Message(uint32_t id,
	        Player::ID sender,
	        Player::ID recipient,
	        time_t time,
	        std::string const& subject,
	        std::string const& message,
	        std::string const& senderLogin):
		id(id),
		sender(sender),
		recipient(recipient),
		time(time),
		subject(subject),
		message(message),
		senderLogin(senderLogin)
	{
	}
};


//! Liste des demande de d'amiti� d'un joueur
struct FriendshipRequests
{
	std::vector<Player> sent;     //!< Demandes que le joueur a envoy�s
	std::vector<Player> received; //!< Demandes que le joueur a re�ues
};


namespace Poco {namespace Data {class Session;}}


//! @brief Acces � la base de donn�e SQL
//! @remark D'une mani�re g�n�ral, un id inexistant entrainera une exception
//! de type Exception
class DataBase
{
public:
	struct ConnectionInfo
	{
		std::string host_;
		uint16_t port_;
		std::string database_;
		std::string user_;
		std::string password_;
	};

	//! Signal une erreur au niveau du SGBD
	class Exception : public std::runtime_error
	{
	public:
		//! Constructeur
		Exception(std::string const& message): std::runtime_error(message) {}
	};

	DataBase(ConnectionInfo const& connection);
	~DataBase();

	//***************************  Player  ************************************

	//! Cr�e un nouveau joueur
	Player::ID addPlayer(std::string const& login,
	                     std::string const& password,
	                     std::vector<std::string> const& codes);

	//! D�finie la plan�te principale d'un joueur
	void setPlayerMainPlanet(Player::ID, Coord mainPlanet);

	//! Extrait les donn�es d'un joueur si il existe
	boost::optional<Player> getPlayer(
	  std::string const& login, std::string const& password) const;

	//! @brief Extrait les donn�es un joueur existant
	//! @throw Exception si pid invalid
	Player getPlayer(Player::ID pid) const;

	//! Extrait la liste des joueurs
	std::vector<Player> getPlayers() const;

	//! Extrait la liste des joueurs
	std::map<Player::ID, Player> getPlayerMap() const;

	//! @brief Supprime le joueur
	//!
	//! Change le login et la mot de passe de mani�re que personne ne peut
	//! plus ce connecter a ce compte. Les code source sont vid�s.
	void eraseAccount(Player::ID pid);

	//! Met a jour le score des joueurs
	void updateScore(std::map<Player::ID, uint64_t> const& scoreMap);

	//! Met a jour l'XP des joueurs
	void updateXP(std::map<Player::ID, uint32_t> const& expMap);

	//! @brief G�re l'achat d'un skill
	//!
	//! Inscremente le skill skillID du joueur pid, et reduit ses skillpoints
	//! de cost
	//! @throw Exception si pid invalid
	//! @pre skillID est valid
	//! @return false si pas assez de skillpoint, sinon true
	bool buySkill(Player::ID pid, int16_t skillID);

	//! @brief Definie les niveau des skill d'un joueur (Util pour les IA)
	void setPlayerSkills(Player::ID pid, Player::SkillTab const& skillTab);

	//***************************  Event  *************************************

	//! Ajoute des eventements Event dans la base
	void addEvents(std::vector<Event> const& events);

	//! Extrait les evenements li� au joueur pid
	std::vector<Event> getPlayerEvents(Player::ID pid) const;

	//! Extrait les evenements li� au joueur pid, et � la plan�te pcoord
	std::vector<Event> getPlanetEvents(Player::ID pid, Coord pcoord) const;

	//! Extrait les evenements li� au joueur pid, et � la flotte fid
	std::vector<Event> getFleetEvents(Player::ID pid, Fleet::ID fid) const;

	//! Supprime les �venements de la plan�te pcoord
	void resetPlanetEvents(Coord pcoord);

	//! Supprime les �venements trop vieux
	void removeOldEvents(std::map<Player::ID, size_t> const& maxEventCount);

	//***************************  Script  ************************************

	//! Ajoute un nouveau code lua de plan�te ou flotte � un joueur
	size_t addScript(Player::ID pid,
	                 CodeData::Target target,
	                 std::string const& code);

	//! Ajoute un nouveau code blockly de plan�te ou flotte � un joueur
	size_t addBlocklyCode(Player::ID pid,
	                      CodeData::Target target,
	                      std::string const& code);

	//! @brief Extrait le dernier code plan�te ou flotte d'un joueur
	//! @remark l'id du CodeData sera 0 si code introuvable
	CodeData getPlayerCode(Player::ID pid, CodeData::Target) const;

	//! Donn�e sur une erreur dans un script
	struct CodeError
	{
		size_t codeDataId;   //!< ID du CodeData
		std::string message; //!< Message d'erreur (En anglais)
	};
	//! Ajoute � la base tout les CodeError g�n�r� dans un round
	void addCodeErrors(std::vector<CodeError> const& errors);

	//***************************  FightReport  *******************************

	//! @brief Ajoute un rapport de combat � la base
	//! @return ID du rapport de combat
	size_t addFightReport(FightReport const& report);

	//! @brief Ajoute une liste de Rapport de combat
	//! @return Dernier ID inser�
	size_t addFightReports(std::vector<FightReport> const& reports);

	//! @brief Extrait un rapport de combat depuit la base
	//! @throw Exception si reportID est invalid
	FightReport getFightReport(size_t reportID);

	//***************************  Tutos  *************************************

	//! Incremente un tag de tutoriel d'une liste de joueur
	void incrementTutoDisplayed(std::vector<Player::ID> const& pid,
	                            std::string const& tutoName);

	//! @brief Incremente un tag de tutoriel d'un joueur
	//! @throw Exception si pid invalide
	void incrementTutoDisplayed(Player::ID pid,
	                            std::string const& tutoName,
	                            int32_t value);

	//! Tableau assosiatif [tag de tutos] => niveau
	typedef std::map<std::string, size_t> PlayerTutoMap;
	//! @brief Extrait la liste des valeurs de toturiel d'un joueur
	//! @return Un tableau associant chaque tag de tutos � son niveau
	PlayerTutoMap getTutoDisplayed(Player::ID pid) const;

	//! @brief Extrait la liste des valeurs de toturiel de tout les joueurs
	//! @return Un tableau assiciatif de structure [Player::ID][tag] => niveau
	std::map<Player::ID, PlayerTutoMap> getAllTutoDisplayed() const;

	//***************************  Messages  **********************************

	//! @brief Ajoute un Message dans la base de donn�e
	//! @throw Exception si sender ou recipient invalide
	void addMessage(Player::ID sender,
	                Player::ID recipient,
	                std::string const& suject,
	                std::string const& message);

	//! @brief Extrait la liste des m�ssages re�ues par un joueur
	//! @remark le retour sera vide si recipient est invalide
	std::vector<Message> getMessages(Player::ID recipient);

	//! Supprime un m�ssage donn�e
	void eraseMesage(Message::ID mid);

	//***************************  Friendship  ********************************

	//! @brief Ajoute une demande d'amiti� dans la base
	//! @throw Exception si sender ou recipient invalide
	void addFriendshipRequest(Player::ID sender, Player::ID recipient);

	//! Accepte ou refuse une demande d'amiti�, si elle �xiste
	void acceptFriendshipRequest(
	  Player::ID sender, Player::ID recipient, bool accept);

	//! Termine une amiti�, si elle �xiste
	void closeFriendship(Player::ID playerA, Player::ID playerB);

	//! Extrait la liste des amies d'un joueur
	std::vector<Player> getFriends(Player::ID player) const;

	//! Extrait la liste des demande d'amiti�s li� � un joueur
	FriendshipRequests getFriendshipRequest(Player::ID player) const;

	//***************************  Alliance  **********************************

	//! @brief Cr�e un alliance
	//! @throw Exception si pid invalide
	//! @return ID de la nouvel alliance, ou Alliance::NoId, si doublon
	Alliance::ID addAlliance(Player::ID pid,
	                         std::string const& name,
	                         std::string const& description);

	//! @brief Extrait les donn�es d'une alliance
	//! @remark si aid invalide l'ID de l'alliance sera zero
	Alliance getAlliance(Alliance::ID aid) const;

	//! @brief Extrait toutes les alliances
	std::vector<Alliance> getAlliances() const;

	//! Modifie les donn�es d'une alliance, si elle existe
	void updateAlliance(Alliance const& alliance);

	//! @brief Transfert la propri�t� d'une alliance � un joueur
	//!
	//! C'est a dire que la joueur devient l'unique propri�taire de l'alliance
	//! @remark N'as aucun effet si le joueur n'est pas membre de l'alliance
	//! @throw Exception si pid invalide
	void transfertAlliance(Alliance::ID aid, Player::ID pid);

	//! @brief Supprime une alliance, si elle existe
	//!
	//! Le propri�taire ne possede plus d'alliance.
	//! Les membres n'ont plus d'alliance.
	//! L'alliance est supprim� de la base
	void eraseAlliance(Alliance::ID aid);

	//! @brief Un joueur rejoin une alliance
	//! @remark N'as aucun effet si pid invalide
	//! @throw Exception si aid invalide
	void joinAlliance(Player::ID pid, Alliance::ID aid);

	//! @brief Un joueur quite son alliance
	//!
	//! Le joueur n'as plus d'alliance.
	//! Si il �tait le propri�taire, l'alliance est dissoute (eraseAlliance)
	//! @throw Exception si pid invalide
	void quitAlliance(Player::ID pid);
private:
	ConnectionInfo connectionInfo_;

	//! Handle de connection a la base de donn�e
	mutable std::unique_ptr<Poco::Data::Session> session_;

	//! Ajoute un script lua plan�te ou flotte, � un joueur, sans transaction
	void addScriptImpl(Player::ID pid,
	                   CodeData::Target target,
	                   std::string const& code);

	//! Ajoute un code blockly plan�te ou flotte, � un joueur, sans transaction
	void addBlocklyCodeImpl(Player::ID pid,
	                        CodeData::Target target,
	                        std::string const& code);
	void checkConnection(std::unique_ptr<Poco::Data::Session>& session) const;

};

#endif //__DRONEWARS_DATABASE__
