//! @file
//! @author Loïc HAMOT

#ifndef __DRONEWARS_DATABASE__
#define __DRONEWARS_DATABASE__

#include "stdafx.h"
#include <memory>
#include "Model.h"


//! Message envoyé d'un joueur à un autre
struct Message
{
	typedef uint32_t ID;     //!< Type de l'identifiant des message
	ID id;                   //!< Identifiant unique
	Player::ID sender;       //!< Joueur qui a envoyé le message
	Player::ID recipient;    //!< Joueur qui a recue le message
	time_t time;             //!< Date de l'envoie
	std::string subject;     //!< Sujet
	std::string message;     //!< Corps du message
	std::string senderLogin; //!< Login de l'éméteur (Pas stoké dans le SGBD)

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


//! Liste des demande de d'amitié d'un joueur
struct FriendshipRequests
{
	std::vector<Player> sent;     //!< Demandes que le joueur a envoyés
	std::vector<Player> received; //!< Demandes que le joueur a reçues
};


namespace Poco {namespace Data {class Session;}}


//! @brief Acces à la base de donnée SQL
//! @remark D'une manière général, un id inexistant entrainera une exception
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

	//! Crée un nouveau joueur
	Player::ID addPlayer(std::string const& login,
	                     std::string const& password,
	                     std::vector<std::string> const& codes);

	//! Définie la planète principale d'un joueur
	void setPlayerMainPlanet(Player::ID, Coord mainPlanet);

	//! Extrait les données d'un joueur si il existe
	boost::optional<Player> getPlayer(
	  std::string const& login, std::string const& password) const;

	//! @brief Extrait les données un joueur existant
	//! @throw Exception si pid invalid
	Player getPlayer(Player::ID pid) const;

	//! Extrait la liste des joueurs
	std::vector<Player> getPlayers() const;

	//! Extrait la liste des joueurs
	std::map<Player::ID, Player> getPlayerMap() const;

	//! @brief Supprime le joueur
	//!
	//! Change le login et la mot de passe de manière que personne ne peut
	//! plus ce connecter a ce compte. Les code source sont vidés.
	void eraseAccount(Player::ID pid);

	//! Met a jour le score des joueurs
	void updateScore(std::map<Player::ID, uint64_t> const& scoreMap);

	//! Met a jour l'XP des joueurs
	void updateXP(std::map<Player::ID, uint32_t> const& expMap);

	//! @brief Gère l'achat d'un skill
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

	//! Extrait les evenements lié au joueur pid
	std::vector<Event> getPlayerEvents(Player::ID pid) const;

	//! Extrait les evenements lié au joueur pid, et à la planète pcoord
	std::vector<Event> getPlanetEvents(Player::ID pid, Coord pcoord) const;

	//! Extrait les evenements lié au joueur pid, et à la flotte fid
	std::vector<Event> getFleetEvents(Player::ID pid, Fleet::ID fid) const;

	//! Supprime les évenements de la planète pcoord
	void resetPlanetEvents(Coord pcoord);

	//! Supprime les évenements trop vieux
	void removeOldEvents(std::map<Player::ID, size_t> const& maxEventCount);

	//***************************  Script  ************************************

	//! Ajoute un nouveau code lua de planète ou flotte à un joueur
	size_t addScript(Player::ID pid,
	                 CodeData::Target target,
	                 std::string const& code);

	//! Ajoute un nouveau code blockly de planète ou flotte à un joueur
	size_t addBlocklyCode(Player::ID pid,
	                      CodeData::Target target,
	                      std::string const& code);

	//! @brief Extrait le dernier code planète ou flotte d'un joueur
	//! @remark l'id du CodeData sera 0 si code introuvable
	CodeData getPlayerCode(Player::ID pid, CodeData::Target) const;

	//! Donnée sur une erreur dans un script
	struct CodeError
	{
		size_t codeDataId;   //!< ID du CodeData
		std::string message; //!< Message d'erreur (En anglais)
	};
	//! Ajoute à la base tout les CodeError généré dans un round
	void addCodeErrors(std::vector<CodeError> const& errors);

	//***************************  FightReport  *******************************

	//! @brief Ajoute un rapport de combat à la base
	//! @return ID du rapport de combat
	size_t addFightReport(FightReport const& report);

