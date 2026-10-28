#include "PluginEditor.h"
#include <BinaryData.h>
using namespace juce;
using namespace ymy;

// ------------------------------------------------------------------ look and feel
LnF::LnF()
{
    setColour (PopupMenu::backgroundColourId, Colour (0xff1b1614)); setColour (PopupMenu::textColourId, Colours::white);
    setColour (PopupMenu::highlightedBackgroundColourId, Colour (0xff2c5da8)); setColour (PopupMenu::headerTextColourId, gold1);
    setColour (TextButton::textColourOnId, Colours::white); setColour (TextButton::textColourOffId, ink);
    setColour (ComboBox::textColourId, ink);
}

void LnF::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, Slider&)
{
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (1.f);
    const float d = jmin (b.getWidth(), b.getHeight()), r = d * 0.5f; const auto c = b.getCentre();
    g.setColour (Colours::black.withAlpha (0.22f)); g.fillEllipse (Rectangle<float> (d, d).withCentre (c + Point<float> (0.f, d * 0.03f)));
    g.setGradientFill (ColourGradient (Colour (0xfff0d68a), c.x - r, c.y - r, Colour (0xff8f6d22), c.x + r, c.y + r, false));
    g.fillEllipse (Rectangle<float> (d, d).withCentre (c));
    const float rc = r * 0.84f;
    g.setGradientFill (ColourGradient (Colour (0xfffffdf8), c.x - rc * 0.5f, c.y - rc * 0.7f, Colour (0xffd9cdb6), c.x + rc * 0.6f, c.y + rc * 0.8f, true));
    g.fillEllipse (Rectangle<float> (rc * 2, rc * 2).withCentre (c));
    g.setColour (Colour (0xff8f6d22).withAlpha (0.45f)); g.drawEllipse (Rectangle<float> (rc * 2, rc * 2).withCentre (c), 1.f);
    const float a = a0 + pos * (a1 - a0);
    Path p; p.addRoundedRectangle (-d * 0.025f - 0.6f, -rc * 0.9f, d * 0.05f + 1.2f, rc * 0.4f, 1.6f);
    g.setColour (Colour (0xffa11b2c)); g.fillPath (p, AffineTransform::rotation (a).translated (c));
}

void LnF::drawToggleButton (Graphics& g, ToggleButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat(); const bool on = b.getToggleState();
    const float ph = jmin (22.f, r.getHeight() * 0.4f), pw = ph * 1.9f;
    auto pill = Rectangle<float> (pw, ph).withCentre ({ r.getCentreX(), r.getY() + ph * 0.7f + 2.f });
    g.setColour (on ? Colour (0xff2f8f83) : Colour (0xffb8b0a4)); g.fillRoundedRectangle (pill, ph * 0.5f);
    g.setColour (Colours::white); g.fillEllipse (Rectangle<float> (ph * 0.78f, ph * 0.78f).withCentre ({ on ? pill.getRight() - ph * 0.5f : pill.getX() + ph * 0.5f, pill.getCentreY() }));
    g.setColour (ink); g.setFont (F (ph * 0.78f));
    g.drawText (b.getButtonText(), r.withTop (pill.getBottom() + 2.f).toNearestInt(), Justification::centredTop);
    g.drawText (on ? "On" : "Off", r.withTop (pill.getBottom() + 2.f + ph * 0.85f).toNearestInt(), Justification::centredTop);
}

void LnF::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState(), dark = (bool) b.getProperties()["dark"];
    Colour fill = dark ? Colour (0xff1c1716) : Colour (0xfff3ece0);
    if (over) fill = fill.brighter (0.08f); if (down) fill = fill.darker (0.1f);
    if (on) fill = Colour (0xff17120f);
    g.setColour (fill); g.fillRoundedRectangle (r, 6.f);
    g.setColour (dark ? gold2.withAlpha (0.55f) : Colour (0xffcbbfa7)); g.drawRoundedRectangle (r, 6.f, 1.f);
}

