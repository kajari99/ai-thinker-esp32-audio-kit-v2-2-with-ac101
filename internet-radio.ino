/*AI Thinker ESP32 Audio Kit v2.2 rev 2748 PCB(with X-Powers Technology© AC101 audio codec)
 * 
 * ORIGINAL PROJECT(with ES8388) FROM: https://github.com/thieu-b55/ESP32-audiokit-webradio-webinterface
 * 
 * Arduino Board Settings: ESP32 WROVER Module
 * Partition Scheme: Huge APP(3MB No OTA/1MB SPISS)
 * 
 * Audio librarie
 * https://github.com/schreibfaul1/ESP32-audioI2S
 * AC101 librarie
 * https://github.com/Yveaux/AC101
 * The AC101-master library gave me an error about a missing paragraph. You can fix this when you go to the AC101-master library. 
 * In this library you must open the file library.properties and add a line       paragraph=geen idee
 * 
 * FIXED IP ADDRESS IS 192.168.1.4
 * 
 * HTML Change UTF-8 Characters for Hungarian accented characters(e.g. á,é,í,ó,ö,ő,ú,ü,ű character)
 * channel switch with 3-4 Key
 * volume setting with 5 KEY-MINUS and 6 KEY-PLUS
 * 
 * translate from Dutch to English
 * 
 * sender_data.csv //read from SD CARD
 * geen header
 * column 1  >>  sendername
 * column 2  >>  sender url
 * Switch S1 SETTING: 1-OFF, 2-ON, 3-ON, 4-OFF, 5-OFF
*/

#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include <SPI.h>
#include <Preferences.h>
#include "FS.h"
#include "SD.h"
#include <CSV_Parser.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Wire.h"
#include "AC101.h"

static AC101 dac;     // AC101                                
int volume = 20;

const uint8_t volume_step = 2;
unsigned long debounce = 0;

Audio audio;
Preferences pref;
AsyncWebServer server(80);

// SPI GPIOs
#define SD_CS         13
#define SPI_MOSI      15
#define SPI_MISO       2
#define SPI_SCK       14

// I2S GPIOs, the names refer on AC101
#define I2S_DSIN      25
#define I2S_BCLK      27
#define I2S_LRC       26
#define I2S_MCLK       0
#define I2S_DOUT      35

// I2C GPIOs
#define IIC_CLK       32
#define IIC_DATA      33

// Buttons
#define BUTTON_1_PIN 36            // KEY 1
#define BUTTON_2_PIN 13            // KEY 2/shared mit SPI_CS
#define PIN_CH_DOWN 19             // KEY 3
#define PIN_CH_UP 23               // KEY 4
#define PIN_VOL_DOWN 18            // KEY 5
#define PIN_VOL_UP 5               // KEY 6

#define PA_EN               21   // amplifier enable
#define MAX_NUMBER_CHANNEL  75

int chosen = 1;
int choice = 1;
int following;
int totalmp3;
int first;
int second;
int songindex;
int row;
int volume_choice;
int volume_chosen;
int low_choice;
int low_chosen;
int middle_choice;
int middle_chosen;
int high_chosen;
int high_choice;
int mp3_per_songlist;
int array_index = MAX_NUMBER_CHANNEL - 1;
int songlist_index_vorig;
int songlist_index;
int mp3_folder_teller;
int teller = 0;
int mp3_number;
int gn_choice = 0;
int ip_1_int = 192;
int ip_2_int = 168;
int ip_3_int = 1;
int ip_4_int = 4;
unsigned long watch_op_network;
unsigned long readin_begin;
unsigned long readin_nu;
unsigned long wachttijd = millis();
bool choose = false;
bool list_maken = false;
bool play_mp3 = false;
bool webradio = false;
bool writing_csv = false;
bool network;
bool nog_mp3;
bool mp3_ok;
bool mp3_list_maken = false;
bool ssid_ingevuld = false;
bool pswd_ingevuld = false;
bool songlisten = false;
bool songlist_bestaat_bool;
char ip_char[20];
char songfile[200];
char mp3file[200];
char song[200];
char datastring[200];
char password[40];
char ssid[40];
char charSenderFile[12];
char player[20];
char gn_actie[20];
char gn_selectie[20];
char sendername[40];
char charUrlFile[12];
char url[100];
char mp3_dir[10];
char folder_mp3[10];
char number_mp3[10];
char songlist_dir[12];
char total_mp3[15];
char mp3_list_folder[10];
char mp3_list_number[5];
char leeg[0];
char senderarray[MAX_NUMBER_CHANNEL][40];
char urlarray[MAX_NUMBER_CHANNEL][100];
const char* IP_1_CHOICE = "ip_1_choice";
const char* IP_2_CHOICE = "ip_2_choice";
const char* IP_3_CHOICE = "ip_3_choice";
const char* IP_4_CHOICE = "ip_4_choice";
const char* CHOICEMIN_INPUT = "minchoice";
const char* CHOICEPLUS_INPUT = "pluschoice";
const char* CONFIRMCHOICE_INPUT ="confirmchoice";
const char* LAAG = "low_choice";
const char* MIDDEN = "middle_choice";
const char* HOOG = "high_choice";
const char* VOLUME = "volume_choice";
const char* VOLUME_CONFIRM = "confirm_volume";
const char* APssid = "ESP32webradio";
const char* APpswd = "ESP32pswd";
const char* STA_SSID = "ssid";
const char* STA_PSWD = "pswd";
const char* SENDER = "sender";
const char* URL = "url";
const char* ARRAY_MIN = "array_index_min";
const char* ARRAY_PLUS = "array_index_plus";
const char* CONFIRM_SENDER = "confirm_sender";
const char* MIN_INPUT = "min";
const char* PLUS_INPUT = "plus";
const char* CONFIRM_MP3 ="confirm_mp3";
const char* h_char = "h";
String songstring =      "                                                                                                                                                                                                        ";
String inputString =     "                                                                                                                                                                                                        ";
String mp3titel =        "                                                                                                                                                                                                        ";      
String senderFile =      "           ";
String urlFile =         "           ";
String maxurl =          "           ";
String total =          "           ";
String streamsong =      "                                                                                                                                                                                                        ";
String mp3_folder =      "                   ";
String songlist_folder = "                   ";
String mp3test = "mp3";
String ip_1_string = "   ";
String ip_2_string = "   ";
String ip_3_string = "   ";
String ip_4_string = "   ";
String ip_string = "                   ";

