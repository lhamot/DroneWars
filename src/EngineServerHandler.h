//! @file
//! @author Loïc HAMOT

#undef printf //A cause de libintl qui a une macro printf, et thrift qui definie une methode printf
#pragma warning(push)
#pragma warning(disable: 4512 4100 4099 4244 4127 4267 4706 4616 4396)
#include "gen-cpp/EngineServer.h"
#include "gen-cpp/thrift_types.h"
#pragma warning(pop)
#include "Engine.h"


//! Server TCP
class EngineServerHandler : virtual public ndw::EngineServerIf
{
public:

	//! Exception signalant un argument erroné
	struct InvalidData : public std::runtime_error
	{
		//! Constructeur
		InvalidData(std::string const& str): std::runtime_error(str) {}
	};

	//! Constructeur
	EngineServerHandler(DataBase::ConnectionInfo const& connInfo,
	                    size_t MinRoundDuration);

	//! Lance le serveur (commence l'écoute)
	void start() override;

	//! Stop le serveur (arrète l'écoute)
	void stop() override;

	//! @brief Ajoute un joueur dans le système, si pas de doublon de login
	//! @return true si ajout effectué, sinon false
	bool addPlayer(const std::string& login, const std::string& password) override;

	//! @brief Extrait une plage des flottes d'un joueur
	//! @throw runtime_error si argument invalide
	void getPlayerFleets(ndw::FleetList& _return,
	                     const ndw::Player_ID pid,
	                     const int32_t beginIndex,
	                     const int32_t endIndex,
	                     const ndw::Sort_Type::type sortType,
	                     const bool asc,
	                     const int32_t value) override;

	//! @brief Extrait une plage des planètes d'un joueur
	//! @throw runtime_error si argument invalide
	void getPlayerPlanets(ndw::PlanetList& _return,
	                      const ndw::Player_ID pid,
	                      const int32_t beginIndex,
	                      const int32_t endIndex,
	                      const  ndw::Sort_Type::type sortType,
	                      const bool asc,
	                      const int32_t value) override;

	//! @brief Change le code (lua) de flotte d'un joueur
	//! @throw DataBase::Exception si pid invalide
	//! @throw InvalidData si code.size() > Player::MaxCodeSize
	void setPlayerFleetCode(const ndw::Player_ID pid, const std::string& code) override;

	//! @brief Change le code (lua) de planète d'un joueur
	//! @throw DataBase::Exception si pid invalide
	//! @throw InvalidData si code.size() > Player::MaxCodeSize
	void setPlayerPlanetCode(const ndw::Player_ID pid,
	                         const std::string& code) override;

	//! @brief Change le code (blockly) de flotte d'un joueur
	//! @throw DataBase::Exception si pid invalide
	//! @throw InvalidData si code.size() > Player::MaxBlocklySize
	void setPlayerFleetBlocklyCode(const ndw::Player_ID pid,
	                               const std::string& code) override;

	//! @brief Change le code (blockly) de planète d'un joueur
	//! @throw DataBase::Exception si pid invalide
	//! @throw InvalidData si code.size() > Player::MaxBlocklySize
	void setPlayerPlanetBlocklyCode(const ndw::Player_ID pid,
	                                const std::string& code) override;

	//! @brief Extrait le dernier code source de flotte d'un joueur
	//! @remark l'id du CodeData sera 0 si code introuvable
	void getPlayerFleetCode(ndw::CodeData& _return, const ndw::Player_ID pid) override;

	//! @brief Extrait le dernier code source de planète d'un joueur
	//! @remark l'id du CodeData sera 0 si code introuvable
	void getPlayerPlanetCode(ndw::CodeData& _return, const ndw::Player_ID pid) override;

	//! Extrait la liste des joueurs
	void getPlayers(std::vector<ndw::Player>& _return) override;

	//! @brief Extrait un joueur donné
	//! @throw DataBase::Exception si pid invalid
	void getPlayer(ndw::Player& _return, const ndw::Player_ID pid) override;

	//! @brief Extrait une planète donné si elle existe
	//! @post _return.size() <= 1
	void getPlanet(std::vector<ndw::Planet>& _return, const ndw::Coord& coord) override;

	//! @brief Extrait une flotte donné
	//! @throw std::runtime_error si la flotte n'éxiste pas
	void getFleet(ndw::Fleet& _return, const ndw::Fleet_ID fid) override;

	//! Extrait les donné d'un joueur si les idantifiants sont corrects
	void logPlayer(ndw::OptionalPlayer& _return,
	               const std::string& login,
	               const std::string& password) override;

