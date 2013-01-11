#include "stdafx.h"
#include "EngineServerHandler.h"
#pragma warning(push)
#pragma warning(disable: 4512 4100 4099 4244 4127 4267 4706)
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#pragma warning(pop)
#include "Engine.h"


using namespace std;


int main()//(int argc, char** argv)
{
	srand(static_cast<unsigned int>(time(NULL)));

	putenv(const_cast<char*>("LANG=fr_FR"));
#ifdef _WIN32
	printf("Locale is: %s\n", setlocale(LC_ALL, "fr"));
#else
	printf("Locale is: %s\n", setlocale(LC_ALL, "fr_FR.UTF8"));
#endif
	bindtextdomain("DroneWars", "./");
	textdomain("DroneWars");

	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if(0 != result)
	{
		return 1;
	}

	using namespace ::apache::thrift;
	using namespace ::apache::thrift::protocol;
	using namespace ::apache::thrift::transport;
	using namespace ::apache::thrift::server;

	int port = 9090;
	boost::shared_ptr<EngineServerHandler> handler(new EngineServerHandler());
	boost::shared_ptr<TProcessor> processor(new ndw::EngineServerProcessor(handler));
	boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

	TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

	try
	{
		server.serve();
	}
	catch(std::exception const& ex)
	{
		std::cerr << typeid(ex).name() << " " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