void readFile(fs::FS &fs, const char * path){
    File file = fs.open(path);
    if(!file){
      Serial.println("Can't open file to read : ");
      Serial.println(path);
      return;
    }
    teller = 0;
    inputString = "";
    while(file.available()){
      inputString += char(file.read());
      teller++;
    }
    file.close();
}

void readIP(fs::FS &fs, const char * path){
    int temp;
    int temp1;
    File file = fs.open(path);
    if(!file){
      return;
    }
    teller = 0;
    inputString = "";
    while(file.available()){
      inputString += char(file.read());
      teller++;
    }
    temp = inputString.indexOf('.');
    ip_1_int = (inputString.substring(0, temp - 1)).toInt();
    temp1 = inputString.indexOf('.', temp + 1);
    ip_2_int = (inputString.substring(temp + 1 , temp1 - 1)).toInt();
    temp = inputString.indexOf('.', temp1 + 1);
    ip_3_int = (inputString.substring(temp1 + 1, temp - 1)).toInt();
    ip_4_int = (inputString.substring(temp + 1, inputString.length() - 1)).toInt();
    file.close();
}

void deleteFile(fs::FS &fs, const char * path){
  fs.remove(path);
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_APPEND);
  file.print(message);
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Cannot open file to write : ");
      Serial.println(path);
      return;
  }
  file.print(message);
  file.close();
}

void testDir(fs::FS &fs, const char * path){
  File root = fs.open(path);
  if(root){
    songlist_bestaat_bool = true;
  }
}

void createDir(fs::FS &fs, const char * path){
  File root = fs.open(path);
  if(root){
    songlisten = true;
    list_maken = false;
    while(1){
      yield();
    }
  }
  else{
    fs.mkdir(path);
  }
}

void audio_showstreamtitle(const char *info){
  if(choose == false){
    streamsong = info;
  }
}

void audio_eof_mp3(const char *info){
  mp3_following();
  streamsong = mp3titel.substring(0, (mp3titel.length() - 4));
}

void files_in_mp3_0(fs::FS &fs, const char * dirname, uint8_t levels){
  File root = fs.open(dirname);
  if(!root){
    Serial.println("No mp3_0 folder");
    return;
  }
  File file = root.openNextFile();
  mp3_per_songlist = 0;
  while(file){
    file = root.openNextFile();
    mp3_per_songlist ++;
  }
  pref.putShort("files", mp3_per_songlist);
  Serial.println("files in mp3_0");
  Serial.println(mp3_per_songlist);
}

void make_list(fs::FS &fs, const char * dirname){
  File root = fs.open(dirname);
  if(!root){
    nog_mp3 = false;
    return;
  }
  File file = root.openNextFile();
  while(file){
    songlist_index =  mp3_number / mp3_per_songlist;
    if(songlist_index != songlist_index_vorig){
      songlist_index_vorig = songlist_index;
      songlist_folder = "/songlist" + String(songlist_index);
      songlist_folder.toCharArray(songlist_dir, (songlist_folder.length() + 1));
      createDir(SD, songlist_dir);
    }
    songstring = file.name();
    songlist_folder = "/songlist" + String(songlist_index) + "/s" + String(mp3_number);
    songlist_folder.toCharArray(songlist_dir, (songlist_folder.length() + 1));
    songstring = file.name();
    songstring.toCharArray(song, (songstring.length() + 1));
    writeFile(SD, songlist_dir, song);
    file = root.openNextFile();
    mp3_number ++;
  }
}

