#include "daisysp.h"
#include "kxmx_bluemchen.h"
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

Parameter waveMinKnob;
Parameter waveMaxKnob;
Parameter noteCV;
Parameter ampCV;


float cvAmpVal = 0.85;
float cvFreqVal = 100.0f;

void loadPresets()
{
    //TODO add JSON loading from SD card
}

void changePreset(char preset)
{
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

    str = std::to_string(static_cast<int>(waveMinKnob.Value()));
    bluemchen.display.SetCursor(0, 8);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    str = ":";
    bluemchen.display.SetCursor(30, 8);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    str = std::to_string(static_cast<int>(waveMaxKnob.Value()));
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
    str = std::to_string(static_cast<int>(noteCV.Value()));
    bluemchen.display.SetCursor(0, 24);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    if (ampCV.Value() > -999.0f)
    {
        str = ":";
        bluemchen.display.SetCursor(30, 24);
        bluemchen.display.WriteString(cstr, Font_6x8, true);
    }
    str = std::to_string(static_cast<int>(ampCV.Value()));
    bluemchen.display.SetCursor((ampCV.Value() > -999.0f) ? 36 : 30, 24);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    bluemchen.display.Update();
}

void HandleMidiMessage(MidiEvent m)
{
	if( m.type == NoteOn && m.channel == 1 ){
		// NoteOnEvent p = m.AsNoteOn();
		//oofreq = mtof( p.note );
		// osc.SetFreq( oofreq  );
	}
}

void UpdateControls()
{
    waveMinKnob.Process();
    waveMaxKnob.Process();

    noteCV.Process();
    ampCV.Process();

    waveIndexMin = static_cast<int>(waveMinKnob.Value());
    waveIndexMax = static_cast<int>(waveMaxKnob.Value());

    cvFreqVal = mtof(noteCV.Value());
    cvAmpVal = ampCV.Value();

    enc_val += bluemchen.encoder.Increment();
    //osc.SetFreq( oofreq + enc_val );

    if(bluemchen.encoder.Pressed() != encoderPrevious ){
        changePreset(0);
    }
    encoderPrevious = bluemchen.encoder.Pressed();

}
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {

    UpdateControls();
    for (size_t i = 0; i < size; i++){
	    float sig;
	    sig = osc.Process();
	    out[0][i] = sig;
	    out[1][i] = sig;

	    if (osc.IsEOC()){
            currentWave++;

            if (waveIndexMin >= waveIndexMax){ // TODO make it work in reverse if max is greater than min
                currentWave = waveIndexMin;
            }

            if (currentWave > waveIndexMax || currentWave < waveIndexMin){
                currentWave = waveIndexMin;
            }

		    osc.SetWaveform(waves[currentWave]);
	    }
        osc.SetAmp(cvAmpVal);
        osc.SetFreq(cvFreqVal);
    }
}

int main(void)
{
    bluemchen.Init();
    bluemchen.StartAdc();

    waveMinKnob.Init(bluemchen.controls[bluemchen.CTRL_1], 0.0f, 6.0f, Parameter::LINEAR);
    waveMaxKnob.Init(bluemchen.controls[bluemchen.CTRL_2], 0.0f, 6.0f, Parameter::LINEAR);

    noteCV.Init(bluemchen.controls[bluemchen.CTRL_3], 0.0f, 127.0f, Parameter::LINEAR);
    ampCV.Init(bluemchen.controls[bluemchen.CTRL_4], 0.0f, 0.85f, Parameter::LINEAR);

    loadPresets();

    osc.Init(bluemchen.AudioSampleRate());
    osc.SetFreq(cvFreqVal);
    osc.SetAmp(cvAmpVal);

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