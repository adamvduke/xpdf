//========================================================================
//
// Stream.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef STREAM_H
#define STREAM_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#include <gtypes.h>

#include "Object.h"

//------------------------------------------------------------------------
// Stream (base class)
//------------------------------------------------------------------------

class Stream {
public:

  // Constructor.
  Stream(): ref(1) {}

  // Destructor.
  virtual ~Stream() {}

  // Reference counting.
  int incRef() { return ++ref; }
  int decRef() { return --ref; }

  // Reset stream to beginning.
  virtual void reset() = 0;

  // Get next char from stream.
  virtual int getChar() = 0;

  // Peek at next char in stream.
  virtual int lookChar() = 0;

  // Get next line from stream.
  virtual char *getLine(char *buf, int size);

  // Get current position in file.
  virtual int getPos() = 0;

  // Go to a position in the stream.
  virtual void setPos(int pos1);

  // Get PostScript command for the filter(s).
  virtual GString *getPSFilter(char *indent);

  // Does this stream type potentially contain non-printable chars?
  virtual GBool isBinary(GBool last = gTrue) = 0;

  // Get the base FileStream or SubStream of this stream.
  virtual Stream *getBaseStream() = 0;

  // Get the base file of this stream.
  virtual FILE *getFile() = 0;

  // Get the dictionary associated with this stream.
  virtual Dict *getDict() = 0;

  // Is this an encoding filter?
  virtual GBool isEncoder() { return gFalse; }

  // Add filters to this stream according to the parameters in <dict>.
  // Returns the new stream.
  Stream *addFilters(Object *dict);

private:

  Stream *makeFilter(char *name, Stream *str, Object *params);

  int ref;			// reference count
};

//------------------------------------------------------------------------
// FileStream
//------------------------------------------------------------------------

class FileStream: public Stream {
public:

  FileStream(FILE *f1, int start1, int length1, Object *dict1);
  virtual ~FileStream();
  virtual void reset();
  virtual int getChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr++ & 0xff); }
  virtual int lookChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr & 0xff); }
  virtual int getPos() { return bufPos + (bufPtr - buf); }
  virtual void setPos(int pos1);
  virtual GBool isBinary(GBool last = gTrue) { return last; }
  virtual Stream *getBaseStream() { return this; }
  virtual FILE *getFile() { return f; }
  virtual Dict *getDict() { return dict.getDict(); }

  // Check for a PDF header on this stream.  Skip past some garbage
  // if necessary.
  GBool checkHeader();

  // Get position of first byte of stream within the file.
  int getStart() { return start; }

private:

  GBool fillBuf();

  FILE *f;
  int start;
  int length;
  char buf[256];
  char *bufPtr;
  char *bufEnd;
  int bufPos;
  int savePos;
  Object dict;
};

//------------------------------------------------------------------------
// SubStream
//------------------------------------------------------------------------

class SubStream: public Stream {
public:

  SubStream(Stream *str1, Object *dict1);
  virtual ~SubStream();
  virtual void reset() {}
  virtual int getChar() { return str->getChar(); }
  virtual int lookChar() { return str->lookChar(); }
  virtual int getPos() { return str->getPos(); }
  virtual GBool isBinary(GBool last = gTrue) { return last; }
  virtual Stream *getBaseStream() { return this; }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return dict.getDict(); }

private:

  Stream *str;
  Object dict;
};

//------------------------------------------------------------------------
// ASCIIHexStream
//------------------------------------------------------------------------

class ASCIIHexStream: public Stream {
public:

  ASCIIHexStream(Stream *str1);
  virtual ~ASCIIHexStream();
  virtual void reset();
  virtual int getChar()
    { int c = lookChar(); buf = EOF; return c; }
  virtual int lookChar();
  virtual int getPos() { return str->getPos(); }
  virtual GString *getPSFilter(char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;
  int buf;
  GBool eof;
};

//------------------------------------------------------------------------
// ASCII85Stream
//------------------------------------------------------------------------

class ASCII85Stream: public Stream {
public:

  ASCII85Stream(Stream *str1);
  virtual ~ASCII85Stream();
  virtual void reset();
  virtual int getChar()
    { int c = lookChar(); ++index; return c; }
  virtual int lookChar();
  virtual int getPos() { return str->getPos(); }
  virtual GString *getPSFilter(char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;
  Gulong c[5];
  Gulong b[4];
  int index, n;
  GBool eof;
};

//------------------------------------------------------------------------
// LZWStream
//------------------------------------------------------------------------

class LZWStream: public Stream {
public:

  LZWStream(Stream *str1, int predictor1, int columns1, int colors1,
	    int bits1, int early1);
  virtual ~LZWStream();
  virtual void reset();
  virtual int getChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr++ & 0xff); }
  virtual int lookChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr & 0xff); }
  virtual int getPos() { return str->getPos(); }
  virtual GString *getPSFilter(char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;			// stream
  int predictor;		// parameters
  int columns;
  int colors;
  int bits;
  int early;
  char zCmd[256];		// uncompress command
  FILE *zPipe;			// uncompress pipe
  char *zName;			// .Z file name (in zCmd)
  int inputBuf;			// input buffer
  int inputBits;		// number of bits in input buffer
  int inCodeBits;		// size of input code
  char buf[256];		// buffer
  char *bufPtr;			// next char to read
  char *bufEnd;			// end of buffer

  void dumpFile(FILE *f);
  int getCode();
  GBool fillBuf();
};