void mp3_list_maken_chosen(){
  readin_begin = millis();
  files_in_mp3_0(SD, "/mp3_0", 1);
  mp3_number = 0;
  nog_mp3 = true;
  mp3_folder_teller = 0;
  songlist_index_vorig = -1;
  while(nog_mp3){
    mp3_folder = "/mp3_" + String(mp3_folder_teller);
    readin_nu = millis() - readin_begin;
    Serial.println(mp3_number);
    Serial.println(readin_nu);
    mp3_folder.toCharArray(mp3_dir, (mp3_folder.length() + 1));
    make_list(SD, mp3_dir);
    mp3_folder_teller ++;
  }
  String(mp3_number).toCharArray(total_mp3, (String(mp3_number - 1).length() +1));
  writeFile(SD, "/total", total_mp3);
  int expiry_time = (millis() - readin_begin) / 1000;
  Serial.println("expiry time");
  Serial.println(expiry_time);
  list_maken = false;
  choice = -1;
  chosen = -1;
  mp3_chosen();
}

void mp3_chosen(){
  readFile(SD, "/total");
  total = inputString.substring(0, teller);
  totalmp3 = total.toInt();
  mp3_following();
  streamsong = mp3titel.substring(0, (mp3titel.length() - 4));
}

void mp3_following(){
  mp3_ok = false;
  while(mp3_ok == false){
    following = random(totalmp3);
    songindex = following / mp3_per_songlist;
    songstring = "/songlist" + String(songindex) + "/s" + String(following);
    songstring.toCharArray(songfile, (songstring.length() +1));
    readFile(SD, songfile);
    inputString.toCharArray(mp3file, inputString.length() + 1);
    mp3_ok = audio.connecttoFS(SD, mp3file);
  }
  songstring = String(mp3file);
  first = songstring.indexOf("/");
  second = songstring.indexOf("/", first + 1);
  mp3titel = songstring.substring(second + 1);
}

void radio_chosen(){
  memset(url, 0, sizeof(url));
  strcpy(url, urlarray[choice]);
  memset(sendername, 0, sizeof(sendername));
  strcpy(sendername, senderarray[choice]);
  audio.stopSong();
  delay(100);
  audio.connecttohost(url);
  choose = false;
}

void writing_naar_csv(){
  char terminator = char(0x0a);
  String datastring = "                                                                                                                                                             ";
  char datastr[150];
  deleteFile(SD, "/sender_data.csv");
  for(int x = 0; x < MAX_NUMBER_CHANNEL; x++){
    datastring = String(senderarray[x]) + "," + String(urlarray[x]) + String(terminator);
    datastring.toCharArray(datastr, (datastring.length() + 1));
    appendFile(SD, "/sender_data.csv", datastr);
  }
  lees_CSV();
}

/*
 * Read in CSV file to Sender and URL Arry
 */
void lees_CSV(){
  CSV_Parser cp("ss", false, ',');
  if(cp.readSDfile("/sender_data.csv")){
    char **station_name = (char**)cp[0];  
    char **station_url = (char**)cp[1]; 
    for(row = 0; row < cp.getRowsCount(); row++){
      memset(senderarray[row], 0, sizeof(senderarray[row]));
      strcpy(senderarray[row], station_name[row]);
      memset(urlarray[row], 0, sizeof(urlarray[row]));
      strcpy(urlarray[row], station_url[row]);
    }
  }
}

