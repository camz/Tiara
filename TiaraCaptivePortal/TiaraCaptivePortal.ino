#include <FastLED.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>

const byte DNS_PORT = 53;
IPAddress apIP(172, 217, 0, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

#define AP_SSID   "SSID"
#define AP_PASSWD "PASSWORD"
#define AP_CHAN   1
#define AP_HIDDEN 0 // 1 = Hidden, 0 = Broadcast

FASTLED_USING_NAMESPACE

#define LED_PIN     D4
#define DATA_PIN    D3
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    19

#define BRIGHTNESS         64
#define FRAMES_PER_SECOND  120

/* LED wiring order
                             [08]  [07]

       [17]                [09] [12] [06]              [01]
                    [14]                   [04]
  [18]          [15]         [10]  [11]        [03]         [00]  <--- Starts here
            [16]                                  [02]

                         [13]          [05]
*/

// Sequence to re-order effects, changes sequence of large "gem"
byte seq[] = { 0, 1, 2, 3, 4, 5,   11, 6, 7, 12, 8, 9, 10,   13, 14, 15, 16, 17, 18 };

int b = 0;
unsigned long last = 0UL;
unsigned long flast = 0UL;
CRGB leds[NUM_LEDS], color;
byte gHue = 0;
byte gPattern = 0;
byte cycle = 1;
byte off = 0;
byte solid = 0;

typedef struct {
  void (*func)(); // effect function
  char *effect;   // effect name
} animation_t;

// function prototypes (for animations)
void boom(void);
void foom(void);
void wipe(void);
void rainbow(void);
void rainbowWithGlitter(void);
void confetti(void);
void sinelon(void);
void juggle(void);
void bpm(void);


// List of animations to cycle through
animation_t gAnimations[] = {
  { boom, "Boom" },
  { wipe, "Color Wipe" },
  { foom, "Foom" },
  { rainbow, "Rainbow" },
  { rainbowWithGlitter, "Rainbow w/Glitter" },
  { confetti, "Confetti" },
  { sinelon, "Sinelon" },
  { juggle, "Juggle" },
  { bpm, "BPM" },
};

String responseHEADER = ""
                        "<!DOCTYPE html><html lang='en'><head>"
                        "<meta name='viewport' content='width=device-width'>"
                        "<title>Tiara</title></head>"
                        "";

String responseJS     = "<script src=\"/tiara.js\"></script>";

String responseJS_IRO = "<script src=\"/iro.js\"></script>"
                        "<script src=\"/tiaraPicker.js\"></script>"
                        "";

String responseBODY   = "<link rel=\"stylesheet\" href=\"/tiara.css\">"
                        "<body><title>Tiara</title>"
                        "<center><img src=\"/banner.jpg\" width=\"500\" height=\"139\"></center>"
                        "";


String responseFOOTER = "</body></html>";

String tiaraModeList  = ""
                        "<center><div id=\"status\"></div></center>"
                        "<ol>"
                        "<li><a href=\"/1\">Boom</a></li>"
                        "<li><a href=\"/3\">Foom</a></li>"
                        "<li><a href=\"/2\">Color Wipe</a></li>"
                        "<li><a href=\"/4\">Rainbow</a></li>"
                        "<li><a href=\"/5\">Rainbow w/Glitter</a></li>"
                        "<li><a href=\"/6\">Confetti<a/></li>"
                        "<li><a href=\"/7\">Sinelon<a/></li>"
                        "<li><a href=\"/8\">Juggle<a/></li>"
                        "<li><a href=\"/9\">BPM<a/></li>"
                        "<li><a href=\"/C\">Solid Color</a></li>"
                        ""
                        "<li><a href=\"/cycle\">Cycle Patterns</a></li>"
                        "<li><a href=\"/off\">All OFF</a></li>"
                        "</ol>"
                        "";

String tiaraColor     = "<br><div class=\"wrap\"><div class=\"colorPicker\"></div></div>"
                        "<br><form method=\"get\" action=\"/\"><button type=\"submit\">Back to Menu</button></form>"
                        "";


void sendResponse( String msg) {
  webServer.send( 200, "text/html", responseHEADER + responseJS + responseBODY + msg + responseFOOTER );
}

void sendPicker() {
  webServer.send( 200, "text/html", responseHEADER + responseJS_IRO + responseBODY + tiaraColor + responseFOOTER );
}


void sendFile( char *filename, char *type )
{
  File file = SPIFFS.open(filename, "r");
  webServer.streamFile(file, type);
  file.close();
  Serial.print("send file ");
  Serial.println(filename);
}

void handleList( int num ) {
  sendResponse( tiaraModeList );

  if( num == 42 ) return;

  if( (num > 0) && (num < 10) ) {
    gPattern = num - 1;
    cycle = off = solid = 0;
  } else if( num < 0 ) {
    cycle = 1; // enable pattern cycling
    off = solid = 0;
  } else {
    solid = 0;
    off = 1; // turn off all patterns and/or animations
  }
}

void handleStatus() {
  char tmp[80];

  if( solid ) {
    sprintf( tmp, "{\"num\":%d,\"pattern\":\"Solid Color\",\"cycle\":%d,\"off\":%d}", gPattern, cycle, off );
  } else {
    sprintf( tmp, "{\"num\":%d,\"pattern\":\"%s\",\"cycle\":%d,\"off\":%d}", gPattern, gAnimations[gPattern].effect, cycle, off );
  }
  webServer.send(200, "application/json", tmp);
}

void handleWheel() {
  char tmp[20];
  String foo;
  unsigned long rgb;

  if( webServer.hasArg("rgb") ) {
    foo = webServer.arg("rgb");
    foo.toCharArray(tmp,8);
    rgb = strtoul( tmp+1, NULL, 16);
    solid = 1;
    for( int i = 0 ; i < NUM_LEDS ; i++ ) leds[i] = rgb;
    FastLED.show();
  }
  webServer.send(200,"text/plain", "");
}

void setup() {
  int i;
  char tmp[16];

  pinMode( LED_PIN, OUTPUT );

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, AP_PASSWD, AP_CHAN, AP_HIDDEN );

  delay(500);

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness( BRIGHTNESS );
  FastLED.show();
  for( i = 0 ; i < NUM_LEDS ; i++ ) leds[i] = CRGB::Purple;
  FastLED.show();

  for( i = 0 ; i < sizeof(leds) ; i++ ) { pinMode( leds[i], OUTPUT ); digitalWrite( leds[i], LOW ); }

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

Serial.begin(115200);
Serial.println();
Serial.println("Tiara Captive Portal");

  SPIFFS.begin();
  Dir dir = SPIFFS.openDir("");
  while( dir.next()) {
    Serial.print(dir.fileName());
    if(dir.fileSize()) {
      File f = dir.openFile("r");
      Serial.print(" ");
      Serial.print(f.size());
    }
    Serial.println();
  }

  webServer.on("/1", []() { handleList(1); } );
  webServer.on("/2", []() { handleList(2); } );
  webServer.on("/3", []() { handleList(3); } );
  webServer.on("/4", []() { handleList(4); } );
  webServer.on("/5", []() { handleList(5); } );
  webServer.on("/6", []() { handleList(6); } );
  webServer.on("/7", []() { handleList(7); } );
  webServer.on("/8", []() { handleList(8); } );
  webServer.on("/9", []() { handleList(9); } );

  webServer.on("/C",     []() { sendPicker();   } );
  webServer.on("/off",   []() { handleList(0);  } );
  webServer.on("/cycle", []() { handleList(-1); } );

  webServer.on("/banner.jpg",     []() { sendFile("/TiaraBanner-sm.jpg", "image/jpeg"); } );
  webServer.on("/iro.js",         []() { sendFile("/iro.js", "application/javascript"); } );
  webServer.on("/tiara.js",       []() { sendFile("/tiara.js", "application/javascript"); } );
  webServer.on("/tiara.css",      []() { sendFile("/tiara.css", "text/css"); } );
  webServer.on("/tiaraPicker.js", []() { sendFile("/tiaraPicker.js", "application/javascript"); } );
  webServer.on("/status", handleStatus );

  webServer.on("/wheel", HTTP_POST, handleWheel );

  // replay to all requests with same HTML
  webServer.onNotFound([]() { handleList( 42 ); });
  webServer.begin();
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  if( cycle && !off ) {
    gPattern = (++gPattern) % ARRAY_SIZE( gAnimations );
  }
}

