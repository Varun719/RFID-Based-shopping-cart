#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>

LiquidCrystal_PCF8574 lcd(0x27);
const char* ssid = "Manmandir Room1104";
const char* password = "Room@1104";

WebServer server(80);
String page;
char input[12];
double total = 0;
int count_prod = 0;

const int buzzer = 19;
const int button = 5;

struct Item {
    String name;
    double price;
};

Item cart[10];
int cartSize = 0;

void setup() {
    Serial.begin(9600);
    pinMode(button, INPUT_PULLUP);
    pinMode(buzzer, OUTPUT);
    
    Wire.begin(21, 22);
    lcd.begin(16, 2);
    lcd.setBacklight(255);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    lcd.clear();
    if (WiFi.status() == WL_CONNECTED) {
        lcd.setCursor(0, 0);
        lcd.print("WiFi Connected");
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        Serial.println(WiFi.localIP());
    } else {
        lcd.print("WiFi Failed!");
    }
    delay(2000);
    lcd.clear();
    lcd.print("Scan Items");

    server.on("/", HTTP_GET, handleRoot);
    server.begin();
}

void loop() {
    int buttonState = digitalRead(button);
    if (Serial.available()) {
        int count = 0;
        while (Serial.available() && count < 12) {
            input[count] = Serial.read();
            count++;
            delay(5);
        }
        
        if (count == 12) {
            if ((strncmp(input, "3E00EA7CAD05", 12) == 0) && (buttonState == 1)) {
                addItem("Sugar", 35.00);
            } else if ((strncmp(input, "3E00EA7CAD05", 12) == 0) && (buttonState == 0)) {
                removeItem("Sugar", 35.00);
            } else if ((strncmp(input, "3E00EA7CC961", 12) == 0) && (buttonState == 1)) {
                addItem("Milk", 24.00);
            } else if ((strncmp(input, "3E00EA7CC961", 12) == 0) && (buttonState == 0)) {
                removeItem("Milk", 24.00);
            } else if ((strncmp(input, "3E00EA7CAE06", 12) == 0) && (buttonState == 1)) {
                addItem("Biscuits", 10.00);
            } else if ((strncmp(input, "3E00EA7CAE06", 12) == 0) && (buttonState == 0)) {
                removeItem("Biscuits", 10.00);
            }
        }
    }
    server.handleClient();
}

void addItem(String itemName, double price) {
    if (cartSize < 10) {
        cart[cartSize] = {itemName, price};
        cartSize++;
        total += price;
        count_prod++;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(itemName);
        lcd.print(" Added");
        lcd.setCursor(0, 1);
        lcd.print("Total: Rs.");
        lcd.print(total);
        digitalWrite(buzzer, HIGH);
        delay(1000);
        digitalWrite(buzzer, LOW);
    }
}

void removeItem(String itemName, double price) {
    for (int i = 0; i < cartSize; i++) {
        if (cart[i].name == itemName) {
            total -= cart[i].price;
            count_prod--;
            for (int j = i; j < cartSize - 1; j++) {
                cart[j] = cart[j + 1];
            }
            cartSize--;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(itemName);
            lcd.print(" Removed");
            lcd.setCursor(0, 1);
            lcd.print("Total: Rs.");
            lcd.print(total);
            digitalWrite(buzzer, HIGH);
            delay(1000);
            digitalWrite(buzzer, LOW);
            break;
        }
    }
}

void handleRoot() {
    String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='3'><title>Smart Cart</title></head><body>";
    html += "<h1>ESP32 Smart Trolley</h1><table border='1'><tr><th>Item</th><th>Price (Rs)</th></tr>";
    for (int i = 0; i < cartSize; i++) {
        html += "<tr><td>" + cart[i].name + "</td><td>" + String(cart[i].price) + "</td></tr>";
    }
    html += "</table><h2>Total Price: Rs." + String(total) + "</h2></body></html>";
    server.send(200, "text/html", html);
}
