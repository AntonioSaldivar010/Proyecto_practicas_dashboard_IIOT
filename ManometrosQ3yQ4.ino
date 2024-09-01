#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Modifique estos valores con los correspondientes a su red.

const char* ssid = "MERCUSYS_2540";
const char* password = "2106634921";
const char* mqtt_server = "192.168.0.103";

WiFiClient espClient;             // Crea un cliente que pueda conectarse a una dirección IP
PubSubClient client(espClient);   // Crea una instancia de cliente parcialmente inicializado

// Se definen las variables que contendran las etiquetas de los pines
#define PotQ3 2
#define PotQ4 14

//Se declaran las variables de tipo Char, así como su tamaño
#define MSG_BUFFER_SIZE	(50)
char msg1[MSG_BUFFER_SIZE];
char msg2[MSG_BUFFER_SIZE];

// Se declaran las variables que se utilizaran en el programa
int L1=0;
int L2=0;
int Y1=0;
int Y2=0;

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Conectandose a ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);                               // Inicializa la conexión Wifi

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("Conectado a Wifi");
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
}

void reconnect() {
  while (!client.connected()) {                             // Entra en un bucle hasta conectarse
    Serial.print("Intentando conexión MQTT...");            // Realiza el intento de conexión
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);                // Crea un identificador de cliente aleatorio
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado");                          // Una vez conectado, publica un mensaje en el monitor serial...
      
    } else {
      Serial.print("Falló, rc=");                           //En caso de fallar, manda el código de error
      Serial.print(client.state());
      Serial.println(" reintentando en 5 segundos");        // Espera 5 segundos para reintentar
      delay(5000);
    }
  }
}

void setup() {
  pinMode(PotQ3, OUTPUT);                         // Declara como salida los pines que accionara 
  pinMode(PotQ4, OUTPUT);                         // los diodos desde una dirección o de otra para el multiplexado
  pinMode(A0, INPUT);                             // Declara como entrada el pin de señal analógica
  Serial.begin(115200);                           // Configura la frecuencia del monitor serial
  setup_wifi();
  client.setServer(mqtt_server, 1883);            // Se declara la configuración del servidor MQTT
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  digitalWrite(2, HIGH);                          // Inicia lectura del potenciometro Q3

  L1=analogRead(A0);                              // Guarda en una variable la lectura de la señal analógica
  Y1=map(L1,0,1024,190310,189690);                // Mapea la entrada analógica del rango [0 1,024] al rango [189,690 190,310]
  snprintf (msg1, MSG_BUFFER_SIZE, "#Q3%ld", Y1); // Añade la etiqueta #Q3 a la entrada analógica para que el programa del Dashboard pueda diferenciar cada señal analógica
  client.publish("Tuberias/Sensores", msg1);      // Publica en el topic "Tuberias/Sensores" los caractéres que engloba msg1

  digitalWrite(2, LOW);                           // Termina lectura del potenciometro Q3

  digitalWrite(14, HIGH);                         // Inicia lectura del potenciometro Q4

  L2=analogRead(A0);                              // Guarda en una variable la lectura de la señal analógica
  Y2=map(L2,0,1024,176710,176090);                // Mapea la entrada analógica del rango [0 1,024] al rango [176,090 176,710]
  snprintf (msg2, MSG_BUFFER_SIZE, "#Q4%ld", Y2); // Añade la etiqueta #Q4 a la entrada analógica para que el programa del Dashboard pueda diferenciar cada señal analógica
  client.publish("Tuberias/Sensores", msg2);      // Publica en el topic "Tuberias/Sensores" los caractéres que engloba msg2

  digitalWrite(14, LOW);                          // Termina lectura del potenciometro Q4
  
  delay(5000);                                    //Tiempo de espera de 5 segundos para volver a iniciar el bucle
  }
