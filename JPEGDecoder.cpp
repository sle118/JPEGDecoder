/*
 JPEGDecoder.cpp
 
 JPEG Decoder for Arduino
 Public domain, Makoto Kurauchi <http://yushakobo.jp>
*/
/********************************
 * ported to ESP8266 by reaper7
 * https://github.com/reaper7/JPEGDecoder
 *
 * ported for Sming by sle118.
 * https://github.com/sle118
 ********************************/


#include "JPEGDecoder.h"
#include "picojpeg.h"

// The declaration below is mandatory; the current implementation of picojpeg uses global static variables to store
// callbacks, so we can only instantiate a single instance of the decoder at a time.

JPEGDecoder JpegDec;

JPEGDecoder::JPEGDecoder(){
	init(&JPEGDecoder::dummyBeginFunc,&JPEGDecoder::dummyEndFunc, &JPEGDecoder::dummyDrawFunc,&JPEGDecoder::dummyDrawRGBFunc);
}
void JPEGDecoder::init(void (*drawPixelFunc)(int x, int y, int color)){
	init(&JPEGDecoder::dummyBeginFunc, &JPEGDecoder::dummyEndFunc,drawPixelFunc,&JPEGDecoder::dummyDrawRGBFunc);
}
void JPEGDecoder::init(void (*drawRGBPixelFunc)(int x, int y, int red, int green, int blue)){
	init(&JPEGDecoder::dummyBeginFunc, &JPEGDecoder::dummyEndFunc,&JPEGDecoder::dummyDrawFunc, drawRGBPixelFunc);
}

void JPEGDecoder::init(void (*beginFunc)(), void (*endFunc)(),void (*drawPixelFunc)(int x, int y, int color),void (*drawRGBPixelFunc)(int x, int y, int red, int green, int blue)){
	mcu_x = 0 ;
	mcu_y = 0 ;
	is_available = 0;
	reduce = 0;
	thisPtr = this;
	this->_drawPixel = drawPixelFunc;
	this->_drawRGBPixel = drawRGBPixelFunc;
	this->_beginFunc = beginFunc;
	this->_endFunc = beginFunc;
}
void JPEGDecoder::setReduce(bool bReduce){
	this->reduce = bReduce?1:0;
}
void JPEGDecoder::setBeginCallback(void (*beginFunc)()){
	this->_beginFunc = beginFunc;
}

void JPEGDecoder::setEndCalback(void (*endFunc)()){
	this->_endFunc = endFunc;
}

JPEGDecoder::~JPEGDecoder(){
	delete pImage;
}
bool JPEGDecoder::display(String pFilename, int xPos, int yPos){
	return display(pFilename,xPos,yPos,-1,-1);
}
bool JPEGDecoder::display(String pFilename, int xPos, int yPos, int xSize, int ySize){
	uint8 *pImg;
	int x, y, bx, by;
	int val, valmax;
	Serial.println(String("displaying file "+pFilename));
char str[200];
	// Decoding start
	if (decode((char *)pFilename.c_str(), 0) < 0) {
		return false;
		Serial.println(String("Could not open file "+pFilename));
	}
	// Image Information
	  Serial.println(String("Showing "+pFilename));
	  Serial.print("Width     :"); Serial.println(width);
	  Serial.print("Height    :"); Serial.println(height);
	  Serial.print("Components:"); Serial.println(comps);
	  Serial.print("MCU / row :"); Serial.println(MCUSPerRow);
	  Serial.print("MCU / col :"); Serial.println(MCUSPerCol);
	  Serial.print("Scan type :"); Serial.println(scanType);
	  Serial.print("MCU width :"); Serial.println(MCUWidth);
	  Serial.print("MCU height:"); Serial.println(MCUHeight);
	  Serial.println("");
	_beginFunc();

	while (read()) {
		pImg = pImage;

		for (by = 0; by < MCUHeight; by++) {
			for (bx = 0; bx < MCUWidth; bx++) {
				x = MCUx * MCUWidth + bx;
				y = MCUy * MCUHeight + by;

				if (x < width && y < height) {

					if (comps == 1) {               // Grayscale

						_drawPixel(xPos+x, yPos+y,pImg[0]);
						// todo: convert GrayScale to a proper color space value
						_drawRGBPixel(xPos+x, yPos+y,pImg[0],pImg[0],pImg[0]);


					} else {                                    // RGB



						// Convert RGB to Gray Scale
						val = 0.299 * pImg[0] + 0.587 * pImg[1]
								+ 0.114 * pImg[2];
						_drawPixel(xPos+x, yPos+y, val );
						_drawRGBPixel(xPos+x, yPos+y, pImg[0], pImg[1], pImg[2] );
//
//
//						if (val > valmax) {
//							valmax = val;
//						}
//						val = val / 55;
//						if (x % 4 == 0) {
//							switch (val) {
//							case 3:
//								_drawPixel(3 * x / 4, y / 2, 0);
//								_drawPixel(3 * x / 4 + 1, y / 2, 0);
//								_drawPixel(3 * x / 4 + 2, y / 2, 0);
//								break;
//							case 2:
//								_drawPixel(3 * x / 4, y / 2, 0);
//								_drawPixel(3 * x / 4 + 1, y / 2, 1);
//								_drawPixel(3 * x / 4 + 2, y / 2, 0);
//								break;
//							case 1:
//								_drawPixel(3 * x / 4, y / 2, 1);
//								_drawPixel(3 * x / 4 + 1, y / 2, 0);
//								_drawPixel(3 * x / 4 + 2, y / 2, 1);
//								break;
//							case 0:
//								_drawPixel(3 * x / 4, y / 2, 1);
//								_drawPixel(3 * x / 4 + 1, y / 2, 1);
//								_drawPixel(3 * x / 4 + 2, y / 2, 1);
//								break;
//							default:
//								_drawPixel(3 * x / 4, y / 2, 1);
//								_drawPixel(3 * x / 4 + 1, y / 2, 1);
//								_drawPixel(3 * x / 4 + 2, y / 2, 1);
//								break;
//							}
//						}
						//pic[x/2][y/2]=val;

					}
				}

				pImg += comps;
			}
		}
	}
//	  Serial.println("Displaying");
}

