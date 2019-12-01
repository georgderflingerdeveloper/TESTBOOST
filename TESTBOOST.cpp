#include <ctime>
#include <boost/chrono.hpp>
#include <boost/asio/ip/detail/endpoint.hpp>
#include <boost/asio/io_service.hpp>
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <sstream>
#include <cstdlib>


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

int main()
{
	TelegrammItems TeleItems;
	TeleItems.AnalogValue1 = 1;
	TeleItems.AnalogValue2 = 2;
	TeleItems.SentCounter  = 3;
	TeleItems.timestamp    = PrintTimestamp();

	cout << PrintTimestamp() << endl;

	cout << BuildTelegrammTree(&TeleItems).str();
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
