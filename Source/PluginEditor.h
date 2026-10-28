#pragma once
#include "PluginProcessor.h"

// GUI skinned from Resources/Interfaz_referencia.png (design size 1536x1024). All coordinates below are in that space.
namespace ymy
{
    inline juce::Font F (float h, bool bold = false) { return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain)); }
    const juce::Colour cream2 (0xffe2dacb), gold1 (0xffe6c777), gold2 (0xff9c7a2b), wine (0xff8a1426), ink (0xff3d342d);

    struct LnF : juce::LookAndFeel_V4
    {
        LnF();
        void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
        void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
        void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
        void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;
        juce::Font getComboBoxFont (juce::ComboBox&) override { return F (15.f); }
        juce::Font getTextButtonFont (juce::TextButton&, int h) override { return F (juce::jmin (15.f, (float) h * 0.42f)); }
    };

    struct Knob : juce::Component
    {
        Knob (juce::AudioProcessorValueTreeState&, const juce::String& id, const juce::String& label, float knobFraction);
        void resized() override;
        juce::Slider s; juce::Label name, value; float frac;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
    };

    struct Meter : juce::Component { float level = 0.f; void paint (juce::Graphics&) override; };

    struct SimpleList : juce::ListBoxModel
    {
        std::function<int()> count; std::function<juce::String (int)> text; std::function<void (int)> select; std::function<bool (int)> starred;
        int getNumRows() override { return count ? juce::jmax (0, count()) : 0; }
        void paintListBoxItem (int row, juce::Graphics&, int w, int h, bool sel) override;
        void selectedRowsChanged (int row) override { if (row >= 0 && select) select (row); }
    };
}

class YmyEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit YmyEditor (YmyProcessor&);
    ~YmyEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void refreshPresetUI (bool rebuildLists);
    void promptSave (bool saveAs);
    void setParam (const char* id, float realValue);
    void stepPreset (int d);
    juce::Rectangle<int> R (float x, float y, float w, float h) const;
    ymy::Knob* mk (const juce::String& id, const juce::String& label, float frac);
    juce::TextButton* mkBtn (const juce::String& text, std::function<void()> fn, bool dark = false);
    void place (ymy::Knob* k, float cx, float cy, int kind);   // kind 0 big, 1 mix, 2 ducking
    void field (juce::Graphics&, juce::Rectangle<int>) const;

    YmyProcessor& proc;
    ymy::LnF lnf;
    juce::Image bgImg, micOnImg;
    std::vector<std::unique_ptr<ymy::Knob>> knobs;
    ymy::Knob *kRoom, *kDecay, *kPre, *kEarly, *kDiff, *kDens, *kMod, *kWarm, *kDamp, *kLow, *kHigh, *kLowD, *kHighD, *kTone,
              *kWidth, *kBal, *kErW, *kSpread, *kDry, *kWet, *kIn, *kOut, *kRet, *kThr, *kDepth, *kAtt, *kRel;

    juce::ComboBox algo, division, modeBox, presetBox;
    juce::ToggleButton sync { "Sync" }, duck { "Ducking" }, makeup { "Auto Makeup" };
    juce::Slider manualBpm;
    juce::Label hostBpmLabel, preLabel, descLabel, warnLabel;
    std::vector<std::unique_ptr<juce::TextButton>> buttons;
    juce::TextButton *favBtn = nullptr, *abBtn = nullptr, *saveBtn = nullptr, *btnNorm = nullptr, *btnTrip = nullptr, *btnDot = nullptr,
                     *modeIns = nullptr, *modeAux = nullptr, *ledOnB = nullptr, *ledOffB = nullptr;
    ymy::SimpleList catModel, listModel; juce::ListBox catList, presetList;
    std::vector<int> shownIdx; juce::StringArray cats; int catSel = 0;
    ymy::Meter meterIn, meterOut;
    float glowState = 0.f, lastGlow = -1.f; bool updatingUI = false, ledEnabled = true;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtt;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bpmAtt;
    juce::ComponentBoundsConstrainer constrainer;
};