unsigned char JPEGDecoder::pjpeg_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
    JpegDec.pjpeg_need_bytes_callback(pBuf, buf_size, pBytes_actually_read, pCallback_data);
}


unsigned char JPEGDecoder::pjpeg_need_bytes_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
    uint n;

    n = min(g_nInFileSize - g_nInFileOfs, buf_size);

#ifdef SMING_VERSION
    n = fileRead(g_pInFile,pBuf,n);
#else
    g_pInFile.read(pBuf,n);
#endif

    *pBytes_actually_read = (unsigned char)(n);
    g_nInFileOfs += n;
    return 0;
}


int JPEGDecoder::decode(char* pFilename, unsigned char pReduce){
    
    if(pReduce) reduce = pReduce;
#ifdef SMING_VERSION
    g_pInFile =  fileOpen(pFilename, eFO_ReadOnly);
    if (!g_pInFile)
    {
    	Serial.print("Unable to open file. Error: ");
    	Serial.println(fileLastError(g_pInFile));
		Vector<String> list = fileList();
		Serial.println("File List:");
		for (int i = 0; i < list.count(); i++)
			Serial.println(String(fileGetSize(list[i])) + " " + list[i] + "\r\n");

        return -1;
    }

#else

#if defined(SPIFS)
    g_pInFile = SPIFFS.open(pFilename, "r");    //valid entry for file is "/somefile.txt" with slash https://github.com/esp8266/Arduino/blob/master/doc/reference.md#open
#else    
    g_pInFile = SD.open(pFilename, FILE_READ);  //valid entry for file is "somefile.txt"
#endif
    if (!g_pInFile)
        return -1;
#endif

    g_nInFileOfs = 0;
#ifdef SMING_VERSION
    g_nInFileSize =  fileGetSize(pFilename);
#else
    g_nInFileSize = g_pInFile.size();
#endif

    status = pjpeg_decode_init(&image_info, JPEGDecoder::pjpeg_callback, NULL, (unsigned char)reduce);
            
    if (status)
    {
        #ifdef DEBUG
        Serial.print("pjpeg_decode_init() failed with status ");
        Serial.println(status);
        
        if (status == PJPG_UNSUPPORTED_MODE)
        {
            Serial.println("Progressive JPEG files are not supported.");
        }
        #endif
        
#ifdef SMING_VERSION
        fileClose(g_pInFile);
#else
        g_pInFile.close();
#endif
        return -1;
    }
    
    // In reduce mode output 1 pixel per 8x8 block.
    decoded_width = reduce ? (image_info.m_MCUSPerRow * image_info.m_MCUWidth) / 8 : image_info.m_width;
    decoded_height = reduce ? (image_info.m_MCUSPerCol * image_info.m_MCUHeight) / 8 : image_info.m_height;

    row_pitch = image_info.m_MCUWidth * image_info.m_comps;
    //pImage = (uint8 *)malloc(image_info.m_MCUWidth * image_info.m_MCUHeight * image_info.m_comps);
    pImage = new uint8[image_info.m_MCUWidth * image_info.m_MCUHeight * image_info.m_comps];
    if (!pImage)
    {
#ifdef SMING_VERSION
        fileClose(g_pInFile);
#else
        g_pInFile.close();
#endif

        #ifdef DEBUG
        Serial.println("Memory Allocation Failure");
        #endif
        
        return -1;
    }
    memset(pImage , 0 , sizeof(pImage));

    row_blocks_per_mcu = image_info.m_MCUWidth >> 3;
    col_blocks_per_mcu = image_info.m_MCUHeight >> 3;
    
    is_available = 1 ;

    width = decoded_width;
    height = decoded_height;
    comps = image_info.m_comps;
    MCUSPerRow = image_info.m_MCUSPerRow;
    MCUSPerCol = image_info.m_MCUSPerCol;
    scanType = image_info.m_scanType;
    MCUWidth = image_info.m_MCUWidth;
    MCUHeight = image_info.m_MCUHeight;
    
    return decode_mcu();
}


