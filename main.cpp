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
#include <thread>
#include <inttypes.h>
#include <chrono>
#include <cstdlib>

bool MP3 :: encodePCM_thread(void *arglist){
    
    static pthread_mutex_t encoderStart = PTHREAD_MUTEX_INITIALIZER;
   
    std::thread::id this_id = std::this_thread::get_id();
     //((targ*)arglist)->threadvector = this_id;
    //bool status = this_id.joinable();
    cout << "this thread start of the loop is " << this_id << endl;

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
    pthread_mutex_lock(&encoderStart);
    cout << "after fwrite  ftell mp3 is before loop start is  " << ftell(mp3) << endl;
    
    std::fseek(mp3, 0, SEEK_END);
    std::size_t filesize = std::ftell(mp3);
    cout << "Filesize mp3 is " << filesize <<endl;
    
    std::fseek(wav, 0, SEEK_END);
    filesize = std::ftell(wav);
    cout << "Filesize wav is " << filesize <<endl;


   
    const size_t fread_sector = 2 * sizeof (short int) * WAV_BUFFER_SIZE;

    //size_t threadbuffer = (filesize/coreCount);
    //cout << "Thread buffer is" << threadbuffer << endl;




    size_t threadbuffer_tail = 0;
    static size_t bufer_per_core_tail =0;
    /*Prepare chunk for each Thread*/
    size_t num_fread_chunks = filesize/fread_sector;
    size_t bufer_per_core = (num_fread_chunks/coreCount) *fread_sector;
    cout << "bufer_per_core before is  " << bufer_per_core << endl;

    if (filesize % fread_sector == 0){
    	bufer_per_core_tail = bufer_per_core;
    cout << "bufer_per_core is  " << bufer_per_core << endl;
    }

    else{
    	bufer_per_core_tail  = bufer_per_core + (filesize % fread_sector);
    	cout << "bufer_per_core_tail is  " << bufer_per_core_tail << endl;
    }
    cout << "num_fread_chunks  " << num_fread_chunks << endl;



    /*if (filesize % coreCount ==0)
        size_t threadbuffer_tail = threadbuffer;
    else
        threadbuffer_tail = (filesize - (coreCount-1)*threadbuffer);
    //cout <<"threadbuffer is " << threadbuffer << "  threadbuffer tail is " << threadbuffer_tail;*/
    
    size_t read{1};
    int32_t write{0};
    
    
    //static std::streamsize s;
    
    short int wav_buffer[WAV_BUFFER_SIZE * 2];
    
    unsigned char mp3_buffer[MP3_BUFFER_SIZE];

    //static bool stopread = false;
    
    //const size_t bytes_per_sample = 1 * sizeof(int16_t);
    
    //cout << "MPEG mode inside function is " << lame_get_mode(((targ*)arglist)->gfp)<< "  and bitrate is  " << lame_get_brate(lameobj.gfp)<< '\n';
    std::fseek(wav, 0, SEEK_SET);
    static int mp3_len =0;
   
    static int access = 0;
    
    cout << "access is" << access << endl ;
    cout << "read is " << read << endl;
   
    /*This thread id store it in a vector if vector_tid [0] -->Process file from shared pointer vector [0]
    If vector_tid[0].joinable() then start vector_tid[1]/
    check if last thread then exit out of while loop */

    if (access ==0){
    std::fseek(wav, 0, SEEK_SET);
        cout <<"setting fseek to start" << endl;
    }
    if (access ==1){
        std::fseek(wav, bufer_per_core, SEEK_SET);
        cout <<"setting fseek to 1" << endl;
    }
    if (access ==2){
        std::fseek(wav, 2*bufer_per_core, SEEK_SET);
        cout <<"setting fseek to 2" << endl;
    }
    if (access ==3){
        std::fseek(wav, 3*bufer_per_core, SEEK_SET);
        cout <<"setting fseek to 3" << endl;
    }
      do{
        using namespace std::this_thread;
        cout << "this thread id in loop is " << this_id << endl;
        cout << "ftell(wav) before fread is  " << ftell(wav) << endl;
        read = std::fread(wav_buffer,  2 * sizeof (short int) , WAV_BUFFER_SIZE, wav);
      
        
        if (read == 0) {
            cout << "read ==0 " << endl;
            
        flush: {
            //if (access ==2)
            write = lame_encode_flush(((targ*)arglist)->gfp, mp3_buffer, MP3_BUFFER_SIZE);
            cout << "coming out of unlock" << endl;
            cout << "mp3 len is" << mp3_len << endl;
            access++;
            pthread_mutex_unlock(&encoderStart);
            return 0;
            
        }
            
        } else {
            //Interleaved mode available from LAME 3.100 ver
            
            write = lame_encode_buffer_interleaved(((targ*)arglist)->gfp, wav_buffer, read, mp3_buffer, MP3_BUFFER_SIZE);
            cout << "ftell before write is and " << ftell(wav) << " and mp3 write is" << write << "bytes" <<endl;
            mp3_len += write;
            }
        
        /* Pseudo code
         * check if Totalfilesize % #cores ==0 (Modulo)
         * if not then assign last thread the last chunk amount of bytes from the wav file
         * Prepare Nbytes for each thread
         *NBytes = Total filesize/ #cores (CN)
         *Thread 1 -fix CPU affinity to work only on NBytes (#chunk 1)
         *Read only NBytes from wav file into mp3buffer
         *Advance or move the static shared pointer to NBytes (Next thread should read from offset)
         *Lock and wait until thread write all buffer to mp3buffer (meaning encode call done for Nbytes)
         *Thread 2- fix CPU affinity to work only on NBytes (#chunk 2)
         *Move the pointer offset to Nbytes and advance it to next Nbytes+
         *Lock and wait until Thread 2 write all buffer to mp3buffer
         *continue for the next threads
         *
         *call lame_encode_for only N Bytes
         *check fwrite only after CN*N Bytes are processed (alternatively read==0)
         *Finally write into mp3 file lock this until all the threads are done (make a call to joinable before fwrite)
         *
         *
         *
         * */
        //cout << "writing with ftell "<< ftell(wav) << endl;
    write:{
    cout << "writing with "<< write << "bytes " << " with ftell " << ftell(wav)<< endl;
    
    fwrite(mp3_buffer, write, 1, mp3);
    cout << "after fwrite  ftell mp3 is " << ftell(mp3) << endl;
   
    }
          
          if (ftell(wav) >= (bufer_per_core) && access ==0){
              cout << "ftell inside first thread is thread access 0  is " << ftell(wav) << endl;
              goto flush;
            }
          if (ftell(wav) >= (2*bufer_per_core) && access ==1){
              cout << "ftell inside second thread is thread access 1 is " << ftell(wav) << endl;
              goto flush;
          }
          if (ftell(wav) >= (3*bufer_per_core) && access ==2){
              cout << "ftell inside third thread is thread access 2 is " << ftell(wav) << endl;
              goto flush;
          }
          if (ftell(wav) >= filesize && access ==3){
              cout << "ftell inside fourth thread is thread access 3 is " << ftell(wav) << endl;
              goto flush;
          }
    } while(read !=0);
    std::this_thread::sleep_for (std::chrono::milliseconds(900));
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
    
    //check if lame init fails (hopefully not :) )
    if (int ret_code = lame_init_params(MP.gfp) == -1)
        cout << "lame initalized failed";
    
    //Dir handling and filtering wav files
    DIR * dir;
    struct dirent *ent;
    static int numFiles = 0;
    if ((dir = opendir(argv[1])) != NULL) {
        /*  parse within the given directory & add to string file vector all the wav files (only)  */
        
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
    
    MP.abs_path = argv[1];
    MP3 spawn(MP);
    
#if(0)
    std::thread t1(MP3::encodePCM_thread, arg);
    std::cout << "t1 before starting, joinable: " << std::boolalpha << t1.joinable() <<endl;
    //t1.join();
    std::cout << "t1 after starting, joinable: " << std::boolalpha << t1.joinable() <<endl;
    
    
    std::thread t2(MP3::encodePCM_thread, arg);
    std::cout << "t2 before starting, joinable: " << std::boolalpha << t2.joinable() <<endl;
    //t2.join();
    std::cout << "t2 after starting, joinable: " << std::boolalpha << t2.joinable() <<endl;
    
    
    std::thread t3(MP3::encodePCM_thread, arg);
    std::cout << "t3 before starting, joinable: " << std::boolalpha << t3.joinable() <<endl;
    t1.join();
    t2.join();
    t3.join();
    std::cout << "t3 after starting, joinable: " << std::boolalpha << t3.joinable() <<endl;
#endif
    
  
     return 0;
}
