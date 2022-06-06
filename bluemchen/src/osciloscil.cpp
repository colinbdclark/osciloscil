#include "daisysp.h"
#include "kxmx_bluemchen.h"
#include <string.h>

using namespace kxmx;
using namespace daisy;
using namespace daisysp;

Bluemchen bluemchen;

Oscillator osc;
std::string waveStrings[] = { "WAVE_SIN", "WAVE_TRI", "WAVE_SAW", "WAVE_RAMP", "WAVE_SQUARE", "WAVE_POLYBLEP_TRI", "WAVE_POLYBLEP_SAW", "WAVE_POLYBLEP_SQUARE", "WAVE_LAST" };

uint32_t currentWave = 0;
uint32_t waveIndexMin = 0;
uint32_t waveIndexMax = 0;
uint32_t numberOfWaves = 6;
uint32_t waves[] = {0, 2 , 1, 0, 5, 0};

bool encoderPrevious = false;
int32_t enc_val = 0;

int32_t midi_note = 0;

Parameter waveMinKnob;
Parameter waveMaxKnob;
Parameter noteCV;
Parameter ampCV;


float cvAmpVal = 0.85f;
float cvFreqVal = 100.0f;

void LoadPresets()
{
    //TODO add JSON loading from SD card
}

void ChangePreset(char preset)
{
    for (uint32_t i = 0; i < numberOfWaves; i++ ){
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

    str = std::to_string(waveIndexMin);
    bluemchen.display.SetCursor(0, 8);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    str = ":";
    bluemchen.display.SetCursor(30, 8);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    str = std::to_string(waveIndexMax);
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

    // TODO: This doesn't work; amplitude is never updated.
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
	if(m.type == NoteOn && m.channel == 1){
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

    float minRounded = roundf(waveMinKnob.Value());
    float maxRounded = roundf(waveMaxKnob.Value());
    waveIndexMin = static_cast<uint32_t>(minRounded);
    waveIndexMax = static_cast<uint32_t>(maxRounded);

    cvFreqVal = mtof(noteCV.Value());
    cvAmpVal = ampCV.Value();

    enc_val += bluemchen.encoder.Increment();
    //osc.SetFreq( oofreq + enc_val );

    if (bluemchen.encoder.Pressed() != encoderPrevious) {
        ChangePreset(0);
    }
    encoderPrevious = bluemchen.encoder.Pressed();
}

void AudioCallback(AudioHandle::InputBuffer in,
    AudioHandle::OutputBuffer out, size_t size) {

    UpdateControls();
    for (size_t i = 0; i < size; i++) {
        osc.SetAmp(cvAmpVal);
        osc.SetFreq(cvFreqVal);

	    float sample = osc.Process();
	    out[0][i] = sample;
	    out[1][i] = sample;

	    if (osc.IsEOC()) {
            currentWave++;

            if (waveIndexMin >= waveIndexMax) { // TODO make it work in reverse if max is greater than min
                currentWave = waveIndexMin;
            }

            if (currentWave > waveIndexMax || currentWave < waveIndexMin) {
                currentWave = waveIndexMin;
            }

		    osc.SetWaveform(waves[currentWave]);
	    }
    }
}

void ProcessMidi() {
    bluemchen.midi.Listen();
    while (bluemchen.midi.HasEvents())
    {
        HandleMidiMessage(bluemchen.midi.PopEvent());
    }
}

void InitializeControls() {
    float maxWaveIdx = (float) numberOfWaves - 1;
    waveMinKnob.Init(bluemchen.controls[bluemchen.CTRL_1], 0.0f,
        maxWaveIdx, Parameter::LINEAR);
    waveMaxKnob.Init(bluemchen.controls[bluemchen.CTRL_2], 0.0f,
        maxWaveIdx, Parameter::LINEAR);

    noteCV.Init(bluemchen.controls[bluemchen.CTRL_3], 0.0f, 127.0f,
        Parameter::LINEAR);
    ampCV.Init(bluemchen.controls[bluemchen.CTRL_4], 0.0f, 0.85f,
        Parameter::LINEAR);
}

int main(void)
{
    bluemchen.Init();
    bluemchen.StartAdc();

    InitializeControls();
    LoadPresets();

    osc.Init(bluemchen.AudioSampleRate());
    bluemchen.StartAudio(AudioCallback);

    while (1)
    {
        UpdateControls();
        UpdateOled();
        ProcessMidi();
    }
}