const char index_html[] = R"rawliteral(
<!DOCTYPE HTML>
<html lang="hu">
  <head>
    <meta charset="utf-8">
    <iframe style="display:none" name="hidden-form"></iframe>
    <title>Internetradio bediening</title>
    <meta name="viewport" content="width=device-width, initial-scale=.85">
  </head>
  <body>
    <h3><center> ESP32 internetradio webinterface </center></h3>
    <h5><center> %senderNu% </center></h5>
    <p><small><center>%song%</center></small></p>
    <center>
      <input type="text" style="text-align:center;" value="%selecteren%" name="choice" size=30>
    </center>
      <br>
    <form action="/get" target="hidden-form">
    <center>
      <input type="submit" name="minchoice" value="   -   " onclick="confirm()">
      &nbsp;&nbsp;&nbsp;
      <input type="submit" name="pluschoice" value="   +   " onclick="confirm()">
      &nbsp;&nbsp;&nbsp;
      <input type="submit" name="confirmchoice" value="OK" onclick="confirm()">
    </center>
    </form>
    <br>
    <p><small><b><center>%text1%</center></b></small></p>
    <p><small><center>%text2%</center></small></p>
    <p><small><b><center>%text3%</center></b></small></p>
    <p><small><center>%text4%</center></small></p>
    <p><small><b><center>%text5%</center></b></small></p>
    <p><small><center>%text6%</center></small></p>
    <p><small><center><b>EQ -40 <-> 6 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Volume 0 <->21</b></center></small></p>
    <form action="/get" target="hidden-form">
    <small>
    <center>
      <labelfor="dummy">L :</label>
      <input type="text" style="text-align:center;" value="%low_choose%"   name="low_choice"   size=1>
      &nbsp;&nbsp;
      <labelfor="dummy">M :</label>
      <input type="text" style="text-align:center;" value="%middle_choose%" name="middle_choice" size=1>
      &nbsp;&nbsp;
      <labelfor="dummy">H :</label>
      <input type="text" style="text-align:center;" value="%high_choose%"   name="high_choice"   size=1>
      &nbsp;&nbsp;
      <labelfor="dummy">V :</label>
      <input type="text" style="text-align:center;" value="%volume_choose%" name="volume_choice" size=1>
    </center>
    </small>
    <br>
    <center>
      <input type="submit" name="confirm_volume" value="OK" onclick="confirm()">
    </center>
    </form>
    <br>
    <h5><center> Set_Sender_and_URL : %array_index% </center></h5>
    <form action="/get" target="hidden-form">
    <center>
    <input type= "text" style="text-align:center;" value="%sender%" name="sender" size = 30>
    </center>
     <br>
    <center>
    <input type= "text" style="text-align:center;" value="%url%" name="url" size = 40>
    </center>
    <br>
    <center>
      <input type="submit" name="array_index_min" value="   -   " onclick="confirm()">
      &nbsp;&nbsp;&nbsp;
      <input type="submit" name="array_index_plus" value="   +   " onclick="confirm()">
      &nbsp;&nbsp;&nbsp;
      <input type="submit" name="confirm_sender" value="OK" onclick="confirm()">
    </center>
    </form>
    <br>
    <br>
    <h6>Peter 2025</h6>
    <script>
      function confirm(){
        setTimeout(function(){document.location.reload();},250);
      }
    </script>
  </body>  
</html>
)rawliteral";

const char network_html[] = R"rawliteral(
<!DOCTYPE HTML>
<html lang="hu">
  <head>
    <meta charset="utf-8">
    <iframe style="display:none" name="hidden-form"></iframe>
    <title>Internetradio bediening</title>
    <meta name="viewport" content="width=device-width, initial-scale=.85">
    <style>
        div.kader {
          position: relative;
          width: 400px;
          height: 12x;
        }
          div.links{
          position: absolute;
          left : 0px;
          width; auto;
          height: 12px;
        }
        div.links_middle{
          position:absolute;
          left:  80px;
          width: auto;
          height: 12px; 
        }
        div.blanco_20{
          width: auto;
          height: 20px;
        }
        div.blanco_40{
          width: auto;
          height: 40px;
        }
    </style>
  </head>
  <body>
    <p><small><center>%song%</center></small></p>
    <center>
      <input type="text" style="text-align:center;" value="%selectie%" name="choice" size=30>
    </center>
      <br>
    <form action="/get" target="hidden-form">
    <center>
      <input type="submit" name="min" value="   -   " onclick="ok()">
      &nbsp;&nbsp;&nbsp;
      <input type="submit" name="plus" value="   +   " onclick="ok()">
      &nbsp;&nbsp;&nbsp;
      <input type="submit" name="confirm_mp3" value="OK" onclick="ok()">
    </center>
    </form>
    <br>
    <p><small><b><center>%text1%</center></b></small></p>
    <p><small><center>%text2%</center></small></p>
    <p><small><b><center>%text3%</center></b></small></p>
    <p><small><center>%text4%</center></small></p>
    <p><small><b><center>%text5%</center></b></small></p>
    <p><small><center>%text6%</center></small></p>
    <p><small><center><b>EQ -40 <-> 6 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Volume 0 <->21</b></center></small></p>
    <form action="/get" target="hidden-form">
    <small>
    <center>
      <labelfor="dummy">L :</label>
      <input type="text" style="text-align:center;" value="%low_choose%"   name="low_choice"   size=1>
      &nbsp;&nbsp;
      <labelfor="dummy">M :</label>
      <input type="text" style="text-align:center;" value="%middle_choose%" name="middle_choice" size=1>
      &nbsp;&nbsp;
      <labelfor="dummy">H :</label>
      <input type="text" style="text-align:center;" value="%high_choose%"   name="high_choice"   size=1>
      &nbsp;&nbsp;
      <labelfor="dummy">V :</label>
      <input type="text" style="text-align:center;" value="%volume_choose%" name="volume_choice" size=1>
    </center>
    </small>
    <br>
    <center>
      <input type="submit" name="confirm_volume" value="OK" onclick="ok()">
    </center>
    </form>
    <br><br>
    <h5><center><strong>ESP32 Network institutions</strong></center></h5>
    <form action="/get">
    <small>
    <div class="kader">
      <div class="links"><b>ssid :</b></div>
      <div class="links_middle"><input type="text" style="text-align:center;" name="ssid"></div>
    </div>
    <div class="blanco_40">&nbsp;</div>
    <div class="kader">
      <div class="links"><b>pswd :</b></div>
      <div class="links_middle"><input type="text" style="text-align:center;" name="pswd"></div>
    </div>
    <div class="blanco_20">&nbsp;</div>
    </small>
    <h5><center>Gewenst IP address (default 192.168.1.4)</center></h5>
    <div class="kader">
      <center>
        <input type="text" style="text-align:center;" value="%ip_address_1%" name="ip_1_choice" size=1>
        &nbsp;&nbsp;
        <input type="text" style="text-align:center;" value="%ip_address_2%" name="ip_2_choice" size=1>
        &nbsp;&nbsp;
        <input type="text" style="text-align:center;" value="%ip_address_3%" name="ip_3_choice" size=1>
        &nbsp;&nbsp;
        <input type="text" style="text-align:center;" value="%ip_address_4%" name="ip_4_choice" size=1>
      </center>
    </div>
    <div class="blanco_20">&nbsp;</div>
    </small>
    <center><input type="submit" value="Bevestig" onclick="ok()"></center>
    </form>
    <br>
    <script>
      function ok(){
        setTimeout(function(){document.location.reload();},250);
      }
    </script>
  </body>  
