/*
  DHCP-based IP printer using TFT
 
 This sketch uses the DHCP extensions to the Ethernet library
 to get an IP address via DHCP and print the address obtained.
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 12 April 2011
 modified 9 Apr 2012
 by Tom Igoe
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Adafruit_GFX.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// work in line numbers.  Font height in ht
int16_t ht = 16, top = 3, line, lines = 15, scroll;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

void setup() {
    tft.reset();
    uint16_t id = tft.readID();
    tft.begin(id);
    tft.setRotation(1);     //Portrait
    tft.fillScreen(WHITE);
    tft.setTextColor(BLACK, WHITE);
    tft.setTextSize(2);     // System font is 8 pixels.  ht = 8*2=16
    tft.setCursor(40, 20);
   // tft.print("ID = 0x");
   // tft.println(id, HEX);
 //   if (id == 0x9320 || id == 0x9325 || id == 0xB509) {
//        top = 0;                      // these controllers scroll full screen
    //    lines = tft.height() / ht;    // we are in portrait mode
//    }
  //  if (id == 0x7783) {
  //      tft.println("can NOT scroll");
   //     while (1);                    // die.
 //   }
 //   tft.setCursor(0, 0);
  //  for (line = 1; line < 21; line++) tft.println(String(line) + ": ");
    
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  // print your local IP address:
  tft.print("My IP address is: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    tft.print(Ethernet.localIP()[thisByte], DEC);
    tft.print("."); 
  }
  tft.println();
}

/*
 * © Francesco Potortì 2013 - GPLv3 - Revision: 1.13
 *
 * Send an NTP packet and wait for the response, return the Unix time
 *
 * To lower the memory footprint, no buffers are allocated for sending
 * and receiving the NTP packets.  Four bytes of memory are allocated
 * for transmision, the rest is random garbage collected from the data
 * memory segment, and the received packet is read one byte at a time.
 * The Unix time is returned, that is, seconds from 1970-01-01T00:00.
 */
unsigned long inline ntpUnixTime (UDP &udp)
{
  static int udpInited = udp.begin(123); // open socket on arbitrary port

  const char timeServer[] = "pool.ntp.org";  // NTP server

  // Only the first four bytes of an outgoing NTP packet need to be set
  // appropriately, the rest can be whatever.
  const long ntpFirstFourBytes = 0xEC0600E3; // NTP request header

  // Fail if WiFiUdp.begin() could not init a socket
  if (! udpInited)
    return 0;

  // Clear received data from possible stray received packets
  udp.flush();

  // Send an NTP request
  if (! (udp.beginPacket(timeServer, 123) // 123 is the NTP port
	 && udp.write((byte *)&ntpFirstFourBytes, 48) == 48
	 && udp.endPacket()))
    return 0;				// sending request failed

  // Wait for response; check every pollIntv ms up to maxPoll times
  const int pollIntv = 150;		// poll every this many ms
  const byte maxPoll = 15;		// poll up to this many times
  int pktLen;				// received packet length
  for (byte i=0; i<maxPoll; i++) {
    if ((pktLen = udp.parsePacket()) == 48)
      break;
    delay(pollIntv);
  }
  if (pktLen != 48)
    return 0;				// no correct packet received

  // Read and discard the first useless bytes
  // Set useless to 32 for speed; set to 40 for accuracy.
  const byte useless = 40;
  for (byte i = 0; i < useless; ++i)
    udp.read();

  // Read the integer part of sending time
  unsigned long time = udp.read();	// NTP time
  for (byte i = 1; i < 4; i++)
    time = time << 8 | udp.read();

  // Round to the nearest second if we want accuracy
  // The fractionary part is the next byte divided by 256: if it is
  // greater than 500ms we round to the next second; we also account
  // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
  // additionally, we account for how much we delayed reading the packet
  // since its arrival, which we assume on average to be pollIntv/2.
  time += (udp.read() > 115 - pollIntv/8);

  // Discard the rest of the packet
  udp.flush();

  return time - 2208988800ul;		// convert NTP time to Unix time
}


void loop() {

}