void loop() {
  unsigned long frameDelay = 1000/FRAMES_PER_SECOND;
  unsigned long m = millis();
  dnsServer.processNextRequest();
  webServer.handleClient();

  if( m > flast ) {
    flast = m + frameDelay;
    if( !off) {
      if( !solid ) gAnimations[gPattern].func();
    } else fadeToBlackBy( leds, NUM_LEDS, 10);
    FastLED.show();
  }

  EVERY_N_MILLISECONDS( 20 )  { gHue++; color = CHSV( gHue, 200, 255 ); } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 )       { nextPattern(); }                          // change patterns periodically
  EVERY_N_MILLISECONDS( 500 ) { digitalWrite( LED_PIN, b ); b ^= 1; }     // blink board LED as "heartbeat"
}

// ----------------

void wipe()
{
  int pos = beat8(600) % NUM_LEDS;

  fadeToBlackBy( leds, NUM_LEDS, 20);
  leds[seq[pos]] = color;
  leds[seq[NUM_LEDS - pos - 1]] = color;
}

void foom()
{
  int pos = beat8(600) % 5;

  fadeToBlackBy( leds, NUM_LEDS, 20 );
  switch( pos ) {
    case 0: // outer group of two "gems"
      leds[0] = leds[1] = leds[17] = leds[18] = color;
      break;
    case 1: // next group of three "gems"
      leds[2] = leds[3] = leds[4] = leds[14] = leds[15] = leds[16] = color;
      break;
    case 2: // bottom two "gems"
      leds[5] = leds[13] = color;
      break;
    case 3: // outer ring large "gem"
      leds[6] = leds[7] = leds[8] = leds[9] = leds[10] = leds[11] = color;
      break;
    case 4: // center of large "gem"
      leds[12] = color;
      break;
  }
}

void boom() // explode out from center of large "gem"
{
  int pos = beat8( 1200 ) % NUM_LEDS;
  int i;

  fadeToBlackBy( leds, NUM_LEDS, 20);
  switch( pos ) {
    case 0: // center of large "gem"
      leds[12] = color;
      break;
    case 1: // outer ring of large "gem"
      for( i = 6 ; i < 12 ; i++ ) leds[i] = color;
      break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7: // remaining "gems"
      leds[7-pos] = leds[11+pos] = color;
      if( i == 7 ) i = 0; // reset sequence
      break;
  }
}

void remap()
{
  int i;
  CRGB tmp[NUM_LEDS];

  for( i = 0 ; i < NUM_LEDS ; i++ ) tmp[i] = leds[i];
  for( i = 0 ; i < NUM_LEDS ; i++ ) leds[seq[i]] = tmp[i];
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  remap();
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  remap();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[seq[pos]] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[seq[i]] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
