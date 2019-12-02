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





class boost_udp_send_faf {

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

	boost_udp_send_faf(const std::string& ip_address, const int port, const bool broadcast = false) : socket(io_service) {

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
			boost::bind(&boost_udp_send_faf::handle_send, this, message_,
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


using boost::asio::ip::udp;

std::string make_daytime_string()
{
	using namespace std; // For time_t, time and ctime;
	time_t now = time(0);
	return ctime(&now);
}

class udp_server
{
public:
	udp_server(boost::asio::io_context& io_context)
		: socket_(io_context, udp::endpoint(udp::v4(), 13))
	{
		start_receive();
	}

private:
	void start_receive()
	{
		socket_.async_receive_from(
			boost::asio::buffer(recv_buffer_), remote_endpoint_,
			boost::bind(&udp_server::handle_receive, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void handle_receive(const boost::system::error_code& error,
		std::size_t /*bytes_transferred*/)
	{
		if (!error)
		{
			boost::shared_ptr<std::string> message(
				new std::string(make_daytime_string()));

			socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_,
				boost::bind(&udp_server::handle_send, this, message,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));

			start_receive();
		}
	}

	void handle_send(boost::shared_ptr<std::string> /*message*/,
		const boost::system::error_code& /*error*/,
		std::size_t /*bytes_transferred*/)
	{
	}

	udp::socket socket_;
	udp::endpoint remote_endpoint_;
	boost::array<char, 1> recv_buffer_;
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

	boost_udp_send_faf sender("127.0.0.1", 10000);

	for (;;)
	{
		TeleItems.SentCounter++;
		printf("Sent Telegramm : %u \n",TeleItems.SentCounter);
		sender.send( BuildTelegrammTree(&TeleItems).str() );
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

// Programm ausführen: STRG+F5 oder "Debuggen" > Menü "Ohne Debuggen starten"
// Programm debuggen: F5 oder "Debuggen" > Menü "Debuggen starten"

// Tipps für den Einstieg: 
//   1. Verwenden Sie das Projektmappen-Explorer-Fenster zum Hinzufügen/Verwalten von Dateien.
//   2. Verwenden Sie das Team Explorer-Fenster zum Herstellen einer Verbindung mit der Quellcodeverwaltung.
//   3. Verwenden Sie das Ausgabefenster, um die Buildausgabe und andere Nachrichten anzuzeigen.
//   4. Verwenden Sie das Fenster "Fehlerliste", um Fehler anzuzeigen.
//   5. Wechseln Sie zu "Projekt" > "Neues Element hinzufügen", um neue Codedateien zu erstellen, bzw. zu "Projekt" > "Vorhandenes Element hinzufügen", um dem Projekt vorhandene Codedateien hinzuzufügen.
//   6. Um dieses Projekt später erneut zu öffnen, wechseln Sie zu "Datei" > "Öffnen" > "Projekt", und wählen Sie die SLN-Datei aus.