int JPEGDecoder::decode_mcu(void){

    status = pjpeg_decode_mcu();
    
    if (status)
    {
        is_available = 0 ;

#ifdef SMING_VERSION
        fileClose(g_pInFile);
#else
        g_pInFile.close();
#endif


        if (status != PJPG_NO_MORE_BLOCKS)
        {
            #ifdef DEBUG
            Serial.print("pjpeg_decode_mcu() failed with status ");
            Serial.println(status);
            #endif
            
            delete pImage;
            return -1;
        }
    }
    return 1;
}


int JPEGDecoder::read(void)
{
    int y, x;
    uint8 *pDst_row;
    
    if(is_available == 0) return 0;

    if (mcu_y >= image_info.m_MCUSPerCol)
    {
        delete pImage;
#ifdef SMING_VERSION
        fileClose(g_pInFile);
#else
        g_pInFile.close();
#endif

        return 0;
    }

    if (reduce)
    {
        // In reduce mode, only the first pixel of each 8x8 block is valid.
        pDst_row = pImage;
        if (image_info.m_scanType == PJPG_GRAYSCALE)
        {
            *pDst_row = image_info.m_pMCUBufR[0];
        }
        else
        {
            //uint y, x;
            for (y = 0; y < col_blocks_per_mcu; y++)
            {
                uint src_ofs = (y * 128U);
                for (x = 0; x < row_blocks_per_mcu; x++)
                {
                    pDst_row[0] = image_info.m_pMCUBufR[src_ofs];
                    pDst_row[1] = image_info.m_pMCUBufG[src_ofs];
                    pDst_row[2] = image_info.m_pMCUBufB[src_ofs];
                    pDst_row += 3;
                    src_ofs += 64;
                }

                pDst_row += row_pitch - 3 * row_blocks_per_mcu;
            }
        }
    }
    else
    {
        // Copy MCU's pixel blocks into the destination bitmap.
        pDst_row = pImage;
        for (y = 0; y < image_info.m_MCUHeight; y += 8)
        {
            const int by_limit = min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));

            for (x = 0; x < image_info.m_MCUWidth; x += 8)
            {
                uint8 *pDst_block = pDst_row + x * image_info.m_comps;

                // Compute source byte offset of the block in the decoder's MCU buffer.
                uint src_ofs = (x * 8U) + (y * 16U);
                const uint8 *pSrcR = image_info.m_pMCUBufR + src_ofs;
                const uint8 *pSrcG = image_info.m_pMCUBufG + src_ofs;
                const uint8 *pSrcB = image_info.m_pMCUBufB + src_ofs;

                const int bx_limit = min(8, image_info.m_width - (mcu_x * image_info.m_MCUWidth + x));

                if (image_info.m_scanType == PJPG_GRAYSCALE)
                {
                    int bx, by;
                    for (by = 0; by < by_limit; by++)
                    {
                        uint8 *pDst = pDst_block;

                        for (bx = 0; bx < bx_limit; bx++)
                            *pDst++ = *pSrcR++;

                        pSrcR += (8 - bx_limit);

                        pDst_block += row_pitch;
                    }
                }
                else
                {
                    int bx, by;
                    for (by = 0; by < by_limit; by++)
                    {
                        uint8 *pDst = pDst_block;

                        for (bx = 0; bx < bx_limit; bx++)
                        {
                            pDst[0] = *pSrcR++;
                            pDst[1] = *pSrcG++;
                            pDst[2] = *pSrcB++;

                            pDst += 3;
                        }

                        pSrcR += (8 - bx_limit);
                        pSrcG += (8 - bx_limit);
                        pSrcB += (8 - bx_limit);

                        pDst_block += row_pitch;
                    }
                }
            }
            pDst_row += (row_pitch * 8);
        }
    }

    MCUx = mcu_x;
    MCUy = mcu_y;
    
    mcu_x++;
    if (mcu_x == image_info.m_MCUSPerRow)
    {
        mcu_x = 0;
        mcu_y++;
    }

    if (mcu_y == image_info.m_MCUSPerCol)
    {
        mcu_y = 0;
    }    

    if(decode_mcu()==-1) is_available = 0 ;

    return 1;
}

int JPEGDecoder::available(void) {
  return (is_available);
}