//------------------------------------------------------------------------
// RunLengthStream
//------------------------------------------------------------------------

class RunLengthStream: public Stream {
public:

  RunLengthStream(Stream *str1);
  virtual ~RunLengthStream();
  virtual void reset();
  virtual int getChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr++ & 0xff); }
  virtual int lookChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr & 0xff); }
  virtual int getPos() { return str->getPos(); }
  virtual GString *getPSFilter(char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;
  char buf[128];		// buffer
  char *bufPtr;			// next char to read
  char *bufEnd;			// end of buffer
  GBool eof;

  GBool fillBuf();
};

//------------------------------------------------------------------------
// CCITTFaxStream
//------------------------------------------------------------------------

struct CCITTCodeTable;

class CCITTFaxStream: public Stream {
public:

  CCITTFaxStream(Stream *str1, int encoding1, GBool byteAlign1,
		 int columns1, int rows1, GBool black1);
  virtual ~CCITTFaxStream();
  virtual void reset();
  virtual int getChar()
    { int c = lookChar(); buf = EOF; return c; }
  virtual int lookChar();
  virtual int getPos() { return str->getPos(); }
  virtual GString *getPSFilter(char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;			// stream
  int encoding;			// 'K' parameter
  GBool byteAlign;		// 'EncodedByteAlign' parameter
  int columns;			// 'Columns' parameter
  int rows;			// 'Rows' parameter
  GBool black;			// 'BlackIs1' parameter
  GBool eof;			// true if at eof
  int inputBuf;			// input buffer
  int inputBits;		// number of bits in input buffer
  short *refLine;		// reference line changing elements
  int b1;			// index into refLine
  short *codingLine;		// coding line changing elements
  int a0;			// index into codingLine
  int outputBits;		// remaining ouput bits
  int buf;			// character buffer

  short getCode(CCITTCodeTable *table);
  int getBit();
};

//------------------------------------------------------------------------
// DCTStream
//------------------------------------------------------------------------

// DCT component info
struct DCTCompInfo {
  int id;			// component ID
  GBool inScan;			// is this component in the current scan?
  int hSample, vSample;		// horiz/vert sampling resolutions
  int quantTable;		// quantization table number
  int dcHuffTable, acHuffTable;	// Huffman table numbers
  int prevDC;			// DC coefficient accumulator
};

// DCT Huffman decoding table
struct DCTHuffTable {
  Guchar firstSym[17];		// first symbol for this bit length
  Gushort firstCode[17];	// first code for this bit length
  Gushort numCodes[17];		// number of codes of this bit length
  Guchar sym[256];		// symbols
};

class DCTStream: public Stream {
public:

  DCTStream(Stream *str1);
  virtual ~DCTStream();
  virtual void reset();
  virtual int getChar();
  virtual int lookChar();
  virtual int getPos() { return str->getPos(); }
  virtual GString *getPSFilter(char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;			// stream
  int width, height;		// image size
  int mcuWidth, mcuHeight;	// size of min coding unit, in data units
  DCTCompInfo compInfo[4];	// info for each component
  int numComps;			// number of components in image
  int colorXform;		// need YCbCr-to-RGB transform?
  int restartInterval;		// restart interval, in MCUs
  Guchar quantTables[4][64];	// quantization tables
  int numQuantTables;		// number of quantization tables
  DCTHuffTable dcHuffTables[4];	// DC Huffman tables
  DCTHuffTable acHuffTables[4];	// AC Huffman tables
  int numDCHuffTables;		// number of DC Huffman tables
  int numACHuffTables;		// number of AC Huffman tables
  Guchar *rowBuf[4][32];	// buffer for one MCU
  int comp, x, y, dy;		// current position within image/MCU
  int restartCtr;		// MCUs left until restart
  int restartMarker;		// next restart marker
  int inputBuf;			// input buffer for variable length codes
  int inputBits;		// number of valid bits in input buffer

  void restart();
  GBool readMCURow();
  GBool readDataUnit(DCTHuffTable *dcHuffTable, DCTHuffTable *acHuffTable,
		     Guchar quantTable[64], int *prevDC, Guchar data[64]);
  int readHuffSym(DCTHuffTable *table);
  int readAmp(int size);
  int readBit();
  GBool readHeader();
  GBool readFrameInfo();
  GBool readScanInfo();
  GBool readQuantTables();
  GBool readHuffmanTables();
  GBool readRestartInterval();
  GBool readAdobeMarker();
  GBool readTrailer();
  int readMarker();
  int read16();
};

//------------------------------------------------------------------------
// FlateStream
//------------------------------------------------------------------------

#define flateWindow          32768    // buffer size
#define flateMask            (flateWindow-1)
#define flateMaxHuffman         15    // max Huffman code length
#define flateMaxCodeLenCodes    19    // max # code length codes
#define flateMaxLitCodes       286    // max # literal codes
#define flateMaxDistCodes       30    // max # distance codes

// Huffman code table entry
struct FlateCode {
  int len;			// code length in bits
  int code;			// code word
  int val;			// value represented by this code
};

// Huffman code table
struct FlateHuffmanTab {
  int start[flateMaxHuffman+2];	// indexes of first code of each length
  FlateCode *codes;		// codes, sorted by length and code word
};

// Decoding info for length and distance code words
struct FlateDecode {
  int bits;			// # extra bits
  int first;			// first length/distance
};

class FlateStream: public Stream {
public:

