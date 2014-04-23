#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

#define SD_CS 4
#define sclk 13
#define mosi 11
#define cs   10
#define dc   8
#define rst  0

#define Neutral 0
#define Press 1
#define Up 2
#define Down 3
#define Right 4
#define Left 5


Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);
uint16_t pallette[16];
char bufflocal[25];
int page = 1;


char * nextfield(File f)
{  
  int i=0;
  char tmp;
  
  while (f.available())
  {
    tmp = f.read();
    if ( tmp == ',')
    {
      bufflocal[i] = 0;
      return bufflocal;
    }
    else
    {
      bufflocal[i] = tmp;
      i++;
    }
  }
  bufflocal[i] = 0;
  return bufflocal;
}

void dispInfo(int id)
{
  char datafile[20];
  sprintf(datafile,"data/%d.dat",id);
  Serial.println(datafile);
  tft.setCursor(0, 100);
    
  File data;
  if ((data = SD.open(datafile)) == NULL) 
  {
    // Serial.print("File not found");
    
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(2);
    tft.print("No data");
    
    return;
  }
  else
  {
    tft.setTextColor(ST7735_WHITE);
    //tft.setTextWrap(true);
    tft.setTextSize(1);
    
    tft.print("id    : ");
    tft.println(nextfield(data));
    
    tft.print("name  : ");
    tft.println(nextfield(data));
    
    nextfield(data);
    
    tft.print("height: ");
    tft.println(nextfield(data));
    
    tft.print("weight: ");
    tft.println(nextfield(data));
    
    data.close();
  }
}

void dispPkm(int id)
{
  char file[15];
  if (id == -1)
  {
    sprintf(file,"img2/%d.bmp",page); // charge shiny 
  }
  else
  {
    sprintf(file,"img/%d.bmp",id); // charge new pkm
  }
  Serial.println(file);
  bmpDraw(file, 16, 0); 
}

void change_page(int d)
{
  /*if ((page+d<0) || (page+d >719 ))
  {
    Serial.println("out");
    delay(500);
    //return;
  }
  else
  {
  */  page+=d;
    tft.fillScreen(ST7735_RED);
    dispInfo(page);
    dispPkm(page);
  //}
}

void setup(void) {
  Serial.begin(9600);

  // Our supplier changed the 1.8" display slightly after Jan 10, 2012
  // so that the alignment of the TFT had to be shifted by a few pixels
  // this just means the init code is slightly different. Check the
  // color of the tab to see which init code to try. If the display is
  // cut off or has extra 'random' pixels on the top & left, try the
  // other option!
  // If you are seeing red and green color inversion, use Black Tab

  // If your TFT's plastic wrap has a Black Tab, use the following:
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  // If your TFT's plastic wrap has a Red Tab, use the following:
  //tft.initR(INITR_REDTAB);   // initialize a ST7735R chip, red tab
  // If your TFT's plastic wrap has a Green Tab, use the following:
  //tft.initR(INITR_GREENTAB); // initialize a ST7735R chip, green tab

  Serial.println("Initializing SD card...");
  pinMode(10, OUTPUT);
  if (!SD.begin(SD_CS))
  {
    Serial.println("failed!");
    return;
  }
  Serial.println("OK!");

  change_page(0);
}

int joy;

void loop() 
{
  
    //delay(1000); 
    //change_page(1);
    //Serial.println(freeMemory(),DEC);
    //delay(1000);
    
    joy = CheckJoystick();
  
      switch (joy)
  {
    case Left:
      //Serial.println("Left");
      change_page(-1);
      //delay(500);
      break;
    case Right:
      //Serial.println("Right");
      change_page(1);
      //delay(500);
      break;
    case Up:
      //Serial.println("Up");
      change_page(10);
      break;
    case Down:
      //Serial.println("Down");
      change_page(-10);
      break;
    case Press:
      // Serial.println("Press");
      dispPkm(-1);
      break;
  }
}

// Check the joystick position
int CheckJoystick()
{
  int joystickState = analogRead(3);
  
  if (joystickState < 50) return Down;
  if (joystickState < 150) return Right;
  if (joystickState < 250) return Press;
  if (joystickState < 500) return Up;
  if (joystickState < 650) return Left;
  return Neutral;
}

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.


#define BUFFPIXEL 20

void bmpDraw(char *filename, uint8_t x, uint8_t y) {
  //delay(0.5);
  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 4)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  //uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  //uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint32_t pos = 0;
  
  uint8_t buf;
  uint32_t color;

  if((x >= tft.width()) || (y >= tft.height())) return;
   Serial.print("Loading image '");
   Serial.print(filename);
   Serial.println('\'');
 
  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    // Serial.print("File not found");
    tft.setTextColor(ST7735_RED);
    tft.setCursor(0, 40);
    tft.setTextSize(2);
    tft.print("No Img");
    return;
  }

  // Parse BMP header

  if(read16(bmpFile) == 0x4D42) { // BMP signature
    (void)read32(bmpFile);//Serial.print("File size: "); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    
    // Read DIB header
    (void)read32(bmpFile);
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      if((bmpDepth == 4) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = 96/2;//(bmpWidth * 3+ 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y; 
        (void)read32(bmpFile); //size
        (void)read32(bmpFile); //
        (void)read32(bmpFile); //
        (void)read32(bmpFile); //
        (void)read32(bmpFile); //
        
        // create pallette
        for (int i=0 ; i < 16 ; i++)
        {
          color = read32(bmpFile);
          pallette[i] = (tft.Color565((color>>16)&0xFF,((color>>8)&0xFF),color&0xFF));
        }
        
        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++)
        {

          if(flip)
          {
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          }
          else
          {
            pos = bmpImageoffset + row * rowSize;
          }
          
          bmpFile.seek(pos);
          
          for (col=0; col<w; col += 2)
          {
            
            buf = bmpFile.read();
            
            tft.pushColor(pallette[buf>>4]);
            tft.pushColor(pallette[buf&0xF]);
          } // end pixel 
        } // end scanline
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp)
  {
    Serial.println("BMP format not recognized.");
    delay(1000);
  }
  else 
  {
    Serial.println("load success");
  }
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