void LnF::drawComboBox (Graphics& g, int w, int h, bool, int, int, int, int, ComboBox& cb)
{
    auto r = Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (0.5f); const bool dark = (bool) cb.getProperties()["dark"];
    g.setColour (dark ? Colour (0xff1c1716) : Colour (0xfffbf8f2)); g.fillRoundedRectangle (r, 6.f);
    g.setColour (dark ? gold2.withAlpha (0.55f) : Colour (0xffcbbfa7)); g.drawRoundedRectangle (r, 6.f, 1.f);
    Path t; const float cy = (float) h * 0.5f; t.addTriangle ((float) w - 22.f, cy - 3.f, (float) w - 10.f, cy - 3.f, (float) w - 16.f, cy + 4.f);
    g.setColour (dark ? Colours::white.withAlpha (0.8f) : ink); g.fillPath (t);
}

void SimpleList::paintListBoxItem (int row, Graphics& g, int w, int h, bool sel)
{
    if (sel) { g.setColour (Colour (0xff2c5da8)); g.fillRect (0, 0, w, h); }
    if (! text || row < 0) return;
    g.setColour (sel ? Colours::white : Colour (0xffd9deea)); g.setFont (F ((float) h * 0.62f));
    g.drawText (text (row), 10, 0, w - 30, h, Justification::centredLeft);
    if (starred && starred (row)) { g.setColour (Colour (0xfff2c14e)); g.drawText (String::fromUTF8 ("\\xe2\\x98\\x85"), w - 24, 0, 18, h, Justification::centred); }
}

// ------------------------------------------------------------------ knob / meter
Knob::Knob (AudioProcessorValueTreeState& a, const String& id, const String& label, float f) : frac (f)
{
    s.setSliderStyle (Slider::RotaryHorizontalVerticalDrag); s.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
    s.setRotaryParameters (MathConstants<float>::pi * 1.2f, MathConstants<float>::pi * 2.8f, true);
    addAndMakeVisible (s); addAndMakeVisible (name); addAndMakeVisible (value);
    for (auto* l : { &name, &value }) { l->setJustificationType (Justification::centred); l->setColour (Label::textColourId, ink); l->setInterceptsMouseClicks (false, false); }
    value.setColour (Label::textColourId, ink.withAlpha (0.85f));
    name.setText (label, dontSendNotification);
    att = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (a, id, s);
    s.onValueChange = [this] { value.setText (s.getTextFromValue (s.getValue()), dontSendNotification); };
    s.onValueChange();
}
void Knob::resized()
{
    auto b = getLocalBounds(); const int sh = roundToInt ((float) getHeight() * frac), lh = (getHeight() - sh) / 2;
    s.setBounds (b.removeFromTop (sh).withSizeKeepingCentre (sh, sh)); name.setBounds (b.removeFromTop (lh)); value.setBounds (b);
    name.setFont (F ((float) lh * 0.8f)); value.setFont (F ((float) lh * 0.8f));
}

void Meter::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (Colour (0xff1b1614)); g.fillRoundedRectangle (b, 2.f);
    const int segs = 36; const float sh = (b.getHeight() - 4.f) / segs;
    const float db = Decibels::gainToDecibels (level, -60.f);
    for (int i = 0; i < segs; ++i)
    {
        const float segDb = -40.f + 40.f * (float) i / (segs - 1);
        Colour c = segDb > -3.f ? Colour (0xffe0442c) : segDb > -12.f ? Colour (0xfff2a33a) : Colour (0xffe9c46a);
        g.setColour (db >= segDb ? c : c.withAlpha (0.16f));
        g.fillRect (b.getX() + 3.f, b.getBottom() - 2.f - (float) (i + 1) * sh + 1.f, b.getWidth() - 6.f, sh - 1.5f);
    }
}

// ------------------------------------------------------------------ editor
Knob* YmyEditor::mk (const String& id, const String& label, float frac)
{ knobs.push_back (std::make_unique<Knob> (proc.apvts, id, label, frac)); auto* k = knobs.back().get(); addAndMakeVisible (k); return k; }

TextButton* YmyEditor::mkBtn (const String& text, std::function<void()> fn, bool dark)
{
    buttons.push_back (std::make_unique<TextButton> (text)); auto* b = buttons.back().get();
    b->onClick = std::move (fn); b->getProperties().set ("dark", dark);
    if (dark) b->setColour (TextButton::textColourOffId, Colours::white);
    addAndMakeVisible (b); return b;
}

void YmyEditor::setParam (const char* id, float v)
{ if (auto* p = proc.apvts.getParameter (id)) p->setValueNotifyingHost (p->convertTo0to1 (v)); }

