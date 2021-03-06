/*
 *	This sketch demonstrates how to set up a simple HTTP-like server.
 *	The server will set a GPIO pin depending on the request
 *		http://server_ip/gpio/0 will set the GPIO2 low,
 *		http://server_ip/gpio/1 will set the GPIO2 high
 *	server_ip is the IP address of the ESP8266 module, which will be
 *	printed to Serial when the module is configured.
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

const byte MAXBUF = 64;
struct wifi_config_t {
	char ssid[MAXBUF]; // SSID max length is 32 octets
	char password[MAXBUF]; // passphrase max length is 63 octets or 64 Hex characters
	bool configured;
} wifi_config;

ESP8266WiFiMulti wifiMulti;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void collectCredentials() {
	byte ssid_char_count, password_char_count;
	wifi_config.configured = false;
	while (!wifi_config.configured) {
		Serial.println("Configuring WiFi. Please enter these credentials:");
		Serial.setTimeout(10000);
		Serial.print("SSID: ");
		ssid_char_count = Serial.readBytesUntil(13, wifi_config.ssid, MAXBUF-1);
		wifi_config.ssid[ssid_char_count-1] = 0; // Delete the line feed.
		Serial.println("");
		Serial.print("Password: ");
		password_char_count = Serial.readBytesUntil(13, wifi_config.password, MAXBUF-1);
		wifi_config.password[password_char_count-1] = 0; // Delete the line feed.
		Serial.println("");
		wifi_config.configured = (ssid_char_count > 0) && (password_char_count > 0);
		wifiMulti.addAP(wifi_config.ssid, wifi_config.password);
	}
}

void setup() {
	Serial.begin(115200);
	delay(10);

	// prepare GPIO2 (NodeMCU dev board ~D4)
	pinMode(2, OUTPUT);
	digitalWrite(2, 0);
	pinMode(4, OUTPUT);

	// Connect to WiFi network
	WiFi.mode(WIFI_STA);
	collectCredentials();
	Serial.print("Connecting Wifi: ");
	Serial.print(wifi_config.ssid);
	Serial.println(".");
	while(WiFi.status() != WL_CONNECTED) {
		if(wifiMulti.run() == WL_CONNECTED) {
			Serial.println("WiFi connected");
			Serial.println("IP address: ");
			Serial.println(WiFi.localIP());
			digitalWrite(4, HIGH);
		}
		delay(1000);
		Serial.println(WiFi.status());
	}


	// Start the server
	server.begin();
	Serial.println("Server started");

	// Print the IP address
	Serial.println(WiFi.localIP());
}

void loop() {
	// Check if a client has connected
	WiFiClient client = server.available();
	if (!client) {
		return;
	}

	// Wait until the client sends some data
	Serial.println("new client");
	while (!client.available()) {
		delay(1);
	}

	// Read the first line of the request
	String req = client.readStringUntil('\r');
	Serial.println(req);
	client.flush();

	// Match the request
	int val;
	if (req.indexOf("/gpio/0") != -1)
		val = 0;
	else if (req.indexOf("/gpio/1") != -1)
		val = 1;
	else {
		Serial.println("invalid request");
		client.stop();
		return;
	}

	// Set GPIO2 according to the request
	digitalWrite(2, val);

	client.flush();

	// Prepare the response
	String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
	s += (val) ? "high" : "low";
	s += "</html>\n";

	// Send the response to the client
	client.print(s);
	delay(1);
	Serial.println("Client disonnected");

	// The client will actually be disconnected
	// when the function returns and 'client' object is detroyed
}

