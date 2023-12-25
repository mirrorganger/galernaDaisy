// This example uses fatfs to access files via SD Card.
//
// A File test.txt is created with the contentt of outbuff.
// The file is then read back in to inbuff.
// If there is any failure, or what was read does not match
// what was written, execution will halt at the breakpoint.
// Otherwise, the Seed's onboard LED will begin to blink.
//
// FATFS is not bundled into the libdaisy, and therefore needs
// to be included in the project to work
//
// The SD implementation of libdaisy is subject to change,
// however wrapping fatfs into libdaisy would be a substantial
// chunk of work, and FatFs itself is a pretty familiar interface.
//
// This particular project will work as long as the following Middleware
// files are included in the project (alongside libdaisy).
// Sources/headers within: libdaisy/Middlewares/Third_Party/FatFs/src/
// options: ccsbcs.c (in src/options within the above path)
//
// The SD card is currently configured to run with 4-bits and a 12MHz clock
// frequency.
//
#include "daisy_seed.h"
#include "fatfs.h"
#include <stdio.h>
#include <string.h>
#include "GalernaFileReader.h"
#define TEST_FILE_NAME "a_test.txt"

using namespace daisy;

static DaisySeed hw;
static galernaUtils::GalernaFileReader<4096> fileReader;

SdmmcHandler sd;
FatFSInterface fsi;
// FIL file;

const char *test_string = "this is a sample this is a sample this is a sample this is a sample this is a sample this is a sample";

void logWavInfo(const WavFileInfo& wavInfo){
   hw.PrintLine("wav file name      :\t%s", wavInfo.name);
   hw.PrintLine("wav file SR        :\t%d", wavInfo.raw_data.SampleRate);
   hw.PrintLine("wav file BPS       :\t%d", wavInfo.raw_data.BitPerSample);
   hw.PrintLine("wav file Channels  :\t%d", wavInfo.raw_data.NbrChannels);
}

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    for(size_t i = 0; i < size; i += 2)
    {
        out[i] = out[i + 1] = s162f(fileReader.getNextSample()) * 0.5f;
    }
}

void printWavFormat(const galernaUtils::WavFormat& format){
  hw.PrintLine("Filesize %d", format.FileSize);
  hw.PrintLine("SubCHunk2Size %d", format.SubCHunk2Size);
  hw.PrintLine("NbrChannels %d", format.NbrChannels);
  hw.PrintLine("SampleRate %d", format.SampleRate);
  hw.PrintLine("BitPerSample %d", format.BitPerSample);
}

int main(void) {

  constexpr size_t blocksize = 64; 

  hw.Init();
  hw.StartLog(true);

  /** SD card next */
  SdmmcHandler::Config sd_config;
  SdmmcHandler sdcard;
  sd_config.Defaults();
  sd_config.speed = SdmmcHandler::Speed::STANDARD;
  sd_config.width = SdmmcHandler::BusWidth::BITS_4;
  sdcard.Init(sd_config);
  fsi.Init(FatFSInterface::Config::MEDIA_SD);

  const char *wave_file_name = "pcm.wav";
  FRESULT fres = FR_DENIED; 
  fres = f_mount(&fsi.GetSDFileSystem(), "/", 1);
  if(fres!=F_OK){
    hw.PrintLine("Error mouting %d",fres);
  }
  auto printer = [&](const char* str){hw.PrintLine("%s",str);};
  fileReader.init(printer);
  if(fileReader.open(wave_file_name));
  // printWavFormat(fileReader.getFormat());
  fileReader.stream();

  hw.SetAudioBlockSize(blocksize);
  // hw.StartAudio(AudioCallback);
  while(1) {
    hw.DelayMs(1);
    fileReader.loadBuffer();
    fileReader.getNextSample();
  }
}