void YmyEditor::stepPreset (int d)
{ const int n = (int) proc.getPresets().size(); if (n == 0) return; proc.loadPreset ((proc.getCurrentPreset() + d + n) % n); refreshPresetUI (false); }

YmyEditor::YmyEditor (YmyProcessor& p) : AudioProcessorEditor (&p), proc (p), catList ("cats", &catModel), presetList ("presets", &listModel)
{
    bgImg = ImageCache::getFromMemory (BinaryData::skin_background_png, BinaryData::skin_background_pngSize);
    micOnImg = ImageCache::getFromMemory (BinaryData::skin_mic_on_png, BinaryData::skin_mic_on_pngSize);
    setLookAndFeel (&lnf);
    kRoom = mk ("room", "Room Size", .655f); kDecay = mk ("decay", "Decay", .655f); kPre = mk ("predelay", "Predelay", .655f);
    kEarly = mk ("early", "Early Reflections", .655f); kDiff = mk ("diffusion", "Diffusion", .655f); kDens = mk ("density", "Density", .655f); kMod = mk ("mod", "Modulation", .655f);
    kWarm = mk ("warmth", "Warmth", .655f); kDamp = mk ("damping", "Damping", .655f); kLow = mk ("lowcut", "Low Cut", .655f); kHigh = mk ("highcut", "High Cut", .655f);
    kLowD = mk ("lowdecay", "Low Decay", .655f); kHighD = mk ("highdecay", "High Decay", .655f); kTone = mk ("tone", "Reverb Tone", .655f);
    kWidth = mk ("width", "Width", .655f); kBal = mk ("balance", "Balance", .655f); kErW = mk ("erwidth", "Early Ref. Width", .655f); kSpread = mk ("spread", "Reverb Spread", .655f);
    kDry = mk ("dry", "Dry", .56f); kWet = mk ("wet", "Wet", .56f); kIn = mk ("in", "Input", .56f); kOut = mk ("out", "Output", .56f); kRet = mk ("ret", "Reverb Return", .56f);
    kThr = mk ("duckthr", "Threshold", .56f); kDepth = mk ("duckdepth", "Depth", .56f); kAtt = mk ("duckatt", "Attack", .56f); kRel = mk ("duckrel", "Release", .56f);

    algo.addItemList ({ "Plate", "Room", "Chamber", "Hall", "Vintage Digital", "Hybrid Bachata" }, 1);
    division.addItemList ({ "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1" }, 1);
    modeBox.addItemList ({ "Insert", "Aux Send" }, 1);
    for (auto* c : { &algo, &division, &modeBox, &presetBox }) addAndMakeVisible (c);
    for (auto* c : { &modeBox, &presetBox }) { c->getProperties().set ("dark", true); c->setColour (ComboBox::textColourId, Colours::white); }
    comboAtt.push_back (std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "algo", algo));
    comboAtt.push_back (std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "div", division));
    auto setMode = [this] (int m)   // user choice only; the timer mirrors the parameter back without notifying
    {
        setParam ("mode", (float) m);
        if (m == 1) { setParam ("dry", 0.f); setParam ("wet", 1.f); } else { setParam ("dry", 1.f); setParam ("wet", 0.28f); }
    };
    modeBox.onChange = [this, setMode] { setMode (modeBox.getSelectedId() - 1); };

    addAndMakeVisible (sync); addAndMakeVisible (duck); addAndMakeVisible (makeup);
    btnAtt.push_back (std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "sync", sync));
    btnAtt.push_back (std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "duckon", duck));
    btnAtt.push_back (std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "makeup", makeup));

    manualBpm.setSliderStyle (Slider::LinearBar); manualBpm.setColour (Slider::trackColourId, gold1.withAlpha (0.0f));
    manualBpm.setColour (Slider::textBoxTextColourId, ink); manualBpm.setColour (Slider::backgroundColourId, Colours::transparentBlack);
    addAndMakeVisible (manualBpm); bpmAtt = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "bpm", manualBpm);
    for (auto* l : { &hostBpmLabel, &preLabel }) { addAndMakeVisible (l); l->setColour (Label::textColourId, ink); l->setJustificationType (Justification::centred);
        l->setColour (Label::backgroundColourId, Colour (0xfffbf8f2)); l->setColour (Label::outlineColourId, Colour (0xffcbbfa7)); }
    addAndMakeVisible (descLabel); addAndMakeVisible (warnLabel);
    warnLabel.setColour (Label::textColourId, Colour (0xffb3261e)); warnLabel.setJustificationType (Justification::topLeft);
    descLabel.setColour (Label::textColourId, Colour (0xffe8ecf5)); descLabel.setJustificationType (Justification::topLeft); descLabel.setBorderSize (BorderSize<int> (6));

    // transport-style preset buttons (header + cluster share the same handlers)
    auto save = [this] { promptSave (false); };
    auto saveAs = [this] { promptSave (true); };
    auto ab = [this] { proc.toggleAB(); abBtn->setButtonText (proc.getABSlot() == 0 ? "A / B  (A)" : "A / B  (B)"); };
    mkBtn ("<", [this] { stepPreset (-1); }, true); mkBtn (">", [this] { stepPreset (1); }, true);
    saveBtn = mkBtn ("Save", save, true); mkBtn ("Save As", saveAs, true); abBtn = mkBtn ("A / B", ab, true);
    mkBtn ("Undo", [this] { proc.undo.undo(); }, true); mkBtn ("Redo", [this] { proc.undo.redo(); }, true);
    mkBtn ("<", [this] { stepPreset (-1); }); mkBtn (">", [this] { stepPreset (1); });
    favBtn = mkBtn ("Fav", [this] { proc.toggleFavorite (proc.getCurrentPreset()); refreshPresetUI (true); });
    mkBtn ("Reset", [this] { proc.loadPreset (proc.getCurrentPreset()); });                       // reload current preset
    mkBtn ("Save", save); mkBtn ("Save As", saveAs);
    mkBtn ("Init", [this] { proc.undo.beginNewTransaction(); for (auto* pr : proc.getParameters()) if (pr->getParameterIndex() >= 0 && pr->getName (32) != "Mode") pr->setValueNotifyingHost (pr->getDefaultValue()); });
    mkBtn ("Scan", [this] { proc.refreshPresets(); refreshPresetUI (true); });
    mkBtn ("A / B", ab); mkBtn ("Undo", [this] { proc.undo.undo(); }); mkBtn ("Redo", [this] { proc.undo.redo(); });
    btnNorm = mkBtn ("Norm", [this] { setParam ("triplet", 0.f); setParam ("dotted", 0.f); });
    btnTrip = mkBtn ("Trip", [this] { setParam ("triplet", 1.f); setParam ("dotted", 0.f); });
    btnDot  = mkBtn ("Dot",  [this] { setParam ("triplet", 0.f); setParam ("dotted", 1.f); });
    modeIns = mkBtn ("Insert", [setMode] { setMode (0); }); modeAux = mkBtn ("Aux Send", [setMode] { setMode (1); });
    ledOnB = mkBtn ("On", [this] { setParam ("led", 1.f); }); ledOffB = mkBtn ("Off", [this] { setParam ("led", 0.f); });
    for (auto* b : { btnNorm, btnTrip, btnDot, modeIns, modeAux, ledOnB, ledOffB, favBtn }) b->setClickingTogglesState (false);

    addAndMakeVisible (catList); addAndMakeVisible (presetList);
    for (auto* l : { &catList, &presetList }) { l->setColour (ListBox::backgroundColourId, Colours::transparentBlack); l->setRowHeight (22); }
    catModel.count = [this] { return cats.size(); };
    catModel.text = [this] (int i) { return isPositiveAndBelow (i, cats.size()) ? cats[i] : String(); };
    catModel.select = [this] (int i) { if (updatingUI || ! isPositiveAndBelow (i, cats.size())) return; catSel = i; refreshPresetUI (true); };
    listModel.count = [this] { return (int) shownIdx.size(); };
    listModel.text = [this] (int i) {
        if (! isPositiveAndBelow (i, (int) shownIdx.size())) return String();
        const int presetIndex = shownIdx[(size_t) i];
        const auto& presets = proc.getPresets();
        return isPositiveAndBelow (presetIndex, (int) presets.size()) ? presets[(size_t) presetIndex].name : String();
    };
    listModel.starred = [this] (int i) {
        return isPositiveAndBelow (i, (int) shownIdx.size()) && proc.isFavorite (shownIdx[(size_t) i]);
    };
    listModel.select = [this] (int i) {
        if (updatingUI || ! isPositiveAndBelow (i, (int) shownIdx.size())) return;
        const int presetIndex = shownIdx[(size_t) i];
        if (! isPositiveAndBelow (presetIndex, (int) proc.getPresets().size())) return;
        proc.loadPreset (presetIndex); refreshPresetUI (false);
    };
    presetBox.onChange = [this] { if (updatingUI) return; const int id = presetBox.getSelectedId(); if (id > 0 && id - 1 != proc.getCurrentPreset()) { proc.loadPreset (id - 1); refreshPresetUI (false); } };

    addAndMakeVisible (meterIn); addAndMakeVisible (meterOut);
    refreshPresetUI (true);
    constrainer.setFixedAspectRatio (1.5); constrainer.setSizeLimits (960, 640, 2304, 1536);
    setConstrainer (&constrainer); setResizable (true, true); setSize (1152, 768);
    startTimerHz (30);
}
YmyEditor::~YmyEditor() { stopTimer(); setLookAndFeel (nullptr); }

