using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using MySql.Data;
using MySql.Data.MySqlClient;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

namespace ARGOS_PLESS
{
    public partial class Form1 : Form
    {
        MqttClient mqttClient;

        public Form1()
        {
            InitializeComponent();
        }

        // Se declaran las variables que se usan en el programa
        int enclaveb1 = 0;
        int enclaveb2 = 0;
        int enclaveb3 = 0;
        int enclaveb4 = 0;
        string Q1cmd="";
        string Q2cmd="";
        string Q3cmd="";
        string Q4cmd="";

        private void Form1_Load(object sender, EventArgs e) // Esta función se encarga de conectarse al servidor MQTT y recibir los mensajes del tópico "Tuberías/Sensores"
        {
            Task.Run(() =>
            {
                mqttClient = new MqttClient("localhost");
                mqttClient.MqttMsgPublishReceived += MqttClient_MqttMsgPublishReceived;
                mqttClient.Subscribe(new string[] { "Tuberias/Sensores" }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE });
                mqttClient.Connect("ARGOS_PLESS");
            });
        }

        private void MqttClient_MqttMsgPublishReceived(object sender, uPLibrary.Networking.M2Mqtt.Messages.MqttMsgPublishEventArgs e)  //Esta función realiza las siguientes instrucciones siempre que se reciba un paquete del tópico al que esta suscrito
        {
            string fullCommand = Encoding.UTF8.GetString(e.Message);  // Se guarda el mensaje recibido en un string
            listBox1.Invoke((MethodInvoker)(() => listBox1.Items.Add(fullCommand)));  // Se imprimen dichos mensajes en una lista
            try
            {
                // Verifica la longitud mínima 
                if (fullCommand[0] != '#' && fullCommand.Length >= 8)
                {
                    return;
                }

                // Separa el comando de identificación del valor real
                int index = 3;  // #Q1 #Q2 #Q3 #Q4 #Q5
                string cmdId = fullCommand.Substring(0, index);
                string cmdValue = fullCommand.Substring(index, fullCommand.Length - index);

                // Se iran filtrando los mensajes según las etiquetas que tengan
                if (cmdId.Equals("#Q1")) 
                {
                    // Una vez pasado el filtro, se procede imprimir estos valores en las barras circulares, asi como en etiquetas para mostrar los valores numéricos
                    int potValue = Convert.ToInt32(cmdValue); // Guarda en una variable de tipo entero la conversión de la parte numérica de la cadena de text
                    Q1cmd = cmdValue;
                    if (potValue >= circularProgressBarAnalog.Minimum && potValue <= circularProgressBarAnalog.Maximum) // Verifica el rango preestablecido de la barra circular
                    {
                        circularProgressBarAnalog.Invoke((MethodInvoker)(() => circularProgressBarAnalog.Value = potValue)); // Imprime dicho valor tanto en la barra de carga
                        circularProgressBarAnalog.Invoke((MethodInvoker)(() => circularProgressBarAnalog.Text = cmdValue));  // como en el cuadro de texto
                    }
                    if (potValue <= 579 && enclaveb2 == 0 && enclaveb1 == 0)                                                            // Si dicho valor cae debajo de un limite predefinido y los enclaves estan desactivados
                    {
                        mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("1"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true); // entonces mandará a encender sus respectivos motores por medio del tópico "Tuberias/Motores"
                        pictureBox1.Image = Properties.Resources.RelayNC;                                                               // modificará la imagen que sirve de referente visual para el operador
                        button1.Invoke((MethodInvoker)(() => button1.Text = "Apagar"));                                                 // así como el texto del botón de encedido/apagado del motor para matener la coherencia de lo que esta sucediendo en el sistema
                        enclaveb1 = 1;
                    }
                    else if(potValue<=579 && enclaveb1==2 && enclaveb2 == 0)                                                            // Si dicho valor debajo debajo del limite y el primer motor esta encendido, se manda a encender el segundo
                    {
                        mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("3"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                        pictureBox2.Image = Properties.Resources.RelayNC;
                        button1.Invoke((MethodInvoker)(() => button2.Text = "Apagar"));
                        enclaveb2 = 1;
                    }
                    else if(potValue>=590 && enclaveb1==1 || enclaveb2 == 1)                                                            // Ahora, si dicho valor supera el limite y los motores estan encendidos, se mandan a apagar
                    {
                        mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("0"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                        pictureBox1.Image = Properties.Resources.RelayNO;
                        button1.Invoke((MethodInvoker)(() => button1.Text = "Encender"));
                        enclaveb1 = 0;
                        mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("2"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                        pictureBox2.Image = Properties.Resources.RelayNO;
                        button2.Invoke((MethodInvoker)(() => button2.Text = "Encender"));
                        enclaveb2 = 0;
                    }
                }
                // Lo mismo se aplica para las demás etiquetas
                if (cmdId.Equals("#Q2"))
                {
                    Q2cmd = cmdValue;
                    circularProgressBar1.Invoke((MethodInvoker)(() => circularProgressBar1.Text=cmdValue));
                    
                }
                // Caso para el valor con punto flotante
                if (cmdId.Equals("#Q5"))
                {
                    int potValue = Convert.ToInt32(cmdValue);
                    if (potValue >= circularProgressBar1.Minimum && potValue <= circularProgressBar1.Maximum)
                    {
                        circularProgressBar1.Invoke((MethodInvoker)(()=>circularProgressBar1.Value = potValue));
                    }
                    if (potValue<=80 && enclaveb3 == 0)
                    {
                        mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("5"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                        pictureBox3.Image = Properties.Resources.RelayNC;
                        button3.Invoke((MethodInvoker)(() => button3.Text = "Apagar"));
                        enclaveb3 = 1;
                    }
                    else if(potValue>=110 && enclaveb3 == 1)
                    {
                        mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("4"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                        pictureBox3.Image = Properties.Resources.RelayNO;
                        button3.Invoke((MethodInvoker)(() => button3.Text = "Encender"));
                        enclaveb3 = 0;
                    }
                }
                if (cmdId.Equals("#Q3"))
                {
                    int potValue = Convert.ToInt32(cmdValue);
                    Q3cmd = cmdValue;
                    if (potValue >= circularProgressBar2.Minimum && potValue <= circularProgressBar2.Maximum)
                    {
                        circularProgressBar2.Invoke((MethodInvoker)(() => circularProgressBar2.Value = potValue));
                        circularProgressBar2.Invoke((MethodInvoker)(() => circularProgressBar2.Text = cmdValue));
                    }
                }
                if (cmdId.Equals("#Q4"))
                {
                    int potValue = Convert.ToInt32(cmdValue);
                    Q4cmd = cmdValue;
                    if (potValue >= circularProgressBar3.Minimum && potValue <= circularProgressBar3.Maximum)
                    {
                        circularProgressBar3.Invoke((MethodInvoker)(() => circularProgressBar3.Value = potValue));
                        circularProgressBar3.Invoke((MethodInvoker)(() => circularProgressBar3.Text = cmdValue));
                    }
                }

                MySqlConnection cnn; // Se crea una conexión al servidor de MySql
                cnn = new MySqlConnection("server=localhost;database=test1;uid=root;pwd=ASMasm1$"); // Se ingresa con los datos de usuario, asi como el nombre de la base de datos
                try
                {
                    cnn.Open();
                    if (Q1cmd != "" && Q2cmd != "" && Q3cmd != "" && Q4cmd != "")    // Se pregunta si todos las variables tienen valores no nulos, esto debido a que la tabla en mysql no admite valores nulos.
                    {
                        MySqlCommand command = new MySqlCommand("insert into prueba5 (Q1,Q2,Q3,Q4) values ('" + Q1cmd + "','" + Q2cmd + "','" + Q3cmd + "','" + Q4cmd + "')", cnn); // Se insertan como una nueva fila los valores de los potenciometros (en este caso, manómetros)
                        command.ExecuteNonQuery();
                        cnn.Close();
                        // Si se cumple esta condición, se inserta la cadena de texto.
                    }
                    else
                    {
                        MessageBox.Show("No se puede guardar");    // De lo contrario, se muestra un mensaje de error
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);     // En caso de llegar a ser algún error imprevisto, se manda el mensaje con el código de error.
                }
            }
            catch (Exception error)
            {
                MessageBox.Show(error.Message);
            }

        }

        private void button4_Click(object sender, EventArgs e) // Esta función se encarga de ocultar y/o mostrar el objeto de cuadro de lista que muestra los mensajes que van llegando 
        {
            if (button4.Text == "Mostrar mensajes")
            {
                listBox1.Visible = true;
                button4.Text = "Ocultar mensajes";
            }
            else if(button4.Text == "Ocultar mensajes")
            {
                listBox1.Visible = false;
                button4.Text = "Mostrar mensajes";
            }
        }

        private void button1_Click(object sender, EventArgs e) // Esta función enciende o apaga los motores y publica el estado de dicho motor por el tópico "Tuberias/Motores"
        {
            if (enclaveb4 == 0 && button1.Text == "Encender")
            {
                mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("1"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                pictureBox1.Image = Properties.Resources.RelayNC;
                button1.Text = "Apagar";
                enclaveb1 = 1;
            }
            else if (button1.Text == "Apagar")
            {
                mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("0"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                pictureBox1.Image = Properties.Resources.RelayNO;
                button1.Text = "Encender";
                enclaveb1 = 0;
            }
            else if (enclaveb4 == 1 && button1.Text == "Encender")
            {
                MessageBox.Show("Actuador B1 fuera de línea");
            }
        }

        // Estas funciones son para los demás botones de encendido y apagado de los motores
        private void button2_Click(object sender, EventArgs e)
        {
            if (button2.Text == "Encender")
            {
                mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("3"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                pictureBox2.Image = Properties.Resources.RelayNC;
                button2.Text = "Apagar";
                enclaveb2 = 1;
            }
            else if (button2.Text == "Apagar")
            {
                mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("2"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                pictureBox2.Image = Properties.Resources.RelayNO;
                button2.Text = "Encender";
                enclaveb2 = 0;
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            if (button3.Text == "Encender")
            {
                mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("5"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                pictureBox3.Image = Properties.Resources.RelayNC;
                button3.Text = "Apagar";
                enclaveb3 = 1;
            }
            else if (button3.Text == "Apagar")
            {
                mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("4"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                pictureBox3.Image = Properties.Resources.RelayNO;
                button3.Text = "Encender";
                enclaveb3 = 0;
            }
        }

        private void button5_Click(object sender, EventArgs e)  // Estos botones se encargan de inutilizar y/o reparar los motores
        {
            if (button5.Text == "Averiar" && enclaveb1 == 0)
            {
                enclaveb4 = 1;
                button5.Text = "Reparar";
            }
            else if (button5.Text == "Reparar" && enclaveb1 == 0 && enclaveb2 == 0)
            {
                enclaveb4 = 0;
                button5.Text = "Averiar";
            }
            else if (button5.Text == "Averiar" && enclaveb1 == 1)
            {
                enclaveb4 = 1;
                mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("0"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                pictureBox1.Image = Properties.Resources.RelayNO;
                enclaveb2 = 1;
                enclaveb1 = 0;
                mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("3"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                pictureBox2.Image = Properties.Resources.RelayNC;
                button5.Text = "Reparar";
                button1.Invoke((MethodInvoker)(() => button1.Text = "Encender"));
                button2.Invoke((MethodInvoker)(() => button2.Text = "Apagar"));
            }
            else if (button5.Text == "Reparar" && enclaveb2 == 1)
            {
                enclaveb4 = 0;
                mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("1"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                pictureBox1.Image = Properties.Resources.RelayNC;
                enclaveb2 = 0;
                enclaveb1 = 1;
                mqttClient.Publish("Tuberias/Motores", Encoding.UTF8.GetBytes("2"), MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, true);
                pictureBox2.Image = Properties.Resources.RelayNO;
                button5.Text = "Averiar";
                button1.Invoke((MethodInvoker)(() => button1.Text = "Apagar"));
                button2.Invoke((MethodInvoker)(() => button2.Text = "Encender"));
            }
        }
    }
}