  FlateStream(Stream *str1);
  virtual ~FlateStream();
  virtual void reset();
  virtual int getChar();
  virtual int lookChar();
  virtual int getPos() { return str->getPos(); }
  virtual GString *getPSFilter(char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;			// stream
  Guchar buf[flateWindow];	// output data buffer
  int index;			// current index into output buffer
  int remain;			// number valid bytes in output buffer
  int codeBuf;			// input buffer
  int codeSize;			// number of bits in input buffer
  FlateCode			// literal and distance codes
    allCodes[flateMaxLitCodes + flateMaxDistCodes];
  FlateHuffmanTab litCodeTab;	// literal code table
  FlateHuffmanTab distCodeTab;	// distance code table
  GBool compressedBlock;	// set if reading a compressed block
  int blockLen;			// remaining length of uncompressed block
  GBool endOfBlock;		// set when end of block is reached
  GBool eof;			// set when end of stream is reached

  static int			// code length code reordering
    codeLenCodeMap[flateMaxCodeLenCodes];
  static FlateDecode		// length decoding info
    lengthDecode[flateMaxLitCodes-257];
  static FlateDecode		// distance decoding info
    distDecode[flateMaxDistCodes];

  void readSome();
  GBool startBlock();
  void loadFixedCodes();
  GBool readDynamicCodes();
  void compHuffmanCodes(FlateHuffmanTab *tab, int n);
  int getHuffmanCodeWord(FlateHuffmanTab *tab);
  int getCodeWord(int bits);
};

//------------------------------------------------------------------------
// EOFStream
//------------------------------------------------------------------------

class EOFStream: public Stream {
public:

  EOFStream(Stream *str1);
  virtual ~EOFStream();
  virtual void reset() {}
  virtual int getChar() { return EOF; }
  virtual int lookChar() { return EOF; }
  virtual int getPos() { return str->getPos(); }
  virtual GString *getPSFilter(char *indent)  { return NULL; }
  virtual GBool isBinary(GBool last = gTrue) { return gFalse; }
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }

private:

  Stream *str;
};

//------------------------------------------------------------------------
// FixedLengthEncoder
//------------------------------------------------------------------------

class FixedLengthEncoder: public Stream {
public:

  FixedLengthEncoder(Stream *str1, int length1);
  ~FixedLengthEncoder();
  virtual void reset();
  virtual int getChar();
  virtual int lookChar();
  virtual int getPos() { return str->getPos(); }
  virtual GString *getPSFilter(char *indent) { return NULL; }
  virtual GBool isBinary(GBool last = gTrue) { return gFalse; }
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }
  virtual GBool isEncoder() { return gTrue; }

private:

  Stream *str;
  int length;
  int count;
};

//------------------------------------------------------------------------
// ASCII85Encoder
//------------------------------------------------------------------------

class ASCII85Encoder: public Stream {
public:

  ASCII85Encoder(Stream *str1);
  virtual ~ASCII85Encoder();
  virtual void reset();
  virtual int getChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr++ & 0xff); }
  virtual int lookChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr & 0xff); }
  virtual int getPos() { return str->getPos(); }
  virtual GString *getPSFilter(char *indent) { return NULL; }
  virtual GBool isBinary(GBool last = gTrue) { return gFalse; }
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }
  virtual GBool isEncoder() { return gTrue; }

private:

  Stream *str;
  char buf[8];
  char *bufPtr;
  char *bufEnd;
  int lineLen;
  GBool eof;

  GBool fillBuf();
};

//------------------------------------------------------------------------
// RunLengthEncoder
//------------------------------------------------------------------------

class RunLengthEncoder: public Stream {
public:

  RunLengthEncoder(Stream *str1);
  virtual ~RunLengthEncoder();
  virtual void reset();
  virtual int getChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr++ & 0xff); }
  virtual int lookChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr & 0xff); }
  virtual int getPos() { return str->getPos(); }
  virtual GString *getPSFilter(char *indent) { return NULL; }
  virtual GBool isBinary(GBool last = gTrue) { return gFalse; }
  virtual Stream *getBaseStream() { return str->getBaseStream(); }
  virtual FILE *getFile() { return str->getFile(); }
  virtual Dict *getDict() { return str->getDict(); }
  virtual GBool isEncoder() { return gTrue; }

private:

  Stream *str;
  char buf[131];
  char *bufPtr;
  char *bufEnd;
  char *nextEnd;
  GBool eof;

  GBool fillBuf();
};

#endif