</html>
)rawliteral";

String processor(const String& var){
  char char_text1[30];
  char char_text2[30];
  char char_text3[30];
  char char_text4[30];
  char char_text5[30];
  char char_text6[30];
  char char_text7[40];
  if(var == "senderNu"){
    if(chosen == -2){
      String mp3_list = "mp3 list maken";
      mp3_list.toCharArray(player, (mp3_list.length() + 1));
      return(player);
    }
    else if(chosen == -1){
      String mp3_player = "mp3 player";
      mp3_player.toCharArray(player, (mp3_player.length() + 1));
      return(player);
    }
    else{
      return(senderarray[chosen]);
    }
  }
  if(var == "song"){
    return(streamsong);
  }
  if(var == "selectie"){
    if(gn_choice == 0){
      String selectie = "Stop mp3 player";
      selectie.toCharArray(gn_selectie, (selectie.length() + 1));
      return(gn_selectie);
    }
    if(gn_choice == 1){
      String selectie = "mp3 player";
      selectie.toCharArray(gn_selectie, (selectie.length() + 1));
      return(gn_selectie);
    }
    if(gn_choice == 2){
      String selectie = "Make mp3 list";
      selectie.toCharArray(gn_selectie, (selectie.length() + 1));
      return(gn_selectie);
    }
  }
  if(var == "selecteren"){
    if(choice == - 2){
      String mp3_list = "mp3 list maken";
      mp3_list.toCharArray(player, (mp3_list.length() + 1));
      return(player);
    }
    else if((choice == -1) || (gn_choice == 1)){
      String mp3_player = "mp3 player";
      mp3_player.toCharArray(player, (mp3_player.length() + 1));
      return(player);
    }
    else{
      return(senderarray[choice]);
    }
  }
  if(var == "text1"){
    if((!list_maken) && (!songlisten)){
      return(leeg);
    }
    if(list_maken){
      String text1 = "read in : ";
      text1.toCharArray(char_text1, (text1.length() + 1));
      return(char_text1);
    }
    if(songlisten){
      String text7 = "First delete all Songlistxx";
      text7.toCharArray(char_text7, (text7.length() + 1));
      return(char_text7);
    }
  }
  if(var == "text2"){
    if(!list_maken){
      return(leeg);
    }
    else{
      mp3_folder.toCharArray(char_text2, (mp3_folder.length() + 1));
      return(char_text2);;
    }
  }
 if(var == "text3"){
    if(!list_maken){
      return(leeg);
    }
    else{
      String text3 = "number mp3's read : ";
      text3.toCharArray(char_text3, (text3.length() + 1));
      return(char_text3);
    }
  }
  if(var == "text4"){
    if(!list_maken){
      return(leeg);
    }
    else{
      String text4 = String(mp3_number);
      text4.toCharArray(char_text4, (text4.length() + 1));
      return(char_text4);
    }
  }
  if(var == "text5"){
    if(!list_maken){
      return(leeg);
    }
    else{
      String text5 = "seconds already : ";
      text5.toCharArray(char_text5, (text5.length() + 1));
      return(char_text5);
    }
  }
  
  if(var == "text6"){
    if(!list_maken){
      return(leeg);
    }
    else{
      int seconden = (millis() - readin_begin) / 1000;
      String(seconden).toCharArray(char_text6, (String(seconden).length() + 1));
      return(char_text6);
    }
  }
  if(var == "low_choose"){
    return(String(low_chosen));
  }
  if(var == "middle_choose"){
    return(String(middle_chosen));
  }
  if(var == "high_choose"){
    return(String(high_chosen));
  }
  if(var == "volume_choose"){
    return(String(volume_chosen));
  }
  if(var == "array_index"){
    return(String(array_index));
  }
  if(var == "sender"){
    return(senderarray[array_index]);
  }
  if(var == "url"){
    return(urlarray[array_index]);
  }
  if(var == "folder"){
    String folder = mp3_folder;
    folder.toCharArray(mp3_list_folder, (folder.length() + 1));
    return(mp3_list_folder);
  }
  if(var == "mp3"){
    String number = String(mp3_number);
    number.toCharArray(mp3_list_number, (number.length() + 1));
    return(mp3_list_folder);
  }
  if(var == "ip_address_1"){
    return(String(ip_1_int));
  }
  if(var == "ip_address_2"){
    return(String(ip_2_int));
  }
  if(var == "ip_address_3"){
    return(String(ip_3_int));
  }
  if(var == "ip_address_4"){
    return(String(ip_4_int));
  }
  return String();
}

