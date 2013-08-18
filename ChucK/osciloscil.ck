class osciloscil extends Chubgraph
{
    float prev;

    Phasor phz => blackhole;
    UGen oo[20];
    1 => int count;
    0 => int index; 

    Gain g => dac; 
    Noise n => g;
    Phasor p => g;
    SinOsc s => g;
    Blit b => g;
    PulseOsc po => g;
    SqrOsc so => g;
    BlitSquare bsq => g;
    TriOsc to => g;
    BlitSaw bsw => g;

    0 => n.gain => p.gain => s.gain => b.gain => po.gain => so.gain => bsq.gain => to.gain => bsw.gain;

    // default to just sinosc playing
    s @=> oo[0];
    1 => oo[0].gain;
    
    fun void setoscs(string str[])
    {
        for ( 0 => int i; i < str.cap(); i++)
        {
              if (str[i] == "SinOsc")
                s @=> oo[i]; 
              if (str[i] == "Noise")
                n @=> oo[i]; 
              if (str[i] == "Phasor")
                p  @=> oo[i]; 
              if (str[i] == "Blit")
                b @=> oo[i]; 
              if (str[i] == "PulseOsc")
                po @=> oo[i]; 
              if (str[i] == "SqrOsc")
                so  @=> oo[i]; 
              if (str[i] == "BlitSquare")
                bsq @=> oo[i]; 
              if (str[i] == "TriOsc")
                to @=> oo[i]; 
              if (str[i] == "BlitSaw")
                bsw @=> oo[i]; 
        }
        str.cap() => count;
    }

    fun void tickk()
    {
        if (phz.phase()  < 0.5 && prev > 0.5)
        {
            //<<< "jkdakf" >>>;
            if ( count > 1) 
            {
                0 => oo[index].gain;
                ( index + 1 ) % count => index;
                1 => oo[index].gain;
            }
        }
        phz.phase() => prev; 
    }

    fun void freq( float f)
    {
        f => p.freq => s.freq => b.freq => po.freq => so.freq => bsq.freq => to.freq => bsw.freq;
    }   
}

osciloscil a => dac;
a.freq(200);

a.setoscs(["Phasor", "SinOsc", "Noise"]);
a.setoscs(["Phasor",  "Noise"]);
a.setoscs(["Blit", "SqrOsc", "TriOsc", "SinOsc"]);
//a.setoscs(["SinOsc", "SqrOsc"]);
a.setoscs(["SinOsc", "TriOsc"]);
//a.setoscs(["SinOsc", "SinOsc", "SinOsc", "TriOsc", "TriOsc", "TriOsc"]);
//a.setoscs( ["SinOsc"] );

while(true)
{ 
    a.tickk();
    1::samp=> now;
}