	//! @brief Incremente le niveau d'avancement d'un tutos d'un joueur
	//! @throw DataBase::Exception si pid invalide
	void incrementTutoDisplayed(const ndw::Player_ID pid,
	                            const std::string& tutoName) override;

	//! @brief Extrait un rapport de combat a un id donné
	//! @throw DataBase::Exception si reportID est invalid
	void getFightReport(ndw::FightReport& _return, const int32_t id) override;

	//! Extrait des info sur l'age du simulateur et du round courrant
	void getTimeInfo(ndw::TimeInfo& _return) override;

	//! @brief Supprime un compe utilisateur si il existe
	//! @throw DataBase::Exception si pid invalid
	bool eraseAccount(const ndw::Player_ID pid, const std::string& password) override;

	//! @brief Extrait tout les Event d'un joueur
	//! @remark si pid invalide, la liste est vide
	void getPlayerEvents(std::vector<ndw::Event>& _return,
	                     const ndw::Player_ID pid) override;

	//! @brief Gère l'achat d'un slill
	//!
	//! Inscremente le skill skillID du joueur pid, et reduit ses skillpoints
	//! @throw Exception si pid invalid
	//! @return false si pas asser de skillpoint, ou skillID trop grand
	bool buySkill(const ndw::Player_ID pid, const int16_t skillID) override;

	//! Recupère les info sur tout les type de Building
	void getBuildingsInfo(std::vector<ndw::Building>& _return) override;

	//! Recupère les info sur tout les type de Cannon
	void getCannonsInfo(std::vector<ndw::Cannon>& _return) override;

	//! Recupère les info sur tout les type de vaisseaux (Ship)
	void getShipsInfo(std::vector<ndw::Ship>& _return) override;

	//***************************  Messages  **********************************

	//! @brief Ajoute un Message dans la base de donnée
	//! @throw Exception si sender ou recipient invalide
	void addMessage(const ndw::Player_ID sender,
	                const ndw::Player_ID recipient,
	                const std::string& suject,
	                const std::string& message) override;

	//! @brief Extrait la liste des méssages reçues par un joueur
	//! @remark le retour sera vide si recipient est invalide
	void getMessages(std::vector<ndw::Message>& _return,
	                 const ndw::Player_ID recipient) override;

	//! Supprime un méssage donnée
	void eraseMesage(const ndw::Message_ID mid) override;

	//***************************  Friendship  ********************************

	//! Redirection vers DataBase::addFriendshipRequest
	void addFriendshipRequest(const ndw::Player_ID playerA,
	                          const ndw::Player_ID playerB) override;

	//! Redirection vers DataBase::acceptFriendshipRequest
	void acceptFriendshipRequest(const ndw::Player_ID playerA,
	                             const ndw::Player_ID playerB,
	                             const bool accept) override;

	//! Redirection vers DataBase::closeFriendship
	void closeFriendship(const ndw::Player_ID playerA,
	                     const ndw::Player_ID playerB);

	//! Redirection vers DataBase::getFriends
	void getFriends(std::vector<ndw::Player>& _return,
	                const ndw::Player_ID player) override;

	//! Redirection vers DataBase::getFriendshipRequest
	void getFriendshipRequest(ndw::FriendshipRequests& _return,
	                          const ndw::Player_ID player) override;

	//***************************  Alliance  **********************************

	//! Redirection vers DataBase::addAlliance
	ndw::Alliance_ID addAlliance(const ndw::Player_ID pid,
	                             const std::string& name,
	                             const std::string& description) override;

	//! Redirection vers DataBase::getAlliance
	void getAlliance(ndw::Alliance& _return, const ndw::Alliance_ID aid);

	//! Redirection vers DataBase::updateAlliance
	void updateAlliance(const ndw::Alliance& alliance) override;

	//! Redirection vers DataBase::transfertAlliance
	void transfertAlliance(const ndw::Alliance_ID aid,
	                       const ndw::Player_ID pid) override;

	//! Redirection vers DataBase::eraseAlliance
	void eraseAlliance(const ndw::Alliance_ID aid) override;

	//! Redirection vers DataBase::joinAlliance
	void joinAlliance(const ndw::Player_ID pid, const ndw::Alliance_ID aid) override;

	//! Redirection vers DataBase::quitAlliance
	void quitAlliance(const ndw::Player_ID pid) override;

private:
	Engine engine_;     //!< Acces à la simulation et ces données
	DataBase database_; //!< Acces à la base de donnée
};