	//! @brief Ajoute une liste de Rapport de combat
	//! @return Dernier ID inseré
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
	//! @return Un tableau associant chaque tag de tutos à son niveau
	PlayerTutoMap getTutoDisplayed(Player::ID pid) const;

	//! @brief Extrait la liste des valeurs de toturiel de tout les joueurs
	//! @return Un tableau assiciatif de structure [Player::ID][tag] => niveau
	std::map<Player::ID, PlayerTutoMap> getAllTutoDisplayed() const;

	//***************************  Messages  **********************************

	//! @brief Ajoute un Message dans la base de donnée
	//! @throw Exception si sender ou recipient invalide
	void addMessage(Player::ID sender,
	                Player::ID recipient,
	                std::string const& suject,
	                std::string const& message);

	//! @brief Extrait la liste des méssages reçues par un joueur
	//! @remark le retour sera vide si recipient est invalide
	std::vector<Message> getMessages(Player::ID recipient);

	//! Supprime un méssage donnée
	void eraseMesage(Message::ID mid);

	//***************************  Friendship  ********************************

	//! @brief Ajoute une demande d'amitié dans la base
	//! @throw Exception si sender ou recipient invalide
	void addFriendshipRequest(Player::ID sender, Player::ID recipient);

	//! Accepte ou refuse une demande d'amitié, si elle éxiste
	void acceptFriendshipRequest(
	  Player::ID sender, Player::ID recipient, bool accept);

	//! Termine une amitié, si elle éxiste
	void closeFriendship(Player::ID playerA, Player::ID playerB);

	//! Extrait la liste des amies d'un joueur
	std::vector<Player> getFriends(Player::ID player) const;

	//! Extrait la liste des demande d'amitiés lié à un joueur
	FriendshipRequests getFriendshipRequest(Player::ID player) const;

	//***************************  Alliance  **********************************

	//! @brief Crée un alliance
	//! @throw Exception si pid invalide
	//! @return ID de la nouvel alliance, ou Alliance::NoId, si doublon
	Alliance::ID addAlliance(Player::ID pid,
	                         std::string const& name,
	                         std::string const& description);

	//! @brief Extrait les données d'une alliance
	//! @remark si aid invalide l'ID de l'alliance sera zero
	Alliance getAlliance(Alliance::ID aid) const;

	//! @brief Extrait toutes les alliances
	std::vector<Alliance> getAlliances() const;

	//! Modifie les données d'une alliance, si elle existe
	void updateAlliance(Alliance const& alliance);

	//! @brief Transfert la propriété d'une alliance à un joueur
	//!
	//! C'est a dire que la joueur devient l'unique propriétaire de l'alliance
	//! @remark N'as aucun effet si le joueur n'est pas membre de l'alliance
	//! @throw Exception si pid invalide
	void transfertAlliance(Alliance::ID aid, Player::ID pid);

	//! @brief Supprime une alliance, si elle existe
	//!
	//! Le propriétaire ne possede plus d'alliance.
	//! Les membres n'ont plus d'alliance.
	//! L'alliance est supprimé de la base
	void eraseAlliance(Alliance::ID aid);

	//! @brief Un joueur rejoin une alliance
	//! @remark N'as aucun effet si pid invalide
	//! @throw Exception si aid invalide
	void joinAlliance(Player::ID pid, Alliance::ID aid);

	//! @brief Un joueur quite son alliance
	//!
	//! Le joueur n'as plus d'alliance.
	//! Si il était le propriétaire, l'alliance est dissoute (eraseAlliance)
	//! @throw Exception si pid invalide
	void quitAlliance(Player::ID pid);
private:
	ConnectionInfo connectionInfo_;

	//! Handle de connection a la base de donnée
	mutable std::unique_ptr<Poco::Data::Session> session_;

	//! Ajoute un script lua planète ou flotte, à un joueur, sans transaction
	void addScriptImpl(Player::ID pid,
	                   CodeData::Target target,
	                   std::string const& code);

	//! Ajoute un code blockly planète ou flotte, à un joueur, sans transaction
	void addBlocklyCodeImpl(Player::ID pid,
	                        CodeData::Target target,
	                        std::string const& code);
	void checkConnection(std::unique_ptr<Poco::Data::Session>& session) const;

};

#endif //__DRONEWARS_DATABASE__