void html_input(){
  server.begin();
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());
  if(network){
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html, processor);
    });
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
      String http_sender = "                         ";
      String http_url = "                                                                             ";
      char terminator = char(0x0a);
      wachttijd = millis();
      choose = true;
      if(request->hasParam(CHOICEMIN_INPUT)){
        choice--;
        while((urlarray[choice][0] != *h_char) && (choice > 0)){
          choice --;
        }
        if(choice < -2){
          choice = MAX_NUMBER_CHANNEL - 1;
            while((urlarray[choice][0] != *h_char) && (choice > 0)){
              choice --;
            }
        }
      }
      if(request->hasParam(CHOICEPLUS_INPUT)){
        choice++;
        if(choice > MAX_NUMBER_CHANNEL + 1){
          choice = 0;
        }
        if((choice > 0) && (choice < MAX_NUMBER_CHANNEL)){
          while((urlarray[choice][0] != *h_char) && (choice < MAX_NUMBER_CHANNEL)){
            choice ++;
          }
        }
        if(choice == MAX_NUMBER_CHANNEL){
          choice = -2;
        }
        if(choice == MAX_NUMBER_CHANNEL + 1 ){
          choice = -1;
        }
      }
      if((request->hasParam(CONFIRMCHOICE_INPUT)) && (choose == true)){
        choose = false;
        if(choice == -2){
          songlist_bestaat_bool = false;
          testDir(SD, "/songlist0");
          if(songlist_bestaat_bool == false){
            list_maken = true;
          }
          else{
            choice = pref.getShort("station");
            webradio = true;
          }
          
        }
        else if(choice == -1){
          chosen = choice;
          play_mp3 = true;
        }
        else{
          chosen = choice;
          pref.putShort("station", chosen);
          webradio = true;
        }
      }
      if(request->hasParam(LAAG)){
        low_choice = ((request->getParam(LAAG)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(MIDDEN)){
        middle_choice = ((request->getParam(MIDDEN)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(HOOG)){
        high_choice = ((request->getParam(HOOG)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(VOLUME)){
        volume_choice = ((request->getParam(VOLUME)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(VOLUME_CONFIRM)){
        if((low_choice > -41) && (low_choice < 7)){
          low_chosen = low_choice;
          }
          if((middle_choice > -41) && (middle_choice < 7)){
            middle_chosen = middle_choice;
          }
          if((high_choice > -41) && (high_choice < 7)){
            high_chosen = high_choice;
          }
          if((volume_choice > -1) && (volume_choice < 22)){
            volume_chosen = volume_choice;
          }
          audio.setVolume(volume_chosen);
          audio.setTone(low_chosen, middle_chosen, high_chosen);
          pref.putShort("low", low_chosen);
          pref.putShort("middle", middle_chosen);
          pref.putShort("high", high_chosen);
          pref.putShort("volume", volume_chosen);
        }
      if(request->hasParam(ARRAY_MIN)){
        array_index -= 1;
        if(array_index < 0){
          array_index = MAX_NUMBER_CHANNEL - 1;
        }
      }
      if(request->hasParam(ARRAY_PLUS)){
        array_index += 1;
        if(array_index > MAX_NUMBER_CHANNEL - 1){
          array_index = 0;
        }
      }
      if(request->hasParam(SENDER)){
        http_sender = (request->getParam(SENDER)->value());
      }
      if(request->hasParam(URL)){
        http_url = (request->getParam(URL)->value());
      }
      if(request->hasParam(CONFIRM_SENDER)){
        memset(senderarray[array_index], 0, sizeof(senderarray[array_index]));
        http_sender.toCharArray(senderarray[array_index], http_sender.length() + 1);
        memset(urlarray[array_index], 0, sizeof(urlarray[array_index]));
        http_url.toCharArray(urlarray[array_index], http_url.length() + 1);
        writing_csv = true; 
      }
    });
  }
  if(!network){
    Serial.println("no network");
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", network_html, processor);
    });
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
      String network = "                         ";
      String paswoord = "                          ";
      char terminator = char(0x0a);
      wachttijd = millis();
      choose = true;
      if(request->hasParam(MIN_INPUT)){
        gn_choice --;
        if(gn_choice < 0){
          gn_choice = 2;
        }
      }
      if(request->hasParam(PLUS_INPUT)){
        gn_choice ++;
        if(gn_choice > 2){
          gn_choice = 0;
        }
      }
      if(request->hasParam(CONFIRM_MP3)){
        if(gn_choice == 0){
          audio.stopSong();
        }
        if(gn_choice == 1){
          play_mp3 = true;
        }
        if(gn_choice == 2){
          songlist_bestaat_bool = false;
          testDir(SD, "/songlist0");
          if(songlist_bestaat_bool == false){
            list_maken = true;
          }
        }
      }
      if(request->hasParam(LAAG)){
        low_choice = ((request->getParam(LAAG)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(MIDDEN)){
        middle_choice = ((request->getParam(MIDDEN)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(HOOG)){
        high_choice = ((request->getParam(HOOG)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(VOLUME)){
        volume_choice = ((request->getParam(VOLUME)->value()) + String(terminator)).toInt();
      }
      if(request->hasParam(VOLUME_CONFIRM)){
        if((low_choice > -41) && (low_choice < 7)){
          low_chosen = low_choice;
        }
        if((middle_choice > -41) && (middle_choice < 7)){
          middle_chosen = middle_choice;
        }
        if((low_choice > -41) && (low_choice < 7)){
          high_chosen = high_choice;
        }
        if((volume_choice > -1) && (low_choice < 22)){
          volume_chosen = volume_choice;
        }
        audio.setVolume(volume_chosen);
        audio.setTone(low_chosen, middle_chosen, high_chosen);
        pref.putShort("low", low_chosen);
        pref.putShort("middle", middle_chosen);
        pref.putShort("high", high_chosen);
        pref.putShort("volume", volume_chosen);
      }
      if(request->hasParam(STA_SSID)){
        network = (request->getParam(STA_SSID)->value());
        network = network + String(terminator);
        network.toCharArray(ssid, (network.length() +1));
        writeFile(SD, "/ssid", ssid);
        ssid_ingevuld = true;
      }
      if(request->hasParam(STA_PSWD)){
        paswoord = (request->getParam(STA_PSWD)->value());
        paswoord = paswoord + String(terminator);
        paswoord.toCharArray(password, (paswoord.length() + 1));
        writeFile(SD, "/pswd", password);
        pswd_ingevuld = true;
      }
      if(request->hasParam(IP_1_CHOICE)){
        ip_1_string = (request->getParam(IP_1_CHOICE)->value()) +String(terminator);
      }
      if(request->hasParam(IP_2_CHOICE)){
        ip_2_string = (request->getParam(IP_2_CHOICE)->value()) +String(terminator);
      }
      if(request->hasParam(IP_3_CHOICE)){
        ip_3_string = (request->getParam(IP_3_CHOICE)->value()) +String(terminator);
      }
      if(request->hasParam(IP_4_CHOICE)){
        ip_4_string = (request->getParam(IP_4_CHOICE)->value()) +String(terminator);
      }
      
      if((ssid_ingevuld) && (pswd_ingevuld)){
        ssid_ingevuld = false;
        pswd_ingevuld = false;
        ip_string = ip_1_string + "." + ip_2_string + "." + ip_3_string + "." + ip_4_string;
        ip_string.toCharArray(ip_char, (ip_string.length() + 1));
        writeFile(SD, "/ip", ip_char);
        Serial.println("Restart over 5 seconden");
        delay(5000);
        ESP.restart();
      }
    });
  }
}

void setup(){
  Serial.begin(115200);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  pinMode(PA_EN, OUTPUT);
  digitalWrite(PA_EN, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  if(!SD.begin(SD_CS)){
    Serial.println("Check SD card");
  }
  while (not dac.begin(IIC_DATA, IIC_CLK)){
    Serial.println("DAC connection failed");
    delay(1000);
  }

 // Configure keys on ESP32 Audio Kit board
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(PIN_CH_UP, INPUT_PULLUP);
  pinMode(PIN_CH_DOWN, INPUT_PULLUP);
  pinMode(PIN_VOL_UP, INPUT_PULLUP);
  pinMode(PIN_VOL_DOWN, INPUT_PULLUP);

// audio.i2s_mclk_pin_select(I2S_MCLK);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DSIN);
  pref.begin("WebRadio", false); 
  if(pref.getString("control") != "dummy loaded"){
    pref.putShort("station", 1);
    pref.putShort("volume", 10);
    pref.putShort("low", 0);
    pref.putShort("middle", 0);
    pref.putShort("high", 0);
    pref.putShort("files", mp3_per_songlist);
    pref.putString("controle", "dummy loaded");
  }
  chosen = pref.getShort("station");
  volume_chosen = pref.getShort("volume");
  volume_choice = volume_chosen;
  low_chosen = pref.getShort("low");
  low_choice = low_chosen;
  middle_chosen = pref.getShort("middle");
  middle_choice = middle_chosen;
  high_chosen = pref.getShort("high");
  high_choice = high_chosen;
  mp3_per_songlist = pref.getShort("files");
  audio.setVolume(volume_chosen);
  audio.setTone(low_chosen, middle_chosen, high_chosen);
  lees_CSV();
  readFile(SD, "/ssid");
  inputString.toCharArray(ssid, teller);
  readFile(SD, "/pswd");
  inputString.toCharArray(password, teller);
  readIP(SD, "/ip");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  network = true;
  watch_op_network = millis();
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    if(millis() - watch_op_network > 15000){
      network = false;
      break;
    }
  }
  if(network == true){  
    IPAddress subnet(WiFi.subnetMask());
    IPAddress gateway(WiFi.gatewayIP());
    IPAddress dns(WiFi.dnsIP(0));
    IPAddress static_ip(ip_1_int, ip_2_int, ip_3_int, ip_4_int);
    WiFi.disconnect();
    if (WiFi.config(static_ip, gateway, subnet, dns, dns) == false) {
      Serial.println("Configuration failed.");
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    watch_op_network = millis();
    while(WiFi.status() != WL_CONNECTED){
      delay(500);
      if(millis() - watch_op_network > 15000){
        network = false;
        break;
      }
    }
    choice = chosen;
    radio_chosen();
    html_input();
  }
  else{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(APssid, APpswd);
    html_input();
  }
}
bool pressed( const int pin )
{
  if (millis() > (debounce + 500))
  {
      if (digitalRead(pin) == LOW)
      {
        debounce = millis();
        return true;
      }
  }
  return false;
}
void loop(){

  if(writing_csv == true){
    writing_csv = false;
    writing_naar_csv();
  }
  if(list_maken == true){
    //list_maken = false;
    mp3_list_maken_chosen();
  }
  if(play_mp3 == true){
    play_mp3 = false;
    mp3_chosen();
  }
  if(webradio == true){
    webradio = false;
    chosen = choice;
    pref.putShort("station", chosen);
    radio_chosen();
  }
  if(((millis() - wachttijd) > 5000) && (choose == true)){
    choose = false;
    choice = chosen;
  }

      if (pressed(PIN_CH_DOWN)) {
        Serial.println("channel-down");
        choice--;
        while((urlarray[choice][0] != *h_char) && (choice > 0)){
          choice --;
        }
        if(choice < -2){
          choice = MAX_NUMBER_CHANNEL - 1;
            while((urlarray[choice][0] != *h_char) && (choice > 0)){
              choice --;
            }
        }
       chosen = choice;
          pref.putShort("station", chosen);
          webradio = true;
      }
      if (pressed(PIN_CH_UP)) {
        Serial.println("channel-up");
        choice++;
        if(choice > MAX_NUMBER_CHANNEL + 1){
          choice = 0;
        }
        if((choice > 0) && (choice < MAX_NUMBER_CHANNEL)){
          while((urlarray[choice][0] != *h_char) && (choice < MAX_NUMBER_CHANNEL)){
            choice ++;
          }
        }
        if(choice == MAX_NUMBER_CHANNEL){
          choice = -2;
        }
        if(choice == MAX_NUMBER_CHANNEL + 1 ){
          choice = -1;
        }
      chosen = choice;
          pref.putShort("station", chosen);
          webradio = true;
      }

 bool updateVolume = false;

  if (pressed(PIN_VOL_UP))
  {
    if (volume <= (63-volume_step))
    {
      // Increase volume
      volume += volume_step;
      updateVolume = true;
    } 
  }
  if (pressed(PIN_VOL_DOWN))
  {
    if (volume >= volume_step)
    {
      // Decrease volume
      volume -= volume_step;
      updateVolume = true;
    } 
  }
  if (updateVolume)
  {
    // Volume change requested
    Serial.printf("Volume %d\n", volume);
    dac.SetVolumeSpeaker(volume);
    dac.SetVolumeHeadphone(volume); 
  }
  
  audio.loop();
}