void YmyEditor::refreshPresetUI (bool rebuild)
{
    const auto& ps = proc.getPresets();
    const ScopedValueSetter<bool> guard (updatingUI, true);
    if (rebuild)
    {
        cats.clear(); cats.add ("All Presets"); cats.add (String::fromUTF8 ("\xe2\x98\x85 Favoritos"));
        for (auto& p : ps) cats.addIfNotAlreadyThere (p.category);
        catSel = jlimit (0, cats.size() - 1, catSel);
        shownIdx.clear();
        for (int i = 0; i < (int) ps.size(); ++i)
            if (catSel == 0 || (catSel == 1 && proc.isFavorite (i)) || (catSel > 1 && ps[(size_t) i].category == cats[catSel])) shownIdx.push_back (i);
        presetBox.clear (dontSendNotification);
        String lastCat; for (int i = 0; i < (int) ps.size(); ++i) { if (ps[(size_t) i].category != lastCat) { lastCat = ps[(size_t) i].category; presetBox.addSectionHeading (lastCat); } presetBox.addItem (ps[(size_t) i].name, i + 1); }
        catList.updateContent(); presetList.updateContent(); catList.selectRow (catSel, false, false);
    }
    const int cur = proc.getCurrentPreset();
    presetBox.setSelectedId (cur + 1, dontSendNotification);
    if (cur >= 0 && cur < (int) ps.size())
    {
        descLabel.setText (ps[(size_t) cur].name + "\n\n" + ps[(size_t) cur].description + "\n\nPunto de partida inspirado en el estilo; no son los ajustes reales de ningun artista.", dontSendNotification);
        for (int r = 0; r < (int) shownIdx.size(); ++r) if (shownIdx[(size_t) r] == cur) presetList.selectRow (r, false, false);
    }
    presetList.repaint(); repaint();
}

