//
// Copyright (c) 2018 Loïc HAMOT
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

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
#include <boost/program_options.hpp>
#include <boost/locale.hpp>
#pragma warning(pop)
#include <boost/make_shared.hpp>

#include "Logger.h"

#include "Engine.h"
#include "Rules.h"


using namespace std;

//! Fonction main
int main(int argc, char** argv)
{
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

	try
	{
		initLogger("DroneWarsLog.ini");

		DataBase::ConnectionInfo connInfo;

		namespace po = boost::program_options;
		size_t minRoundDuration = 10;
		// Declare the supported options.
		po::options_description desc("Allowed options");
		desc.add_options()
		("help,h", "produce help message")
		("host,a", po::value<std::string>(&connInfo.host_)->default_value("localhost"), "MySQL host")
		("port,p", po::value<uint16_t>(&connInfo.port_)->default_value(3306), "MySQL port")
		("database,d", po::value<std::string>(&connInfo.database_)->required(), "MySQL database name")
		("user,u", po::value<std::string>(&connInfo.user_)->required(), "MySQL user name")
		("password,w", po::value<std::string>(&connInfo.password_)->required(), "MySQL user password")
		("xp_coef,x", po::value<double>(&coefXP)->default_value(0.1), "XP multiplier")
		("min_round,r", po::value<size_t>(&minRoundDuration)->default_value(10), "Minimum round duration (second)")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);

		ifstream ifs("dronewars.ini");
		if(ifs.is_open())
			store(parse_config_file(ifs, desc), vm);

		if(vm.count("help"))
		{
			cout << desc << "\n";
			return EXIT_SUCCESS;
		}

		notify(vm);

		using namespace ::apache::thrift;
		using namespace ::apache::thrift::protocol;
		using namespace ::apache::thrift::transport;
		using namespace ::apache::thrift::server;

		int port = 9090;
		auto handler = boost::make_shared<EngineServerHandler>(connInfo, minRoundDuration);
		auto proc = boost::make_shared<ndw::EngineServerProcessor>(handler);
		auto serverTransport = boost::make_shared<TServerSocket>(port);
		auto transpFactory = boost::make_shared<TBufferedTransportFactory>();
		auto protocolFactory = boost::make_shared<TBinaryProtocolFactory>();

		TSimpleServer server(
		  proc, serverTransport, transpFactory, protocolFactory);

		server.serve();
		DW_LOG_ERROR << "Unexpected server stop";
	}
	catch(std::exception const& ex)
	{
		std::cerr << ex.what() << std::endl;
		DW_LOG_ERROR << boost::diagnostic_information(ex);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
