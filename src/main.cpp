//! @file
//! @author Loïc HAMOT

/*! @mainpage Documentation en ligne de Neutrynos - DroneWars
 *
 * DroneWars est un jeu de conquêtes spatiales par navigateur.
 * Il fonctionne en tour par tour comme un jeu de société.
 * A chaque tour, chacune de vos planètes et chacune de vos flottes devra
 * décider quelle action accomplir. La particularité de DroneWars est qu'il
 * ne vous demande pas d'être présent pour jouer à chaque tour, mais
 * plutôt de créer une intelligence artificielle qui travaillera pour vous.
 *
 * Vous trouverez le site de gestion ici:
 * <A HREF="http://www.nogane.net/projects/dronewars">http://www.nogane.net/projects/dronewars</A>
 *
 */

#include "stdafx.h"
#include "EngineServerHandler.h"

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#pragma warning(push)
#pragma warning(disable: 4512 4100 4099 4244 4127 4267 4706)
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#pragma warning(pop)
#include <boost/make_shared.hpp>

#include <log4cplus/configurator.h>

#include "Engine.h"


using namespace std;
using namespace log4cplus;

//! Fonction main
int main()//(int argc, char** argv)
{
	static Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("main"));

	log4cplus::PropertyConfigurator::doConfigure("DroneWarsLog.properties");

	srand(static_cast<unsigned int>(time(NULL)));

	boost::locale::generator gen;
	// Specify location of dictionaries
	gen.add_messages_path(".");
	gen.add_messages_domain("DroneWars");
	// Generate locales and imbue them to iostream
	std::locale::global(gen("fr_FR.UTF8"));

#if defined(WIN32)
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if(0 != result)
	{
		return 1;
	}
#endif

	using namespace ::apache::thrift;
	using namespace ::apache::thrift::protocol;
	using namespace ::apache::thrift::transport;
	using namespace ::apache::thrift::server;

	try
	{
		int port = 9090;
		auto handler = boost::make_shared<EngineServerHandler>();
		auto proc = boost::make_shared<ndw::EngineServerProcessor>(handler);
		auto serverTransport = boost::make_shared<TServerSocket>(port);
		auto transpFactory = boost::make_shared<TBufferedTransportFactory>();
		auto protocolFactory = boost::make_shared<TBinaryProtocolFactory>();

		TSimpleServer server(
		  proc, serverTransport, transpFactory, protocolFactory);

		server.serve();
		LOG4CPLUS_ERROR(logger, "Unexpected server stop");
	}
	catch(std::exception const& ex)
	{
		std::cerr << ex.what() << std::endl;
		LOG4CPLUS_ERROR(logger, boost::diagnostic_information(ex));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
