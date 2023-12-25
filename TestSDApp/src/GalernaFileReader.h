

#include "ff.h"
#include <functional>
#include <string.h>
#include <cstdint>
namespace galernaUtils {

/** Constants for In-Header IDs */
const uint32_t kWavFileChunkId     = 0x46464952; /**< "RIFF" */
const uint32_t kWavFileWaveId      = 0x45564157; /**< "WAVE" */
const uint32_t kWavFileSubChunk1Id = 0x20746d66; /**< "fmt " */
const uint32_t kWavFileSubChunk2Id = 0x61746164; /**< "data" */

/** Standard Format codes for the waveform data.
 ** 
 ** According to spec, extensible should be used whenever:
 ** * PCM data has more than 16 bits/sample
 ** * The number of channels is more than 2
 ** * The actual number of bits/sample is not equal to the container size
 ** * The mapping from channels to speakers needs to be specified.
 ** */
enum WavFileFormatCode 
{
    WAVE_FORMAT_PCM        = 0x0001,
    WAVE_FORMAT_IEEE_FLOAT = 0x0003,
    WAVE_FORMAT_ALAW       = 0x0006,
    WAVE_FORMAT_ULAW       = 0x0007,
    WAVE_FORMAT_EXTENSIBLE = 0xFFFE,
};

struct WavFormat
{
    uint32_t ChunkId;       /**< & */
    uint32_t FileSize;      /**< & */
    uint32_t FileFormat;    /**< & */
    uint32_t SubChunk1ID;   /**< & */
    uint32_t SubChunk1Size; /**< & */
    uint16_t AudioFormat;   /**< & */
    uint16_t NbrChannels;   /**< & */
    uint32_t SampleRate;    /**< & */
    uint32_t ByteRate;      /**< & */
    uint16_t BlockAlign;    /**< & */
    uint16_t BitPerSample;  /**< & */
    uint32_t SubChunk2ID;   /**< & */
    uint32_t SubCHunk2Size; /**< & */
};

enum class ReaderState : uint_fast8_t
{
    IDLE = 0,
    PLAYING = 1
};

enum class LoadBufferState : uint_fast8_t{
    IDLE = 0,
    LOADING_LOW = 1,
    LOADING_HIGH = 2
};


template<size_t BufferSize>
class GalernaFileReader
{
public:
  using LoggerCallbackT = std::function<void(const char *)>;

  void init(LoggerCallbackT loggerCallback){ 
    _logerCallback = loggerCallback;
  }
  
  bool open(const char *fileName){
    bool ret = false;
    size_t bytesRead;
    if(checkReponse(f_open(&_fileHandle, fileName, (FA_OPEN_EXISTING | FA_READ)),"Open file") &&
       checkReponse(f_read(&_fileHandle, (void*)(&_wavInfo), sizeof(WavFormat), &bytesRead),"Reaf info")){
      ret = true;
    }
    _bufferState = LoadBufferState::LOADING_LOW;
    moveToDataStart();
    loadBuffer();
    _readPtr = 0;
    return ret;
  }
 
  std::size_t getFileSize() {
    return _wavInfo.FileSize;
  }

  void stream(){
    _state = ReaderState::PLAYING;
  }

  // Gets next sample for pre-loaded buffer  
  uint16_t getNextSample()
  {
    uint16_t sample = 0U;
    switch (_state)
    {
    case ReaderState::PLAYING:
        sample = _buffer[_readPtr];
        incrementReadPtr();
        break;
    case ReaderState::IDLE:
    default:
        break;
    }
    return sample; 
  }

  // Load low/high buffer   
  void loadBuffer(){
    if(_bufferState != LoadBufferState::IDLE){
        _logerCallback("Loading");
        size_t bytesRead = 0U;
        auto halfBufferSize = sizeof(_buffer[0]) * (_buffer.size() / 2U);
        size_t offset = (_bufferState == LoadBufferState::LOADING_LOW) ? 0U : halfBufferSize;  
        checkReponse(f_read(&_fileHandle,&_buffer[offset],halfBufferSize,&bytesRead), "reading ");
        _totalRead += bytesRead;
        if((bytesRead < halfBufferSize) || f_eof(&_fileHandle)){
            _state = ReaderState::IDLE;
            // Continue reading if the half buffer is not full 
            // or we have reached the end of the file (and looping is enabled)
            // in the latter case, seek to begin.
            _logerCallback("End of file");
        }
        // Finished loading one half.
        _bufferState = LoadBufferState::IDLE;
    }
    // Nothing to do
  }       
  WavFormat getFormat() const{
    return _wavInfo;
  }
private:
  using Buffer = std::array<int16_t, BufferSize>;

  void incrementReadPtr(){
    _readPtr = (_readPtr+1U) % _buffer.size();
    if(_readPtr==0U){
        _bufferState = LoadBufferState::LOADING_HIGH;
        _logerCallback("LoadBufferState::LOADING_HIGH");
    }else if(_readPtr==(_buffer.size()/2)){
        _bufferState = LoadBufferState::LOADING_LOW;
        _logerCallback("LoadBufferState::LOADING_LOW");
    }
  }

  // seek start of data 
  void moveToDataStart(){
    checkReponse(f_lseek(&_fileHandle,sizeof(WavFormat) + _wavInfo.SubChunk1Size),"move to data");
  }

  bool checkReponse(FRESULT fres, const char *operationStr) {
    bool ret = false;
    if (fres != FR_OK) {
      char infoStr[30];
      strcpy(infoStr, operationStr);
      strcat(infoStr, " Error");
      char errorCode[4];
      sprintf(errorCode, " %d", fres);
      strcat(infoStr, errorCode);
      _logerCallback(infoStr);
    } else {
      ret = true;
    }
    return ret;
  }

  void printBuffer(){
    for(const auto & sample : _buffer){
        char infoStr[10];
        sprintf(infoStr, " %d", sample);
        _logerCallback(infoStr);
    }
  }

  FIL _fileHandle;
  LoggerCallbackT _logerCallback;
  Buffer _buffer;
  LoadBufferState _bufferState = LoadBufferState::IDLE;
  ReaderState _state = ReaderState::IDLE;  
  uint16_t _readPtr = 0U;
  WavFormat _wavInfo;
  size_t _totalRead = 0;
};

} // namespace galernaUtils