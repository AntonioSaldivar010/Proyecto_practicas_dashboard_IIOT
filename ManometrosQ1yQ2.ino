#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Modifique estos valores con los correspondientes a su red.

const char* ssid = "MERCUSYS_2540";
const char* password = "2106634921";
const char* mqtt_server = "192.168.0.103";

WiFiClient espClient;             // Crea un cliente que pueda conectarse a una dirección IP
PubSubClient client(espClient);   // Crea una instancia de cliente parcialmente inicializado

// Se definen las variables que contendran las etiquetas de los pines
#define relayQ1 2
#define relayQ2 14

// Se declaran las variables de tipo Char, así como su tamaño
#define MSG_BUFFER_SIZE	(50)
char msg1[MSG_BUFFER_SIZE];
char msg2[MSG_BUFFER_SIZE];
char msg3[MSG_BUFFER_SIZE];
char mensaje[MSG_BUFFER_SIZE];

// Se declaran las variables que se utilizaran en el programa
int L1=0;
int Y1=0;
float f1,X;

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
  pinMode(relayQ1, OUTPUT);                                 // Declara como salida los pines que accionara    
  pinMode(relayQ2, OUTPUT);                                 // los diodos desde una dirección o de otra para el multiplexado
  pinMode(A0, INPUT);                                       // Declara como entrada el pin de señal analógica
  Serial.begin(115200);                                     // Configura la frecuencia del monitor serial
  setup_wifi();
  client.setServer(mqtt_server, 1883);                      // Se declara la configuración del servidor MQTT
  client.setCallback(callback);
}

// Esta función se encarga de realizar el mapeo para los valores de tipo flotante
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
     float result;
     result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
     return result;
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  digitalWrite(2, HIGH);                          // Inicia lectura del potenciometro Q1

  L1=analogRead(A0);                              // Guarda en una variable la lectura de la señal analógica
  Y1=map(L1,0,1024,595,560);                      // Mapea la entrada analógica del rango [0 1,024] al rango [560 595]
  snprintf (msg1, MSG_BUFFER_SIZE, "#Q1%ld", Y1); // Añade la etiqueta #Q1 a la entrada analógica para que el programa del Dashboard pueda diferenciar cada señal analógica
  client.publish("Tuberias/Sensores", msg1);      // Publicación de la variable con etiqueta #Q1 

  digitalWrite(2, LOW);                           // Termina lectura del potenciometro Q1
  
  digitalWrite(14, HIGH);                         // Inicia lectura del potenciometro Q2

  f1=analogRead(A0);                              // Guarda en una variable la lectura de la señal analógica
  X=mapfloat(f1,37,800,125,65);                   // Mapea la entrada analógica del rango [0 1,024] al rango [65 125]
  dtostrf(X,3,1,msg2);                            

  snprintf (msg3, MSG_BUFFER_SIZE, "#Q5%ld", f1); // Añade la etiqueta #Q5 a la entrada analógica para que el programa del Dashboard pueda diferenciar cada señal analógica

  mensaje[0]='#';                                 // Con estos caractéres, guardamos en 
  mensaje[1]='Q';                                 // un nuevo mensaje la etiqueta #Q2 y la
  mensaje[2]='2';                                 // lectura del potenciometro
  mensaje[3]=msg2[0];
  mensaje[4]=msg2[1];
  mensaje[5]=msg2[2];
  mensaje[6]=msg2[3];
  mensaje[7]=msg2[4];
  mensaje[8]=msg2[5];
  client.publish("Tuberias/Sensores", mensaje);    // Publicación de la variable con etiqueta #Q2 
  client.publish("Tuberias/Sensores", msg3);       // Publicación de la variable con etiqueta #Q5

  digitalWrite(14, LOW);                           // Termina lectura del potenciometro Q2
      
  delay(5000);                                     // Tiempo de espera de 5 segundos
  }
