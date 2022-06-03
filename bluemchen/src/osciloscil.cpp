#include "daisysp.h"
#include "../src/kxmx_bluemchen.h"
#include <string.h>

using namespace kxmx;
using namespace daisy;
using namespace daisysp;

Bluemchen bluemchen;

Oscillator osc;
std::string waveStrings[] = { "WAVE_SIN", "WAVE_TRI", "WAVE_SAW", "WAVE_RAMP", "WAVE_SQUARE", "WAVE_POLYBLEP_TRI", "WAVE_POLYBLEP_SAW", "WAVE_POLYBLEP_SQUARE", "WAVE_LAST" };

uint currentWave = 0;
uint waveIndexMin = 0;
uint waveIndexMax = 0;
uint numberOfWaves = 6;
uint waves[] = {0, 2 , 1, 0, 5, 0};

bool encoderPrevious = false;
int enc_val = 0;

int midi_note = 0;

Parameter knob1;
Parameter knob2;
Parameter cv1;
Parameter cv2;


float cvAmpVal = 0.85;
float cvFreqVal = 100.0f;

FixedCapStr<20> displayStr;

void loadPresets(){
    //TODO add JSON loading from SD card
}

void changePreset(char preset){
    for( int i = 0; i < 6; i++ ){
        waves[i] = 0;
    }    
}

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
    bluemchen.display.SetCursor(0, 16);
    std::string wvstring = waveStrings[ waves[currentWave]];
    wvstring.resize(10);
    bluemchen.display.WriteString(wvstring.c_str(), Font_6x8, currentWave != waveIndexMin );

    // // Display MIDI input note number
    // str = "M:";
    // bluemchen.display.SetCursor(36, 16);
    // bluemchen.display.WriteString(cstr, Font_6x8, true);

    // str = std::to_string(static_cast<int>(midi_note));
    // bluemchen.display.SetCursor(48, 16);
    // bluemchen.display.WriteString(cstr, Font_6x8, true);

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
		//oofreq = mtof( p.note );
		// osc.SetFreq( oofreq  );
	}
}

void UpdateControls()
{
    bluemchen.ProcessAllControls();

    knob1.Process();
    knob2.Process();

    cv1.Process();
    cv2.Process();

    // waveIndexMin = static_cast<int>(cv1.Value());
    // waveIndexMax = static_cast<int>(cv2.Value());

    waveIndexMin = static_cast<int>(knob1.Value());
    waveIndexMax = static_cast<int>(knob2.Value());

    cvFreqVal = cv1.Value();
    cvAmpVal = cv2.Value();

    enc_val += bluemchen.encoder.Increment();
    //osc.SetFreq( oofreq + enc_val );

    if(bluemchen.encoder.Pressed() != encoderPrevious ){
        changePreset(0);
    }
    encoderPrevious = bluemchen.encoder.Pressed();

}
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {

    UpdateControls();
    for( size_t i = 0; i < size; i++){
	    float sig;
	    sig = osc.Process();
	    out[0][i] = sig;
	    out[1][i] = sig;

	    if(osc.IsEOC()){
            currentWave++;
		    
            if(waveIndexMin >= waveIndexMax){ // TODO make it work in reverse if max is greater than min
                currentWave = waveIndexMin;
            }
            
            if(currentWave > waveIndexMax || currentWave < waveIndexMin){
                currentWave = waveIndexMin;
            } 

		    osc.SetWaveform( waves[currentWave] );
	    } 
        osc.SetAmp( cvAmpVal );
        osc.SetFreq( cvFreqVal );
    }
}

int main(void)
{
    bluemchen.Init();
    bluemchen.StartAdc();

    knob1.Init(bluemchen.controls[bluemchen.CTRL_1], 0.0f, 6.0f, Parameter::LINEAR);
    knob2.Init(bluemchen.controls[bluemchen.CTRL_2], 0.0f, 6.0f, Parameter::LINEAR);

    cv1.Init(bluemchen.controls[bluemchen.CTRL_3], 0.0f, 10000.00f, Parameter::LOGARITHMIC);
    cv2.Init(bluemchen.controls[bluemchen.CTRL_4], 0.0f, 0.85f, Parameter::LINEAR);
    
    loadPresets();

    osc.Init( bluemchen.AudioSampleRate() );
    osc.SetFreq( cvFreqVal );
    osc.SetAmp( cvAmpVal );

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
