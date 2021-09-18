//
//  encoder.h
//  Lame_MP3_Enocder
//
//  Created by Tilak on 08.09.21.
//  Copyright © 2021 Tilak. All rights reserved.
//
#include <lame/lame.h>
#ifndef encoder_h
#define encoder_h
using namespace std;

class MP3{
    
private:
    size_t sample_rate;
    size_t bit_rate;
    
    
public:
    lame_global_flags * gfp;
    string abs_path;
    vector<string> wavFiles;
    
public:
    MP3()
    {
        gfp = lame_init();
        cout << "Initialized the encoder with Lame " << get_lame_version() << " version" <<endl;
    }
    
    void setSamplerate(size_t samp_rate){
        sample_rate = samp_rate;
        cout << "setting samplerate is " << sample_rate <<endl;
        lame_set_in_samplerate(gfp, sample_rate);
    }
    
    void setBitrate(size_t brate)
    {
        bit_rate = brate;
        lame_set_brate(gfp, bit_rate);
    }
    
    void setLameMode(int  mode)
    {
        lame_set_mode(gfp, static_cast<MPEG_mode>(mode));
    }
    int lamegetQuality()
    {
        cout << "getting quality  ";
        return lame_get_quality(gfp);
    }
    void setNumchannels(int  channel)
    {
        lame_set_num_channels(gfp, channel);
    }
    bool checkfileExtension(const string  filename)
    {
        std::string::size_type idx = NULL;
        
        if(idx != std::string::npos)
        {
            idx = filename.rfind('.');
            std::string extension = filename.substr(idx+1);
            if (extension == "wav")
            {
                cout << "Found wav extension" <<endl;
                return true;
            }
        }
        else
        {
            cout << "No wav extension files in the folder" << endl;
        }
        
        return false;
    }
    
    bool encodePCM( const string pcmInput, const string path, MP3 lameobj);
    bool encodePCM_thread(void *arglist);

};

class targ : public MP3
{
private:
    int p;
    char q[10];
public:
    string  path;
    void *arg;
   };
#endif /* encoder_h */
