#include <Arduino.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include "esp_wifi.h"
#include "ESPAsyncWebServer.h"

// paramètres du réseau
const char* ssid     = "ESP-32";    // SSID
const char* password = NULL;        // MDP

// webserver port 80
AsyncWebServer server(80);

// nombre d'appareils connectés
uint8_t nb_devices_connected = 0;

// stockage du dernier appareil connecté
tcpip_adapter_sta_info_t station;

// stockage de tous les appareils connectés
String connected_devices = "";



// remplacement placeholder
String processor(const String& var)
{
    // vérification si bouton appuyé est MAC
    if(var == "MAC") {
        
        // récupération des appareils connectés
        get_all_connected_devices();
        return connected_devices;
    }
    return String();
}


// récupération de tous les appareils connectés
void get_all_connected_devices()
{
    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

    // réinitialisation des appareils connectés
    connected_devices = "";

    // on regarde toutes les stations trouvées
    for (uint8_t i = 0; i < adapter_sta_list.num; i++)
    {
        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
        connected_devices += ((String)"Device " + (i+1) + " | MAC : ");
        
        // Ajouter chaque octet de l'adresse MAC à la chaîne
        for (int i = 0; i < 6; ++i) {
            String hexValue = String(station.mac[i], HEX);
            
            // Ajouter un zéro à gauche si la longueur de la chaîne est inférieure à deux caractères
            if (hexValue.length() == 1) {
                hexValue = "0" + hexValue;
            }

            // ajout de la partie de l'adresse MAC au string
            connected_devices += hexValue;
            if (i < 5) {
                // ajout de séparateur sauf pour dernière partie adresse
                connected_devices += ":";
            }
        }
        
        // retour ligne
        connected_devices += "</br>";
    }
}


// affichage des derniers appareils connectés
void display_connected_devices()
{
    // récupération de la liste des appareils
    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

    // si on a récupéré un appareil [+]
    if (adapter_sta_list.num > nb_devices_connected) {

        // on l'indexe
        nb_devices_connected++;

        uint8_t last_device = adapter_sta_list.num;

        // on l'affiche
        station = adapter_sta_list.sta[last_device -1];
        Serial.print((String)"[+] Device " + nb_devices_connected + " | MAC : ");
        Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X", station.mac[0], station.mac[1], station.mac[2], station.mac[3], station.mac[4], station.mac[5]);
        Serial.println();
    }

    // si on a perdu un appareil [-]
    else if (adapter_sta_list.num < nb_devices_connected) {

        // on l'indexe
        nb_devices_connected--;

        // affichage message
        Serial.println((String)"[-] Device disconnected | remaining : " + nb_devices_connected);
    }
}


void setup()
{
    // port série
    Serial.begin(115200);

    // test fonctionnement SPIFFS
    if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // création point accès
    Serial.println("\n[*] Creating AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    // webserver adresse
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());


    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    
    // Route to load style.css file
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
    });

    // Route to set GPIO to HIGH
    server.on("/mac", HTTP_GET, [](AsyncWebServerRequest *request){ 
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    // Start server
    server.begin();
}

void loop()
{
    display_connected_devices();
    delay(100);
}