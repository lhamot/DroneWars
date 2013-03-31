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

#include <log4cplus/configurator.h>

#include "Engine.h"


using namespace std;
using namespace log4cplus;
static Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("main"));


int main()//(int argc, char** argv)
{
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
		boost::shared_ptr<EngineServerHandler> handler(new EngineServerHandler());
		boost::shared_ptr<TProcessor> processor(new ndw::EngineServerProcessor(handler));
		boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
		boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
		boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

		TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

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
