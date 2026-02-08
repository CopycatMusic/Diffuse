#include "PluginProcessor.h"
#include "PluginEditor.h"

CopycatDiffuseProcessor::CopycatDiffuseProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("CopycatDiffuse"),
                 createParameterLayout()) {}

CopycatDiffuseProcessor::~CopycatDiffuseProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout
CopycatDiffuseProcessor::createParameterLayout() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

  // Diffuse Amount (0-2000ms, default 0, skew 0.5 for fine control of lower
  // values)
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"diffuse", 1}, "Diffuse",
      juce::NormalisableRange<float>(0.0f, 2000.0f, 0.1f, 0.5f), 0.0f,
      juce::AudioParameterFloatAttributes().withLabel("ms")));

  // Low Cut (20-2000Hz, default 400Hz, skew 0.5)
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"lowcut", 1}, "Low Cut",
      juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 0.5f), 400.0f,
      juce::AudioParameterFloatAttributes().withLabel("Hz")));

  // FFT Size (3-level choice: Low/Medium/High)
  params.push_back(std::make_unique<juce::AudioParameterChoice>(
      juce::ParameterID{"fftsize", 1}, "FFT Size",
      juce::StringArray{"Low", "Medium", "High"},
      1 // Default: Medium (index 1 = 2048)
      ));

  // Decay toggle
  params.push_back(std::make_unique<juce::AudioParameterBool>(
      juce::ParameterID{"decay", 1}, "Decay", false));

  // Mix (0-100%, default 100% wet)
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"mix", 1}, "Mix",
      juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f,
      juce::AudioParameterFloatAttributes().withLabel("%")));

  return {params.begin(), params.end()};
}

void CopycatDiffuseProcessor::prepareToPlay(double sampleRate,
                                            int samplesPerBlock) {
  engine.prepare(sampleRate, samplesPerBlock);

  // Set initial FFT size from parameter
  auto *fftParam = parameters.getRawParameterValue("fftsize");
  if (fftParam) {
    int fftIndex = static_cast<int>(fftParam->load());
    int fftSizes[] = {512, 1024, 2048}; // Low, Medium, High
    engine.setFFTSize(fftSizes[fftIndex]);
  }
}

void CopycatDiffuseProcessor::releaseResources() { engine.reset(); }

bool CopycatDiffuseProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;

  return true;
}

void CopycatDiffuseProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                           juce::MidiBuffer &) {
  juce::ScopedNoDenormals noDenormals;

  // Update engine parameters from APVTS
  auto *diffuseParam = parameters.getRawParameterValue("diffuse");
  auto *lowcutParam = parameters.getRawParameterValue("lowcut");
  auto *fftsizeParam = parameters.getRawParameterValue("fftsize");

  if (diffuseParam) {
    engine.setDiffuseAmount(diffuseParam->load());
  }

  if (lowcutParam) {
    engine.setLowCut(lowcutParam->load());
  }

  if (fftsizeParam) {
    int fftIndex = static_cast<int>(fftsizeParam->load());
    int fftSizes[] = {512, 1024, 2048}; // Low, Medium, High
    engine.setFFTSize(fftSizes[fftIndex]);
  }

  auto *decayParam = parameters.getRawParameterValue("decay");
  if (decayParam) {
    engine.setDecayEnabled(decayParam->load() > 0.5f);
  }

  auto *mixParam = parameters.getRawParameterValue("mix");
  if (mixParam) {
    engine.setMix(mixParam->load() / 100.0f); // Convert % to 0-1
  }

  // Process audio
  engine.process(buffer);
}

juce::AudioProcessorEditor *CopycatDiffuseProcessor::createEditor() {
  return new CopycatDiffuseEditor(*this);
}

bool CopycatDiffuseProcessor::hasEditor() const { return true; }

const juce::String CopycatDiffuseProcessor::getName() const {
  return JucePlugin_Name;
}

bool CopycatDiffuseProcessor::acceptsMidi() const { return false; }
bool CopycatDiffuseProcessor::producesMidi() const { return false; }
bool CopycatDiffuseProcessor::isMidiEffect() const { return false; }
double CopycatDiffuseProcessor::getTailLengthSeconds() const { return 0.0; }

int CopycatDiffuseProcessor::getNumPrograms() { return 1; }
int CopycatDiffuseProcessor::getCurrentProgram() { return 0; }
void CopycatDiffuseProcessor::setCurrentProgram(int) {}
const juce::String CopycatDiffuseProcessor::getProgramName(int) { return {}; }
void CopycatDiffuseProcessor::changeProgramName(int, const juce::String &) {}

void CopycatDiffuseProcessor::getStateInformation(juce::MemoryBlock &destData) {
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void CopycatDiffuseProcessor::setStateInformation(const void *data,
                                                  int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr) {
    if (xmlState->hasTagName(parameters.state.getType())) {
      parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
  }
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new CopycatDiffuseProcessor();
}