void YmyEditor::promptSave (bool saveAs)
{
    const auto& ps = proc.getPresets(); const int cur = proc.getCurrentPreset();
    String nm = (cur >= 0 && cur < (int) ps.size()) ? ps[(size_t) cur].name : String ("Mi preset");
    if (! saveAs && cur >= 0 && cur < (int) ps.size() && ps[(size_t) cur].user) { proc.saveUserPreset (nm); refreshPresetUI (true); return; }
    if (! saveAs) nm += " (user)";
    auto* w = new AlertWindow ("Guardar preset", "Nombre del preset:", MessageBoxIconType::NoIcon);
    w->addTextEditor ("n", nm); w->addButton ("Guardar", 1, KeyPress (KeyPress::returnKey)); w->addButton ("Cancelar", 0, KeyPress (KeyPress::escapeKey));
    Component::SafePointer<YmyEditor> self (this);
    w->enterModalState (true, ModalCallbackFunction::create ([self, w] (int r) {
        if (r == 1 && self != nullptr) { self->proc.saveUserPreset (w->getTextEditorContents ("n")); self->refreshPresetUI (true); }
    }), true);
}

void YmyEditor::timerCallback()
{
    const float in = proc.inLevel.load(), out = proc.outLevel.load();
    meterIn.level = in; meterOut.level = out; meterIn.repaint(); meterOut.repaint();
    // mic activity from the REAL input level: threshold about -50 dB, fast attack, slow release (30 Hz timer)
    const float db = Decibels::gainToDecibels (in, -80.f);
    const float target = jlimit (0.f, 1.f, (db + 50.f) / 40.f);
    glowState += (target - glowState) * (target > glowState ? 0.55f : 0.09f);
    if (glowState < 0.004f) glowState = 0.f;
    ledEnabled = proc.apvts.getRawParameterValue ("led")->load() > 0.5f;
    const float shown = ledEnabled ? glowState : 0.f;
    if (std::abs (shown - lastGlow) > 0.006f || (shown == 0.f && lastGlow != 0.f)) { lastGlow = shown; repaint (R (672, 50, 188, 678)); }

    auto sync1 = [] (TextButton* b, bool on) { if (b->getToggleState() != on) b->setToggleState (on, dontSendNotification); };
    const bool trip = proc.apvts.getRawParameterValue ("triplet")->load() > 0.5f, dot = proc.apvts.getRawParameterValue ("dotted")->load() > 0.5f;
    sync1 (btnTrip, trip); sync1 (btnDot, dot); sync1 (btnNorm, ! trip && ! dot);
    sync1 (ledOnB, ledEnabled); sync1 (ledOffB, ! ledEnabled); sync1 (favBtn, proc.isFavorite (proc.getCurrentPreset()));
    const int modeNow = (int) proc.apvts.getRawParameterValue ("mode")->load();
    sync1 (modeIns, modeNow == 0); sync1 (modeAux, modeNow == 1);
    if (modeBox.getSelectedId() != modeNow + 1) modeBox.setSelectedId (modeNow + 1, dontSendNotification);

    const float hb = proc.hostBpm.load();
    hostBpmLabel.setText (hb > 20.f ? String (hb, 1) : String ("--"), dontSendNotification);
    const bool syncOn = sync.getToggleState();
    preLabel.setText (String (roundToInt (proc.effectivePredelay.load())) + "ms", dontSendNotification);
    kPre->s.setEnabled (! syncOn); kPre->setAlpha (syncOn ? 0.5f : 1.f);
    warnLabel.setText (modeNow == 1 && proc.apvts.getRawParameterValue ("dry")->load() > 0.01f ? "Aux Send con Dry > 0 %: duplicas la senal seca. Baja Dry a 0 %." : "", dontSendNotification);
    const auto& ps = proc.getPresets();
    const String nm = proc.getCurrentPreset() < (int) ps.size() ? ps[(size_t) proc.getCurrentPreset()].name : String();
    if (presetBox.getText() != nm) { const ScopedValueSetter<bool> g (updatingUI, true); presetBox.setText (nm, dontSendNotification); }
    saveBtn->setButtonText (proc.isModified() ? "Save *" : "Save");
}

