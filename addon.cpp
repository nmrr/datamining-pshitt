/*
MIT License

Copyright 2017 nmrr (https://github.com/nmrr)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <node.h>
#include <nan.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <algorithm>
#include <fstream>
#include <random>
#include <mutex>
#include <unistd.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "countrycode.h"

#define BUFFER_SIZE_MAX 1024*1024*32

using namespace Nan;
using namespace std;
using namespace v8;

//////////////////////////////////////////////

// Classe permettant de donner le pays correspondant à une IP
class IP
{
private:

    bool fichierCharge = false;
    bool erreur = false;
    string erreurMessage = "";

    struct ipRange
    {
        uint32_t start, end;
        uint16_t country;
    };

    vector<ipRange> vectorIpRange;

    map<string, uint32_t> mapPays;
    vector<string> vectorPays;
    vector<string> vectorCode;
    vector<string> vectorCodeAlpha3;

    int arrayPrefixIP[256];

    uint8_t getPrefixIP(uint32_t ip)
    {
        return (ip & 0xFF000000) >> 24;
    }

public:

    uint32_t getVectorPaysSize()
    {
        return vectorPays.size();
    }

    string countryCodeToCODE(uint16_t country)
    {
        if (country < vectorCode.size())
        {
            return vectorCode[country];
        }

        return "ERROR : OUT OF RANGE";
    }

    string countryCodeToAlpha3(uint16_t country)
    {
        if (country < vectorCodeAlpha3.size())
        {
            return vectorCodeAlpha3[country];
        }

        return "ERROR : OUT OF RANGE";
    }


    string countryCodeToCountryString(uint16_t country)
    {
        if (country < vectorPays.size())
        {
            return vectorPays[country];
        }

        return "ERROR : OUT OF RANGE";
    }

    string getErreurMessage()
    {
        return erreurMessage;
    }

    bool getErreur()
    {
        return erreur;
    }

    uint32_t charToIntIP(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    {
        uint32_t ip = a;
        ip <<= 8;
        ip |= b;
        ip <<= 8;
        ip |= c;
        ip <<= 8;
        ip |= d;
        return ip;
    }

    string intToStringIP(uint32_t ip)
    {
        uint32_t array[4];
        array[0] = (ip & 0xFF000000) >> 24;
        array[1] = (ip & 0x00FF0000) >> 16;
        array[2] = (ip & 0x0000FF00) >> 8;
        array[3] = (ip & 0x000000FF);

        return to_string(array[0]) + "." + to_string(array[1]) + "." + to_string(array[2]) + "." + to_string(array[3]);
    }

    uint32_t stringToIntIP(string ip)
    {
        string nombre = "";
        uint32_t IP_int = 0;

        for (uint32_t i=0; i<ip.length(); i++)
        {
            if (ip[i] == '.')
            {
                try
                {
                    IP_int |= stol(nombre);
                }
                catch(...)
                {
                    IP_int |= 0;
                }
                IP_int <<= 8;
                nombre = "";
            }
            else
            {
                nombre += ip[i];
            }
        }

        try
        {
            IP_int |= stol(nombre);
        }
        catch(...)
        {
            IP_int |= 0;
        }

        return IP_int;
    }

    IP()
    {

    }

    // Méthode de lecture de la base de données des plages IP
    void readFile(string nomFichier)
    {
        for (int i=0; i<256; i++) arrayPrefixIP[i] = -1;
        vectorPays.push_back("Unknown country");
        vectorCode.push_back("UNKNOWN");
        vectorCodeAlpha3.push_back("UNKNOWN");

        ifstream file (nomFichier, ios::in|ios::binary|ios::ate);
        if (file.is_open())
        {
            char * buffer  = NULL;
            size_t bufferSize = 0;
            const size_t fileSize = file.tellg();

            if (fileSize > BUFFER_SIZE_MAX) bufferSize = BUFFER_SIZE_MAX;
            else bufferSize = fileSize;

            try
            {
                buffer = new char [bufferSize];
            }
            catch (...)
            {
                erreur = true;
                erreurMessage = "Unable the create the buffer";
                delete [] buffer;
                file.close();
                return;
            }

            size_t bufferPosition = 0;
            string ligne = "";

            for (size_t j = 0; j < fileSize; j++)
            {
                if (bufferPosition == bufferSize || j == 0)
                {
                    if (fileSize - j < bufferSize)
                    {
                        bufferSize = fileSize - j;
                    }

                    try
                    {
                        file.seekg (j, ios::beg);
                        file.read (buffer, bufferSize);
                    }
                    catch (...)
                    {
                        erreur = true;
                        erreurMessage = "Unable to read the file";
                        delete [] buffer;
                        file.close();
                        return;
                    }

                    bufferPosition = 0;
                }

                if (buffer[bufferPosition] == 10)
                {
                    uint32_t IP_start = 0, IP_end = 0;
                    string IP_code = "", IP_country = "";

                    uint32_t position = 0;
                    uint32_t compteur = 0;
                    for (uint32_t j=0; j<ligne.length(); j++)
                    {
                        if (ligne[j] == '"') compteur++;
                        if (compteur == 5)
                        {
                            position = j;
                            break;
                        }
                    }

                    string tmp = "";
                    for (uint32_t j=position+1; j<ligne.length(); j++)
                    {
                        if (ligne[j] !='"')
                        {
                            tmp += ligne[j];
                        }
                        else
                        {
                            IP_start = stol(tmp);
                            position = j+2;
                            break;
                        }
                    }

                    tmp = "";
                    for (uint32_t j=position+1; j<ligne.length(); j++)
                    {
                        if (ligne[j] !='"')
                        {
                            tmp += ligne[j];
                        }
                        else
                        {
                            IP_end = stol(tmp);
                            position = j+2;
                            break;
                        }
                    }

                    tmp = "";
                    for (uint32_t j=position+1; j<ligne.length(); j++)
                    {
                        if (ligne[j] !='"')
                        {
                            tmp += ligne[j];
                        }
                        else
                        {
                            IP_code = tmp;
                            position = j+2;
                            break;
                        }
                    }

                    tmp = "";
                    for (uint32_t j=position+1; j<ligne.length(); j++)
                    {
                        if (ligne[j] !='"')
                        {
                            tmp += ligne[j];
                        }
                        else
                        {
                            IP_country = tmp;
                            break;
                        }
                    }

                    uint8_t prefixIP = getPrefixIP(IP_start);
                    if (arrayPrefixIP[prefixIP] == -1)
                    {
                        arrayPrefixIP[prefixIP] = vectorIpRange.size();
                    }

                    map<string,uint32_t>::iterator it = mapPays.find(IP_country);
                    if (it == mapPays.end())
                    {
                        ipRange tmp;
                        tmp.start = IP_start;
                        tmp.end = IP_end;
                        tmp.country = vectorPays.size();

                        mapPays[IP_country] = vectorPays.size();
                        vectorIpRange.push_back(tmp);
                        vectorPays.push_back(IP_country);
                        vectorCode.push_back(IP_code);
                        vectorCodeAlpha3.push_back(Alpha2toAlpha3(IP_code));
                    }
                    else
                    {
                        ipRange tmp;
                        tmp.start = IP_start;
                        tmp.end = IP_end;
                        tmp.country = it->second;

                        vectorIpRange.push_back(tmp);
                    }

                    ligne = "";
                }
                else
                {
                    ligne += buffer[bufferPosition];
                }

                bufferPosition++;
            }

            mapPays.clear();
            delete [] buffer;
            file.close();
        }
        else
        {
            erreur = true;
            erreurMessage = "Unable the open the file";
            return;
        }

        if (arrayPrefixIP[0] == -1) arrayPrefixIP[0] = 0;
        for (int i=1; i<256; i++)
        {
            if (arrayPrefixIP[i] == -1) arrayPrefixIP[i] = arrayPrefixIP[i-1];
        }
    }

    uint16_t getCountryCode(string recherche)
    {
        return getCountryCode(stringToIntIP(recherche));
    }

    uint16_t getCountryCode(uint32_t recherche)
    {
        for (uint32_t i=arrayPrefixIP[getPrefixIP(recherche)]; i<vectorIpRange.size(); i++)
        {
            if (recherche >= vectorIpRange[i].start && recherche <= vectorIpRange[i].end)
            {
                if (vectorIpRange[i].country < vectorPays.size())
                {
                    return vectorIpRange[i].country;
                }

                return 0;
            }
            else if (vectorIpRange[i].end > recherche)
            {
                break;
            }
        }

        return 0;
    }

};

//http://stackoverflow.com/questions/279854/how-do-i-sort-a-vector-of-pairs-based-on-the-second-element-of-the-pair
template <class T1, class T2, class Pred = std::less<T2> >
struct sort_pair_second
{
    bool operator()(const std::pair<T1,T2>&left, const std::pair<T1,T2>&right)
    {
        Pred p;
        return p(left.second, right.second);
    }
};

// Structure des logs
struct logSSH
{
    uint32_t username = 0, software = 0, mac = 0, cipher = 0, password = 0;
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, microsecond = 0;
    uint32_t sourceIP;
    //bool compression = false;
    uint32_t sourcePort = 0, attempt = 0;
    uint16_t country = 0;
};

void showLog(logSSH log, IP &ipInformation, vector<string> &vectorUsername, vector<string> &vectorSoftware, vector<string> &vectorMac, vector<string> &vectorCipher, vector<string> &vectorPassword)
{
    cout << "country : " << ipInformation.countryCodeToCountryString(log.country);
    cout << ", username : " << vectorUsername[log.username];
    cout << ", software : " << vectorSoftware[log.software];
    cout << ", time : " << log.year << "/" << log.month << "/" << log.day << " - " << log.hour << ":" << log.minute << ":" << log.second << "." << log.microsecond;
    cout << ", mac : " << vectorMac[log.mac];
    cout << ", cipher : " << vectorCipher[log.cipher];
    cout << ", sourceIP : " << ipInformation.intToStringIP(log.sourceIP);
    //cout << ", compression : " << (log.compression == false ? "false" : "true");
    cout << ", sourcePort : " << log.sourcePort;
    cout << ", attempt : " << log.attempt << endl;
}

bool dateAtteinte(int year, int month, int day, int yearNow, int monthNow, int dayNow, bool plusGrande=false)
{
    if (yearNow > year) return true;
    else if (yearNow == year)
    {
        if (monthNow > month) return true;
        else if (monthNow == month)
        {
            if (plusGrande == false)
            {
                if (dayNow >= day) return true;
            }
            else
            {
                if (dayNow > day) return true;
            }
        }
    }

    return false;
}
//////////////////////////////////////////////

struct datePosition
{
    int year, month, day;
    uint32_t value;
};

// Variables globales utilisées dans les différentes classes
vector<logSSH> vectorLogSSH;
vector<string> vectorUsername;
vector<string> vectorPassword;
vector<string> vectorSoftware;
vector<string> vectorMac;
vector<string> vectorCipher;

vector<datePosition> vectorDatePosition;

map<string,uint32_t> mapUsername;
map<string,uint32_t> mapPassword;

IP ipInformation;

//////////////////////////////////////////////

// Mutex pour réserver l'accès aux données ci-dessus
mutex mtx;

// Classe de visualisation de données
class visualisationAsync : public AsyncWorker
{

private:

    string outputString;
    int year1, month1, day1;
    int year2, month2, day2;
    uint32_t limit;
    int data;
    int error = 0;
    bool emptyData = true;

public:

    visualisationAsync(Callback *callback, int year1, int month1, int day1, int year2, int month2, int day2, uint32_t limit, int data) : AsyncWorker(callback), year1(year1), month1(month1), day1(day1), year2(year2), month2(month2), day2(day2), limit(limit), data(data) {}
    ~visualisationAsync()
    {
        mtx.unlock();
    }

    void Execute()
    {
        mtx.lock();

        // Pour éviter la saturation de la mémoire
        if (limit == 0 || limit > 1000) limit = 1000;

        if (!dateAtteinte(vectorLogSSH[0].year, vectorLogSSH[0].month, vectorLogSSH[0].day, year1, month1, day1))
        {
            error = 1;
        }
        else if (dateAtteinte(vectorLogSSH[vectorLogSSH.size()-1].year, vectorLogSSH[vectorLogSSH.size()-1].month, vectorLogSSH[vectorLogSSH.size()-1].day, year2, month2, day2) && !(vectorLogSSH[vectorLogSSH.size()-1].year == year2 && vectorLogSSH[vectorLogSSH.size()-1].month == month2 && vectorLogSSH[vectorLogSSH.size()-1].day == day2))
        {
            error = 2;
        }
        else if (dateAtteinte(year2, month2, day2, year1, month1, day1) && !(year1 == year2 && month1 == month2 && day1 == day2))
        {
            error = 3;
        }

        if (error == 0)
        {
            if (data == 0)
            {
                // Trier les pays
                map<uint16_t,uint32_t> mapPaysAttaqueLePlus;
                vector<pair<uint16_t,uint32_t>> vectorPaysAttaqueLePlus;

                bool lireDonnees = false;
                bool dateAtteinteBool = false;

                for (uint32_t i=0; i<vectorDatePosition.size(); i++)
                {
                    if (lireDonnees == false)
                    {
                        if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                        {
                            lireDonnees = true;
                        }
                        else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                        {
                            lireDonnees = true;
                        }
                    }

                    if (lireDonnees == true)
                    {
                        if (dateAtteinteBool == true)
                        {
                            break;
                        }
                        else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                        {
                            break;
                        }
                        else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                        {
                            dateAtteinteBool = true;
                        }

                        for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                        {
                            map<uint16_t,uint32_t>::iterator it = mapPaysAttaqueLePlus.find(vectorLogSSH[j].country);
                            if (it == mapPaysAttaqueLePlus.end())
                            {
                                mapPaysAttaqueLePlus[vectorLogSSH[j].country] = vectorPaysAttaqueLePlus.size();
                                vectorPaysAttaqueLePlus.push_back(make_pair(vectorLogSSH[j].country, 1));
                            }
                            else
                            {
                                vectorPaysAttaqueLePlus[it->second].second++;
                            }
                        }
                    }
                }

                mapPaysAttaqueLePlus.clear();
                sort(vectorPaysAttaqueLePlus.begin(), vectorPaysAttaqueLePlus.end(), sort_pair_second<uint16_t, uint32_t>());

                rapidjson::Document document;
                document.SetObject();
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

                //////////////////////////////////////////////////////

                rapidjson::Value recipients3(rapidjson::kArrayType);

                uint32_t compteurLimit = 0;

                for (uint32_t i=0; i<vectorPaysAttaqueLePlus.size(); i++)
                {
                    emptyData = false;

                    uint32_t position = vectorPaysAttaqueLePlus.size() - 1 - i;

                    rapidjson::Value recipient3(rapidjson::kObjectType);

                    rapidjson::Value textPart;
                    textPart.SetString(ipInformation.countryCodeToCountryString(vectorPaysAttaqueLePlus[position].first).c_str(), allocator);


                    recipient3.AddMember("name", textPart, allocator);
                    recipient3.AddMember("size", vectorPaysAttaqueLePlus[position].second, allocator);
                    recipients3.PushBack(recipient3, allocator);

                      compteurLimit++;
                      if (compteurLimit == limit) break;
                }

                if (emptyData == false)
                {
                    document.AddMember("name", "flare", allocator);
                    document.AddMember("children", recipients3, allocator);

                    //////////////////////////////////////////////////////

                    rapidjson::StringBuffer strbuf;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                    document.Accept(writer);

                    outputString = strbuf.GetString();
                }
            }
            else if (data <= 4 || data == 9)
            {
                map<uint32_t,uint32_t> mapUsernameLePlus;
                vector<pair<uint32_t,uint32_t>> vectorUsernameLePlus;

                bool lireDonnees = false;
                bool dateAtteinteBool = false;

                for (uint32_t i=0; i<vectorDatePosition.size(); i++)
                {
                    if (lireDonnees == false)
                    {
                        if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                        {
                            lireDonnees = true;
                        }
                        else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                        {
                            lireDonnees = true;
                        }
                    }

                    if (lireDonnees == true)
                    {
                        if (dateAtteinteBool == true)
                        {
                            break;
                        }
                        else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                        {
                            break;
                        }
                        else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                        {
                            dateAtteinteBool = true;
                        }

                        for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                        {
                            uint32_t searchData;
                            if (data == 1) searchData = vectorLogSSH[j].username;
                            else if (data == 2) searchData = vectorLogSSH[j].password;
                            else if (data == 3) searchData = vectorLogSSH[j].cipher;
                            else if (data == 4) searchData = vectorLogSSH[j].mac;
                            else if (data == 9) searchData = vectorLogSSH[j].software;

                            map<uint32_t,uint32_t>::iterator it = mapUsernameLePlus.find(searchData);
                            if (it == mapUsernameLePlus.end())
                            {
                                mapUsernameLePlus[searchData] = vectorUsernameLePlus.size();
                                vectorUsernameLePlus.push_back(make_pair(searchData, 1));
                            }
                            else
                            {
                                vectorUsernameLePlus[it->second].second++;
                            }
                        }
                    }
                }

                mapUsernameLePlus.clear();
                sort(vectorUsernameLePlus.begin(), vectorUsernameLePlus.end(), sort_pair_second<uint32_t, uint32_t>());

                rapidjson::Document document;
                document.SetObject();
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

                //////////////////////////////////////////////////////

                rapidjson::Value recipients3(rapidjson::kArrayType);

                uint32_t compteurLimit = 0;

                for (uint32_t i=0; i<vectorUsernameLePlus.size(); i++)
                {
                    emptyData = false;

                    uint32_t position = vectorUsernameLePlus.size() - 1 - i;

                    rapidjson::Value recipient3(rapidjson::kObjectType);

                    rapidjson::Value textPart;

                    if (data == 1) textPart.SetString(vectorUsername[vectorUsernameLePlus[position].first].c_str(), allocator);
                    else if (data == 2) textPart.SetString(vectorPassword[vectorUsernameLePlus[position].first].c_str(), allocator);
                    else if (data == 3) textPart.SetString(vectorCipher[vectorUsernameLePlus[position].first].c_str(), allocator);
                    else if (data == 4) textPart.SetString(vectorMac[vectorUsernameLePlus[position].first].c_str(), allocator);
                    else if (data == 9) textPart.SetString(vectorSoftware[vectorUsernameLePlus[position].first].c_str(), allocator);

                    recipient3.AddMember("name", textPart, allocator);
                    recipient3.AddMember("size", vectorUsernameLePlus[position].second, allocator);
                    recipients3.PushBack(recipient3, allocator);

                    compteurLimit++;
                    if (compteurLimit == limit) break;
                }

                if (emptyData == false)
                {
                    document.AddMember("name", "flare", allocator);
                    document.AddMember("children", recipients3, allocator);

                    //////////////////////////////////////////////////////

                    rapidjson::StringBuffer strbuf;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                    document.Accept(writer);

                    outputString = strbuf.GetString();
                }
            }
            else if (data <= 6)
            {
                map<uint32_t,uint32_t> mapUsernameLePlus;
                vector<pair<uint32_t,uint32_t>> vectorUsernameLePlus;

                bool lireDonnees = false;
                bool dateAtteinteBool = false;

                for (uint32_t i=0; i<vectorDatePosition.size(); i++)
                {
                    if (lireDonnees == false)
                    {
                        if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                        {
                            lireDonnees = true;
                        }
                        else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                        {
                            lireDonnees = true;
                        }
                    }

                    if (lireDonnees == true)
                    {
                        if (dateAtteinteBool == true)
                        {
                            break;
                        }
                        else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                        {
                            break;
                        }
                        else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                        {
                            dateAtteinteBool = true;
                        }

                        for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                        {
                            uint32_t searchData;
                            if (data == 5) searchData = vectorLogSSH[j].sourcePort;
                            else if (data == 6) searchData = vectorLogSSH[j].sourceIP;

                            map<uint32_t,uint32_t>::iterator it = mapUsernameLePlus.find(searchData);
                            if (it == mapUsernameLePlus.end())
                            {
                                mapUsernameLePlus[searchData] = vectorUsernameLePlus.size();
                                vectorUsernameLePlus.push_back(make_pair(searchData, 1));
                            }
                            else
                            {
                                vectorUsernameLePlus[it->second].second++;
                            }
                        }
                    }
                }

                mapUsernameLePlus.clear();
                sort(vectorUsernameLePlus.begin(), vectorUsernameLePlus.end(), sort_pair_second<uint32_t, uint32_t>());

                rapidjson::Document document;
                document.SetObject();
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

                //////////////////////////////////////////////////////

                rapidjson::Value recipients3(rapidjson::kArrayType);

                uint32_t compteurLimit = 0;

                for (uint32_t i=0; i<vectorUsernameLePlus.size(); i++)
                {
                    emptyData = false;

                    uint32_t position = vectorUsernameLePlus.size() - 1 - i;

                    rapidjson::Value recipient3(rapidjson::kObjectType);

                    rapidjson::Value textPart;

                    if (data == 5) textPart.SetString(to_string(vectorUsernameLePlus[position].first).c_str(), allocator);
                    else if (data == 6)
                    {
                        // Chercher le pays de l'IP
                        string output = ipInformation.intToStringIP(vectorUsernameLePlus[position].first) + " (" + ipInformation.countryCodeToCountryString(ipInformation.getCountryCode(vectorUsernameLePlus[position].first)) +")";

                        textPart.SetString(output.c_str(), allocator);
                    }

                    recipient3.AddMember("name", textPart, allocator);
                    recipient3.AddMember("size", vectorUsernameLePlus[position].second, allocator);
                    recipients3.PushBack(recipient3, allocator);

                    compteurLimit++;
                    if (compteurLimit == limit) break;
                }

                if (emptyData == false)
                {
                    document.AddMember("name", "flare", allocator);
                    document.AddMember("children", recipients3, allocator);

                    //////////////////////////////////////////////////////

                    rapidjson::StringBuffer strbuf;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                    document.Accept(writer);

                    outputString = strbuf.GetString();
                }
            }
            else if (data == 7)
            {
                map<pair<uint32_t,uint32_t>,uint32_t> mapUsernameLePlus;
                vector<pair<pair<uint32_t,uint32_t>,uint32_t>> vectorUsernameLePlus;

                bool lireDonnees = false;
                bool dateAtteinteBool = false;

                for (uint32_t i=0; i<vectorDatePosition.size(); i++)
                {
                    if (lireDonnees == false)
                    {
                        if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                        {
                            lireDonnees = true;
                        }
                        else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                        {
                            lireDonnees = true;
                        }
                    }

                    if (lireDonnees == true)
                    {
                        if (dateAtteinteBool == true)
                        {
                            break;
                        }
                        else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                        {
                            break;
                        }
                        else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                        {
                            dateAtteinteBool = true;
                        }

                        for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                        {
                            pair<uint32_t,uint32_t> searchData = make_pair(vectorLogSSH[j].username, vectorLogSSH[j].password);

                            map<pair<uint32_t,uint32_t>,uint32_t>::iterator it = mapUsernameLePlus.find(searchData);
                            if (it == mapUsernameLePlus.end())
                            {
                                mapUsernameLePlus[searchData] = vectorUsernameLePlus.size();
                                vectorUsernameLePlus.push_back(make_pair(searchData, 1));
                            }
                            else
                            {
                                vectorUsernameLePlus[it->second].second++;
                            }
                        }
                    }
                }

                mapUsernameLePlus.clear();
                sort(vectorUsernameLePlus.begin(), vectorUsernameLePlus.end(), sort_pair_second<pair<uint32_t,uint32_t>, uint32_t>());

                rapidjson::Document document;
                document.SetObject();
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

                //////////////////////////////////////////////////////

                rapidjson::Value recipients3(rapidjson::kArrayType);

                uint32_t compteurLimit = 0;

                for (uint32_t i=0; i<vectorUsernameLePlus.size(); i++)
                {
                    emptyData = false;

                    uint32_t position = vectorUsernameLePlus.size() - 1 - i;

                    rapidjson::Value recipient3(rapidjson::kObjectType);

                    rapidjson::Value textPart;

                    string usernamepassword = vectorUsername[vectorUsernameLePlus[position].first.first] + ", " + vectorPassword[vectorUsernameLePlus[position].first.second];
                    textPart.SetString(usernamepassword.c_str(), allocator);

                    recipient3.AddMember("name", textPart, allocator);
                    recipient3.AddMember("size", vectorUsernameLePlus[position].second, allocator);
                    recipients3.PushBack(recipient3, allocator);

                    compteurLimit++;
                    if (compteurLimit == limit) break;
                }

                if (emptyData == false)
                {
                    document.AddMember("name", "flare", allocator);
                    document.AddMember("children", recipients3, allocator);

                    //////////////////////////////////////////////////////

                    rapidjson::StringBuffer strbuf;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                    document.Accept(writer);

                    outputString = strbuf.GetString();
                }
            }
            else if (data == 8)
            {
                bool lireDonnees = false;
                bool dateAtteinteBool = false;

                rapidjson::Document document;
                document.SetObject();
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

                rapidjson::Value recipients3(rapidjson::kArrayType);

                for (uint32_t i=0; i<vectorDatePosition.size(); i++)
                {
                    if (lireDonnees == false)
                    {
                        if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                        {
                            lireDonnees = true;
                        }
                        else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                        {
                            lireDonnees = true;
                        }
                    }

                    if (lireDonnees == true)
                    {
                        if (dateAtteinteBool == true)
                        {
                            break;
                        }
                        else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                        {
                            break;
                        }
                        else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                        {
                            dateAtteinteBool = true;
                        }

                        uint32_t compteurAttaque = ((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value) - vectorDatePosition[i].value;

                        cout << "compteurAttaque : " << compteurAttaque << endl;

                        rapidjson::Value recipient3(rapidjson::kObjectType);
                        rapidjson::Value textPart;

                        string dateString = to_string(vectorDatePosition[i].year) + "/" + (vectorDatePosition[i].month < 10 ? "0" : "") + to_string(vectorDatePosition[i].month) + "/" + (vectorDatePosition[i].day < 10 ? "0" : "") + to_string(vectorDatePosition[i].day);
                        textPart.SetString(dateString.c_str(), allocator);

                        recipient3.AddMember("name", textPart, allocator);
                        recipient3.AddMember("size", compteurAttaque, allocator);
                        recipients3.PushBack(recipient3, allocator);

                        compteurAttaque = 0;
                        emptyData = false;
                    }
                }

                if (emptyData == false)
                {
                    document.AddMember("name", "flare", allocator);
                    document.AddMember("children", recipients3, allocator);

                    //////////////////////////////////////////////////////

                    rapidjson::StringBuffer strbuf;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                    document.Accept(writer);

                    outputString = strbuf.GetString();
                }

            }
        }
    }

    // Passage du monde de l'addon à celui de Node.js
    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        if (error == 0)
        {
            if (emptyData == false)
            {
                char * bufferChar = new char[outputString.length()];
                for (uint32_t i=0; i<outputString.length(); i++) bufferChar[i] = outputString[i];

                Local<Object> buf = Nan::NewBuffer(bufferChar, outputString.length()).ToLocalChecked();

                Local<Value> argv[] = {buf, Null(), False()};
                callback->Call(3, argv);
            }
            else
            {
                Local<Value> argv[] = {Null(), Null(), True()};
                callback->Call(3, argv);
            }
        }
        else
        {
            Local<Number> num = Nan::New(error);

            Local<Value> argv[] = {Null(), num, Null()};
            callback->Call(3, argv);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

// Classe permettant de générer les données pour le graphe des attaques dans le temps
class linearAsync : public AsyncWorker
{

private:

    int year1, month1, day1;
    int year2, month2, day2;
    int error = 0;
    bool emptyData = true;
    string outputString;

public:

    linearAsync(Callback *callback, int year1, int month1, int day1, int year2, int month2, int day2) : AsyncWorker(callback), year1(year1), month1(month1), day1(day1), year2(year2), month2(month2), day2(day2) {}
    ~linearAsync()
    {
        mtx.unlock();
    }

    void Execute()
    {
        mtx.lock();

        if (!dateAtteinte(vectorLogSSH[0].year, vectorLogSSH[0].month, vectorLogSSH[0].day, year1, month1, day1))
        {
            error = 1;
        }
        else if (dateAtteinte(vectorLogSSH[vectorLogSSH.size()-1].year, vectorLogSSH[vectorLogSSH.size()-1].month, vectorLogSSH[vectorLogSSH.size()-1].day, year2, month2, day2) && !(vectorLogSSH[vectorLogSSH.size()-1].year == year2 && vectorLogSSH[vectorLogSSH.size()-1].month == month2 && vectorLogSSH[vectorLogSSH.size()-1].day == day2))
        {
            error = 2;
        }
        else if (dateAtteinte(year2, month2, day2, year1, month1, day1) && !(year1 == year2 && month1 == month2 && day1 == day2))
        {
            error = 3;
        }

        if (error == 0)
        {
            outputString = "date,value\n";
            bool compter = false;
            bool dateAtteinteBool = false;

            for (uint32_t i=0; i<vectorDatePosition.size(); i++)
            {
                if (compter == false)
                {
                    if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                    {
                        compter = true;
                    }
                }

                if (compter == true)
                {

                    if (dateAtteinteBool == true)
                    {
                        break;
                    }
                    else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                    {
                        break;
                    }
                    else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                    {
                        dateAtteinteBool = true;
                    }

                    uint32_t nombreAttaques = 0;
                    if (i != vectorDatePosition.size() - 1) nombreAttaques = vectorDatePosition[i+1].value - vectorDatePosition[i].value;
                    else nombreAttaques = vectorLogSSH.size() - vectorDatePosition[i].value;

                    outputString += to_string(vectorDatePosition[i].year) + "-" + (vectorDatePosition[i].month < 10 ? "0" : "") + to_string(vectorDatePosition[i].month) + "-" + (vectorDatePosition[i].day < 10 ? "0" : "") + to_string(vectorDatePosition[i].day) + "," + to_string(nombreAttaques) + "\n";

                    emptyData = false;
                }
            }
        }
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        if (error == 0)
        {
            if (emptyData == false)
            {
                char * bufferChar = new char[outputString.length()];
                for (uint32_t i=0; i<outputString.length(); i++) bufferChar[i] = outputString[i];

                Local<Object> buf = Nan::NewBuffer(bufferChar, outputString.length()).ToLocalChecked();

                Local<Value> argv[] = {buf, Null(), False()};
                callback->Call(3, argv);
            }
            else
            {
                Local<Value> argv[] = {Null(), Null(), True()};
                callback->Call(3, argv);
            }
        }
        else
        {
            Local<Number> num = Nan::New(error);

            Local<Value> argv[] = {Null(), num, Null()};
            callback->Call(3, argv);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

// Classe permettant de générer les données pour le graphe des attaques dans le temps en spécifiant le login, le password ou les deux
class linear2Async : public AsyncWorker
{

private:

    int year1, month1, day1;
    int year2, month2, day2;
    int data;
    string username, password;
    int error = 0;
    bool emptyData = true;
    string outputString;

public:

    linear2Async(Callback *callback, int year1, int month1, int day1, int year2, int month2, int day2, int data, string username, string password) : AsyncWorker(callback), year1(year1), month1(month1), day1(day1), year2(year2), month2(month2), day2(day2), data(data), username(username), password(password) {}
    ~linear2Async()
    {
        mtx.unlock();
    }

    void Execute()
    {
        mtx.lock();

        if (!dateAtteinte(vectorLogSSH[0].year, vectorLogSSH[0].month, vectorLogSSH[0].day, year1, month1, day1))
        {
            error = 1;
        }
        else if (dateAtteinte(vectorLogSSH[vectorLogSSH.size()-1].year, vectorLogSSH[vectorLogSSH.size()-1].month, vectorLogSSH[vectorLogSSH.size()-1].day, year2, month2, day2) && !(vectorLogSSH[vectorLogSSH.size()-1].year == year2 && vectorLogSSH[vectorLogSSH.size()-1].month == month2 && vectorLogSSH[vectorLogSSH.size()-1].day == day2))
        {
            error = 2;
        }
        else if (dateAtteinte(year2, month2, day2, year1, month1, day1) && !(year1 == year2 && month1 == month2 && day1 == day2))
        {
            error = 3;
        }

        uint32_t recherche1 = 0, recherche2 = 0;
        if (data == 0) // username
        {
            map<string,uint32_t>::iterator it = mapUsername.find(username);
            if (it != mapUsername.end())
            {
                recherche1 = it->second;
            }
            else
            {
                error = 4;
            }
        }
        else if (data == 1) // password
        {
            map<string,uint32_t>::iterator it = mapPassword.find(password);
            if (it != mapPassword.end())
            {
                recherche1 = it->second;
            }
            else
            {
                error = 5;
            }
        }
        else // username password
        {
            map<string,uint32_t>::iterator it = mapUsername.find(username);
            if (it != mapUsername.end())
            {
                recherche1 = it->second;

                map<string,uint32_t>::iterator it2 = mapPassword.find(password);
                if (it2 != mapPassword.end())
                {
                    recherche2 = it2->second;
                }
                else
                {
                    error = 5;
                }
            }
            else
            {
                error = 4;
            }
        }

        if (error == 0)
        {
            outputString += "date,value\n";
            bool dateAtteinteBool = false;
            bool lireDonnees = false;

            for (uint32_t i=0; i<vectorDatePosition.size(); i++)
            {
                if (lireDonnees == false)
                {
                    if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                    {
                        lireDonnees = true;
                    }
                    else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                    {
                        lireDonnees = true;
                    }
                }

                if (lireDonnees == true)
                {
                    if (dateAtteinteBool == true)
                    {
                        break;
                    }
                    else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                    {
                        break;
                    }
                    else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                    {
                        dateAtteinteBool = true;
                    }

                    uint32_t compteurAttaque = 0;

                    for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                    {
                        if (data == 0)
                        {
                            if (vectorLogSSH[j].username == recherche1)
                            {
                                compteurAttaque++;
                            }
                        }
                        else if (data == 1)
                        {
                            if (vectorLogSSH[j].password == recherche1)
                            {
                                compteurAttaque++;
                            }
                        }
                        else
                        {
                            if (vectorLogSSH[j].username == recherche1 && vectorLogSSH[j].password == recherche2)
                            {
                                compteurAttaque++;
                            }
                        }
                    }

                    outputString += to_string(vectorDatePosition[i].year) + "-" + (vectorDatePosition[i].month < 10 ? "0" : "") + to_string(vectorDatePosition[i].month) + "-" + (vectorDatePosition[i].day < 10 ? "0" : "") + to_string(vectorDatePosition[i].day) + "," + to_string(compteurAttaque) + "\n";
                    if (compteurAttaque != 0) emptyData = false;
                }
            }
        }
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        if (error == 0)
        {
            if (emptyData == false)
            {
                char * bufferChar = new char[outputString.length()];
                for (uint32_t i=0; i<outputString.length(); i++) bufferChar[i] = outputString[i];

                Local<Object> buf = Nan::NewBuffer(bufferChar, outputString.length()).ToLocalChecked();

                Local<Value> argv[] = {buf, Null(), False()};
                callback->Call(3, argv);
            }
            else
            {
                Local<Value> argv[] = {Null(), Null(), True()};
                callback->Call(3, argv);
            }
        }
        else
        {
            Local<Number> num = Nan::New(error);

            Local<Value> argv[] = {Null(), num, Null()};
            callback->Call(3, argv);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

// Visualisation des données lorsque l'utilisateur spécifie un username, un password ou un couple username : password
class visualisation2Async : public AsyncWorker
{

private:

    int year1, month1, day1;
    int year2, month2, day2;
    int data;
    string username, password;
    int error = 0;
    bool emptyData = true;
    string outputString;

public:

    visualisation2Async(Callback *callback, int year1, int month1, int day1, int year2, int month2, int day2, int data, string username, string password) : AsyncWorker(callback), year1(year1), month1(month1), day1(day1), year2(year2), month2(month2), day2(day2), data(data), username(username), password(password) {}
    ~visualisation2Async()
    {
        mtx.unlock();
    }

    void Execute()
    {
        mtx.lock();

        if (!dateAtteinte(vectorLogSSH[0].year, vectorLogSSH[0].month, vectorLogSSH[0].day, year1, month1, day1))
        {
            error = 1;
        }
        else if (dateAtteinte(vectorLogSSH[vectorLogSSH.size()-1].year, vectorLogSSH[vectorLogSSH.size()-1].month, vectorLogSSH[vectorLogSSH.size()-1].day, year2, month2, day2) && !(vectorLogSSH[vectorLogSSH.size()-1].year == year2 && vectorLogSSH[vectorLogSSH.size()-1].month == month2 && vectorLogSSH[vectorLogSSH.size()-1].day == day2))
        {
            error = 2;
        }
        else if (dateAtteinte(year2, month2, day2, year1, month1, day1) && !(year1 == year2 && month1 == month2 && day1 == day2))
        {
            error = 3;
        }

        uint32_t recherche1 = 0, recherche2 = 0;
        if (data == 0) // username
        {
            map<string,uint32_t>::iterator it = mapUsername.find(username);
            if (it != mapUsername.end())
            {
                recherche1 = it->second;
            }
            else
            {
                error = 4;
            }
        }
        else if (data == 1) // password
        {
            map<string,uint32_t>::iterator it = mapPassword.find(password);
            if (it != mapPassword.end())
            {
                recherche1 = it->second;
            }
            else
            {
                error = 5;
            }
        }
        else // username password
        {
            map<string,uint32_t>::iterator it = mapUsername.find(username);
            if (it != mapUsername.end())
            {
                recherche1 = it->second;

                map<string,uint32_t>::iterator it2 = mapPassword.find(password);
                if (it2 != mapPassword.end())
                {
                    recherche2 = it2->second;
                }
                else
                {
                    error = 5;
                }
            }
            else
            {
                error = 4;
            }
        }

        if (error == 0)
        {
            bool lireDonnees = false;
            bool dateAtteinteBool = false;

            vector<pair<uint16_t, uint32_t>> vectorPaysAttaque;
            map<uint16_t,uint32_t> mapPaysAttaque;

            for (uint32_t i=0; i<vectorDatePosition.size(); i++)
            {
                if (lireDonnees == false)
                {
                    if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                    {
                        lireDonnees = true;
                    }
                    else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                    {
                        lireDonnees = true;
                    }
                }

                if (lireDonnees == true)
                {
                    if (dateAtteinteBool == true)
                    {
                        break;
                    }
                    else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                    {
                        break;
                    }
                    else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                    {
                        dateAtteinteBool = true;
                    }

                    for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                    {
                        if (data == 0)
                        {
                            if (vectorLogSSH[j].username == recherche1)
                            {
                                map<uint16_t,uint32_t>::iterator it = mapPaysAttaque.find(vectorLogSSH[j].country);
                                if (it == mapPaysAttaque.end())
                                {
                                    mapPaysAttaque[vectorLogSSH[j].country] = vectorPaysAttaque.size();
                                    vectorPaysAttaque.push_back(make_pair(vectorLogSSH[j].country, 1));
                                }
                                else
                                {
                                    vectorPaysAttaque[it->second].second++;
                                }
                            }
                        }
                        else if (data == 1)
                        {
                            if (vectorLogSSH[j].password == recherche1)
                            {
                                map<uint16_t,uint32_t>::iterator it = mapPaysAttaque.find(vectorLogSSH[j].country);
                                if (it == mapPaysAttaque.end())
                                {
                                    mapPaysAttaque[vectorLogSSH[j].country] = vectorPaysAttaque.size();
                                    vectorPaysAttaque.push_back(make_pair(vectorLogSSH[j].country, 1));
                                }
                                else
                                {
                                    vectorPaysAttaque[it->second].second++;
                                }
                            }
                        }
                        else
                        {
                            if (vectorLogSSH[j].username == recherche1 && vectorLogSSH[j].password == recherche2)
                            {
                                map<uint16_t,uint32_t>::iterator it = mapPaysAttaque.find(vectorLogSSH[j].country);
                                if (it == mapPaysAttaque.end())
                                {
                                    mapPaysAttaque[vectorLogSSH[j].country] = vectorPaysAttaque.size();
                                    vectorPaysAttaque.push_back(make_pair(vectorLogSSH[j].country, 1));
                                }
                                else
                                {
                                    vectorPaysAttaque[it->second].second++;
                                }
                            }
                        }
                    }
                }
            }

            mapPaysAttaque.clear();
            sort(vectorPaysAttaque.begin(), vectorPaysAttaque.end(), sort_pair_second<uint16_t, uint32_t>());

            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

            //////////////////////////////////////////////////////

            rapidjson::Value recipients3(rapidjson::kArrayType);

            for (uint32_t i=0; i<vectorPaysAttaque.size(); i++)
            {
                uint32_t position = vectorPaysAttaque.size() - 1 - i;

                rapidjson::Value recipient3(rapidjson::kObjectType);

                rapidjson::Value textPart;
                textPart.SetString(ipInformation.countryCodeToCountryString(vectorPaysAttaque[position].first).c_str(), allocator);

                recipient3.AddMember("name", textPart, allocator);
                recipient3.AddMember("size", vectorPaysAttaque[position].second, allocator);
                recipients3.PushBack(recipient3, allocator);

                emptyData = false;
            }


            if (emptyData == false)
            {
                document.AddMember("name", "flare", allocator);
                document.AddMember("children", recipients3, allocator);

                //////////////////////////////////////////////////////

                rapidjson::StringBuffer strbuf;
                rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                document.Accept(writer);

                outputString = strbuf.GetString();
            }

        }
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        if (error == 0)
        {
            if (emptyData == false)
            {
                char * bufferChar = new char[outputString.length()];
                for (uint32_t i=0; i<outputString.length(); i++) bufferChar[i] = outputString[i];

                Local<Object> buf = Nan::NewBuffer(bufferChar, outputString.length()).ToLocalChecked();

                Local<Value> argv[] = {buf, Null(), False()};
                callback->Call(3, argv);
            }
            else
            {
                Local<Value> argv[] = {Null(), Null(), True()};
                callback->Call(3, argv);
            }
        }
        else
        {
            Local<Number> num = Nan::New(error);

            Local<Value> argv[] = {Null(), num, Null()};
            callback->Call(3, argv);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

// Pour obtenir les limites temporelles des données
class dateAsync : public AsyncWorker
{

private:

    string outputString;

public:

    dateAsync(Callback *callback) : AsyncWorker(callback) {}
    ~dateAsync()
    {
        mtx.unlock();

    }

    void Execute()
    {
        mtx.lock();

        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        rapidjson::Value textPart;

        document.AddMember("year1", vectorLogSSH[0].year, allocator);
        document.AddMember("month1", vectorLogSSH[0].month, allocator);
        document.AddMember("day1", vectorLogSSH[0].day, allocator);

        document.AddMember("year2", vectorLogSSH[vectorLogSSH.size()-1].year, allocator);
        document.AddMember("month2", vectorLogSSH[vectorLogSSH.size()-1].month, allocator);
        document.AddMember("day2", vectorLogSSH[vectorLogSSH.size()-1].day, allocator);

        rapidjson::StringBuffer strbuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
        document.Accept(writer);

        outputString = strbuf.GetString();
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        char * bufferChar = new char[outputString.length()];
        for (uint32_t i=0; i<outputString.length(); i++) bufferChar[i] = outputString[i];

        Local<Object> buf = Nan::NewBuffer(bufferChar, outputString.length()).ToLocalChecked();

        Local<Value> argv[] = {buf};
        callback->Call(1, argv);
    }
};

//////////////////////////////////////////////////////////////////////////////

// Pour obtenir la liste des pays qui ont attaqué entre deux dates
class paysAsync : public AsyncWorker
{

private:

    string outputString;
    int year1, month1, day1;
    int year2, month2, day2;
    bool suivant;
    int error = 0;

public:

    paysAsync(Callback *callback, int year1, int month1, int day1, int year2, int month2, int day2, bool suivant) : AsyncWorker(callback), year1(year1), month1(month1), day1(day1), year2(year2), month2(month2), day2(day2), suivant(suivant) {}
    ~paysAsync()
    {
        mtx.unlock();
    }

    void Execute()
    {
        mtx.lock();

        if (!dateAtteinte(vectorLogSSH[0].year, vectorLogSSH[0].month, vectorLogSSH[0].day, year1, month1, day1))
        {
            error = 1;
        }
        else if (dateAtteinte(vectorLogSSH[vectorLogSSH.size()-1].year, vectorLogSSH[vectorLogSSH.size()-1].month, vectorLogSSH[vectorLogSSH.size()-1].day, year2, month2, day2) && !(vectorLogSSH[vectorLogSSH.size()-1].year == year2 && vectorLogSSH[vectorLogSSH.size()-1].month == month2 && vectorLogSSH[vectorLogSSH.size()-1].day == day2))
        {
            error = 2;
        }
        else if (dateAtteinte(year2, month2, day2, year1, month1, day1) && !(year1 == year2 && month1 == month2 && day1 == day2))
        {
            error = 3;
        }
        else if (suivant == true && year1 == year2 && month1 == month2 && day1 == day2)
        {
            error = 4;
        }

        if (error == 0)
        {
            bool lireDonnees = false;
            bool dateAtteinteBool = false;
            bool attente = false;

            map<uint16_t,uint32_t> mapPaysAttaque;

            for (uint32_t i=0; i<vectorDatePosition.size(); i++)
            {
                if (lireDonnees == false)
                {
                    if (attente == true) lireDonnees = true;

                    if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                    {
                        if (suivant == true) attente = true;
                        else lireDonnees = true;
                    }
                    else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                    {
                        if (suivant == true) attente = true;
                        else lireDonnees = true;
                    }
                }

                if (lireDonnees == true)
                {
                    if (dateAtteinteBool == true)
                    {
                        break;
                    }
                    else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                    {
                        break;
                    }
                    else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                    {
                        dateAtteinteBool = true;
                    }

                    for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                    {
                        map<uint16_t,uint32_t>::iterator it = mapPaysAttaque.find(vectorLogSSH[j].country);
                        if (it == mapPaysAttaque.end())
                        {
                            mapPaysAttaque[vectorLogSSH[j].country] = 1;
                        }
                        else
                        {
                            it->second++;
                        }
                    }
                }
            }

            map<pair<string,uint16_t>,uint32_t> mapPaysAttaqueString;
            for (map<uint16_t,uint32_t>::iterator it = mapPaysAttaque.begin(); it != mapPaysAttaque.end(); it++)
            {
                mapPaysAttaqueString[make_pair(ipInformation.countryCodeToCountryString(it->first), it->first)] = it->second;
            }

            mapPaysAttaque.clear();

            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

            rapidjson::Value TableauPays(rapidjson::kArrayType);

            for (map<pair<string,uint16_t>,uint32_t>::iterator it = mapPaysAttaqueString.begin(); it != mapPaysAttaqueString.end(); it++)
            {
                //cout << it->first.first << ", " << it->first.second << " -> " << it->second << endl;

                rapidjson::Value pays(rapidjson::kObjectType);
                rapidjson::Value textPart;
                textPart.SetString(it->first.first.c_str(), allocator);

                pays.AddMember("string", textPart, allocator);
                pays.AddMember("id", it->first.second, allocator);
                pays.AddMember("value", it->second, allocator);

                TableauPays.PushBack(pays, allocator);
            }

            document.AddMember("country", TableauPays, allocator);

            /////////////////////////////////////////////////

            rapidjson::StringBuffer strbuf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
            document.Accept(writer);
            outputString = strbuf.GetString();
        }
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        if (error == 0)
        {
            char * bufferChar = new char[outputString.length()];
            for (uint32_t i=0; i<outputString.length(); i++) bufferChar[i] = outputString[i];

            Local<Object> buf = Nan::NewBuffer(bufferChar, outputString.length()).ToLocalChecked();

            Local<Value> argv[] = {buf, Null()};
            callback->Call(2, argv);
        }
        else
        {
            Local<Number> num = Nan::New(error);

            Local<Value> argv[] = {Null(), num};
            callback->Call(2, argv);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

// Génération du graphe des anomalies (nouvelles attaques)
class anomalieAsync : public AsyncWorker
{

private:

    const uint32_t LIMIT_ITEM = 20000;
    const uint32_t LIMIT_EDGE = 50000;
    bool limitAtteinte = false;

    string outputString;
    int year1, month1, day1;
    int year2, month2, day2;
    int year3, month3, day3;
    string listepaysapp;
    string listepays;
    int data;
    uint32_t seuilApp, seuilAnomalie;
    int error = 0;
    bool emptyData = true;

    struct structPaysAnomalie
    {
        uint32_t compteur = 0, id = 0;
        bool ajoute = false;
        uint32_t couleur = 0;
    };

    string getColor(uint32_t color)
    {
        string colorString = "";

        switch (color & 0b111)
        {
        case 0:
            colorString = "#00FF00";
            break;
        case 1:
            colorString = "#0000FF";
            break;
        case 2:
            colorString = "#FF00FF";
            break;
        case 3:
            colorString = "#00FFFF";
            break;
        case 4:
            colorString = "#FF8400";
            break;
        case 5:
            colorString = "#228632";
            break;
        case 6:
            colorString = "#4DFFFA";
            break;
        case 7:
            colorString = "#6300C0";
            break;
        default:
            colorString = "#ABABAB";
        }

        return colorString;
    }

public:

    anomalieAsync(Callback *callback, int year1, int month1, int day1, int year2, int month2, int day2, int year3, int month3, int day3, string listepaysapp, string listepays, int data, uint32_t seuilApp, uint32_t seuilAnomalie) : AsyncWorker(callback), year1(year1), month1(month1), day1(day1), year2(year2), month2(month2), day2(day2), year3(year3), month3(month3), day3(day3), listepaysapp(listepaysapp), listepays(listepays), data(data), seuilApp(seuilApp), seuilAnomalie(seuilAnomalie) {}
    ~anomalieAsync()
    {
        mtx.unlock();
    }

    void Execute()
    {
        mtx.lock();

        if (!dateAtteinte(vectorLogSSH[0].year, vectorLogSSH[0].month, vectorLogSSH[0].day, year1, month1, day1))
        {
            error = 1;
        }
        else if (dateAtteinte(vectorLogSSH[vectorLogSSH.size()-1].year, vectorLogSSH[vectorLogSSH.size()-1].month, vectorLogSSH[vectorLogSSH.size()-1].day, year2, month2, day2) && !(vectorLogSSH[vectorLogSSH.size()-1].year == year2 && vectorLogSSH[vectorLogSSH.size()-1].month == month2 && vectorLogSSH[vectorLogSSH.size()-1].day == day2))
        {
            error = 2;
        }
        else if (dateAtteinte(year2, month2, day2, year1, month1, day1) && !(year1 == year2 && month1 == month2 && day1 == day2))
        {
            error = 3;
        }
        else if (dateAtteinte(year3, month3, day3, year2, month2, day2)) // Si date2 >= date3
        {
            error = 4;
        }
        else if (dateAtteinte(vectorLogSSH[vectorLogSSH.size()-1].year, vectorLogSSH[vectorLogSSH.size()-1].month, vectorLogSSH[vectorLogSSH.size()-1].day, year3, month3, day3) && !(vectorLogSSH[vectorLogSSH.size()-1].year == year3 && vectorLogSSH[vectorLogSSH.size()-1].month == month3 && vectorLogSSH[vectorLogSSH.size()-1].day == day3))
        {
            error = 5;
        }

        if (error == 0)
        {
            map<uint16_t,uint32_t> mapPaysApp;
            map<uint16_t,uint32_t> mapPays;
            map<uint16_t,structPaysAnomalie> mapPaysAnomalie;

            bool tousPaysApp = false;
            bool tousPays = false;

            if (listepaysapp == "false")
            {
                tousPaysApp = true;
            }

            if (tousPaysApp == false)
            {
                string recuperation = "";
                for (uint32_t i=0; i<listepaysapp.length(); i++)
                {
                    bool verifier = false;

                    if (listepaysapp[i] != '-')
                    {
                        recuperation += listepaysapp[i];
                        if (i == listepaysapp.length() - 1) verifier = true;
                    }
                    else
                    {
                        verifier = true;
                    }

                    if (verifier == true)
                    {
                        uint32_t paysInt = -1;
                        bool reussite = true;

                        try
                        {
                            int nombre = stol(recuperation);
                            if (nombre >= 0)
                            {
                                paysInt = (uint32_t) nombre;
                            }
                            else
                            {
                                reussite = false;
                            }
                        }
                        catch(...)
                        {
                            reussite = false;
                        }

                        recuperation = "";

                        if (reussite == true)
                        {
                            if (paysInt < ipInformation.getVectorPaysSize())
                            {
                                map<uint16_t,uint32_t>::iterator itPaysApp = mapPaysApp.find(paysInt);
                                if (itPaysApp == mapPaysApp.end())
                                {
                                    mapPaysApp[paysInt] = 0;
                                }
                            }
                        }
                    }
                }
            }

            if (listepays == "false")
            {
                tousPays = true;
            }

            if (tousPays == false)
            {
                string recuperation = "";
                for (uint32_t i=0; i<listepays.length(); i++)
                {
                    bool verifier = false;

                    if (listepays[i] != '-')
                    {
                        recuperation += listepays[i];
                        if (i == listepays.length() - 1) verifier = true;
                    }
                    else
                    {
                        verifier = true;
                    }

                    if (verifier == true)
                    {
                        uint32_t paysInt = -1;
                        bool reussite = true;

                        try
                        {
                            int nombre = stol(recuperation);
                            if (nombre >= 0)
                            {
                                paysInt = (uint32_t) nombre;
                            }
                            else
                            {
                                reussite = false;
                            }
                        }
                        catch(...)
                        {
                            reussite = false;
                        }

                        recuperation = "";

                        if (reussite == true)
                        {
                            if (paysInt < ipInformation.getVectorPaysSize())
                            {
                                map<uint16_t,uint32_t>::iterator itPays = mapPays.find(paysInt);
                                if (itPays == mapPays.end())
                                {
                                    mapPays[paysInt] = 0;
                                }
                            }
                        }
                    }
                }
            }

            if ((mapPaysApp.size() != 0 || tousPaysApp == true) && (mapPays.size() != 0 || tousPays == true))
            {

                if (data < 2)
                {
                    // préparer le dictionnaire
                    map<uint32_t, uint32_t> mapDataApp;

                    bool lireDonnees = false;
                    bool dateAtteinteBool = false;
                    uint32_t positionVectorDatePosition = 0;

                    for (uint32_t i=0; i<vectorDatePosition.size(); i++)
                    {
                        if (lireDonnees == false)
                        {
                            if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                            {
                                lireDonnees = true;
                            }
                            else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                            {
                                lireDonnees = true;
                            }
                        }

                        if (lireDonnees == true)
                        {
                            if (dateAtteinteBool == true)
                            {
                                positionVectorDatePosition = i;
                                break;
                            }
                            else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                            {
                                positionVectorDatePosition = i;
                                break;
                            }
                            else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                            {
                                dateAtteinteBool = true;
                            }

                            for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                            {
                                if (tousPaysApp == false)
                                {
                                    map<uint16_t,uint32_t>::iterator itPaysApp = mapPaysApp.find(vectorLogSSH[j].country);
                                    // Le pays correspond avec la liste de l'apprentissage
                                    if (itPaysApp != mapPaysApp.end())
                                    {
                                        uint32_t rechercheData = (data == 0 ? vectorLogSSH[j].username : vectorLogSSH[j].password);

                                        map<uint32_t,uint32_t>::iterator it = mapDataApp.find(rechercheData);
                                        if (it == mapDataApp.end())
                                        {
                                            mapDataApp[rechercheData] = 1;
                                        }
                                        else
                                        {
                                            it->second++;
                                        }
                                    }
                                }
                                else
                                {
                                    uint32_t rechercheData = (data == 0 ? vectorLogSSH[j].username : vectorLogSSH[j].password);

                                    map<uint32_t,uint32_t>::iterator it = mapDataApp.find(rechercheData);
                                    if (it == mapDataApp.end())
                                    {
                                        mapDataApp[rechercheData] = 1;
                                    }
                                    else
                                    {
                                        it->second++;
                                    }
                                }
                            }
                        }
                    }

                    //cout << "mapDataApp.size() : " << mapDataApp.size() << endl;

                    // Creer un vector pour trier
                    if (seuilApp > 1)
                    {
                        vector<pair<uint32_t,uint32_t>> vectorDataApp;

                        for (map<uint32_t,uint32_t>::iterator it = mapDataApp.begin(); it != mapDataApp.end(); it++)
                        {
                            vectorDataApp.push_back(make_pair(it->first, it->second));
                        }

                        mapDataApp.clear();
                        sort(vectorDataApp.begin(), vectorDataApp.end(), sort_pair_second<uint32_t, uint32_t>());

                        //cout << "seuilApp : " << seuilApp << endl;

                        for (uint32_t i=0; i<vectorDataApp.size(); i++)
                        {
                            uint32_t position = vectorDataApp.size() - 1 - i;
                            if(vectorDataApp[position].second >= seuilApp)
                            {
                                mapDataApp[vectorDataApp[position].first] = vectorDataApp[position].second;
                            }
                            else break;
                        }

                        //cout << "seuillage map : " << mapDataApp.size() << endl;
                    }

                    if (mapDataApp.size() == 0)
                    {
                        error = 7;
                    }
                    else
                    {
                        map<uint32_t, pair<uint32_t, map<uint16_t, uint32_t>>> mapDataAnomalie;
                        dateAtteinteBool = false;

                        for (uint32_t i=positionVectorDatePosition; i<vectorDatePosition.size(); i++)
                        {
                            if (dateAtteinteBool == true)
                            {
                                break;
                            }
                            else if (dateAtteinte(year3, month3, day3, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                            {
                                break;
                            }
                            else if (vectorDatePosition[i].year == year3 && vectorDatePosition[i].month == month3 && vectorDatePosition[i].day == day3)
                            {
                                dateAtteinteBool = true;
                            }

                            for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                            {
                                if (tousPays == false)
                                {
                                    //cout << "touspays false, mapPays.size : " << mapPaysAnomalie.size() << endl;

                                    map<uint16_t,uint32_t>::iterator itPays = mapPays.find(vectorLogSSH[j].country);
                                    if (itPays != mapPays.end())
                                    {
                                        uint32_t rechercheData = (data == 0 ? vectorLogSSH[j].username : vectorLogSSH[j].password);

                                        // Chercher dans le dico
                                        map<uint32_t,uint32_t>::iterator itApp = mapDataApp.find(rechercheData);
                                        if (itApp == mapDataApp.end())
                                        {
                                            // Verifier si le pays existe dans la base
                                            map<uint16_t,structPaysAnomalie>::iterator itPays = mapPaysAnomalie.find(vectorLogSSH[j].country);
                                            if (itPays == mapPaysAnomalie.end())
                                            {
                                                structPaysAnomalie structTmp;
                                                structTmp.compteur = 1;
                                                mapPaysAnomalie[vectorLogSSH[j].country] = structTmp;
                                            }
                                            else
                                            {
                                                itPays->second.compteur++;
                                            }

                                            map<uint32_t, pair<uint32_t, map<uint16_t, uint32_t>>>::iterator it = mapDataAnomalie.find(rechercheData);
                                            if (it == mapDataAnomalie.end())
                                            {

                                                map<uint16_t, uint32_t> mapTmp;
                                                mapTmp[vectorLogSSH[j].country] = 1;

                                                mapDataAnomalie[rechercheData] = make_pair(1, mapTmp);
                                            }
                                            else
                                            {
                                                it->second.first++;

                                                // Ajouter le pays si besoin
                                                map<uint16_t,uint32_t>::iterator itPaysAnomalie = it->second.second.find(vectorLogSSH[j].country);
                                                if (itPaysAnomalie == it->second.second.end())
                                                {
                                                    it->second.second[vectorLogSSH[j].country] = 1;
                                                }
                                                else
                                                {
                                                    itPaysAnomalie->second++;
                                                }

                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    //cout << "touspays true, mapPays.size : " << mapPaysAnomalie.size() << endl;

                                    uint32_t rechercheData = (data == 0 ? vectorLogSSH[j].username : vectorLogSSH[j].password);

                                    // Chercher dans le dico
                                    map<uint32_t,uint32_t>::iterator itApp = mapDataApp.find(rechercheData);
                                    if (itApp == mapDataApp.end())
                                    {
                                        // Verifier si le pays existe dans la base
                                        map<uint16_t,structPaysAnomalie>::iterator itPays = mapPaysAnomalie.find(vectorLogSSH[j].country);
                                        if (itPays == mapPaysAnomalie.end())
                                        {
                                            structPaysAnomalie structTmp;
                                            structTmp.compteur = 1;
                                            mapPaysAnomalie[vectorLogSSH[j].country] = structTmp;
                                        }
                                        else
                                        {
                                            itPays->second.compteur++;
                                        }

                                        map<uint32_t, pair<uint32_t, map<uint16_t, uint32_t>>>::iterator it = mapDataAnomalie.find(rechercheData);
                                        if (it == mapDataAnomalie.end())
                                        {
                                            map<uint16_t, uint32_t> mapTmp;
                                            mapTmp[vectorLogSSH[j].country] = 1;

                                            mapDataAnomalie[rechercheData] = make_pair(1, mapTmp);
                                        }
                                        else
                                        {
                                            it->second.first++;

                                            // Ajouter le pays si besoin
                                            map<uint16_t,uint32_t>::iterator itPaysAnomalie = it->second.second.find(vectorLogSSH[j].country);
                                            if (itPaysAnomalie == it->second.second.end())
                                            {
                                                it->second.second[vectorLogSSH[j].country] = 1;
                                            }
                                            else
                                            {
                                                itPaysAnomalie->second++;
                                            }

                                        }
                                    }
                                }
                            }
                        }

                        if (mapDataAnomalie.size() == 0)
                        {
                            error = 8;
                        }
                        else
                        {
                            //cout << "mapPays.size() : " << mapPays.size() << endl;
                            //cout << "mapPaysAnomalie.size() : " << mapPaysAnomalie.size() << endl;
                            //cout << "mapDataAnomalie.size() : " << mapDataAnomalie.size() << endl;

                            // Construire le graphe

                            uniform_real_distribution<double> dist(-100, 100);
                            random_device rd;

                            rapidjson::Document document;
                            document.SetObject();
                            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
                            rapidjson::Value nodeArray(rapidjson::kArrayType);
                            rapidjson::Value edgeArray(rapidjson::kArrayType);

                            uint32_t compteurNodeID = 0, compteurEdgeID = 0;
                            uint32_t compteurCouleurPays = 0;

                            rapidjson::Value textPart;

                            //////////////////////////////////////////////////////

                            // Ajouter les anomalies
                            for (map<uint32_t, pair<uint32_t, map<uint16_t, uint32_t>>>::iterator itAnomalie = mapDataAnomalie.begin(); itAnomalie != mapDataAnomalie.end(); itAnomalie++)
                            {
                                if (itAnomalie->second.first >= seuilAnomalie)
                                {
                                    rapidjson::Value node(rapidjson::kObjectType);

                                    textPart.SetString((data == 0 ? vectorUsername[itAnomalie->first].c_str() : vectorPassword[itAnomalie->first].c_str()), allocator);
                                    node.AddMember("id", compteurNodeID, allocator);
                                    node.AddMember("label", textPart, allocator);
                                    node.AddMember("x", dist(rd), allocator);
                                    node.AddMember("y", dist(rd), allocator);
                                    node.AddMember("size", 5, allocator);
                                    node.AddMember("color", "#FF0000", allocator);
                                    node.AddMember("type", "square", allocator);

                                    nodeArray.PushBack(node, allocator);

                                    //cout << "size : " << itAnomalie->second.second.size() << endl;

                                    // Ajouter les liens

                                    uint32_t idNoeud = compteurNodeID++;
                                    for (map<uint16_t, uint32_t>::iterator itAnomaliePays = itAnomalie->second.second.begin(); itAnomaliePays != itAnomalie->second.second.end(); itAnomaliePays++)
                                    {
                                        //cout << itAnomaliePays->first << endl;
                                        // trouver l'ID du pays
                                        map<uint16_t,structPaysAnomalie>::iterator itTrouverIDPays = mapPaysAnomalie.find(itAnomaliePays->first);
                                        if(itTrouverIDPays != mapPaysAnomalie.end())
                                        {
                                            //cout << itAnomaliePays->first << ", " << itTrouverIDPays->second.second << endl;

                                            if (itTrouverIDPays->second.ajoute == false)
                                            {
                                                rapidjson::Value node(rapidjson::kObjectType);
                                                textPart.SetString(ipInformation.countryCodeToCountryString(itTrouverIDPays->first).c_str(), allocator);
                                                node.AddMember("id", compteurNodeID, allocator);
                                                node.AddMember("label", textPart, allocator);
                                                node.AddMember("x", dist(rd), allocator);
                                                node.AddMember("y", dist(rd), allocator);
                                                node.AddMember("size", 5, allocator);

                                                textPart.SetString(getColor(compteurCouleurPays).c_str(), allocator);
                                                node.AddMember("color", textPart, allocator);

                                                nodeArray.PushBack(node, allocator);
                                                itTrouverIDPays->second.id = compteurNodeID++;
                                                itTrouverIDPays->second.couleur = compteurCouleurPays++;
                                                itTrouverIDPays->second.ajoute = true;

                                                emptyData = false;
                                            }

                                            rapidjson::Value edge(rapidjson::kObjectType);
                                            edge.AddMember("id", compteurEdgeID++, allocator);
                                            edge.AddMember("source", idNoeud, allocator);
                                            edge.AddMember("target", itTrouverIDPays->second.id, allocator);

                                            textPart.SetString(getColor(itTrouverIDPays->second.couleur).c_str(), allocator);
                                            edge.AddMember("color", textPart, allocator);

                                            edgeArray.PushBack(edge, allocator);
                                        }
                                        else // Impossible que ça arrive normalement
                                        {
                                            cout << "ERREUR !!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
                                        }
                                    }

                                    if (compteurNodeID >= LIMIT_ITEM || compteurEdgeID >= LIMIT_EDGE)
                                    {
                                        limitAtteinte = true;
                                        cout << "limitAtteinte anomalie : " << compteurNodeID << ", " << compteurEdgeID << endl;
                                        break;
                                    }

                                }
                            }

                            //////////////////////////////////////////////////////

                            if (emptyData == false)
                            {
                                document.AddMember("nodes", nodeArray, allocator);
                                document.AddMember("edges", edgeArray, allocator);

                                string stringTitre = "anomalie - " + (data == 0 ? string("username") : (data == 1 ? string("password") : string("usernamepassword"))) + " - " + to_string(year1) + "/" + to_string(month1) + "/" + to_string(day1) + ", " + to_string(year2) + "/" + to_string(month2) + "/" + to_string(day2) + ", " + to_string(year3) + "/" + to_string(month3) + "/" + to_string(day3) + " - seuil1 : " + to_string(seuilApp) + ", seuil2 : " + to_string(seuilAnomalie);

                                textPart.SetString(stringTitre.c_str(), allocator);
                                document.AddMember("text", textPart, allocator);
                                document.AddMember("limit", limitAtteinte, allocator);

                                rapidjson::StringBuffer strbuf;
                                rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                                document.Accept(writer);
                                outputString = strbuf.GetString();
                            }
                        }
                    }
                }
                else // LOGINPASSWORD
                {
                    // préparer le dictionnaire
                    map<pair<uint32_t, uint32_t>, uint32_t> mapDataApp;

                    bool lireDonnees = false;
                    bool dateAtteinteBool = false;
                    uint32_t positionVectorDatePosition = 0;

                    for (uint32_t i=0; i<vectorDatePosition.size(); i++)
                    {
                        if (lireDonnees == false)
                        {
                            if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                            {
                                lireDonnees = true;
                            }
                            else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                            {
                                lireDonnees = true;
                            }
                        }

                        if (lireDonnees == true)
                        {
                            if (dateAtteinteBool == true)
                            {
                                positionVectorDatePosition = i;
                                break;
                            }
                            else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                            {
                                positionVectorDatePosition = i;
                                break;
                            }
                            else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                            {
                                dateAtteinteBool = true;
                            }

                            for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                            {
                                if (tousPaysApp == false)
                                {
                                    map<uint16_t,uint32_t>::iterator itPaysApp = mapPaysApp.find(vectorLogSSH[j].country);
                                    // Le pays correspond avec la liste de l'apprentissage
                                    if (itPaysApp != mapPaysApp.end())
                                    {
                                        pair<uint32_t, uint32_t> rechercheData = make_pair(vectorLogSSH[j].username, vectorLogSSH[j].password);

                                        map<pair<uint32_t, uint32_t>,uint32_t>::iterator it = mapDataApp.find(rechercheData);
                                        if (it == mapDataApp.end())
                                        {
                                            mapDataApp[rechercheData] = 1;
                                        }
                                        else
                                        {
                                            it->second++;
                                        }
                                    }
                                }
                                else
                                {
                                    pair<uint32_t, uint32_t> rechercheData = make_pair(vectorLogSSH[j].username, vectorLogSSH[j].password);

                                    map<pair<uint32_t, uint32_t>,uint32_t>::iterator it = mapDataApp.find(rechercheData);
                                    if (it == mapDataApp.end())
                                    {
                                        mapDataApp[rechercheData] = 1;
                                    }
                                    else
                                    {
                                        it->second++;
                                    }
                                }
                            }
                        }
                    }

                    //cout << "mapDataApp.size() : " << mapDataApp.size() << endl;

                    // Creer un vector pour trier
                    if (seuilApp > 1)
                    {
                        vector<pair<pair<uint32_t, uint32_t>,uint32_t>> vectorDataApp;

                        for (map<pair<uint32_t, uint32_t>,uint32_t>::iterator it = mapDataApp.begin(); it != mapDataApp.end(); it++)
                        {
                            vectorDataApp.push_back(make_pair(it->first, it->second));
                        }

                        mapDataApp.clear();
                        sort(vectorDataApp.begin(), vectorDataApp.end(), sort_pair_second<pair<uint32_t, uint32_t>, uint32_t>());

                        //cout << "seuilApp : " << seuilApp << endl;

                        for (uint32_t i=0; i<vectorDataApp.size(); i++)
                        {
                            uint32_t position = vectorDataApp.size() - 1 - i;
                            if(vectorDataApp[position].second >= seuilApp)
                            {
                                mapDataApp[vectorDataApp[position].first] = vectorDataApp[position].second;
                            }
                            else break;
                        }

                        //cout << "seuillage map : " << mapDataApp.size() << endl;
                    }

                    if (mapDataApp.size() == 0)
                    {
                        error = 7;
                    }
                    else
                    {
                        map<pair<uint32_t, uint32_t>, pair<uint32_t, map<uint16_t, uint32_t>>> mapDataAnomalie;
                        dateAtteinteBool = false;

                        for (uint32_t i=positionVectorDatePosition; i<vectorDatePosition.size(); i++)
                        {
                            if (dateAtteinteBool == true)
                            {
                                break;
                            }
                            else if (dateAtteinte(year3, month3, day3, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                            {
                                break;
                            }
                            else if (vectorDatePosition[i].year == year3 && vectorDatePosition[i].month == month3 && vectorDatePosition[i].day == day3)
                            {
                                dateAtteinteBool = true;
                            }

                            for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                            {
                                if (tousPays == false)
                                {
                                    //cout << "touspays false, mapPays.size : " << mapPaysAnomalie.size() << endl;

                                    map<uint16_t,uint32_t>::iterator itPays = mapPays.find(vectorLogSSH[j].country);
                                    if (itPays != mapPays.end())
                                    {
                                        pair<uint32_t, uint32_t> rechercheData = make_pair(vectorLogSSH[j].username, vectorLogSSH[j].password);

                                        // Chercher dans le dico
                                        map<pair<uint32_t, uint32_t>,uint32_t>::iterator itApp = mapDataApp.find(rechercheData);
                                        if (itApp == mapDataApp.end())
                                        {
                                            // Verifier si le pays existe dans la base
                                            map<uint16_t,structPaysAnomalie>::iterator itPays = mapPaysAnomalie.find(vectorLogSSH[j].country);
                                            if (itPays == mapPaysAnomalie.end())
                                            {
                                                structPaysAnomalie structTmp;
                                                structTmp.compteur = 1;
                                                mapPaysAnomalie[vectorLogSSH[j].country] = structTmp;
                                            }
                                            else
                                            {
                                                itPays->second.compteur++;
                                            }

                                            map<pair<uint32_t, uint32_t>, pair<uint32_t, map<uint16_t, uint32_t>>>::iterator it = mapDataAnomalie.find(rechercheData);
                                            if (it == mapDataAnomalie.end())
                                            {

                                                map<uint16_t, uint32_t> mapTmp;
                                                mapTmp[vectorLogSSH[j].country] = 1;

                                                mapDataAnomalie[rechercheData] = make_pair(1, mapTmp);
                                            }
                                            else
                                            {
                                                it->second.first++;

                                                // Ajouter le pays si besoin
                                                map<uint16_t,uint32_t>::iterator itPaysAnomalie = it->second.second.find(vectorLogSSH[j].country);
                                                if (itPaysAnomalie == it->second.second.end())
                                                {
                                                    it->second.second[vectorLogSSH[j].country] = 1;
                                                }
                                                else
                                                {
                                                    itPaysAnomalie->second++;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    //cout << "touspays true, mapPays.size : " << mapPaysAnomalie.size() << endl;

                                    pair<uint32_t, uint32_t> rechercheData = make_pair(vectorLogSSH[j].username, vectorLogSSH[j].password);

                                    // Chercher dans le dico
                                    map<pair<uint32_t, uint32_t>,uint32_t>::iterator itApp = mapDataApp.find(rechercheData);
                                    if (itApp == mapDataApp.end())
                                    {
                                        // Verifier si le pays existe dans la base
                                        map<uint16_t,structPaysAnomalie>::iterator itPays = mapPaysAnomalie.find(vectorLogSSH[j].country);
                                        if (itPays == mapPaysAnomalie.end())
                                        {
                                            structPaysAnomalie structTmp;
                                            structTmp.compteur = 1;
                                            mapPaysAnomalie[vectorLogSSH[j].country] = structTmp;
                                        }
                                        else
                                        {
                                            itPays->second.compteur++;
                                        }

                                        map<pair<uint32_t, uint32_t>, pair<uint32_t, map<uint16_t, uint32_t>>>::iterator it = mapDataAnomalie.find(rechercheData);
                                        if (it == mapDataAnomalie.end())
                                        {

                                            map<uint16_t, uint32_t> mapTmp;
                                            mapTmp[vectorLogSSH[j].country] = 1;

                                            mapDataAnomalie[rechercheData] = make_pair(1, mapTmp);
                                        }
                                        else
                                        {
                                            it->second.first++;

                                            // Ajouter le pays si besoin
                                            map<uint16_t,uint32_t>::iterator itPaysAnomalie = it->second.second.find(vectorLogSSH[j].country);
                                            if (itPaysAnomalie == it->second.second.end())
                                            {
                                                it->second.second[vectorLogSSH[j].country] = 1;
                                            }
                                            else
                                            {
                                                itPaysAnomalie->second++;
                                            }

                                        }
                                    }
                                }
                            }
                        }

                        if (mapDataAnomalie.size() == 0)
                        {
                            error = 8;
                        }
                        else
                        {

                            //cout << "mapPays.size() : " << mapPays.size() << endl;
                            //cout << "mapPaysAnomalie.size() : " << mapPaysAnomalie.size() << endl;
                            //cout << "mapDataAnomalie.size() : " << mapDataAnomalie.size() << endl;

                            // Construire le graphe

                            uniform_real_distribution<double> dist(-100, 100);
                            random_device rd;

                            rapidjson::Document document;
                            document.SetObject();
                            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
                            rapidjson::Value nodeArray(rapidjson::kArrayType);
                            rapidjson::Value edgeArray(rapidjson::kArrayType);

                            uint32_t compteurNodeID = 0, compteurEdgeID = 0;
                            uint32_t compteurCouleurPays = 0;

                            rapidjson::Value textPart;

                            //////////////////////////////////////////////////////

                            // Ajouter les anomalies
                            for (map<pair<uint32_t, uint32_t>, pair<uint32_t, map<uint16_t, uint32_t>>>::iterator itAnomalie = mapDataAnomalie.begin(); itAnomalie != mapDataAnomalie.end(); itAnomalie++)
                            {
                                if (itAnomalie->second.first >= seuilAnomalie)
                                {
                                    rapidjson::Value node(rapidjson::kObjectType);

                                    textPart.SetString(string(vectorUsername[itAnomalie->first.first] + " - " + vectorPassword[itAnomalie->first.second]).c_str(), allocator);
                                    node.AddMember("id", compteurNodeID, allocator);
                                    node.AddMember("label", textPart, allocator);
                                    node.AddMember("x", dist(rd), allocator);
                                    node.AddMember("y", dist(rd), allocator);
                                    node.AddMember("size", 5, allocator);
                                    node.AddMember("color", "#FF0000", allocator);
                                    node.AddMember("type", "square", allocator);

                                    nodeArray.PushBack(node, allocator);

                                    //cout << "size : " << itAnomalie->second.second.size() << endl;

                                    // Ajouter les liens

                                    uint32_t idNoeud = compteurNodeID++;
                                    for (map<uint16_t, uint32_t>::iterator itAnomaliePays = itAnomalie->second.second.begin(); itAnomaliePays != itAnomalie->second.second.end(); itAnomaliePays++)
                                    {
                                        //cout << itAnomaliePays->first << endl;
                                        // trouver l'ID du pays
                                        map<uint16_t,structPaysAnomalie>::iterator itTrouverIDPays = mapPaysAnomalie.find(itAnomaliePays->first);
                                        if(itTrouverIDPays != mapPaysAnomalie.end())
                                        {
                                            //cout << itAnomaliePays->first << ", " << itTrouverIDPays->second.second << endl;

                                            if (itTrouverIDPays->second.ajoute == false)
                                            {
                                                rapidjson::Value node(rapidjson::kObjectType);
                                                textPart.SetString(ipInformation.countryCodeToCountryString(itTrouverIDPays->first).c_str(), allocator);
                                                node.AddMember("id", compteurNodeID, allocator);
                                                node.AddMember("label", textPart, allocator);
                                                node.AddMember("x", dist(rd), allocator);
                                                node.AddMember("y", dist(rd), allocator);
                                                node.AddMember("size", 5, allocator);

                                                textPart.SetString(getColor(compteurCouleurPays).c_str(), allocator);
                                                node.AddMember("color", textPart, allocator);

                                                nodeArray.PushBack(node, allocator);
                                                itTrouverIDPays->second.id = compteurNodeID++;
                                                itTrouverIDPays->second.couleur = compteurCouleurPays++;
                                                itTrouverIDPays->second.ajoute = true;

                                                emptyData = false;
                                            }

                                            rapidjson::Value edge(rapidjson::kObjectType);
                                            edge.AddMember("id", compteurEdgeID++, allocator);
                                            edge.AddMember("source", idNoeud, allocator);
                                            edge.AddMember("target", itTrouverIDPays->second.id, allocator);

                                            textPart.SetString(getColor(itTrouverIDPays->second.couleur).c_str(), allocator);
                                            edge.AddMember("color", textPart, allocator);

                                            edgeArray.PushBack(edge, allocator);
                                        }
                                        else
                                        {
                                            cout << "ERREUR !!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
                                        }
                                    }

                                    if (compteurNodeID >= LIMIT_ITEM || compteurEdgeID >= LIMIT_EDGE)
                                    {
                                        limitAtteinte = true;
                                        cout << "limitAtteinte anomalie : " << compteurNodeID << ", " << compteurEdgeID << endl;
                                        break;
                                    }

                                }
                            }

                            //////////////////////////////////////////////////////

                            if (emptyData == false)
                            {
                                document.AddMember("nodes", nodeArray, allocator);
                                document.AddMember("edges", edgeArray, allocator);

                                string stringTitre = "anomalie - usernamepassword - " + to_string(year1) + "/" + to_string(month1) + "/" + to_string(day1) + ", " + to_string(year2) + "/" + to_string(month2) + "/" + to_string(day2) + ", " + to_string(year3) + "/" + to_string(month3) + "/" + to_string(day3) + " - seuil1 : " + to_string(seuilApp) + ", seuil2 : " + to_string(seuilAnomalie);

                                textPart.SetString(stringTitre.c_str(), allocator);
                                document.AddMember("text", textPart, allocator);
                                document.AddMember("limit", limitAtteinte, allocator);

                                rapidjson::StringBuffer strbuf;
                                rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                                document.Accept(writer);
                                outputString = strbuf.GetString();
                            }
                        }
                    }
                }

            }
            else
            {
                error = 6;
            }
        }
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        if (error == 0)
        {
            if (emptyData == false)
            {
                char * bufferChar = new char[outputString.length()];
                for (uint32_t i=0; i<outputString.length(); i++) bufferChar[i] = outputString[i];

                Local<Object> buf = Nan::NewBuffer(bufferChar, outputString.length()).ToLocalChecked();

                Local<Value> argv[] = {buf, Null(), False()};
                callback->Call(3, argv);
            }
            else
            {
                Local<Value> argv[] = {Null(), Null(), True()};
                callback->Call(3, argv);
            }
        }
        else
        {
            Local<Number> num = Nan::New(error);

            Local<Value> argv[] = {Null(), num, Null()};
            callback->Call(3, argv);
        }
    }
};

//Génération du graphe des attaques par pays
class paysGrapheAsync : public AsyncWorker
{

private:

    const uint32_t LIMIT_ITEM = 20000;
    const uint32_t LIMIT_EDGE = 50000;
    bool limitAtteinte = false;

    string outputString;
    int year1, month1, day1;
    int year2, month2, day2;
    string listepays;
    int data;
    int error = 0;
    bool emptyData = true;
    int isolate = 0;
    int ip = 0;

    struct structNode
    {
        uint32_t id = 0, compteur = 0, compteurEdge = 0;
        uint32_t paysNoeudID = 0;
        uint32_t paysNoeud = 0;
    };

    struct structEdge
    {
        uint32_t id, color = 0;
    };

    struct paysGraphe
    {
        uint32_t id = 0, compteur = 0, compteurEdge = 0;
        uint32_t color = 0;
    };

    string getColor(uint32_t color)
    {
        string colorString = "";

        switch (color & 0b111)
        {
        case 0:
            colorString = "#00FF00";
            break;
        case 1:
            colorString = "#0000FF";
            break;
        case 2:
            colorString = "#FF00FF";
            break;
        case 3:
            colorString = "#00FFFF";
            break;
        case 4:
            colorString = "#FF8400";
            break;
        case 5:
            colorString = "#228632";
            break;
        case 6:
            colorString = "#4DFFFA";
            break;
        case 7:
            colorString = "#6300C0";
            break;
        default:
            colorString = "#ABABAB";
        }

        return colorString;
    }

public:

    paysGrapheAsync(Callback *callback, int year1, int month1, int day1, int year2, int month2, int day2, string listepays, int data, int isolate, int ip) : AsyncWorker(callback), year1(year1), month1(month1), day1(day1), year2(year2), month2(month2), day2(day2), listepays(listepays), data(data), isolate(isolate), ip(ip) {}
    ~paysGrapheAsync()
    {
        mtx.unlock();
    }

    void Execute()
    {
        mtx.lock();

        cout << "IP : " << ip << endl;

        if (!dateAtteinte(vectorLogSSH[0].year, vectorLogSSH[0].month, vectorLogSSH[0].day, year1, month1, day1))
        {
            error = 1;
        }
        else if (dateAtteinte(vectorLogSSH[vectorLogSSH.size()-1].year, vectorLogSSH[vectorLogSSH.size()-1].month, vectorLogSSH[vectorLogSSH.size()-1].day, year2, month2, day2) && !(vectorLogSSH[vectorLogSSH.size()-1].year == year2 && vectorLogSSH[vectorLogSSH.size()-1].month == month2 && vectorLogSSH[vectorLogSSH.size()-1].day == day2))
        {
            error = 2;
        }
        else if (dateAtteinte(year2, month2, day2, year1, month1, day1) && !(year1 == year2 && month1 == month2 && day1 == day2))
        {
            error = 3;
        }

        if (error == 0)
        {
            uniform_real_distribution<double> dist(-100, 100);
            random_device rd;

            uint32_t compteurNodeID = 0, compteurEdgeID = 0;
            map<uint16_t,paysGraphe> mapPaysArgs;
            map<uint32_t,paysGraphe> mapIP;

            bool tousLesPays = false;

            if (listepays == "false")
            {
                tousLesPays = true;
            }
            else
            {
                // 105-1-23-56-113-39
                string recuperation = "";
                for (uint32_t i=0; i<listepays.length(); i++)
                {
                    bool verifier = false;

                    if (listepays[i] != '-')
                    {
                        recuperation += listepays[i];
                        if (i == listepays.length() - 1) verifier = true;
                    }
                    else
                    {
                        verifier = true;
                    }

                    if (verifier == true)
                    {
                        uint32_t paysInt = -1;
                        bool reussite = true;

                        try
                        {
                            int nombre = stol(recuperation);
                            if (nombre >= 0)
                            {
                                paysInt = (uint32_t) nombre;
                            }
                            else
                            {
                                reussite = false;
                            }
                        }
                        catch(...)
                        {
                            reussite = false;
                        }

                        recuperation = "";

                        if (reussite == true)
                        {
                            if (paysInt < ipInformation.getVectorPaysSize())
                            {
                                map<uint16_t,paysGraphe>::iterator itPaysArgs = mapPaysArgs.find(paysInt);
                                if (itPaysArgs == mapPaysArgs.end())
                                {
                                    paysGraphe tmp;

                                    if (ip == 0)
                                    {
                                        tmp.id = compteurNodeID++;
                                    }

                                    tmp.color = mapPaysArgs.size();

                                    mapPaysArgs[paysInt] = tmp;
                                }
                            }
                        }
                    }
                }
            }

            if (data < 2)
            {
                map<uint32_t,structNode> mapNode;
                map<pair<uint32_t, uint32_t>,structEdge> mapEdge;

                if (mapPaysArgs.size() != 0 || tousLesPays == true)
                {
                    bool lireDonnees = false;
                    bool dateAtteinteBool = false;

                    for (uint32_t i=0;i<vectorDatePosition.size();i++)
                    {
                        if (lireDonnees == false)
                        {
                            if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                            {
                                lireDonnees = true;
                            }
                            else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                            {
                                lireDonnees = true;
                            }
                        }

                        if (lireDonnees == true)
                        {
                            if (dateAtteinteBool == true || limitAtteinte == true)
                            {
                                break;
                            }
                            else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                            {
                                break;
                            }
                            else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                            {
                                dateAtteinteBool = true;
                            }

                            for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                            {
                                map<uint16_t,paysGraphe>::iterator itCountry = mapPaysArgs.find(vectorLogSSH[j].country);

                                if (itCountry != mapPaysArgs.end() || tousLesPays == true)
                                {
                                    if (tousLesPays == true)
                                    {
                                        // Ajouter le pays s'il n'est pas dans la liste
                                        if (itCountry == mapPaysArgs.end())
                                        {
                                            paysGraphe tmp;
                                            if (ip == 0)
                                            {
                                                tmp.id = compteurNodeID++;
                                            }

                                            tmp.color = mapPaysArgs.size();

                                            mapPaysArgs[vectorLogSSH[j].country] = tmp;
                                            itCountry = mapPaysArgs.find(vectorLogSSH[j].country);

                                            //cout << "Ajout pays : " << vectorLogSSH[j].country << endl;
                                        }
                                    }

                                    map<uint32_t,paysGraphe>::iterator itIP;
                                    if (ip == 1)
                                    {
                                        itIP = mapIP.find(vectorLogSSH[j].sourceIP);
                                        if (itIP == mapIP.end())
                                        {
                                            paysGraphe tmp;
                                            tmp.id = compteurNodeID++;
                                            tmp.color = itCountry->second.color;

                                            mapIP[vectorLogSSH[j].sourceIP] = tmp;
                                            itIP = mapIP.find(vectorLogSSH[j].sourceIP);

                                            //cout << "Ajout IP : " << "color : " << itCountry->second.color << ", " << ipInformation.intToStringIP(vectorLogSSH[j].sourceIP) << ", " << ipInformation.countryCodeToCountryString(itCountry->first) << endl;
                                        }
                                    }

                                    //cout << j << " " << vectorUsername[vectorLogSSH[j].username] << " " << vectorPassword[vectorLogSSH[j].password] << endl;

                                    map<uint32_t,structNode>::iterator itNode = mapNode.find((data == 0 ? vectorLogSSH[j].username : vectorLogSSH[j].password));
                                    if (itNode == mapNode.end())
                                    {
                                        structNode tmpNode;
                                        tmpNode.id = compteurNodeID;
                                        tmpNode.compteur = 1;
                                        tmpNode.compteurEdge = 1;

                                        if (ip == 0)
                                        {
                                            tmpNode.paysNoeudID = itCountry->second.id;
                                            tmpNode.paysNoeud = itCountry->first;
                                        }
                                        else
                                        {
                                            tmpNode.paysNoeudID = itIP->second.id;
                                            tmpNode.paysNoeud = itIP->first;
                                        }

                                        structEdge tmpEdge;
                                        tmpEdge.id = compteurEdgeID;
                                        tmpEdge.color = itCountry->second.color;

                                        mapNode[(data == 0 ? vectorLogSSH[j].username : vectorLogSSH[j].password)] = tmpNode;

                                        if (ip == 0) mapEdge[make_pair(itCountry->second.id, compteurNodeID)] = tmpEdge;
                                        else mapEdge[make_pair(itIP->second.id, compteurNodeID)] = tmpEdge;

                                        //cout << "nouveau Noeud : " << vectorUsername[vectorLogSSH[j].username] << endl;
                                        //cout << "nouveau Lien : " << itCountry->second.id << " -> " << compteurNodeID << endl;

                                        compteurNodeID++;
                                        compteurEdgeID++;

                                        if (ip == 0)
                                        {
                                            itCountry->second.compteur++;
                                            itCountry->second.compteurEdge++;
                                        }
                                        else
                                        {
                                            itIP->second.compteur++;
                                            itIP->second.compteurEdge++;
                                        }
                                    }
                                    else
                                    {
                                        // Verifier si le lien existe
                                        map<pair<uint32_t, uint32_t>,structEdge>::iterator itEdge;

                                        if (ip == 0) itEdge = mapEdge.find(make_pair(itCountry->second.id, itNode->second.id));
                                        else itEdge = mapEdge.find(make_pair(itIP->second.id, itNode->second.id));

                                        if (itEdge == mapEdge.end())
                                        {
                                            structEdge tmpEdge;
                                            tmpEdge.id = compteurEdgeID;
                                            tmpEdge.color = itCountry->second.color;

                                            if (ip == 0) mapEdge[make_pair(itCountry->second.id, itNode->second.id)] = tmpEdge;
                                            else mapEdge[make_pair(itIP->second.id, itNode->second.id)] = tmpEdge;

                                            //cout << "nouveau Lien : " << itCountry->second.id << " -> " << itNode->second.id << endl;

                                            compteurEdgeID++;
                                            if (ip == 0) itCountry->second.compteurEdge++;
                                            else itIP->second.compteurEdge++;
                                            itNode->second.compteurEdge++;
                                        }

                                        itNode->second.compteur++;
                                        if (ip == 0) itCountry->second.compteur++;
                                        else itIP->second.compteur++;
                                    }
                                }

                                if (compteurNodeID >= LIMIT_ITEM || compteurEdgeID >= LIMIT_EDGE)
                                {
                                    limitAtteinte = true;
                                    year2 = vectorDatePosition[i].year;
                                    month2 = vectorDatePosition[i].month;
                                    day2 = vectorDatePosition[i].day;
                                    cout << "limitAtteinte : " << compteurNodeID << ", " << compteurEdgeID << endl;
                                    break;
                                }
                            }
                        }
                    }

                    //////////////////////////////////

                    // Determiner la taille min max des noeuds
                    uint32_t paysMaximum = 0;

                    if (ip == 0)
                    {
                        for (map<uint16_t,paysGraphe>::iterator it = mapPaysArgs.begin(); it != mapPaysArgs.end(); it++)
                        {
                            //if (vectorNode[i].size > maximum) maximum = vectorNode[i].size;
                            if (it->second.compteur > paysMaximum) paysMaximum = it->second.compteur;
                        }
                    }
                    else
                    {
                        for (map<uint32_t,paysGraphe>::iterator it = mapIP.begin(); it != mapIP.end(); it++)
                        {
                            //if (vectorNode[i].size > maximum) maximum = vectorNode[i].size;
                            if (it->second.compteur > paysMaximum) paysMaximum = it->second.compteur;
                        }
                    }

                    uint32_t paysMinimum = paysMaximum;

                    if (ip == 0)
                    {
                        for (map<uint16_t,paysGraphe>::iterator it = mapPaysArgs.begin(); it != mapPaysArgs.end(); it++)
                        {
                            if (it->second.compteur < paysMinimum) paysMinimum = it->second.compteur;
                        }
                    }
                    else
                    {
                        for (map<uint32_t,paysGraphe>::iterator it = mapIP.begin(); it != mapIP.end(); it++)
                        {
                            if (it->second.compteur < paysMinimum) paysMinimum = it->second.compteur;
                        }
                    }

                    const uint32_t paysNombreCran = 5;
                    double paysArrayCran[paysNombreCran-1];
                    double paysCran = double(paysMaximum - paysMinimum)/double(paysNombreCran);
                    for (uint32_t i=0; i<paysNombreCran-1; i++)
                    {
                        paysArrayCran[i] = double(paysMinimum)+double(paysCran*double(i+1));
                    }

                    //////////////////////////////////

                    // Determiner la taille min max des noeuds
                    uint32_t nodeMaximum = 0;
                    for (map<uint32_t,structNode>::iterator it = mapNode.begin(); it != mapNode.end(); it++)
                    {
                        //if (vectorNode[i].size > maximum) maximum = vectorNode[i].size;
                        if (it->second.compteur > nodeMaximum) nodeMaximum = it->second.compteur;
                    }

                    uint32_t nodeMinimum = nodeMaximum;
                    for (map<uint32_t,structNode>::iterator it = mapNode.begin(); it != mapNode.end(); it++)
                    {
                        if (it->second.compteur < nodeMinimum) nodeMinimum = it->second.compteur;
                    }

                    const uint32_t nodeNombreCran = 5;
                    double nodeArrayCran[nodeNombreCran-1];
                    double nodeCran = double(nodeMaximum - nodeMinimum)/double(nodeNombreCran);
                    for (uint32_t i=0; i<nodeNombreCran-1; i++)
                    {
                        nodeArrayCran[i] = double(nodeMinimum)+double(nodeCran*double(i+1));
                    }

                    //////////////////////////////////////////////////////////////////////////////////////////

                    // suppression des noeuds isolés
                    if (isolate != 0)
                    {
                        for (map<uint32_t,structNode>::iterator it = mapNode.begin(); it != mapNode.end();)
                        {
                            if (it->second.compteurEdge == 1)
                            {
                                //cout << "isolé : " << (data == 0 ? vectorUsername[it->first].c_str() : vectorPassword[it->first].c_str()) << endl;

                                // Supprimer le lienitCountry
                                mapEdge.erase(make_pair(it->second.paysNoeudID, it->second.id));

                                if (ip == 0)
                                {
                                    // Décrémentation du compteur du pays
                                    map<uint16_t,paysGraphe>::iterator itCountry = mapPaysArgs.find(it->second.paysNoeud);
                                    if (itCountry != mapPaysArgs.end())
                                    {
                                        //cout << "compteurEdge : " << itCountry->second.compteurEdge << ", " << ipInformation.countryCodeToCountryString(itCountry->first) << endl;

                                        itCountry->second.compteurEdge--;
                                        if (itCountry->second.compteurEdge == 0)
                                        {
                                            cout << "pays isolé : " << ipInformation.countryCodeToCountryString(itCountry->first) << endl;

                                            // Suppression du pays
                                            mapPaysArgs.erase(itCountry);
                                        }
                                    }
                                }
                                else
                                {
                                    // Décrémentation du compteur de l'IP
                                    map<uint32_t,paysGraphe>::iterator itIP = mapIP.find(it->second.paysNoeud);
                                    if (itIP != mapIP.end())
                                    {
                                        itIP->second.compteurEdge--;
                                        if (itIP->second.compteurEdge == 0)
                                        {
                                            cout << "IP isolée : " << ipInformation.intToStringIP(itIP->first) << endl;

                                            // Suppression de l'IP
                                            mapIP.erase(itIP);
                                        }
                                    }
                                }

                                // Supprimer le noeud
                                it = mapNode.erase(it);
                            }
                            else
                            {
                                it++;
                            }
                        }
                    }

                    //////////////////////////////////////////////////////////////////////////////////////////

                    rapidjson::Document document;
                    document.SetObject();
                    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
                    rapidjson::Value nodeArray(rapidjson::kArrayType);
                    rapidjson::Value edgeArray(rapidjson::kArrayType);

                    rapidjson::Value textPart;

                    // Ajouter les pays
                    if (ip == 0)
                    {
                        for (map<uint16_t,paysGraphe>::iterator it = mapPaysArgs.begin(); it != mapPaysArgs.end(); it++)
                        {
                            rapidjson::Value node(rapidjson::kObjectType);

                            uint32_t tailleNoeud = paysNombreCran*10;
                            for (uint32_t j=0; j<paysNombreCran-1; j++)
                            {
                                if (double(it->second.compteur) <= paysArrayCran[j])
                                {
                                    tailleNoeud = (j+1)*10;
                                    break;
                                }
                            }

                            textPart.SetString(ipInformation.countryCodeToCountryString(it->first).c_str(), allocator);
                            node.AddMember("id", it->second.id, allocator);
                            node.AddMember("label", textPart, allocator);
                            node.AddMember("x", dist(rd), allocator);
                            node.AddMember("y", dist(rd), allocator);

                            node.AddMember("size", tailleNoeud, allocator);
                            textPart.SetString(getColor(it->second.color).c_str(), allocator);
                            node.AddMember("color", textPart, allocator);

                            nodeArray.PushBack(node, allocator);

                            emptyData = false;
                        }
                    }
                    else
                    {
                        for (map<uint32_t,paysGraphe>::iterator it = mapIP.begin(); it != mapIP.end(); it++)
                        {
                            rapidjson::Value node(rapidjson::kObjectType);

                            uint32_t tailleNoeud = paysNombreCran*10;
                            for (uint32_t j=0; j<paysNombreCran-1; j++)
                            {
                                if (double(it->second.compteur) <= paysArrayCran[j])
                                {
                                    tailleNoeud = (j+1)*10;
                                    break;
                                }
                            }

                            textPart.SetString(string(ipInformation.intToStringIP(it->first) + " (" + ipInformation.countryCodeToCountryString(ipInformation.getCountryCode(it->first)) + ")").c_str(), allocator);
                            node.AddMember("id", it->second.id, allocator);
                            node.AddMember("label", textPart, allocator);
                            node.AddMember("x", dist(rd), allocator);
                            node.AddMember("y", dist(rd), allocator);

                            node.AddMember("size", tailleNoeud, allocator);
                            textPart.SetString(getColor(it->second.color).c_str(), allocator);
                            node.AddMember("color", textPart, allocator);

                            nodeArray.PushBack(node, allocator);

                            emptyData = false;
                        }
                    }

                    // Ajouter les noeuds
                    for (map<uint32_t,structNode>::iterator it = mapNode.begin(); it != mapNode.end(); it++)
                    {
                        rapidjson::Value node(rapidjson::kObjectType);

                        uint32_t tailleNoeud = nodeNombreCran*10;
                        for (uint32_t j=0; j<nodeNombreCran-1; j++)
                        {
                            if (double(it->second.compteur) <= nodeArrayCran[j])
                            {
                                tailleNoeud = (j+1)*10;
                                break;
                            }
                        }

                        textPart.SetString((data == 0 ? vectorUsername[it->first].c_str() : vectorPassword[it->first].c_str()), allocator);
                        node.AddMember("id", it->second.id, allocator);
                        node.AddMember("label", textPart, allocator);
                        node.AddMember("x", dist(rd), allocator);
                        node.AddMember("y", dist(rd), allocator);
                        node.AddMember("type", "square", allocator);

                        node.AddMember("size", tailleNoeud, allocator);
                        node.AddMember("color", "#FF0000", allocator);

                        nodeArray.PushBack(node, allocator);
                    }

                    // Ajouter les liens
                    for (map<pair<uint32_t, uint32_t>,structEdge>::iterator it = mapEdge.begin(); it != mapEdge.end(); it++)
                    {
                        rapidjson::Value node(rapidjson::kObjectType);

                        node.AddMember("id", it->second.id, allocator);
                        node.AddMember("source", it->first.first, allocator);
                        node.AddMember("target", it->first.second, allocator);

                        textPart.SetString(getColor(it->second.color).c_str(), allocator);
                        node.AddMember("color", textPart, allocator);

                        edgeArray.PushBack(node, allocator);
                    }


                    document.AddMember("nodes", nodeArray, allocator);
                    document.AddMember("edges", edgeArray, allocator);

                    string textString = "pays - " + (data == 0 ? string("username") : string("password")) + " - " + to_string(year1) + "/" + to_string(month1) + "/" + to_string(day1) + ", " + to_string(year2) + "/" + to_string(month2) + "/" + to_string(day2);
                    textPart.SetString(textString.c_str(), allocator);
                    document.AddMember("text", textPart, allocator);

                    document.AddMember("limit", limitAtteinte, allocator);

                    rapidjson::StringBuffer strbuf;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                    document.Accept(writer);
                    outputString = strbuf.GetString();
                }
            }
            else // username - password
            {
                map<pair<uint32_t, uint32_t>,structNode> mapNode;
                map<pair<uint32_t, uint32_t>,structEdge> mapEdge;

                if (mapPaysArgs.size() != 0 || tousLesPays == true)
                {
                    bool lireDonnees = false;
                    bool dateAtteinteBool = false;

                    for (uint32_t i=0; i<vectorDatePosition.size(); i++)
                    {
                        if (lireDonnees == false)
                        {
                            if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                            {
                                lireDonnees = true;
                            }
                            else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                            {
                                lireDonnees = true;
                            }
                        }

                        if (lireDonnees == true)
                        {
                            if (dateAtteinteBool == true || limitAtteinte == true)
                            {
                                break;
                            }
                            else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                            {
                                break;
                            }
                            else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                            {
                                dateAtteinteBool = true;
                            }

                            for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                            {
                                map<uint16_t,paysGraphe>::iterator itCountry = mapPaysArgs.find(vectorLogSSH[j].country);

                                if (itCountry != mapPaysArgs.end() || tousLesPays == true)
                                {
                                    if (tousLesPays == true)
                                    {
                                        // Ajouter le pays s'il n'est pas dans la liste
                                        if (itCountry == mapPaysArgs.end())
                                        {
                                            paysGraphe tmp;

                                            if (ip == 0)
                                            {
                                                tmp.id = compteurNodeID++;
                                            }

                                            tmp.color = mapPaysArgs.size();

                                            mapPaysArgs[vectorLogSSH[j].country] = tmp;
                                            itCountry = mapPaysArgs.find(vectorLogSSH[j].country);

                                            //cout << "Ajout pays : " << vectorLogSSH[j].country << endl;
                                        }
                                    }

                                    map<uint32_t,paysGraphe>::iterator itIP;
                                    if (ip == 1)
                                    {
                                        itIP = mapIP.find(vectorLogSSH[j].sourceIP);
                                        if (itIP == mapIP.end())
                                        {
                                            paysGraphe tmp;
                                            tmp.id = compteurNodeID++;
                                            tmp.color = itCountry->second.color;

                                            mapIP[vectorLogSSH[j].sourceIP] = tmp;
                                            itIP = mapIP.find(vectorLogSSH[j].sourceIP);

                                            //cout << "Ajout IP : " << "color : " << itCountry->second.color << ", " << ipInformation.intToStringIP(vectorLogSSH[j].sourceIP) << ", " << ipInformation.countryCodeToCountryString(itCountry->first) << endl;
                                        }
                                    }

                                    //cout << j << " " << vectorUsername[vectorLogSSH[j].username] << " " << vectorPassword[vectorLogSSH[j].password] << endl;

                                    map<pair<uint32_t, uint32_t>,structNode>::iterator itNode = mapNode.find(make_pair(vectorLogSSH[j].username, vectorLogSSH[j].password));
                                    if (itNode == mapNode.end())
                                    {
                                        structNode tmpNode;
                                        tmpNode.id = compteurNodeID;
                                        tmpNode.compteur = 1;
                                        tmpNode.compteurEdge = 1;

                                        if (ip == 0)
                                        {
                                            tmpNode.paysNoeudID = itCountry->second.id;
                                            tmpNode.paysNoeud = itCountry->first;
                                        }
                                        else
                                        {
                                            tmpNode.paysNoeudID = itIP->second.id;
                                            tmpNode.paysNoeud = itIP->first;
                                        }

                                        structEdge tmpEdge;
                                        tmpEdge.id = compteurEdgeID;
                                        tmpEdge.color = itCountry->second.color;

                                        mapNode[make_pair(vectorLogSSH[j].username, vectorLogSSH[j].password)] = tmpNode;

                                        if (ip == 0) mapEdge[make_pair(itCountry->second.id, compteurNodeID)] = tmpEdge;
                                        else mapEdge[make_pair(itIP->second.id, compteurNodeID)] = tmpEdge;

                                        //cout << "nouveau Noeud : " << vectorUsername[vectorLogSSH[j].username] << endl;
                                        //cout << "nouveau Lien : " << itCountry->second.id << " -> " << compteurNodeID << endl;

                                        compteurNodeID++;
                                        compteurEdgeID++;

                                        if (ip == 0)
                                        {
                                            itCountry->second.compteur++;
                                            itCountry->second.compteurEdge++;
                                        }
                                        else
                                        {
                                            itIP->second.compteur++;
                                            itIP->second.compteurEdge++;
                                        }
                                    }
                                    else
                                    {
                                        // Verifier si le lien existe
                                        map<pair<uint32_t, uint32_t>,structEdge>::iterator itEdge;

                                        if (ip == 0) itEdge = mapEdge.find(make_pair(itCountry->second.id, itNode->second.id));
                                        else itEdge = mapEdge.find(make_pair(itIP->second.id, itNode->second.id));

                                        if (itEdge == mapEdge.end())
                                        {
                                            structEdge tmpEdge;
                                            tmpEdge.id = compteurEdgeID;
                                            tmpEdge.color = itCountry->second.color;

                                            if (ip == 0) mapEdge[make_pair(itCountry->second.id, itNode->second.id)] = tmpEdge;
                                            else mapEdge[make_pair(itIP->second.id, itNode->second.id)] = tmpEdge;

                                            //cout << "nouveau Lien : " << itCountry->second.id << " -> " << itNode->second.id << endl;

                                            compteurEdgeID++;
                                            if (ip == 0) itCountry->second.compteurEdge++;
                                            else itIP->second.compteurEdge++;
                                            itNode->second.compteurEdge++;
                                        }

                                        itNode->second.compteur++;
                                        if (ip == 0) itCountry->second.compteur++;
                                        else itIP->second.compteur++;
                                    }
                                }

                                if (compteurNodeID >= LIMIT_ITEM || compteurEdgeID >= LIMIT_EDGE)
                                {
                                    limitAtteinte = true;
                                    year2 = vectorDatePosition[i].year;
                                    month2 = vectorDatePosition[i].month;
                                    day2 = vectorDatePosition[i].day;
                                    cout << "limitAtteinte : " << compteurNodeID << ", " << compteurEdgeID << endl;
                                    break;
                                }

                            }
                        }
                    }

                    //////////////////////////////////

                    // Determiner la taille min max des noeuds
                    uint32_t paysMaximum = 0;

                    if (ip == 0)
                    {
                        for (map<uint16_t,paysGraphe>::iterator it = mapPaysArgs.begin(); it != mapPaysArgs.end(); it++)
                        {
                            //if (vectorNode[i].size > maximum) maximum = vectorNode[i].size;
                            if (it->second.compteur > paysMaximum) paysMaximum = it->second.compteur;
                        }
                    }
                    else
                    {
                        for (map<uint32_t,paysGraphe>::iterator it = mapIP.begin(); it != mapIP.end(); it++)
                        {
                            //if (vectorNode[i].size > maximum) maximum = vectorNode[i].size;
                            if (it->second.compteur > paysMaximum) paysMaximum = it->second.compteur;
                        }
                    }

                    uint32_t paysMinimum = paysMaximum;

                    if (ip == 0)
                    {
                        for (map<uint16_t,paysGraphe>::iterator it = mapPaysArgs.begin(); it != mapPaysArgs.end(); it++)
                        {
                            if (it->second.compteur < paysMinimum) paysMinimum = it->second.compteur;
                        }
                    }
                    else
                    {
                        for (map<uint32_t,paysGraphe>::iterator it = mapIP.begin(); it != mapIP.end(); it++)
                        {
                            if (it->second.compteur < paysMinimum) paysMinimum = it->second.compteur;
                        }
                    }

                    const uint32_t paysNombreCran = 5;
                    double paysArrayCran[paysNombreCran-1];
                    double paysCran = double(paysMaximum - paysMinimum)/double(paysNombreCran);
                    for (uint32_t i=0; i<paysNombreCran-1; i++)
                    {
                        paysArrayCran[i] = double(paysMinimum)+double(paysCran*double(i+1));
                    }

                    //////////////////////////////////

                    // Determiner la taille min max des noeuds
                    uint32_t nodeMaximum = 0;
                    for (map<pair<uint32_t, uint32_t>,structNode>::iterator it = mapNode.begin(); it != mapNode.end(); it++)
                    {
                        //if (vectorNode[i].size > maximum) maximum = vectorNode[i].size;
                        if (it->second.compteur > nodeMaximum) nodeMaximum = it->second.compteur;
                    }

                    uint32_t nodeMinimum = nodeMaximum;
                    for (map<pair<uint32_t, uint32_t>,structNode>::iterator it = mapNode.begin(); it != mapNode.end(); it++)
                    {
                        if (it->second.compteur < nodeMinimum) nodeMinimum = it->second.compteur;
                    }

                    const uint32_t nodeNombreCran = 5;
                    double nodeArrayCran[nodeNombreCran-1];
                    double nodeCran = double(nodeMaximum - nodeMinimum)/double(nodeNombreCran);
                    for (uint32_t i=0; i<nodeNombreCran-1; i++)
                    {
                        nodeArrayCran[i] = double(nodeMinimum)+double(nodeCran*double(i+1));
                    }

                    //////////////////////////////////////////////////////////////////////////////////////////

                    // suppression des noeuds isolés
                    if (isolate != 0)
                    {
                        for (map<pair<uint32_t, uint32_t>,structNode>::iterator it = mapNode.begin(); it != mapNode.end();)
                        {
                            if (it->second.compteurEdge == 1)
                            {
                                //cout << "isolé : " << (data == 0 ? vectorUsername[it->first].c_str() : vectorPassword[it->first].c_str()) << endl;

                                // Supprimer le lien
                                mapEdge.erase(make_pair(it->second.paysNoeudID, it->second.id));

                                if (ip == 0)
                                {
                                    // Décrémentation du compteur du pays
                                    map<uint16_t,paysGraphe>::iterator itCountry = mapPaysArgs.find(it->second.paysNoeud);
                                    if (itCountry != mapPaysArgs.end())
                                    {
                                        //cout << "compteurEdge : " << itCountry->second.compteurEdge << ", " << ipInformation.countryCodeToCountryString(itCountry->first) << endl;

                                        itCountry->second.compteurEdge--;
                                        if (itCountry->second.compteurEdge == 0)
                                        {
                                            cout << "pays isolé : " << ipInformation.countryCodeToCountryString(itCountry->first) << endl;

                                            // Suppression du pays
                                            mapPaysArgs.erase(itCountry);
                                        }
                                    }
                                }
                                else
                                {
                                    // Décrémentation du compteur de l'IP
                                    map<uint32_t,paysGraphe>::iterator itIP = mapIP.find(it->second.paysNoeud);
                                    if (itIP != mapIP.end())
                                    {
                                        itIP->second.compteurEdge--;
                                        if (itIP->second.compteurEdge == 0)
                                        {
                                            cout << "IP isolée : " << ipInformation.intToStringIP(itIP->first) << endl;

                                            // Suppression de l'IP
                                            mapIP.erase(itIP);
                                        }
                                    }
                                }

                                // Supprimer le noeud
                                it = mapNode.erase(it);
                            }
                            else
                            {
                                it++;
                            }
                        }
                    }

                    //////////////////////////////////////////////////////////////////////////////////////////

                    rapidjson::Document document;
                    document.SetObject();
                    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
                    rapidjson::Value nodeArray(rapidjson::kArrayType);
                    rapidjson::Value edgeArray(rapidjson::kArrayType);

                    rapidjson::Value textPart;

                    // Ajouter les pays
                    if (ip == 0)
                    {
                        for (map<uint16_t,paysGraphe>::iterator it = mapPaysArgs.begin(); it != mapPaysArgs.end(); it++)
                        {
                            rapidjson::Value node(rapidjson::kObjectType);

                            uint32_t tailleNoeud = paysNombreCran*10;
                            for (uint32_t j=0; j<paysNombreCran-1; j++)
                            {
                                if (double(it->second.compteur) <= paysArrayCran[j])
                                {
                                    tailleNoeud = (j+1)*10;
                                    break;
                                }
                            }

                            textPart.SetString(ipInformation.countryCodeToCountryString(it->first).c_str(), allocator);
                            node.AddMember("id", it->second.id, allocator);
                            node.AddMember("label", textPart, allocator);
                            node.AddMember("x", dist(rd), allocator);
                            node.AddMember("y", dist(rd), allocator);

                            node.AddMember("size", tailleNoeud, allocator);
                            textPart.SetString(getColor(it->second.color).c_str(), allocator);
                            node.AddMember("color", textPart, allocator);

                            nodeArray.PushBack(node, allocator);

                            emptyData = false;
                        }
                    }
                    else
                    {
                        for (map<uint32_t,paysGraphe>::iterator it = mapIP.begin(); it != mapIP.end(); it++)
                        {
                            rapidjson::Value node(rapidjson::kObjectType);

                            uint32_t tailleNoeud = paysNombreCran*10;
                            for (uint32_t j=0; j<paysNombreCran-1; j++)
                            {
                                if (double(it->second.compteur) <= paysArrayCran[j])
                                {
                                    tailleNoeud = (j+1)*10;
                                    break;
                                }
                            }

                            textPart.SetString(string(ipInformation.intToStringIP(it->first) + " (" + ipInformation.countryCodeToCountryString(ipInformation.getCountryCode(it->first)) + ")").c_str(), allocator);
                            node.AddMember("id", it->second.id, allocator);
                            node.AddMember("label", textPart, allocator);
                            node.AddMember("x", dist(rd), allocator);
                            node.AddMember("y", dist(rd), allocator);

                            node.AddMember("size", tailleNoeud, allocator);
                            textPart.SetString(getColor(it->second.color).c_str(), allocator);
                            node.AddMember("color", textPart, allocator);

                            nodeArray.PushBack(node, allocator);

                            emptyData = false;
                        }
                    }

                    // Ajouter les noeuds
                    for (map<pair<uint32_t, uint32_t>,structNode>::iterator it = mapNode.begin(); it != mapNode.end(); it++)
                    {
                        rapidjson::Value node(rapidjson::kObjectType);

                        uint32_t tailleNoeud = nodeNombreCran*10;
                        for (uint32_t j=0; j<nodeNombreCran-1; j++)
                        {
                            if (double(it->second.compteur) <= nodeArrayCran[j])
                            {
                                tailleNoeud = (j+1)*10;
                                break;
                            }
                        }

                        textPart.SetString(string(vectorUsername[it->first.first] + " - " + vectorPassword[it->first.second]).c_str(), allocator);
                        node.AddMember("id", it->second.id, allocator);
                        node.AddMember("label", textPart, allocator);
                        node.AddMember("x", dist(rd), allocator);
                        node.AddMember("y", dist(rd), allocator);
                        node.AddMember("type", "square", allocator);

                        node.AddMember("size", tailleNoeud, allocator);
                        node.AddMember("color", "#FF0000", allocator);

                        nodeArray.PushBack(node, allocator);
                    }

                    // Ajouter les liens
                    for (map<pair<uint32_t, uint32_t>,structEdge>::iterator it = mapEdge.begin(); it != mapEdge.end(); it++)
                    {
                        rapidjson::Value node(rapidjson::kObjectType);

                        node.AddMember("id", it->second.id, allocator);
                        node.AddMember("source", it->first.first, allocator);
                        node.AddMember("target", it->first.second, allocator);

                        textPart.SetString(getColor(it->second.color).c_str(), allocator);
                        node.AddMember("color", textPart, allocator);

                        edgeArray.PushBack(node, allocator);
                    }


                    document.AddMember("nodes", nodeArray, allocator);
                    document.AddMember("edges", edgeArray, allocator);

                    string textString = "pays - usernamepassword - " + to_string(year1) + "/" + to_string(month1) + "/" + to_string(day1) + ", " + to_string(year2) + "/" + to_string(month2) + "/" + to_string(day2);
                    textPart.SetString(textString.c_str(), allocator);
                    document.AddMember("text", textPart, allocator);

                    document.AddMember("limit", limitAtteinte, allocator);

                    rapidjson::StringBuffer strbuf;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                    document.Accept(writer);
                    outputString = strbuf.GetString();
                }
            }
        }
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        if (error == 0)
        {
            if (emptyData == false)
            {
                char * bufferChar = new char[outputString.length()];
                for (uint32_t i=0; i<outputString.length(); i++) bufferChar[i] = outputString[i];

                Local<Object> buf = Nan::NewBuffer(bufferChar, outputString.length()).ToLocalChecked();

                Local<Value> argv[] = {buf, Null(), False()};
                callback->Call(3, argv);
            }
            else
            {
                Local<Value> argv[] = {Null(), Null(), True()};
                callback->Call(3, argv);
            }
        }
        else
        {
            Local<Number> num = Nan::New(error);

            Local<Value> argv[] = {Null(), num, Null()};
            callback->Call(3, argv);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

// Générer les données pour la carte lors de la visualisation de données
class visualisationCarteAsync : public AsyncWorker
{

private:

    string outputString;
    int year1, month1, day1;
    int year2, month2, day2;
    uint32_t limit;
    int error = 0;
    bool emptyData = true;

public:

    visualisationCarteAsync(Callback *callback, int year1, int month1, int day1, int year2, int month2, int day2, uint32_t limit) : AsyncWorker(callback), year1(year1), month1(month1), day1(day1), year2(year2), month2(month2), day2(day2), limit(limit) {}
    ~visualisationCarteAsync()
    {
        mtx.unlock();
    }

    void Execute()
    {
        mtx.lock();

        if (!dateAtteinte(vectorLogSSH[0].year, vectorLogSSH[0].month, vectorLogSSH[0].day, year1, month1, day1))
        {
            error = 1;
        }
        else if (dateAtteinte(vectorLogSSH[vectorLogSSH.size()-1].year, vectorLogSSH[vectorLogSSH.size()-1].month, vectorLogSSH[vectorLogSSH.size()-1].day, year2, month2, day2) && !(vectorLogSSH[vectorLogSSH.size()-1].year == year2 && vectorLogSSH[vectorLogSSH.size()-1].month == month2 && vectorLogSSH[vectorLogSSH.size()-1].day == day2))
        {
            error = 2;
        }
        else if (dateAtteinte(year2, month2, day2, year1, month1, day1) && !(year1 == year2 && month1 == month2 && day1 == day2))
        {
            error = 3;
        }

        if (error == 0)
        {
            // Trier les pays
            map<uint16_t,uint32_t> mapPaysAttaqueLePlus;
            vector<pair<uint16_t,uint32_t>> vectorPaysAttaqueLePlus;

            bool lireDonnees = false;
            bool dateAtteinteBool = false;

            for (uint32_t i=0; i<vectorDatePosition.size(); i++)
            {
                if (lireDonnees == false)
                {
                    if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                    {
                        lireDonnees = true;
                    }
                    else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                    {
                        lireDonnees = true;
                    }
                }

                if (lireDonnees == true)
                {
                    if (dateAtteinteBool == true)
                    {
                        break;
                    }
                    else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                    {
                        break;
                    }
                    else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                    {
                        dateAtteinteBool = true;
                    }

                    for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                    {
                        map<uint16_t,uint32_t>::iterator it = mapPaysAttaqueLePlus.find(vectorLogSSH[j].country);
                        if (it == mapPaysAttaqueLePlus.end())
                        {
                            mapPaysAttaqueLePlus[vectorLogSSH[j].country] = vectorPaysAttaqueLePlus.size();
                            vectorPaysAttaqueLePlus.push_back(make_pair(vectorLogSSH[j].country, 1));
                        }
                        else
                        {
                            vectorPaysAttaqueLePlus[it->second].second++;
                        }
                    }
                }
            }

            mapPaysAttaqueLePlus.clear();
            sort(vectorPaysAttaqueLePlus.begin(), vectorPaysAttaqueLePlus.end(), sort_pair_second<uint16_t, uint32_t>());

            ///////////////////////////////////////////////////////////////////////////////////////

            uint32_t compteurLimit = 0;
            bool compter = (limit == 0 ? false : true);

            outputString = "country,value\n";

            uint32_t nombreAttaquesTousLesPays = 0;
            for (uint32_t i=0; i<vectorPaysAttaqueLePlus.size(); i++)
            {
                nombreAttaquesTousLesPays += vectorPaysAttaqueLePlus[i].second;
            }

            for (uint32_t i=0; i<vectorPaysAttaqueLePlus.size(); i++)
            {
                uint32_t position = vectorPaysAttaqueLePlus.size() - 1 - i;

                if (vectorPaysAttaqueLePlus[position].first != 0)
                {
                    string paysCode = ipInformation.countryCodeToAlpha3(vectorPaysAttaqueLePlus[position].first);

                    if (paysCode != "???")
                    {
                        double pourcentage = 100*(double(vectorPaysAttaqueLePlus[position].second))/nombreAttaquesTousLesPays;
                        outputString += paysCode + "," + to_string(pourcentage) + '\n';

                        if (compter == true)
                        {
                            compteurLimit++;
                            if (compteurLimit == limit) break;
                        }

                        emptyData = false;
                    }
                }
            }

        }
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        if (error == 0)
        {
            if (emptyData == false)
            {
                char * bufferChar = new char[outputString.length()];
                for (uint32_t i=0; i<outputString.length(); i++) bufferChar[i] = outputString[i];

                Local<Object> buf = Nan::NewBuffer(bufferChar, outputString.length()).ToLocalChecked();

                Local<Value> argv[] = {buf, Null(), False()};
                callback->Call(3, argv);
            }
            else
            {
                Local<Value> argv[] = {Null(), Null(), True()};
                callback->Call(3, argv);
            }
        }
        else
        {
            Local<Number> num = Nan::New(error);

            Local<Value> argv[] = {Null(), num, Null()};
            callback->Call(3, argv);
        }
    }

};

//////////////////////////////////////////////////////////////////////////////

// Génération des données pour la carte lorsque l'utilisateur a spécifié un username, un password ou les deux
class visualisation2CarteAsync : public AsyncWorker
{

private:

    int year1, month1, day1;
    int year2, month2, day2;
    int data;
    string username, password;
    int error = 0;
    bool emptyData = true;
    string outputString;

public:

    visualisation2CarteAsync(Callback *callback, int year1, int month1, int day1, int year2, int month2, int day2, int data, string username, string password) : AsyncWorker(callback), year1(year1), month1(month1), day1(day1), year2(year2), month2(month2), day2(day2), data(data), username(username), password(password) {}
    ~visualisation2CarteAsync()
    {
        mtx.unlock();
    }

    void Execute()
    {
        mtx.lock();

        if (!dateAtteinte(vectorLogSSH[0].year, vectorLogSSH[0].month, vectorLogSSH[0].day, year1, month1, day1))
        {
            error = 1;
        }
        else if (dateAtteinte(vectorLogSSH[vectorLogSSH.size()-1].year, vectorLogSSH[vectorLogSSH.size()-1].month, vectorLogSSH[vectorLogSSH.size()-1].day, year2, month2, day2) && !(vectorLogSSH[vectorLogSSH.size()-1].year == year2 && vectorLogSSH[vectorLogSSH.size()-1].month == month2 && vectorLogSSH[vectorLogSSH.size()-1].day == day2))
        {
            error = 2;
        }
        else if (dateAtteinte(year2, month2, day2, year1, month1, day1) && !(year1 == year2 && month1 == month2 && day1 == day2))
        {
            error = 3;
        }

        uint32_t recherche1 = 0, recherche2 = 0;
        if (data == 0) // username
        {
            map<string,uint32_t>::iterator it = mapUsername.find(username);
            if (it != mapUsername.end())
            {
                recherche1 = it->second;
            }
            else
            {
                error = 4;
            }
        }
        else if (data == 1) // password
        {
            map<string,uint32_t>::iterator it = mapPassword.find(password);
            if (it != mapPassword.end())
            {
                recherche1 = it->second;
            }
            else
            {
                error = 5;
            }
        }
        else // username password
        {
            map<string,uint32_t>::iterator it = mapUsername.find(username);
            if (it != mapUsername.end())
            {
                recherche1 = it->second;

                map<string,uint32_t>::iterator it2 = mapPassword.find(password);
                if (it2 != mapPassword.end())
                {
                    recherche2 = it2->second;
                }
                else
                {
                    error = 5;
                }
            }
            else
            {
                error = 4;
            }
        }

        if (error == 0)
        {
            bool lireDonnees = false;
            bool dateAtteinteBool = false;

            vector<pair<uint16_t, uint32_t>> vectorPaysAttaque;
            map<uint16_t,uint32_t> mapPaysAttaque;

            for (uint32_t i=0; i<vectorDatePosition.size(); i++)
            {
                if (lireDonnees == false)
                {
                    if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                    {
                        lireDonnees = true;
                    }
                    else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                    {
                        lireDonnees = true;
                    }
                }

                if (lireDonnees == true)
                {
                    if (dateAtteinteBool == true)
                    {
                        break;
                    }
                    else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                    {
                        break;
                    }
                    else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                    {
                        dateAtteinteBool = true;
                    }

                    for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                    {
                        if (data == 0)
                        {
                            if (vectorLogSSH[j].username == recherche1)
                            {
                                map<uint16_t,uint32_t>::iterator it = mapPaysAttaque.find(vectorLogSSH[j].country);
                                if (it == mapPaysAttaque.end())
                                {
                                    mapPaysAttaque[vectorLogSSH[j].country] = vectorPaysAttaque.size();
                                    vectorPaysAttaque.push_back(make_pair(vectorLogSSH[j].country, 1));
                                }
                                else
                                {
                                    vectorPaysAttaque[it->second].second++;
                                }
                            }
                        }
                        else if (data == 1)
                        {
                            if (vectorLogSSH[j].password == recherche1)
                            {
                                map<uint16_t,uint32_t>::iterator it = mapPaysAttaque.find(vectorLogSSH[j].country);
                                if (it == mapPaysAttaque.end())
                                {
                                    mapPaysAttaque[vectorLogSSH[j].country] = vectorPaysAttaque.size();
                                    vectorPaysAttaque.push_back(make_pair(vectorLogSSH[j].country, 1));
                                }
                                else
                                {
                                    vectorPaysAttaque[it->second].second++;
                                }
                            }
                        }
                        else
                        {
                            if (vectorLogSSH[j].username == recherche1 && vectorLogSSH[j].password == recherche2)
                            {
                                map<uint16_t,uint32_t>::iterator it = mapPaysAttaque.find(vectorLogSSH[j].country);
                                if (it == mapPaysAttaque.end())
                                {
                                    mapPaysAttaque[vectorLogSSH[j].country] = vectorPaysAttaque.size();
                                    vectorPaysAttaque.push_back(make_pair(vectorLogSSH[j].country, 1));
                                }
                                else
                                {
                                    vectorPaysAttaque[it->second].second++;
                                }
                            }
                        }
                    }
                }
            }

            mapPaysAttaque.clear();
            sort(vectorPaysAttaque.begin(), vectorPaysAttaque.end(), sort_pair_second<uint16_t, uint32_t>());

            ///////////////////////////////////////////////////////////////////////////////////////

            outputString = "country,value\n";

            uint32_t nombreAttaquesTousLesPays = 0;
            for (uint32_t i=0; i<vectorPaysAttaque.size(); i++)
            {
                nombreAttaquesTousLesPays += vectorPaysAttaque[i].second;
            }

            for (uint32_t i=0; i<vectorPaysAttaque.size(); i++)
            {
                uint32_t position = vectorPaysAttaque.size() - 1 - i;

                if (vectorPaysAttaque[position].first != 0)
                {
                    string paysCode = ipInformation.countryCodeToAlpha3(vectorPaysAttaque[position].first);

                    if (paysCode != "???")
                    {
                        double pourcentage = 100*(double(vectorPaysAttaque[position].second))/nombreAttaquesTousLesPays;
                        outputString += paysCode + "," + to_string(pourcentage) + '\n';
                        emptyData = false;
                    }
                }
            }

        }
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        if (error == 0)
        {
            if (emptyData == false)
            {
                char * bufferChar = new char[outputString.length()];
                for (uint32_t i=0; i<outputString.length(); i++) bufferChar[i] = outputString[i];

                Local<Object> buf = Nan::NewBuffer(bufferChar, outputString.length()).ToLocalChecked();

                Local<Value> argv[] = {buf, Null(), False()};
                callback->Call(3, argv);
            }
            else
            {
                Local<Value> argv[] = {Null(), Null(), True()};
                callback->Call(3, argv);
            }
        }
        else
        {
            Local<Number> num = Nan::New(error);

            Local<Value> argv[] = {Null(), num, Null()};
            callback->Call(3, argv);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

// Pour les relations username/password lors de la visualisation de données
class visualisationUsernamePasswordCercleAsync : public AsyncWorker
{

private:

    string outputString;
    int year1, month1, day1;
    int year2, month2, day2;
    uint32_t limit;
    int error = 0;
    bool emptyData = true;

public:

    visualisationUsernamePasswordCercleAsync(Callback *callback, int year1, int month1, int day1, int year2, int month2, int day2, uint32_t limit) : AsyncWorker(callback), year1(year1), month1(month1), day1(day1), year2(year2), month2(month2), day2(day2), limit(limit) {}
    ~visualisationUsernamePasswordCercleAsync()
    {
        mtx.unlock();
    }

    void Execute()
    {
        mtx.lock();

        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        vector<uint32_t> vectorUsernameIndex;
        map<uint32_t,uint32_t> mapUsernameIndex;

        vector<uint32_t> vectorPasswordIndex;
        map<uint32_t,uint32_t> mapPasswordIndex;

        vector<pair<pair<uint32_t,uint32_t>, uint32_t>> vectorUsernamePassword;
        map<pair<uint32_t,uint32_t>, uint32_t> mapUsernamePassword;
        map<pair<uint32_t,uint32_t>, char> mapUsernamePasswordGlobal;

        uint32_t compteurLimit = 0;
        bool compter = (limit == 0 ? false : true);

        /////////////////////////////////////////////////

        if (!dateAtteinte(vectorLogSSH[0].year, vectorLogSSH[0].month, vectorLogSSH[0].day, year1, month1, day1))
        {
            error = 1;
        }
        else if (dateAtteinte(vectorLogSSH[vectorLogSSH.size()-1].year, vectorLogSSH[vectorLogSSH.size()-1].month, vectorLogSSH[vectorLogSSH.size()-1].day, year2, month2, day2) && !(vectorLogSSH[vectorLogSSH.size()-1].year == year2 && vectorLogSSH[vectorLogSSH.size()-1].month == month2 && vectorLogSSH[vectorLogSSH.size()-1].day == day2))
        {
            error = 2;
        }
        else if (dateAtteinte(year2, month2, day2, year1, month1, day1) && !(year1 == year2 && month1 == month2 && day1 == day2))
        {
            error = 3;
        }

        if (error == 0)
        {
            bool lireDonnees = false;
            bool dateAtteinteBool = false;

            for (uint32_t i=0; i<vectorDatePosition.size(); i++)
            {
                if (lireDonnees == false)
                {
                    if (vectorDatePosition[i].year == year1 && vectorDatePosition[i].month == month1 && vectorDatePosition[i].day == day1)
                    {
                        lireDonnees = true;
                    }
                    else if (dateAtteinte(year1, month1, day1, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day))
                    {
                        lireDonnees = true;
                    }
                }

                if (lireDonnees == true)
                {
                    if (dateAtteinteBool == true)
                    {
                        break;
                    }
                    else if (dateAtteinte(year2, month2, day2, vectorDatePosition[i].year, vectorDatePosition[i].month, vectorDatePosition[i].day, true))
                    {
                        break;
                    }
                    else if (vectorDatePosition[i].year == year2 && vectorDatePosition[i].month == month2 && vectorDatePosition[i].day == day2)
                    {
                        dateAtteinteBool = true;
                    }

                    for (uint32_t j=vectorDatePosition[i].value; j<((i == vectorDatePosition.size() - 1) ? vectorLogSSH.size() : vectorDatePosition[i+1].value); j++)
                    {
                        pair<uint32_t,uint32_t> pairSearch = make_pair(vectorLogSSH[j].username, vectorLogSSH[j].password);

                        map<pair<uint32_t,uint32_t>,uint32_t>::iterator it = mapUsernamePassword.find(pairSearch);
                        if (it == mapUsernamePassword.end())
                        {
                            mapUsernamePassword[pairSearch] = vectorUsernamePassword.size();
                            vectorUsernamePassword.push_back(make_pair(pairSearch,1));
                        }
                        else
                        {
                            vectorUsernamePassword[it->second].second++;
                        }
                    }
                }
            }

            if (vectorUsernamePassword.size() != 0)
            {
                emptyData = false;

                rapidjson::Value TableauArrayInside2(rapidjson::kArrayType);

                sort(vectorUsernamePassword.begin(), vectorUsernamePassword.end(), sort_pair_second<pair<uint32_t,uint32_t>, uint32_t>());

                for (uint32_t j=0; j<vectorUsernamePassword.size(); j++)
                {
                    uint32_t position = vectorUsernamePassword.size() - 1 - j;

                    rapidjson::Value coupleJson(rapidjson::kObjectType);

                    map<uint32_t,uint32_t>::iterator iteratorUsernameIndex = mapUsernameIndex.find(vectorUsernamePassword[position].first.first);
                    if (iteratorUsernameIndex == mapUsernameIndex.end())
                    {
                        mapUsernameIndex[vectorUsernamePassword[position].first.first] = vectorUsernameIndex.size();
                        coupleJson.AddMember("u", vectorUsernameIndex.size(), allocator);
                        vectorUsernameIndex.push_back(vectorUsernamePassword[position].first.first);
                    }
                    else
                    {
                        coupleJson.AddMember("u", iteratorUsernameIndex->second, allocator);
                    }

                    map<uint32_t,uint32_t>::iterator iteratorPasswordIndex = mapPasswordIndex.find(vectorUsernamePassword[position].first.second);
                    if (iteratorPasswordIndex == mapPasswordIndex.end())
                    {
                        mapPasswordIndex[vectorUsernamePassword[position].first.second] = vectorPasswordIndex.size();
                        coupleJson.AddMember("p", vectorPasswordIndex.size(), allocator);
                        vectorPasswordIndex.push_back(vectorUsernamePassword[position].first.second);
                    }
                    else
                    {
                        coupleJson.AddMember("p", iteratorPasswordIndex->second, allocator);
                    }

                    // En cas de besoin du poids du couple
                    //coupleJson.AddMember("a", vectorUsernamePassword[position].second, allocator);

                    TableauArrayInside2.PushBack(coupleJson, allocator);

                    if (compter == true)
                    {
                        compteurLimit++;
                        if (compteurLimit == limit) break;
                    }
                }

                document.AddMember("couples", TableauArrayInside2, allocator);

                /////////////////////////////////////////////////

                rapidjson::Value indexUsername(rapidjson::kArrayType);

                for (uint32_t i=0; i<vectorUsernameIndex.size(); i++)
                {
                    rapidjson::Value textPart;
                    textPart.SetString(vectorUsername[vectorUsernameIndex[i]].c_str(), allocator);
                    indexUsername.PushBack(textPart, allocator);
                }

                document.AddMember("username", indexUsername, allocator);

                rapidjson::Value indexPassword(rapidjson::kArrayType);

                for (uint32_t i=0; i<vectorPasswordIndex.size(); i++)
                {
                    rapidjson::Value textPart;
                    textPart.SetString(vectorPassword[vectorPasswordIndex[i]].c_str(), allocator);
                    indexPassword.PushBack(textPart, allocator);
                }

                document.AddMember("password", indexPassword, allocator);

                /////////////////////////////////////////////////

                rapidjson::StringBuffer strbuf;
                rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
                document.Accept(writer);
                outputString = strbuf.GetString();
            }
        }
    }

    void HandleOKCallback()
    {
        Nan::HandleScope scope;

        if (error == 0)
        {
            if (emptyData == false)
            {
                char * bufferChar = new char[outputString.length()];
                for (uint32_t i=0; i<outputString.length(); i++) bufferChar[i] = outputString[i];

                Local<Object> buf = Nan::NewBuffer(bufferChar, outputString.length()).ToLocalChecked();

                Local<Value> argv[] = {buf, Null(), False()};
                callback->Call(3, argv);
            }
            else
            {
                Local<Value> argv[] = {Null(), Null(), True()};
                callback->Call(3, argv);
            }
        }
        else
        {
            Local<Number> num = Nan::New(error);

            Local<Value> argv[] = {Null(), num, Null()};
            callback->Call(3, argv);
        }
    }

};

//////////////////////////////////////////////////////////////////////////////

// Méthodes de passage du monde de Node.js à celui de l'addon

NAN_METHOD(visualisation)
{
    int year1 = info[0]->NumberValue();
    int month1 = info[1]->NumberValue();
    int day1 = info[2]->NumberValue();

    int year2 = info[3]->NumberValue();
    int month2 = info[4]->NumberValue();
    int day2 = info[5]->NumberValue();

    int limit = info[6]->NumberValue();
    int data = info[7]->NumberValue();


    Callback *callback = new Callback(info[8].As<Function>());
    AsyncQueueWorker(new visualisationAsync(callback, year1, month1, day1, year2, month2, day2, limit, data));
}

NAN_METHOD(visualisationCarte)
{
    int year1 = info[0]->NumberValue();
    int month1 = info[1]->NumberValue();
    int day1 = info[2]->NumberValue();

    int year2 = info[3]->NumberValue();
    int month2 = info[4]->NumberValue();
    int day2 = info[5]->NumberValue();

    int limit = info[6]->NumberValue();

    Callback *callback = new Callback(info[7].As<Function>());
    AsyncQueueWorker(new visualisationCarteAsync(callback, year1, month1, day1, year2, month2, day2, limit));
}

NAN_METHOD(visualisationUsernamePasswordCercle)
{
    int year1 = info[0]->NumberValue();
    int month1 = info[1]->NumberValue();
    int day1 = info[2]->NumberValue();

    int year2 = info[3]->NumberValue();
    int month2 = info[4]->NumberValue();
    int day2 = info[5]->NumberValue();

    int limit = info[6]->NumberValue();

    Callback *callback = new Callback(info[7].As<Function>());
    AsyncQueueWorker(new visualisationUsernamePasswordCercleAsync(callback, year1, month1, day1, year2, month2, day2, limit));
}

NAN_METHOD(date)
{
    Callback *callback = new Callback(info[0].As<Function>());
    AsyncQueueWorker(new dateAsync(callback));
}

NAN_METHOD(pays)
{
    int year1 = info[0]->NumberValue();
    int month1 = info[1]->NumberValue();
    int day1 = info[2]->NumberValue();

    int year2 = info[3]->NumberValue();
    int month2 = info[4]->NumberValue();
    int day2 = info[5]->NumberValue();

    bool suivant = (info[6]->NumberValue() == 0 ? false : true);

    Callback *callback = new Callback(info[7].As<Function>());
    AsyncQueueWorker(new paysAsync(callback, year1, month1, day1, year2, month2, day2, suivant));
}

NAN_METHOD(paysGraphe)
{
    int year1 = info[0]->NumberValue();
    int month1 = info[1]->NumberValue();
    int day1 = info[2]->NumberValue();

    int year2 = info[3]->NumberValue();
    int month2 = info[4]->NumberValue();
    int day2 = info[5]->NumberValue();

    String::Utf8Value listepaysV8(info[6]->ToString());
    string listepaysString = string(*listepaysV8);

    int data = info[7]->NumberValue();
    int isolate = info[8]->NumberValue();

    int IP = info[9]->NumberValue();

    Callback *callback = new Callback(info[10].As<Function>());
    AsyncQueueWorker(new paysGrapheAsync(callback, year1, month1, day1, year2, month2, day2, listepaysString, data, isolate, IP));
}

NAN_METHOD(anomalie)
{
    int year1 = info[0]->NumberValue();
    int month1 = info[1]->NumberValue();
    int day1 = info[2]->NumberValue();

    int year2 = info[3]->NumberValue();
    int month2 = info[4]->NumberValue();
    int day2 = info[5]->NumberValue();

    int year3 = info[6]->NumberValue();
    int month3 = info[7]->NumberValue();
    int day3 = info[8]->NumberValue();

    String::Utf8Value listepaysAppV8(info[9]->ToString());
    string listepaysAppString = string(*listepaysAppV8);

    String::Utf8Value listepaysV8(info[10]->ToString());
    string listepaysString = string(*listepaysV8);

    int data = info[11]->NumberValue();

    uint32_t seuilApp  = info[12]->NumberValue();
    uint32_t seuilAnomalie = info[13]->NumberValue();

    Callback *callback = new Callback(info[14].As<Function>());
    AsyncQueueWorker(new anomalieAsync(callback, year1, month1, day1, year2, month2, day2, year3, month3, day3, listepaysAppString, listepaysString, data, seuilApp, seuilAnomalie));
}

NAN_METHOD(linear)
{
    int year1 = info[0]->NumberValue();
    int month1 = info[1]->NumberValue();
    int day1 = info[2]->NumberValue();

    int year2 = info[3]->NumberValue();
    int month2 = info[4]->NumberValue();
    int day2 = info[5]->NumberValue();

    Callback *callback = new Callback(info[6].As<Function>());
    AsyncQueueWorker(new linearAsync(callback, year1, month1, day1, year2, month2, day2));
}

NAN_METHOD(linear2)
{
    int year1 = info[0]->NumberValue();
    int month1 = info[1]->NumberValue();
    int day1 = info[2]->NumberValue();

    int year2 = info[3]->NumberValue();
    int month2 = info[4]->NumberValue();
    int day2 = info[5]->NumberValue();

    int data = info[6]->NumberValue();

    String::Utf8Value usernameV8(info[7]->ToString());
    string usernameString = string(*usernameV8);

    String::Utf8Value passwordV8(info[8]->ToString());
    string passwordString = string(*passwordV8);

    Callback *callback = new Callback(info[9].As<Function>());
    AsyncQueueWorker(new linear2Async(callback, year1, month1, day1, year2, month2, day2, data, usernameString, passwordString));
}

NAN_METHOD(visualisation2)
{
    int year1 = info[0]->NumberValue();
    int month1 = info[1]->NumberValue();
    int day1 = info[2]->NumberValue();

    int year2 = info[3]->NumberValue();
    int month2 = info[4]->NumberValue();
    int day2 = info[5]->NumberValue();

    int data = info[6]->NumberValue();

    String::Utf8Value usernameV8(info[7]->ToString());
    string usernameString = string(*usernameV8);

    String::Utf8Value passwordV8(info[8]->ToString());
    string passwordString = string(*passwordV8);

    Callback *callback = new Callback(info[9].As<Function>());
    AsyncQueueWorker(new visualisation2Async(callback, year1, month1, day1, year2, month2, day2, data, usernameString, passwordString));
}

NAN_METHOD(visualisation2Carte)
{
    int year1 = info[0]->NumberValue();
    int month1 = info[1]->NumberValue();
    int day1 = info[2]->NumberValue();

    int year2 = info[3]->NumberValue();
    int month2 = info[4]->NumberValue();
    int day2 = info[5]->NumberValue();

    int data = info[6]->NumberValue();

    String::Utf8Value usernameV8(info[7]->ToString());
    string usernameString = string(*usernameV8);

    String::Utf8Value passwordV8(info[8]->ToString());
    string passwordString = string(*passwordV8);

    Callback *callback = new Callback(info[9].As<Function>());
    AsyncQueueWorker(new visualisation2CarteAsync(callback, year1, month1, day1, year2, month2, day2, data, usernameString, passwordString));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Lecture du fichier de logs, fonction synchrone
void Readfile(const v8::FunctionCallbackInfo<Value>& args)
{

    Isolate* isolate = args.GetIsolate();

    if (args.Length() < 2)
    {
        isolate->ThrowException(Exception::TypeError(
                                    String::NewFromUtf8(isolate, "No enough arguments (geo location file, ssh logs)")));
        return;
    }

    String::Utf8Value geoLocationV8(args[0]->ToString());
    string geoLocation = string(*geoLocationV8);

    String::Utf8Value sshLogsV8(args[1]->ToString());
    string sshLogs = string(*sshLogsV8);

    map<string,uint32_t> mapSoftware;
    map<string,uint32_t> mapMac;
    map<string,uint32_t> mapCipher;

    bool dateDefinie = false;
    int previousYear = 0, previousMonth = 0, previousDay = 0;

    ipInformation.readFile(geoLocation);
    if (ipInformation.getErreur() == true)
    {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, ipInformation.getErreurMessage().c_str())));
        return;
    }

    ifstream file (sshLogs, ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        char * buffer  = NULL;
        size_t bufferSize = 0;
        const size_t fileSize = file.tellg();

        if (fileSize > BUFFER_SIZE_MAX) bufferSize = BUFFER_SIZE_MAX;
        else bufferSize = fileSize;

        try
        {
            buffer = new char [bufferSize];
        }
        catch (...)
        {
            delete [] buffer;
            file.close();
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unable the create the buffer")));
            return;
        }

        size_t bufferPosition = 0;
        string ligne = "";

        for (size_t i = 0; i < fileSize; i++)
        {
            if (bufferPosition == bufferSize || i == 0)
            {
                if (fileSize - i < bufferSize)
                {
                    bufferSize = fileSize - i;
                }

                try
                {
                    file.seekg (i, ios::beg);
                    file.read (buffer, bufferSize);
                }
                catch (...)
                {
                    delete [] buffer;
                    file.close();
                    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unable to read the file")));
                    return;
                }

                bufferPosition = 0;
            }

            if (buffer[bufferPosition] == 10)
            {
                rapidjson::Document document;

                if (document.Parse(ligne.c_str()).HasParseError() == false)
                {
                    logSSH logLigne;

                    if (document.HasMember("username") == true)
                    {
                        rapidjson::Value& value1 = document["username"];
                        if (value1.IsString() == true)
                        {
                            string username = value1.GetString();

                            map<string,uint32_t>::iterator it = mapUsername.find(username);
                            if (it == mapUsername.end())
                            {
                                logLigne.username = vectorUsername.size();
                                mapUsername[username] = vectorUsername.size();
                                vectorUsername.push_back(username);
                            }
                            else
                            {
                                logLigne.username = it->second;
                            }
                        }
                    }

                    if (document.HasMember("try") == true)
                    {
                        rapidjson::Value& value2 = document["try"];
                        if (value2.IsInt() == true)
                        {
                            logLigne.attempt = value2.GetInt();
                        }
                    }

                    if (document.HasMember("src_port") == true)
                    {
                        rapidjson::Value& value3 = document["src_port"];
                        if (value3.IsInt() == true)
                        {
                            logLigne.sourcePort = value3.GetInt();
                        }
                    }

                    if (document.HasMember("software_version") == true)
                    {
                        rapidjson::Value& value5 = document["software_version"];
                        if (value5.IsString() == true)
                        {
                            string software = value5.GetString();

                            map<string,uint32_t>::iterator it = mapSoftware.find(software);
                            if (it == mapSoftware.end())
                            {
                                logLigne.software = vectorSoftware.size();
                                mapSoftware[software] = vectorSoftware.size();
                                vectorSoftware.push_back(software);
                            }
                            else
                            {
                                logLigne.software = it->second;
                            }
                        }
                    }

                    if (document.HasMember("timestamp") == true)
                    {
                        rapidjson::Value& value6 = document["timestamp"];
                        if (value6.IsString() == true)
                        {
                            //2017-02-21T11:28:22.640054
                            string timestamp = value6.GetString();

                            uint32_t position = 0;
                            string recuperation = "";

                            for (uint32_t j=0; j<timestamp.length(); j++)
                            {
                                if (timestamp[j] != '-') recuperation += timestamp[j];
                                else
                                {
                                    try
                                    {
                                        logLigne.year = stol(recuperation);
                                    }
                                    catch(...)
                                    {
                                        logLigne.year = -1;
                                    }
                                    position = j+1;
                                    break;
                                }
                            }

                            recuperation = "";

                            for (uint32_t j=position; j<timestamp.length(); j++)
                            {
                                if (timestamp[j] != '-') recuperation += timestamp[j];
                                else
                                {
                                    try
                                    {
                                        logLigne.month = stol(recuperation);
                                    }
                                    catch(...)
                                    {
                                        logLigne.month = -1;
                                    }
                                    position = j+1;
                                    break;
                                }
                            }

                            recuperation = "";

                            for (uint32_t j=position; j<timestamp.length(); j++)
                            {
                                if (timestamp[j] != 'T') recuperation += timestamp[j];
                                else
                                {
                                    try
                                    {
                                        logLigne.day = stol(recuperation);
                                    }
                                    catch(...)
                                    {
                                        logLigne.day = -1;
                                    }
                                    position = j+1;
                                    break;
                                }
                            }

                            recuperation = "";

                            for (uint32_t j=position; j<timestamp.length(); j++)
                            {
                                if (timestamp[j] != ':') recuperation += timestamp[j];
                                else
                                {
                                    try
                                    {
                                        logLigne.hour = stol(recuperation);
                                    }
                                    catch(...)
                                    {
                                        logLigne.hour = -1;
                                    }
                                    position = j+1;
                                    break;
                                }
                            }

                            recuperation = "";

                            for (uint32_t j=position; j<timestamp.length(); j++)
                            {
                                if (timestamp[j] != ':') recuperation += timestamp[j];
                                else
                                {
                                    try
                                    {
                                        logLigne.minute = stol(recuperation);
                                    }
                                    catch(...)
                                    {
                                        logLigne.minute = -1;
                                    }
                                    position = j+1;
                                    break;
                                }
                            }

                            recuperation = "";

                            for (uint32_t j=position; j<timestamp.length(); j++)
                            {
                                if (timestamp[j] != '.') recuperation += timestamp[j];
                                else
                                {
                                    try
                                    {
                                        logLigne.second = stol(recuperation);
                                    }
                                    catch(...)
                                    {
                                        logLigne.second = -1;
                                    }
                                    position = j+1;
                                    break;
                                }
                            }

                            recuperation = "";

                            for (uint32_t j=position; j<timestamp.length(); j++)
                            {
                                recuperation += timestamp[j];
                            }

                            try
                            {
                                logLigne.microsecond = stol(recuperation);
                            }
                            catch(...)
                            {
                                logLigne.microsecond = -1;
                            }
                        }
                    }

                    if (document.HasMember("src_ip") == true)
                    {
                        rapidjson::Value& value7 = document["src_ip"];
                        if (value7.IsString() == true)
                        {
                            logLigne.sourceIP = ipInformation.stringToIntIP(string(value7.GetString()));
                            logLigne.country = ipInformation.getCountryCode(string(value7.GetString()));
                        }
                    }

                    if (document.HasMember("mac") == true)
                    {
                        rapidjson::Value& value8 = document["mac"];
                        if (value8.IsString() == true)
                        {
                            string mac = value8.GetString();

                            map<string,uint32_t>::iterator it = mapMac.find(mac);
                            if (it == mapMac.end())
                            {
                                logLigne.mac = vectorMac.size();
                                mapMac[mac] = vectorMac.size();
                                vectorMac.push_back(mac);
                            }
                            else
                            {
                                logLigne.mac = it->second;
                            }
                        }
                    }

                    if (document.HasMember("cipher") == true)
                    {
                        rapidjson::Value& value9 = document["cipher"];
                        if (value9.IsString() == true)
                        {
                            string cipher = value9.GetString();

                            map<string,uint32_t>::iterator it = mapCipher.find(cipher);
                            if (it == mapCipher.end())
                            {
                                logLigne.cipher = vectorCipher.size();
                                mapCipher[cipher] = vectorCipher.size();
                                vectorCipher.push_back(cipher);
                            }
                            else
                            {
                                logLigne.cipher = it->second;
                            }
                        }
                    }

                    if (document.HasMember("password") == true)
                    {
                        rapidjson::Value& value10 = document["password"];
                        if (value10.IsString() == true)
                        {
                            string password = value10.GetString();

                            map<string,uint32_t>::iterator it = mapPassword.find(password);
                            if (it == mapPassword.end())
                            {
                                logLigne.password = vectorPassword.size();
                                mapPassword[password] = vectorPassword.size();
                                vectorPassword.push_back(password);
                            }
                            else
                            {
                                logLigne.password = it->second;
                            }
                        }
                    }

                    if (dateDefinie == false || !(logLigne.year == previousYear && logLigne.month == previousMonth && logLigne.day == previousDay))
                    {
                        datePosition newDatePosition;
                        newDatePosition.year = logLigne.year;
                        newDatePosition.month = logLigne.month;
                        newDatePosition.day = logLigne.day;
                        newDatePosition.value = vectorLogSSH.size();


                        vectorDatePosition.push_back(newDatePosition);

                        previousYear = logLigne.year;
                        previousMonth = logLigne.month;
                        previousDay = logLigne.day;

                        cout << "nouvelle date : " << previousYear << ", " << previousMonth << ", " << previousDay << "   " << newDatePosition.value << endl;

                        dateDefinie = true;
                    }

                    vectorLogSSH.push_back(logLigne);

                }
                else
                {
                    cerr << "json.parse error" << endl;
                }

                ligne = "";
            }
            else
            {
                ligne += buffer[bufferPosition];
            }

            bufferPosition++;
        }

        delete [] buffer;
        file.close();

        if (vectorLogSSH.size() == 0 || vectorDatePosition.size() == 0)
        {
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "vectorLogSSH empty")));
            return;
        }

    }
    else
    {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unable the open the file")));
        return;
    }

    cout << "vectorLogSSH : " << vectorLogSSH.size() << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Définition des noms des méthodes à appeler depuis Node.js pour utiliser les méthodes présentent dans l'addon
NAN_MODULE_INIT(Init)
{
    // Fonctions de traitements des données, fonctions asynchrones
    Nan::Set(target, New<String>("visualisation").ToLocalChecked(), GetFunction(New<FunctionTemplate>(visualisation)).ToLocalChecked());
    Nan::Set(target, New<String>("date").ToLocalChecked(), GetFunction(New<FunctionTemplate>(date)).ToLocalChecked());
    Nan::Set(target, New<String>("visualisationCarte").ToLocalChecked(), GetFunction(New<FunctionTemplate>(visualisationCarte)).ToLocalChecked());
    Nan::Set(target, New<String>("visualisationUsernamePasswordCercle").ToLocalChecked(), GetFunction(New<FunctionTemplate>(visualisationUsernamePasswordCercle)).ToLocalChecked());
    Nan::Set(target, New<String>("pays").ToLocalChecked(), GetFunction(New<FunctionTemplate>(pays)).ToLocalChecked());
    Nan::Set(target, New<String>("paysGraphe").ToLocalChecked(), GetFunction(New<FunctionTemplate>(paysGraphe)).ToLocalChecked());
    Nan::Set(target, New<String>("linear").ToLocalChecked(), GetFunction(New<FunctionTemplate>(linear)).ToLocalChecked());
    Nan::Set(target, New<String>("linear2").ToLocalChecked(), GetFunction(New<FunctionTemplate>(linear2)).ToLocalChecked());
    Nan::Set(target, New<String>("visualisation2").ToLocalChecked(), GetFunction(New<FunctionTemplate>(visualisation2)).ToLocalChecked());
    Nan::Set(target, New<String>("visualisation2Carte").ToLocalChecked(), GetFunction(New<FunctionTemplate>(visualisation2Carte)).ToLocalChecked());
    Nan::Set(target, New<String>("anomalieGraphe").ToLocalChecked(), GetFunction(New<FunctionTemplate>(anomalie)).ToLocalChecked());

    // Lecture du fichier, fonction synchrone
    NODE_SET_METHOD(target, "readfile", Readfile);
}


NODE_MODULE(addon, Init)
