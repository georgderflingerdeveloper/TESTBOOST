#include <ctime>
#include <boost/chrono.hpp>
#include <boost/asio/ip/detail/endpoint.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>
#include <string>
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <thread>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>



using namespace boost::chrono;
using namespace boost::asio;
using namespace std;

#include <iostream>

#pragma warning(disable : 4996)

long PrintTimestamp( )
{
	auto TimePoint = high_resolution_clock::now();
	auto now       = system_clock::now();
	auto in_time_t = system_clock::to_time_t(now);

	milliseconds ms = duration_cast<milliseconds>(TimePoint.time_since_epoch());

	seconds s = duration_cast<seconds>(ms);
	time_t t = s.count();
	size_t fractional_seconds = ms.count() % 1000;

	cout << put_time(localtime(&in_time_t), "%Y-%m-%d %X:");
	cout << fractional_seconds << endl;

	return(in_time_t);
}

struct TelegrammItems
{
   boost::property_tree::ptree TelegrammTree;
   long timestamp;
   int AnalogValue1;
   int AnalogValue2;
   long SentCounter;
};

std::stringstream BuildTelegrammTree(TelegrammItems *pItems )
{
	std::stringstream Telegramm;

	pItems->TelegrammTree.put("Timestamp",    pItems->timestamp);
	pItems->TelegrammTree.put("AnalogValue1", pItems->AnalogValue1);
	pItems->TelegrammTree.put("AnalogValue2", pItems->AnalogValue2);
	pItems->TelegrammTree.put("SentCounter",  pItems->SentCounter);

	boost::property_tree::json_parser::write_json(Telegramm, pItems->TelegrammTree);

	return(Telegramm);
}

class UdpSender {

private:
	boost::asio::io_service io_service;
	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint remote_endpoint;

	void handle_send(boost::shared_ptr<std::string> /*message*/,
		const boost::system::error_code& /*error*/,
		std::size_t /*bytes_transferred*/)
	{
	}


public:

	UdpSender(const std::string& ip_address, const int port, const bool broadcast = false) : socket(io_service) {

		// Open socket
		socket.open(boost::asio::ip::udp::v4());

		// I wouldn't recommend broadcasting unless you are
		// in complete control of your subnet and know
		// what's on it and how it will react
		if (broadcast) {
			boost::asio::socket_base::broadcast option(true);
			socket.set_option(option);
		}

		// make endpoint
		remote_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::make_address(ip_address.c_str()), port);
	}



	// Send a string to the preconfigured endpoint
	// via the open socket.
	void send(const std::string& message) {	
		boost::system::error_code ignored_error;
		boost::shared_ptr<std::string> message_(
			new std::string(message));

		socket.async_send_to(boost::asio::buffer(*message_), remote_endpoint,
			boost::bind(&UdpSender::handle_send, this, message_,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));

		
	}

	// Send some binary data to the preconfigured endpoint
	// via the open socket.
	void send(const unsigned char* data, const int len) {
		boost::system::error_code ignored_error;
		socket.send_to(boost::asio::buffer(data, len), remote_endpoint, 0, ignored_error);
	}
};

int main()
{
	TelegrammItems TeleItems;
	TeleItems.AnalogValue1 = 1;
	TeleItems.AnalogValue2 = 2;
	TeleItems.SentCounter  = 0;
	TeleItems.timestamp    = PrintTimestamp();

	cout << PrintTimestamp() << endl;
    cout << BuildTelegrammTree(&TeleItems).str();

	UdpSender sender("127.0.0.1", 10000);

	for (;;)
	{
		TeleItems.SentCounter++;
		printf("Sent Telegramm : %u \n",TeleItems.SentCounter);
		sender.send( BuildTelegrammTree(&TeleItems).str() );
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