Rectangle<int> YmyEditor::R (float x, float y, float w, float h) const
{ const float s = (float) getWidth() / 1536.f; return { roundToInt (x * s), roundToInt (y * s), roundToInt (w * s), roundToInt (h * s) }; }

void YmyEditor::field (Graphics& g, Rectangle<int> r) const
{ g.setColour (Colour (0xfffbf8f2)); g.fillRoundedRectangle (r.toFloat(), 6.f); g.setColour (Colour (0xffcbbfa7)); g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 6.f, 1.f); }

void YmyEditor::paint (Graphics& g)
{
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    g.drawImage (bgImg, getLocalBounds().toFloat());
    if (lastGlow > 0.f)                                    // lit microphone fades in with the real signal level
    { g.setOpacity (lastGlow); g.drawImage (micOnImg, R (672, 50, 188, 678).toFloat()); g.setOpacity (1.f); }
    // the reference's "TONE" title was clipped; redrawn here
    g.setColour (Colour (0xff8c6a38)); g.setFont (F ((float) R (0, 0, 0, 25).getHeight(), true).withExtraKerningFactor (0.14f));
    g.drawText ("TONE", R (1005, 212, 120, 28), Justification::centredLeft);
    field (g, R (188, 817, 87, 32));
}

void YmyEditor::place (Knob* k, float cx, float cy, int kind)
{
    if (kind == 0) k->setBounds (R (cx - 62, cy - 38, 124, 116));
    else if (kind == 1) k->setBounds (R (cx - 40, cy - 23, 80, 82));
    else k->setBounds (R (cx - 33, cy - 24, 66, 86));
}

