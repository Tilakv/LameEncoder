//
//  main.cpp
//  Lame_MP3_Enocder
//
//  Created by Tilak on 05.09.21.
//  Copyright © 2021 Tilak. All rights reserved.
//

#include <iostream>
#include <dirent.h>
#include <string>
#include <vector>
#include <fstream>
#include "wav_parser.h"
using namespace std;
#include "encoder.h"
extern "C" {
#include <lame/lame.h>
}
#include <pthread.h>
bool MP3 :: encodePCM_thread(void *arglist){
    static constexpr auto WAV_BUFFER_SIZE = 8192;
    static constexpr auto MP3_BUFFER_SIZE = 8192;
    const string path = ((targ*)arglist)->abs_path;
    const string pcmInput =((targ*)arglist)->wavFiles[1];
    string absolute_fpath = path + "/" + pcmInput;
    std::FILE *wav = std::fopen(absolute_fpath.c_str(), "rb");
    
    const string ext = {"mp3"};
    string output(absolute_fpath);
    output.replace(output.end() - ext.length(), output.end(), ext);
    
    std::FILE *mp3 = std::fopen(output.c_str(), "wb");
    
    
    size_t read{1};
    int32_t write{0};
    
    //static int count = 0;
    //static std::streamsize s;
    short int wav_buffer[WAV_BUFFER_SIZE * 2];
    unsigned char mp3_buffer[MP3_BUFFER_SIZE];
    const size_t bytes_per_sample = 1 * sizeof(int16_t);
    //cout << "MPEG mode inside function is " << lame_get_mode(((targ*)arglist)->gfp)<< "  and bitrate is  " << lame_get_brate(lameobj.gfp)<< '\n';
    while (read != 0){
        
        read = std::fread(wav_buffer, 2 * sizeof (short int), WAV_BUFFER_SIZE, wav);
        //read = read / bytes_per_sample;
        //cout << "reading  "<< read << "bytes each one size " << 2 * sizeof (wav_buffer)<< endl;
        if (read == 0) {
            cout << "Finally flushing " << endl;
            write = lame_encode_flush(((targ*)arglist)->gfp, mp3_buffer, MP3_BUFFER_SIZE);
        } else {
            
            //Interleaved mode available from LAME 3.100 ver
            
            write = lame_encode_buffer_interleaved(((targ*)arglist)->gfp, wav_buffer, read, mp3_buffer, MP3_BUFFER_SIZE);
            //cout << "writing to mp3  " << write << "bytes" << endl;
        }
        
        fwrite(mp3_buffer, write, 1, mp3);
    }
    fclose(mp3);
    fclose(wav);
    lame_close(((targ*)arglist)->gfp);
    cout << "MP3 buffer size is " << sizeof(mp3_buffer) << endl;
    
    return true;
    
}

int main(int argc, const char * argv[])
{
    if(argc != 2) {
        cout << "Please provide wav dir path as argument to this program.";
        return -1;
    }
    MP3 MP;
    MP.setSamplerate(48000);
    //MP.getSamplerate();
    MP.setBitrate(64);
    //MP.setLameMode(0);
    //MP.setNumchannels(2);
    lame_set_mode(MP.gfp, STEREO);
    lame_set_VBR(MP.gfp, vbr_default);
    lame_set_VBR_q(MP.gfp, 5);
    std::cout << "Hello, this should create Lame mp3 encoder!\n";
    lame_set_quality(MP.gfp,2);
    cout << "MPEG mode is " << lame_get_mode(MP.gfp)<< '\n';
    cout << "quality is " << MP.lamegetQuality()<< '\n';
    
    //check if lame initn fails (hopefully not :) )
    if (int ret_code = lame_init_params(MP.gfp) == -1)
        cout << "lame initalized failed";
    
    //Dir handling and filtering wav files
    DIR * dir;
    struct dirent *ent;
    static int numFiles = 0;
    if ((dir = opendir(argv[1])) != NULL) {
        /* add to string file vector all the wav files (only) & parse within the given directory */
        
        while ((ent = readdir (dir)) != NULL) {
            if (MP.checkfileExtension(ent->d_name) == true && check_wav(reinterpret_cast<char*>(&ent->d_name), argv[1])== 0) {
            MP.wavFiles.push_back(ent->d_name);
            numFiles++;
            }
        }
        cout << "Total wav files in the dir are " << numFiles << endl;
        closedir (dir);
        
    } else {
        /* could not open directory */
        perror ("");
        return EXIT_FAILURE;
      
    }
    
    static string path = argv[1];
    void * arg = & MP;
    MP.abs_path = argv[1];
    MP.encodePCM_thread(arg);
    
    return 0;
    
    
  
    
}



