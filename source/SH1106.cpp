/*
 * SH1106.cpp
 *
 *  Created on: 25.12.2019
 *      Author: Marcin
 */

#include "SH1106.h"

SH1106::SH1106(PinName dataPin, PinName clkPin, PinName resetPin, PinName cdPin, PinName csPin) :
    interface(dataPin, NC, clkPin),
    resetSignal(PE_15),
    cdSignal(cdPin),
    csSignal(csPin)
{
    csSignal = 1;
}

/*
 * initialization of SH1106 controller
 */
void SH1106::init(void)
{
    resetSignal = 0;
    // send a dummy byte to set proper signal levels
    interface.write(0);
    resetSignal = 1;
    // wait after reset
    ThisThread::sleep_for(1);
    // send initialization data
    write(SH1106InitData, true);
    // clear screen
    update();
    //display on
    write(std::vector<uint8_t>{0xAF}, true);
    // wait after init
    ThisThread::sleep_for(100);
}

/*
 * updates display according to the range in updateArray
 */
void SH1106::update(void)
{
    // check update of every page
    for(uint8_t page = 0; page < noOfPages; page++)
    {
        // check whether this page must be updated
        if((updateArray[page][0] < sizeX) && (updateArray[page][1] < sizeX) && (updateArray[page][0] <= updateArray[page][1]))
        {
            // set display page and column
            uint8_t displayColumn = updateArray[page][0] + 2;   // physical display starts from column number 2
            std::vector<uint8_t> coordinateData =
            {
                    static_cast<uint8_t>(displayColumn & 0x0F),     // lower part of column value
                    static_cast<uint8_t>(0x10 | ((displayColumn >> 4) & 0x0F)),     // higher part of column value
                    static_cast<uint8_t>(0xB0 | page)       // page value
            };
            write(coordinateData, true);
            // send data from buffer to display
            write(&dataBuffer[page][updateArray[page][0]], updateArray[page][1]-updateArray[page][0]+1);
            // clear the range to update
            updateArray[page][0] = updateArray[page][1] = 0xFF;
        }
    }
}

/*
 * send command/data to display controller
 */
void SH1106::write(uint8_t* data, int length, bool command)
{
    cdSignal = command ? 0 : 1;
    csSignal = 0;
    interface.write((const char*)data, length, nullptr, 0);
    csSignal = 1;
}

/*
 * set new font parameters
 */
void SH1106::setFont(const uint8_t* newFont, bool newInvertion, uint8_t newXLimit)
{
    font = newFont;
    inverted = newInvertion;
    upToX = newXLimit;
}

/*
 * set or clear a single pixel in X,Y coordinates
 */
void SH1106::setPoint(uint8_t X, uint8_t Y, bool clear)
{
    if((X>sizeX) || (Y>sizeY))
    {
        // out of range
        return;
    }
    uint8_t page = Y / 8;
    uint8_t mask = 1 << (Y % 8);

    // set or clear point in display buffer
    if(clear)
    {
        dataBuffer[page][X] &= ~mask;
    }
    else
    {
        dataBuffer[page][X] |= mask;
    }

    // set lower limit of refreshing range
    if(updateArray[page][0] > X)
    {
        updateArray[page][0] = X;
    }
    // set upper limit of refreshing range
    if((updateArray[page][1] < X) || (updateArray[page][1] >= sizeX))
    {
        updateArray[page][1] = X;
    }
}

/*
 * test of the display functions
 */
void SH1106::test(uint32_t argument)
{
    setPoint(0, 0);
    setPoint(0, sizeY-1);
    setPoint(sizeX-1, 0);
    setPoint(sizeX-1, sizeY-1);
    for(uint8_t x=0; x<argument; x++)
    {
        //x^2 + y^2 = r^2
        uint8_t y = (uint8_t)sqrt(argument * argument - x * x);
        setPoint(64+x, 32 - y);
        setPoint(64-x, 32 - y);
        setPoint(64+x, 32 + y);
        setPoint(64-x, 32 + y);
    }
    update();
}

/*
 * displays character on the screen
 * ch - ascii code
 * cX,cY - upper left corner of character placement (to be assigned to X,Y)
 * font - font array from fonts.h
 * inverted - clears pixels if true
 * upToX - if >0, stops at X==upToX
 */
void SH1106::putChar(uint8_t cX, uint8_t cY,uint8_t ch)
{
    assert_param(font != nullptr);
    X = cX;
    Y = cY;
    bool isSpace = false;

    if((ch < font[4]) || (ch >= font[4]+font[5]))
    {
        // ascii code out of this font range
        return;
    }

    // width of this char
    uint8_t charWidth = font[6 + ch - font[4]];
    if(charWidth == 0)
    {
        isSpace = true;
        charWidth = font[2];
    }

    // height of this char
    uint8_t charHeight = font[3];

    // calculate index of this char definition in array
    uint16_t charDefinitionIndex = 6 + font[5];
    for(uint8_t i = 0; i < ch - font[4]; i++)
    {
        charDefinitionIndex += font[6 + i] * (1 + (charHeight - 1) / 8);
    }

    // for every column
    uint8_t ix;
    for(ix = 0; ix < charWidth; ix++)
    {
        // if upToX!=0 then print up to this X limit
        if((upToX != 0) && (X+ix > upToX))
        {
            break;
        }
        // for every horizontal row
        for(uint8_t iy = 0; iy < charHeight; iy++)
        {
            uint8_t bitPattern = isSpace ? 0 : font[charDefinitionIndex + ix + (iy / 8) * charWidth];
            bool lastByte = iy / 8 == charHeight / 8;
            uint8_t extraShift = lastByte ? 8 - charHeight % 8 : 0;
            setPoint(X + ix, Y + iy, ((bitPattern >> (extraShift + iy % 8)) & 0x01) == inverted);
        }
    }

    X += ix;
}


/*
 * puts a tiny space between printed characters
 */
void SH1106::putChar2CharSpace(void)
{
    assert_param(font != nullptr);

    // height of this space
    uint8_t charHeight = font[3];

    // width of this space
    uint8_t charWidth = 1 + (charHeight - 2) / 8;

    // for every column
    uint8_t ix;
    for(ix = 0; ix < charWidth; ix++)
    {
        // if upToX!=0 then print up to this X limit
        if((upToX != 0) && (X+ix > upToX))
        {
            break;
        }
        // for every horizontal row
        for(uint8_t iy = 0; iy < charHeight; iy++)
        {
            setPoint(X + ix, Y + iy, !inverted);
        }
    }

    X += ix;
}

/*
 * displays string on the screen
 * text - string to be displayed
 * sX,sY - upper left corner of string placement
 */
void SH1106::print(uint8_t sX, uint8_t sY, std::string text)
{
    X = sX;
    Y = sY;

    for(size_t index = 0; index < text.size(); index++)
    {
        putChar(X, Y, text[index]);
        if(index < text.size() - 1)
        {
            putChar2CharSpace();
        }
    }
}