void YmyEditor::resized()
{
    place (kRoom, 128, 315, 0); place (kDecay, 277, 315, 0); place (kPre, 431, 315, 0);
    place (kEarly, 111, 444, 0); place (kDiff, 238, 444, 0); place (kDens, 362, 444, 0); place (kMod, 483, 444, 0);
    place (kWidth, 102, 637, 0); place (kBal, 226, 637, 0); place (kErW, 361, 637, 0); place (kSpread, 498, 637, 0);
    place (kWarm, 1067, 287, 0); place (kDamp, 1191, 287, 0); place (kLow, 1310, 287, 0); place (kHigh, 1430, 287, 0);
    place (kLowD, 1081, 421, 0); place (kHighD, 1237, 421, 0); place (kTone, 1395, 421, 0);
    place (kDry, 941, 600, 1); place (kWet, 1020, 600, 1); place (kIn, 1101, 600, 1); place (kOut, 1177, 600, 1); place (kRet, 962, 702, 1);
    place (kThr, 1271, 630, 2); place (kDepth, 1336, 630, 2); place (kAtt, 1400, 630, 2); place (kRel, 1464, 630, 2);
    algo.setBounds (R (187, 241, 163, 32));
    duck.setBounds (R (1030, 684, 76, 72)); makeup.setBounds (R (1118, 684, 84, 72));
    // tempo
    hostBpmLabel.setBounds (R (57, 810, 111, 36)); hostBpmLabel.setFont (F ((float) R (0, 0, 0, 24).getHeight(), true));
    manualBpm.setBounds (R (188, 817, 87, 32));
    sync.setBounds (R (318, 796, 84, 76));
    division.setBounds (R (58, 896, 139, 32));
    btnNorm->setBounds (R (212, 894, 38, 36)); btnTrip->setBounds (R (254, 894, 38, 36)); btnDot->setBounds (R (296, 894, 38, 36));
    preLabel.setBounds (R (360, 897, 90, 36)); preLabel.setFont (F ((float) R (0, 0, 0, 20).getHeight(), true));
    // presets
    catList.setBounds (R (505, 799, 150, 167)); presetList.setBounds (R (658, 799, 237, 167)); descLabel.setBounds (R (900, 799, 208, 167));
    descLabel.setFont (F ((float) R (0, 0, 0, 14).getHeight()));
    warnLabel.setBounds (R (400, 92, 170, 56)); warnLabel.setFont (F ((float) R (0, 0, 0, 13).getHeight()));
    // header
    auto& b = buttons;   // creation order: 0 "<" 1 ">" 2 Save 3 SaveAs 4 A/B 5 Undo 6 Redo | cluster 7.. | tempo/mode buttons
    b[0]->setBounds (R (352, 8, 32, 30)); b[1]->setBounds (R (388, 8, 32, 30));
    presetBox.setBounds (R (428, 8, 312, 30));
    b[2]->setBounds (R (750, 8, 68, 30)); b[3]->setBounds (R (826, 8, 78, 30)); b[4]->setBounds (R (920, 8, 90, 30));
    b[5]->setBounds (R (1018, 8, 52, 30)); b[6]->setBounds (R (1074, 8, 52, 30));
    modeBox.setBounds (R (1366, 8, 120, 30));
    // cluster (3 rows x 4)
    const float cx[4] = { 1152, 1197, 1241, 1285 };
    const float rowY[3] = { 788, 831, 890 };
    const int idx[3][4] = { { 7, 8, 9, 10 }, { 11, 12, 13, 14 }, { 15, 16, 17, -1 } };
    for (int r = 0; r < 3; ++r) for (int c = 0; c < 4; ++c) if (idx[r][c] >= 0) b[(size_t) idx[r][c]]->setBounds (R (cx[c] - 20, rowY[r], 40, 34));
    modeIns->setBounds (R (1343, 808, 68, 26)); modeAux->setBounds (R (1424, 808, 64, 26));
    ledOnB->setBounds (R (1343, 871, 68, 26)); ledOffB->setBounds (R (1424, 871, 64, 26));
    meterIn.setBounds (R (617, 243, 23, 264)); meterOut.setBounds (R (892, 243, 23, 264));
}
