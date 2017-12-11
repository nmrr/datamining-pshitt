/*
MIT License

Copyright 2017 nmrr (https://github.com/nmrr)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "countrycode.h"

using namespace std;

string Alpha2toAlpha3(string ISO3166_1_Alpha_2)
{
    if (ISO3166_1_Alpha_2 == "AF")
    {
        return "AFG";
    }
    else if (ISO3166_1_Alpha_2 == "AL")
    {
        return "ALB";
    }
    else if (ISO3166_1_Alpha_2 == "DZ")
    {
        return "DZA";
    }
    else if (ISO3166_1_Alpha_2 == "AS")
    {
        return "ASM";
    }
    else if (ISO3166_1_Alpha_2 == "AD")
    {
        return "AND";
    }
    else if (ISO3166_1_Alpha_2 == "AO")
    {
        return "AGO";
    }
    else if (ISO3166_1_Alpha_2 == "AI")
    {
        return "AIA";
    }
    else if (ISO3166_1_Alpha_2 == "AQ")
    {
        return "ATA";
    }
    else if (ISO3166_1_Alpha_2 == "AG")
    {
        return "ATG";
    }
    else if (ISO3166_1_Alpha_2 == "AR")
    {
        return "ARG";
    }
    else if (ISO3166_1_Alpha_2 == "AM")
    {
        return "ARM";
    }
    else if (ISO3166_1_Alpha_2 == "AW")
    {
        return "ABW";
    }
    else if (ISO3166_1_Alpha_2 == "AU")
    {
        return "AUS";
    }
    else if (ISO3166_1_Alpha_2 == "AT")
    {
        return "AUT";
    }
    else if (ISO3166_1_Alpha_2 == "AZ")
    {
        return "AZE";
    }
    else if (ISO3166_1_Alpha_2 == "BS")
    {
        return "BHS";
    }
    else if (ISO3166_1_Alpha_2 == "BH")
    {
        return "BHR";
    }
    else if (ISO3166_1_Alpha_2 == "BD")
    {
        return "BGD";
    }
    else if (ISO3166_1_Alpha_2 == "BB")
    {
        return "BRB";
    }
    else if (ISO3166_1_Alpha_2 == "BY")
    {
        return "BLR";
    }
    else if (ISO3166_1_Alpha_2 == "BE")
    {
        return "BEL";
    }
    else if (ISO3166_1_Alpha_2 == "BZ")
    {
        return "BLZ";
    }
    else if (ISO3166_1_Alpha_2 == "BJ")
    {
        return "BEN";
    }
    else if (ISO3166_1_Alpha_2 == "BM")
    {
        return "BMU";
    }
    else if (ISO3166_1_Alpha_2 == "BT")
    {
        return "BTN";
    }
    else if (ISO3166_1_Alpha_2 == "BO")
    {
        return "BOL";
    }
    else if (ISO3166_1_Alpha_2 == "BA")
    {
        return "BIH";
    }
    else if (ISO3166_1_Alpha_2 == "BW")
    {
        return "BWA";
    }
    else if (ISO3166_1_Alpha_2 == "BV")
    {
        return "BVT";
    }
    else if (ISO3166_1_Alpha_2 == "BR")
    {
        return "BRA";
    }
    else if (ISO3166_1_Alpha_2 == "IO")
    {
        return "IOT";
    }
    else if (ISO3166_1_Alpha_2 == "VG")
    {
        return "VGB";
    }
    else if (ISO3166_1_Alpha_2 == "BN")
    {
        return "BRN";
    }
    else if (ISO3166_1_Alpha_2 == "BG")
    {
        return "BGR";
    }
    else if (ISO3166_1_Alpha_2 == "BF")
    {
        return "BFA";
    }
    else if (ISO3166_1_Alpha_2 == "BI")
    {
        return "BDI";
    }
    else if (ISO3166_1_Alpha_2 == "KH")
    {
        return "KHM";
    }
    else if (ISO3166_1_Alpha_2 == "CM")
    {
        return "CMR";
    }
    else if (ISO3166_1_Alpha_2 == "CA")
    {
        return "CAN";
    }
    else if (ISO3166_1_Alpha_2 == "CV")
    {
        return "CPV";
    }
    else if (ISO3166_1_Alpha_2 == "BQ")
    {
        return "BES";
    }
    else if (ISO3166_1_Alpha_2 == "KY")
    {
        return "CYM";
    }
    else if (ISO3166_1_Alpha_2 == "CF")
    {
        return "CAF";
    }
    else if (ISO3166_1_Alpha_2 == "TD")
    {
        return "TCD";
    }
    else if (ISO3166_1_Alpha_2 == "CL")
    {
        return "CHL";
    }
    else if (ISO3166_1_Alpha_2 == "CN")
    {
        return "CHN";
    }
    else if (ISO3166_1_Alpha_2 == "CX")
    {
        return "CXR";
    }
    else if (ISO3166_1_Alpha_2 == "CC")
    {
        return "CCK";
    }
    else if (ISO3166_1_Alpha_2 == "CO")
    {
        return "COL";
    }
    else if (ISO3166_1_Alpha_2 == "KM")
    {
        return "COM";
    }
    else if (ISO3166_1_Alpha_2 == "CG")
    {
        return "COG";
    }
    else if (ISO3166_1_Alpha_2 == "CD")
    {
        return "COD";
    }
    else if (ISO3166_1_Alpha_2 == "CK")
    {
        return "COK";
    }
    else if (ISO3166_1_Alpha_2 == "CR")
    {
        return "CRI";
    }
    else if (ISO3166_1_Alpha_2 == "HR")
    {
        return "HRV";
    }
    else if (ISO3166_1_Alpha_2 == "CU")
    {
        return "CUB";
    }
    else if (ISO3166_1_Alpha_2 == "CW")
    {
        return "CUW";
    }
    else if (ISO3166_1_Alpha_2 == "CY")
    {
        return "CYP";
    }
    else if (ISO3166_1_Alpha_2 == "CZ")
    {
        return "CZE";
    }
    else if (ISO3166_1_Alpha_2 == "CI")
    {
        return "CIV";
    }
    else if (ISO3166_1_Alpha_2 == "DK")
    {
        return "DNK";
    }
    else if (ISO3166_1_Alpha_2 == "DJ")
    {
        return "DJI";
    }
    else if (ISO3166_1_Alpha_2 == "DM")
    {
        return "DMA";
    }
    else if (ISO3166_1_Alpha_2 == "DO")
    {
        return "DOM";
    }
    else if (ISO3166_1_Alpha_2 == "EC")
    {
        return "ECU";
    }
    else if (ISO3166_1_Alpha_2 == "EG")
    {
        return "EGY";
    }
    else if (ISO3166_1_Alpha_2 == "SV")
    {
        return "SLV";
    }
    else if (ISO3166_1_Alpha_2 == "GQ")
    {
        return "GNQ";
    }
    else if (ISO3166_1_Alpha_2 == "ER")
    {
        return "ERI";
    }
    else if (ISO3166_1_Alpha_2 == "EE")
    {
        return "EST";
    }
    else if (ISO3166_1_Alpha_2 == "ET")
    {
        return "ETH";
    }
    else if (ISO3166_1_Alpha_2 == "FK")
    {
        return "FLK";
    }
    else if (ISO3166_1_Alpha_2 == "FO")
    {
        return "FRO";
    }
    else if (ISO3166_1_Alpha_2 == "FJ")
    {
        return "FJI";
    }
    else if (ISO3166_1_Alpha_2 == "FI")
    {
        return "FIN";
    }
    else if (ISO3166_1_Alpha_2 == "FR")
    {
        return "FRA";
    }
    else if (ISO3166_1_Alpha_2 == "GF")
    {
        return "GUF";
    }
    else if (ISO3166_1_Alpha_2 == "PF")
    {
        return "PYF";
    }
    else if (ISO3166_1_Alpha_2 == "TF")
    {
        return "ATF";
    }
    else if (ISO3166_1_Alpha_2 == "GA")
    {
        return "GAB";
    }
    else if (ISO3166_1_Alpha_2 == "GM")
    {
        return "GMB";
    }
    else if (ISO3166_1_Alpha_2 == "GE")
    {
        return "GEO";
    }
    else if (ISO3166_1_Alpha_2 == "DE")
    {
        return "DEU";
    }
    else if (ISO3166_1_Alpha_2 == "GH")
    {
        return "GHA";
    }
    else if (ISO3166_1_Alpha_2 == "GI")
    {
        return "GIB";
    }
    else if (ISO3166_1_Alpha_2 == "GR")
    {
        return "GRC";
    }
    else if (ISO3166_1_Alpha_2 == "GL")
    {
        return "GRL";
    }
    else if (ISO3166_1_Alpha_2 == "GD")
    {
        return "GRD";
    }
    else if (ISO3166_1_Alpha_2 == "GP")
    {
        return "GLP";
    }
    else if (ISO3166_1_Alpha_2 == "GU")
    {
        return "GUM";
    }
    else if (ISO3166_1_Alpha_2 == "GT")
    {
        return "GTM";
    }
    else if (ISO3166_1_Alpha_2 == "GG")
    {
        return "GGY";
    }
    else if (ISO3166_1_Alpha_2 == "GN")
    {
        return "GIN";
    }
    else if (ISO3166_1_Alpha_2 == "GW")
    {
        return "GNB";
    }
    else if (ISO3166_1_Alpha_2 == "GY")
    {
        return "GUY";
    }
    else if (ISO3166_1_Alpha_2 == "HT")
    {
        return "HTI";
    }
    else if (ISO3166_1_Alpha_2 == "HM")
    {
        return "HMD";
    }
    else if (ISO3166_1_Alpha_2 == "HN")
    {
        return "HND";
    }
    else if (ISO3166_1_Alpha_2 == "HK")
    {
        return "HKG";
    }
    else if (ISO3166_1_Alpha_2 == "HU")
    {
        return "HUN";
    }
    else if (ISO3166_1_Alpha_2 == "IS")
    {
        return "ISL";
    }
    else if (ISO3166_1_Alpha_2 == "IN")
    {
        return "IND";
    }
    else if (ISO3166_1_Alpha_2 == "ID")
    {
        return "IDN";
    }
    else if (ISO3166_1_Alpha_2 == "IR")
    {
        return "IRN";
    }
    else if (ISO3166_1_Alpha_2 == "IQ")
    {
        return "IRQ";
    }
    else if (ISO3166_1_Alpha_2 == "IE")
    {
        return "IRL";
    }
    else if (ISO3166_1_Alpha_2 == "IM")
    {
        return "IMN";
    }
    else if (ISO3166_1_Alpha_2 == "IL")
    {
        return "ISR";
    }
    else if (ISO3166_1_Alpha_2 == "IT")
    {
        return "ITA";
    }
    else if (ISO3166_1_Alpha_2 == "JM")
    {
        return "JAM";
    }
    else if (ISO3166_1_Alpha_2 == "JP")
    {
        return "JPN";
    }
    else if (ISO3166_1_Alpha_2 == "JE")
    {
        return "JEY";
    }
    else if (ISO3166_1_Alpha_2 == "JO")
    {
        return "JOR";
    }
    else if (ISO3166_1_Alpha_2 == "KZ")
    {
        return "KAZ";
    }
    else if (ISO3166_1_Alpha_2 == "KE")
    {
        return "KEN";
    }
    else if (ISO3166_1_Alpha_2 == "KI")
    {
        return "KIR";
    }
    else if (ISO3166_1_Alpha_2 == "KW")
    {
        return "KWT";
    }
    else if (ISO3166_1_Alpha_2 == "KG")
    {
        return "KGZ";
    }
    else if (ISO3166_1_Alpha_2 == "LA")
    {
        return "LAO";
    }
    else if (ISO3166_1_Alpha_2 == "LV")
    {
        return "LVA";
    }
    else if (ISO3166_1_Alpha_2 == "LB")
    {
        return "LBN";
    }
    else if (ISO3166_1_Alpha_2 == "LS")
    {
        return "LSO";
    }
    else if (ISO3166_1_Alpha_2 == "LR")
    {
        return "LBR";
    }
    else if (ISO3166_1_Alpha_2 == "LY")
    {
        return "LBY";
    }
    else if (ISO3166_1_Alpha_2 == "LI")
    {
        return "LIE";
    }
    else if (ISO3166_1_Alpha_2 == "LT")
    {
        return "LTU";
    }
    else if (ISO3166_1_Alpha_2 == "LU")
    {
        return "LUX";
    }
    else if (ISO3166_1_Alpha_2 == "MO")
    {
        return "MAC";
    }
    else if (ISO3166_1_Alpha_2 == "MK")
    {
        return "MKD";
    }
    else if (ISO3166_1_Alpha_2 == "MG")
    {
        return "MDG";
    }
    else if (ISO3166_1_Alpha_2 == "MW")
    {
        return "MWI";
    }
    else if (ISO3166_1_Alpha_2 == "MY")
    {
        return "MYS";
    }
    else if (ISO3166_1_Alpha_2 == "MV")
    {
        return "MDV";
    }
    else if (ISO3166_1_Alpha_2 == "ML")
    {
        return "MLI";
    }
    else if (ISO3166_1_Alpha_2 == "MT")
    {
        return "MLT";
    }
    else if (ISO3166_1_Alpha_2 == "MH")
    {
        return "MHL";
    }
    else if (ISO3166_1_Alpha_2 == "MQ")
    {
        return "MTQ";
    }
    else if (ISO3166_1_Alpha_2 == "MR")
    {
        return "MRT";
    }
    else if (ISO3166_1_Alpha_2 == "MU")
    {
        return "MUS";
    }
    else if (ISO3166_1_Alpha_2 == "YT")
    {
        return "MYT";
    }
    else if (ISO3166_1_Alpha_2 == "MX")
    {
        return "MEX";
    }
    else if (ISO3166_1_Alpha_2 == "FM")
    {
        return "FSM";
    }
    else if (ISO3166_1_Alpha_2 == "MD")
    {
        return "MDA";
    }
    else if (ISO3166_1_Alpha_2 == "MC")
    {
        return "MCO";
    }
    else if (ISO3166_1_Alpha_2 == "MN")
    {
        return "MNG";
    }
    else if (ISO3166_1_Alpha_2 == "ME")
    {
        return "MNE";
    }
    else if (ISO3166_1_Alpha_2 == "MS")
    {
        return "MSR";
    }
    else if (ISO3166_1_Alpha_2 == "MA")
    {
        return "MAR";
    }
    else if (ISO3166_1_Alpha_2 == "MZ")
    {
        return "MOZ";
    }
    else if (ISO3166_1_Alpha_2 == "MM")
    {
        return "MMR";
    }
    else if (ISO3166_1_Alpha_2 == "NA")
    {
        return "NAM";
    }
    else if (ISO3166_1_Alpha_2 == "NR")
    {
        return "NRU";
    }
    else if (ISO3166_1_Alpha_2 == "NP")
    {
        return "NPL";
    }
    else if (ISO3166_1_Alpha_2 == "NL")
    {
        return "NLD";
    }
    else if (ISO3166_1_Alpha_2 == "NC")
    {
        return "NCL";
    }
    else if (ISO3166_1_Alpha_2 == "NZ")
    {
        return "NZL";
    }
    else if (ISO3166_1_Alpha_2 == "NI")
    {
        return "NIC";
    }
    else if (ISO3166_1_Alpha_2 == "NE")
    {
        return "NER";
    }
    else if (ISO3166_1_Alpha_2 == "NG")
    {
        return "NGA";
    }
    else if (ISO3166_1_Alpha_2 == "NU")
    {
        return "NIU";
    }
    else if (ISO3166_1_Alpha_2 == "NF")
    {
        return "NFK";
    }
    else if (ISO3166_1_Alpha_2 == "KP")
    {
        return "PRK";
    }
    else if (ISO3166_1_Alpha_2 == "MP")
    {
        return "MNP";
    }
    else if (ISO3166_1_Alpha_2 == "NO")
    {
        return "NOR";
    }
    else if (ISO3166_1_Alpha_2 == "OM")
    {
        return "OMN";
    }
    else if (ISO3166_1_Alpha_2 == "PK")
    {
        return "PAK";
    }
    else if (ISO3166_1_Alpha_2 == "PW")
    {
        return "PLW";
    }
    else if (ISO3166_1_Alpha_2 == "PS")
    {
        return "PSE";
    }
    else if (ISO3166_1_Alpha_2 == "PA")
    {
        return "PAN";
    }
    else if (ISO3166_1_Alpha_2 == "PG")
    {
        return "PNG";
    }
    else if (ISO3166_1_Alpha_2 == "PY")
    {
        return "PRY";
    }
    else if (ISO3166_1_Alpha_2 == "PE")
    {
        return "PER";
    }
    else if (ISO3166_1_Alpha_2 == "PH")
    {
        return "PHL";
    }
    else if (ISO3166_1_Alpha_2 == "PN")
    {
        return "PCN";
    }
    else if (ISO3166_1_Alpha_2 == "PL")
    {
        return "POL";
    }
    else if (ISO3166_1_Alpha_2 == "PT")
    {
        return "PRT";
    }
    else if (ISO3166_1_Alpha_2 == "PR")
    {
        return "PRI";
    }
    else if (ISO3166_1_Alpha_2 == "QA")
    {
        return "QAT";
    }
    else if (ISO3166_1_Alpha_2 == "RO")
    {
        return "ROU";
    }
    else if (ISO3166_1_Alpha_2 == "RU")
    {
        return "RUS";
    }
    else if (ISO3166_1_Alpha_2 == "RW")
    {
        return "RWA";
    }
    else if (ISO3166_1_Alpha_2 == "RE")
    {
        return "REU";
    }
    else if (ISO3166_1_Alpha_2 == "WS")
    {
        return "WSM";
    }
    else if (ISO3166_1_Alpha_2 == "SM")
    {
        return "SMR";
    }
    else if (ISO3166_1_Alpha_2 == "SA")
    {
        return "SAU";
    }
    else if (ISO3166_1_Alpha_2 == "SN")
    {
        return "SEN";
    }
    else if (ISO3166_1_Alpha_2 == "RS")
    {
        return "SRB";
    }
    else if (ISO3166_1_Alpha_2 == "SC")
    {
        return "SYC";
    }
    else if (ISO3166_1_Alpha_2 == "SL")
    {
        return "SLE";
    }
    else if (ISO3166_1_Alpha_2 == "SG")
    {
        return "SGP";
    }
    else if (ISO3166_1_Alpha_2 == "SX")
    {
        return "SXM";
    }
    else if (ISO3166_1_Alpha_2 == "SK")
    {
        return "SVK";
    }
    else if (ISO3166_1_Alpha_2 == "SI")
    {
        return "SVN";
    }
    else if (ISO3166_1_Alpha_2 == "SB")
    {
        return "SLB";
    }
    else if (ISO3166_1_Alpha_2 == "SO")
    {
        return "SOM";
    }
    else if (ISO3166_1_Alpha_2 == "ZA")
    {
        return "ZAF";
    }
    else if (ISO3166_1_Alpha_2 == "GS")
    {
        return "SGS";
    }
    else if (ISO3166_1_Alpha_2 == "KR")
    {
        return "KOR";
    }
    else if (ISO3166_1_Alpha_2 == "SS")
    {
        return "SSD";
    }
    else if (ISO3166_1_Alpha_2 == "ES")
    {
        return "ESP";
    }
    else if (ISO3166_1_Alpha_2 == "LK")
    {
        return "LKA";
    }
    else if (ISO3166_1_Alpha_2 == "BL")
    {
        return "BLM";
    }
    else if (ISO3166_1_Alpha_2 == "SH")
    {
        return "SHN";
    }
    else if (ISO3166_1_Alpha_2 == "KN")
    {
        return "KNA";
    }
    else if (ISO3166_1_Alpha_2 == "LC")
    {
        return "LCA";
    }
    else if (ISO3166_1_Alpha_2 == "MF")
    {
        return "MAF";
    }
    else if (ISO3166_1_Alpha_2 == "PM")
    {
        return "SPM";
    }
    else if (ISO3166_1_Alpha_2 == "VC")
    {
        return "VCT";
    }
    else if (ISO3166_1_Alpha_2 == "SD")
    {
        return "SDN";
    }
    else if (ISO3166_1_Alpha_2 == "SR")
    {
        return "SUR";
    }
    else if (ISO3166_1_Alpha_2 == "SJ")
    {
        return "SJM";
    }
    else if (ISO3166_1_Alpha_2 == "SZ")
    {
        return "SWZ";
    }
    else if (ISO3166_1_Alpha_2 == "SE")
    {
        return "SWE";
    }
    else if (ISO3166_1_Alpha_2 == "CH")
    {
        return "CHE";
    }
    else if (ISO3166_1_Alpha_2 == "SY")
    {
        return "SYR";
    }
    else if (ISO3166_1_Alpha_2 == "ST")
    {
        return "STP";
    }
    else if (ISO3166_1_Alpha_2 == "TW")
    {
        return "TWN";
    }
    else if (ISO3166_1_Alpha_2 == "TJ")
    {
        return "TJK";
    }
    else if (ISO3166_1_Alpha_2 == "TZ")
    {
        return "TZA";
    }
    else if (ISO3166_1_Alpha_2 == "TH")
    {
        return "THA";
    }
    else if (ISO3166_1_Alpha_2 == "TL")
    {
        return "TLS";
    }
    else if (ISO3166_1_Alpha_2 == "TG")
    {
        return "TGO";
    }
    else if (ISO3166_1_Alpha_2 == "TK")
    {
        return "TKL";
    }
    else if (ISO3166_1_Alpha_2 == "TO")
    {
        return "TON";
    }
    else if (ISO3166_1_Alpha_2 == "TT")
    {
        return "TTO";
    }
    else if (ISO3166_1_Alpha_2 == "TN")
    {
        return "TUN";
    }
    else if (ISO3166_1_Alpha_2 == "TR")
    {
        return "TUR";
    }
    else if (ISO3166_1_Alpha_2 == "TM")
    {
        return "TKM";
    }
    else if (ISO3166_1_Alpha_2 == "TC")
    {
        return "TCA";
    }
    else if (ISO3166_1_Alpha_2 == "TV")
    {
        return "TUV";
    }
    else if (ISO3166_1_Alpha_2 == "UM")
    {
        return "UMI";
    }
    else if (ISO3166_1_Alpha_2 == "VI")
    {
        return "VIR";
    }
    else if (ISO3166_1_Alpha_2 == "GB")
    {
        return "GBR";
    }
    else if (ISO3166_1_Alpha_2 == "US")
    {
        return "USA";
    }
    else if (ISO3166_1_Alpha_2 == "UG")
    {
        return "UGA";
    }
    else if (ISO3166_1_Alpha_2 == "UA")
    {
        return "UKR";
    }
    else if (ISO3166_1_Alpha_2 == "AE")
    {
        return "ARE";
    }
    else if (ISO3166_1_Alpha_2 == "UY")
    {
        return "URY";
    }
    else if (ISO3166_1_Alpha_2 == "UZ")
    {
        return "UZB";
    }
    else if (ISO3166_1_Alpha_2 == "VU")
    {
        return "VUT";
    }
    else if (ISO3166_1_Alpha_2 == "VA")
    {
        return "VAT";
    }
    else if (ISO3166_1_Alpha_2 == "VE")
    {
        return "VEN";
    }
    else if (ISO3166_1_Alpha_2 == "VN")
    {
        return "VNM";
    }
    else if (ISO3166_1_Alpha_2 == "WF")
    {
        return "WLF";
    }
    else if (ISO3166_1_Alpha_2 == "EH")
    {
        return "ESH";
    }
    else if (ISO3166_1_Alpha_2 == "YE")
    {
        return "YEM";
    }
    else if (ISO3166_1_Alpha_2 == "ZM")
    {
        return "ZMB";
    }
    else if (ISO3166_1_Alpha_2 == "ZW")
    {
        return "ZWE";
    }
    else if (ISO3166_1_Alpha_2 == "AX")
    {
        return "ALA";
    }
    else return "???";
}
