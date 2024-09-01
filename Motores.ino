#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Modifique estos valores con los correspondientes a su red.

const char* ssid = "MERCUSYS_2540";
const char* password = "2106634921";
const char* mqtt_server = "192.168.0.103";

WiFiClient espClient;             // Crea un cliente que pueda conectarse a una dirección IP
PubSubClient client(espClient);   // Crea una instancia de cliente parcialmente inicializado

// Se definen las variables que contendran las etiquetas de los pines
#define PinMotor1 5
#define PinMotor2 12
#define PinMotor3 13

// Se declaran las variables que se utilizaran en el programa
int Motor1;
int Motor2;
int Motor3;
int enclaveM1=0;
int enclaveM2=0;
int enclaveM3=0;

void setup() {
  pinMode(PinMotor1, OUTPUT);                         // Declara como salida los pines que accionara     
  pinMode(PinMotor2, OUTPUT);                         // En este caso, los relays que accionarán a los motores
  pinMode(PinMotor3, OUTPUT);
  Serial.begin(115200);                               // Configura la frecuencia del monitor serial
  setup_wifi();
  client.setServer(mqtt_server, 1883);                // Se declara la configuración del servidor MQTT
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Conectandose a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);                               // Inicializa la conexión Wifi

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a WiFi");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Llego mensaje [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '0') {            // Si el mensaje que llega es 0,
    Motor1=0;                               // El circuito normalmente abierto del relay 1 se abre
  }
  if ((char)payload[0] == '1') {            // Si el mensaje que llega es 1,
    Motor1=1;                               // El circuito normalmente abierto del relay 1 se cierra
  }
  if ((char)payload[0] == '2') {            // Si el mensaje que llega es 2,
    Motor2=0;                               // El circuito normalmente abierto del relay 2 se abre
  }
  if ((char)payload[0] == '3') {            // Si el mensaje que llega es 3,
    Motor2=1;                               // El circuito normalmente abierto del relay 2 se cierra
  }
  if ((char)payload[0] == '4') {            // Si el mensaje que llega es 4,
    Motor3=0;                               // El circuito normalmente abierto del relay 3 se abre
  }
  if ((char)payload[0] == '5') {            // Si el mensaje que llega es 5,
    Motor3=1;                               // El circuito normalmente abierto del relay 3 se cierra
  }
  if(Motor1 == 0 && enclaveM1 == 1){        // Se pregunta si ya esta cerrado el relay 1
      digitalWrite(PinMotor1, LOW);         // Si es asi, lo manda a abrir
      Serial.println("Motor 1 apagado");
      enclaveM1=0;                          // Y se quita el enclavamiento
    }
    if(Motor1 == 1 && enclaveM1 == 0){      // Se pregunta si ya esta abierto el relay 1
      digitalWrite(PinMotor1, HIGH);        // Si es asi, lo manda a cerrar
      Serial.println("Motor 1 encendido");
      enclaveM1=1;                          // Y se pone el enclavamiento
    }
    if(Motor2 == 0 && enclaveM2 == 1){      // Se pregunta si ya esta cerrado el relay 2
      digitalWrite(PinMotor2, LOW);         // Si es asi, lo manda a abrir
      Serial.println("Motor 2 apagado");
      enclaveM2=0;                          // Y se quita el enclavamiento
    }
    if(Motor2 == 1 && enclaveM2 == 0){      // Se pregunta si ya esta abierto el relay 2
      digitalWrite(PinMotor2, HIGH);        // Si es asi, lo manda a cerrar
      Serial.println("Motor 2 encendido");
      enclaveM2=1;                          // Y se pone el enclavamiento
    }
    if(Motor3 == 0 && enclaveM3 == 1){      // Se pregunta si ya esta cerrado el relay 3
      digitalWrite(PinMotor3, LOW);         // Si es asi, lo manda a abrir
      Serial.println("Motor 3 apagado");
      enclaveM3=0;                          // Y se quita el enclavamiento
    }
    if(Motor3 == 1 && enclaveM3 == 0){      // Se pregunta si ya esta abierto el relay 3
      digitalWrite(PinMotor3, HIGH);        // Si es asi, lo manda a cerrar
      Serial.println("Motor 3 encendido");
      enclaveM3=1;                          // Y se pone el enclavamiento
    }
  }

void reconnect() {
  while (!client.connected()) {                                 // Entra en un bucle hasta conectarse
    Serial.print("Intentando conexión MQTT...");                // Realiza el intento de conexión
    if (client.connect("ESP8266Client")) {
      Serial.println("Conectado");                              // Una vez conectado, publica un mensaje en el monitor serial...
      client.subscribe("Tuberias/Motores");                     // ... y se suscribe al Topic
    } else {
      Serial.print("Falló, rc=");                               //En caso de fallar, manda el código de error
      Serial.print(client.state());
      Serial.println(" reintentando en 5 segundos");            // Espera 5 segundos para reintentar
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
    
}
