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

using namespace std;
#include "encoder.h"
extern "C" {
#include <lame/lame.h>
}

bool MP3 :: encodePCM(const string pcmInput, const string path, MP3 &lameobj ){
    string absolute_fpath = path + "/" + pcmInput;
    std::ifstream wav;
    std::ofstream mp3;
    const string ext = {"mp3"};
    
    wav.exceptions(std::ifstream::failbit);
    mp3.exceptions(std::ifstream::failbit);
    
    const size_t bytes_per_sample = 2 * sizeof(int16_t);
    
    const size_t PCM_SIZE = 8192;
    const size_t MP3_SIZE = 8192;
    int16_t pcm_buffer[PCM_SIZE * 2];
    
    
    
    string output(absolute_fpath);
    output.replace(output.end() - ext.length(), output.end(), ext);
    
    
    wav.exceptions(std::ifstream::failbit);
    try {
        wav.open(absolute_fpath, std::ios_base::binary);
        mp3.open(output, std::ios_base::binary);
        cout << "input file stream has no errors" <<endl ;
        
        }
     catch (std::ifstream::failure e) {
            cout << "Error opening input/output file: " << std::strerror(errno) << endl;
         return false;
         
        }
    wav.seekg (0, wav.end);
    size_t length = wav.tellg();
    wav.seekg (0, wav.beg);
    unsigned char mp3_buffer[length];
    char * buffer = new char [length];
    std::cout << "Reading " << length << " characters... " << endl;
    // read data as a block:
    wav.read (reinterpret_cast<char*> (buffer),length);
    cout << "wav buffer size  after read is " << sizeof(buffer) << endl;
    
    int write = 0;
    std::cout << "Read after " << length << " characters... " << endl ;
    
    write = lame_encode_buffer_interleaved(lameobj.gfp,reinterpret_cast<int16_t *> (buffer), bytes_per_sample, mp3_buffer, length);
    
    std::cout << "encoded " << write << " characters... " << endl;
    
    write = lame_encode_flush(lameobj.gfp, reinterpret_cast<int16_t *> (buffer) , length);
    
    mp3.write(reinterpret_cast<char*>(mp3_buffer), write);
    
    cout << "MP3 buffer size is " << sizeof(mp3_buffer) << endl;

    wav.close();
    mp3.close();
    
    /*while (wav.good()) {
        int write = 0;
        wav.read(reinterpret_cast<char*>(pcm_buffer), sizeof(pcm_buffer));
        size_t read = wav.gcount()/ bytes_per_sample;
        
        cout << "read " << read << "  bytes" << endl;
        
        
        if (read == 0)
        {
            cout << "Read is now zero, flushing " << endl;
            write = lame_encode_flush(MP.gfp, mp3_buffer, MP3_SIZE);
        }
        else
        {
           
           write = lame_encode_buffer_interleaved(MP.gfp, pcm_buffer, read, mp3_buffer, MP3_SIZE);
           cout << "in else block writing PCM data " << write  << "  bytes written  "<< endl;
           mp3.write(reinterpret_cast<char*>(mp3_buffer), write);
        
        }
    }*/
    return true;
    
}

int main(int argc, const char * argv[])
{
    MP3 MP;
    MP.setSamplerate(44100);
    MP.setBitrate(128);
    MP.setLameMode(0);
    MP.setNumchannels(2);
    
    std::cout << "Hello, this should create Lame mp3 encoder!\n";
    
    lame_set_quality(MP.gfp,5);
    //lame_set_mode(MP.gfp, DUAL_CHANNEL);
    cout << "MPEG mode is " << lame_get_mode(MP.gfp)<< '\n';
    
    if (int ret_code = lame_init_params(MP.gfp) == -1)
        cout << "lame initalized failed";
    
    //Dir handling and filtering wav files
    DIR * dir;
    struct dirent *ent;
    
    vector<string> wavFiles;

    static int numFiles = 0;
    
    if ((dir = opendir(argv[1])) != NULL) {
        /* print all the wav files (only) within the given directory */
        
        while ((ent = readdir (dir)) != NULL) {
            if (MP.checkfileExtension(ent->d_name) == true){
            wavFiles.push_back(ent->d_name);
            numFiles++;
            }
        }
        cout << "Total wav files in the dir are " << numFiles << endl;
        
        
    closedir (dir);
 
        //int numofBytes = lame_encode_buffer(MP.gfp, short int leftpcm[], short int rightpcm[], int num_samples,char *mp3buffer,int  mp3buffer_size);
        
    } else {
        /* could not open directory */
        perror ("");
        return EXIT_FAILURE;
    }
  
    if(argc != 2) {
        cout << "You need to supply one argument to this program.";
        return -1;
    }
    
    string path = argv[1];
    
    MP.encodePCM(wavFiles[0], path, MP);
    return 0;
    
}



