#include "daisysp.h"
#include "../src/kxmx_bluemchen.h"
#include <string.h>

using namespace kxmx;
using namespace daisy;
using namespace daisysp;

Bluemchen bluemchen;

Oscillator osc;
float oofreq = 400.0;

uint numWaves = 2; 
std::vector<uint> waves = { 0, 1 } ;
uint currentWave = 0;

int enc_val = 0;
int midi_note = 0;

Parameter knob1;
Parameter knob2;
Parameter cv1;
Parameter cv2;


void UpdateOled()
{
    bluemchen.display.Fill(false);

    // Display Encoder test increment value and pressed state
    bluemchen.display.SetCursor(0, 0);
    std::string str = "Enc: ";
    char *cstr = &str[0];
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    str = std::to_string(enc_val);
    bluemchen.display.SetCursor(30, 0);
    bluemchen.display.WriteString(cstr, Font_6x8, !bluemchen.encoder.Pressed());

    // Display the knob values in millivolts
    str = std::to_string(static_cast<int>(knob1.Value()));
    bluemchen.display.SetCursor(0, 8);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    str = ":";
    bluemchen.display.SetCursor(30, 8);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    str = std::to_string(static_cast<int>(knob2.Value()));
    bluemchen.display.SetCursor(36, 8);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    // Display Waveform
    str = "W:";
    bluemchen.display.SetCursor(0, 16);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    if( currentWave == 0 ){
	    str = "SIN";
    }else{
	    str = "TRI";
    }
    bluemchen.display.SetCursor(12, 16);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    str = ":";
    bluemchen.display.SetCursor(30, 16);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    // Display MIDI input note number
    str = "M:";
    bluemchen.display.SetCursor(36, 16);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    str = std::to_string(static_cast<int>(midi_note));
    bluemchen.display.SetCursor(48, 16);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    // Display CV input in millivolts
    str = std::to_string(static_cast<int>(cv1.Value()));
    bluemchen.display.SetCursor(0, 24);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    if (cv2.Value() > -999.0f)
    {
        str = ":";
        bluemchen.display.SetCursor(30, 24);
        bluemchen.display.WriteString(cstr, Font_6x8, true);
    }
    str = std::to_string(static_cast<int>(cv2.Value()));
    bluemchen.display.SetCursor((cv2.Value() > -999.0f) ? 36 : 30, 24);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    bluemchen.display.Update();
}

void HandleMidiMessage(MidiEvent m)
{
	if( m.type == NoteOn && m.channel == 1 ){
		NoteOnEvent p = m.AsNoteOn();
		oofreq = mtof( p.note );
		osc.SetFreq( oofreq  );
	}
}

void UpdateControls()
{
    bluemchen.ProcessAllControls();

    knob1.Process();
    knob2.Process();

    cv1.Process();
    cv2.Process();

    enc_val += bluemchen.encoder.Increment();
}


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{

    UpdateControls();
    for( size_t i = 0; i < size; i++){
	    float sig;
	    sig = osc.Process();
	    out[0][i] = sig;
	    out[1][i] = sig;

	    if(osc.IsEOC()){
		    if( currentWave == 0 ){
			    osc.SetWaveform( 1 );
			    currentWave = 1;
		    }else{
			    osc.SetWaveform( 0 );
			    currentWave = 0;
		    }
	    } 
    }
}

int main(void)
{
    bluemchen.Init();
    bluemchen.StartAdc();


    knob1.Init(bluemchen.controls[bluemchen.CTRL_1], 0.0f, 5000.0f, Parameter::LINEAR);
    knob2.Init(bluemchen.controls[bluemchen.CTRL_2], 0.0f, 5000.0f, Parameter::LINEAR);

    cv1.Init(bluemchen.controls[bluemchen.CTRL_3], -5000.0f, 5000.0f, Parameter::LINEAR);
    cv2.Init(bluemchen.controls[bluemchen.CTRL_4], -5000.0f, 5000.0f, Parameter::LINEAR);
    

    osc.Init( bluemchen.AudioSampleRate() );
    osc.SetFreq( oofreq );
    osc.SetAmp(0.85f);

    bluemchen.StartAudio(AudioCallback);

    while (1)
    {
        UpdateControls();
        UpdateOled();

        bluemchen.midi.Listen();
        while (bluemchen.midi.HasEvents())
        {
            HandleMidiMessage(bluemchen.midi.PopEvent());
        }
    }
}
