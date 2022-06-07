#include "daisysp.h"
#include "kxmx_bluemchen.h"
#include <string.h>

using namespace kxmx;
using namespace daisy;
using namespace daisysp;

Bluemchen bluemchen;

CpuLoadMeter meter;

Oscillator osc;
std::string waveStrings[] = { "Sine", "Tri", "Saw", "Ramp", "Square", "BLTri", "BLSaw", "BLSquare"};

int32_t currentWave = 0;
int32_t waveIndexMin = 0;
int32_t waveIndexMax = 0;
uint32_t numberOfWaves = 6;
uint32_t waves[] = {0, 2 , 1, 0, 5, 0};

float cvAmpVal = 0.85f;
float cvFreqVal = 100.0f;
int32_t midiNote = -1;
float midiNoteFreq = 0.0f;

bool encoderPrevious = false;
int32_t encoderVal = 0;

Parameter waveMinKnob;
Parameter waveMaxKnob;
Parameter noteCV;
Parameter ampCV;

void LoadPresets()
{
    // TODO add JSON (or CSV?) loading from SD card.
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

    // Display CPU stats.
    bluemchen.display.SetCursor(0, 0);
    std::string str = "CPU: ";
    char *cstr = &str[0];
    bluemchen.display.WriteString(cstr, Font_6x8, true);
    str = std::to_string(static_cast<uint32_t>(meter.GetAvgCpuLoad() * 100.0f)) + "%";
    bluemchen.display.SetCursor(30, 0);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    // Wave min/max indices.
    str = std::to_string(waveIndexMin);
    bluemchen.display.SetCursor(0, 8);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    str = std::to_string(waveIndexMax);
    bluemchen.display.SetCursor(54, 8);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    // Display Waveform
    bluemchen.display.SetCursor(0, 16);
    std::string wvstring = waveStrings[ waves[currentWave]];
    wvstring.resize(10);
    bluemchen.display.WriteString(wvstring.c_str(), Font_6x8, currentWave != waveIndexMin );

    // Display MIDI input note number
    str = "M:";
    bluemchen.display.SetCursor(36, 16);
    bluemchen.display.WriteString(cstr, Font_6x8, true);
    str = std::to_string(static_cast<int>(midiNote));
    bluemchen.display.SetCursor(48, 16);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    // CV note input
    str = std::to_string(static_cast<int>(noteCV.Value()));
    bluemchen.display.SetCursor(0, 24);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    // CV amplitude
    // TODO: Use FixCapString so that we can print floats
    // instead of scaling this to a weird int value
    // because libDaisy doesn't include float formatters.
    str = std::to_string(static_cast<int>(ampCV.Value() * 100));
    bluemchen.display.SetCursor(48, 24);
    bluemchen.display.WriteString(cstr, Font_6x8, true);

    bluemchen.display.Update();
}

void HandleNoteOn(uint8_t note) {
    midiNote = note;
    midiNoteFreq = mtof(midiNote);
}

void HandleNoteOff(uint8_t note) {
    if (note == midiNote) {
        midiNote = -1;
    }
}

void HandleMidiMessage(MidiEvent m)
{
    // Currently, we only support omni mode,
    // but channel filtering would otherwise go here.

    if (m.type == NoteOn) {
        NoteOnEvent e = m.AsNoteOn();
        if (e.velocity > 0) {
            HandleNoteOn(e.note);
        } else {
            // Note on with velocity zero is note off
            HandleNoteOff(e.note);
        }
    } else if (m.type == NoteOff) {
        HandleNoteOff(m.AsNoteOff().note);
    }
}

void UpdateControls()
{
    waveMinKnob.Process();
    waveMaxKnob.Process();

    noteCV.Process();
    ampCV.Process();

    // Truncate knob values into wave table indices.
    waveIndexMin = static_cast<uint32_t>(waveMinKnob.Value());
    waveIndexMax = static_cast<uint32_t>(waveMaxKnob.Value());

    cvFreqVal = mtof(noteCV.Value());
    cvAmpVal = ampCV.Value();

    encoderVal += bluemchen.encoder.Increment();

    if (bluemchen.encoder.Pressed() != encoderPrevious) {
        ChangePreset(0);
    }
    encoderPrevious = bluemchen.encoder.Pressed();
}

void AudioCallback(AudioHandle::InputBuffer in,
    AudioHandle::OutputBuffer out, size_t size) {
    meter.OnBlockStart();

    UpdateControls();

    for (size_t i = 0; i < size; i++) {
        osc.SetFreq(midiNote > -1 ? midiNoteFreq : cvFreqVal);

	    float sample = osc.Process() * cvAmpVal;
	    out[0][i] = sample;
	    out[1][i] = sample;

	    if (osc.IsEOC()) {
            int32_t min = waveIndexMin;
            int32_t max = waveIndexMax;

            if (waveIndexMin > waveIndexMax) {
                // The knobs are reversed,
                // read the wave list backwards.
                currentWave--;
                min = waveIndexMax;
                max = waveIndexMin;
            } else {
                currentWave++;
            }

            if (currentWave < min) {
                currentWave = max;
            } else if (currentWave > max) {
                currentWave = min;
            }

            osc.SetWaveform(waves[currentWave]);
	    }
    }
    meter.OnBlockEnd();
}

void ProcessMidi() {
    bluemchen.midi.Listen();
    while (bluemchen.midi.HasEvents())
    {
        HandleMidiMessage(bluemchen.midi.PopEvent());
    }
}

void InitializeControls() {
    // Ensure the max wave index value won't be too high
    // when it is truncated (if the voltage reaches +5V).
    float maxWaveIdx = ((float) numberOfWaves) - 0.001f;

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
    meter.Init(bluemchen.AudioSampleRate(),
        bluemchen.AudioBlockSize());
    osc.Init(bluemchen.AudioSampleRate());
    osc.SetAmp(1.0f);

    bluemchen.StartAudio(AudioCallback);
    bluemchen.midi.StartReceive();

    // TODO: Improve the factoring of this or remove it.
    uint32_t lastDrawTime = System::GetNow();
    uint32_t now;

    while (1)
    {
        now = System::GetNow();
        if (now - lastDrawTime >= 40) { // 25 fps
            UpdateOled();
            lastDrawTime = now;
        }
        ProcessMidi();
    }
}
