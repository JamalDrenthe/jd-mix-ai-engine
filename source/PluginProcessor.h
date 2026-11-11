#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

#include "core/VariableBlockFifo.h"
#include "core/LatencyManager.h"
#include "core/SafetyGuard.h"
#include "core/ParamRegistry.h"

#include "dsp/Stage00_Restoration.h"
#include "dsp/Stage01_ClipGainPreRider.h"
#include "dsp/Stage02_SubtractiveDynEq.h"
#include "dsp/Stage03_DeEsserPass1.h"
#include "dsp/Stage04_SerialParallelComp.h"
#include "dsp/Stage05_DeEsserPass2.h"
#include "dsp/Stage06_PitchStudio.h"
#include "dsp/Stage07_HarmonicSaturator.h"
#include "dsp/Stage08_AdditiveToneEq.h"
#include "dsp/Stage09_SpatialSends.h"
#include "dsp/Stage10_11_StemMasterLimiter.h"

#include "analysis/LufsMeter.h"
#include "analysis/SpectrumAnalyzer.h"
#include "analysis/MirFeatureExtractor.h"
#include "ipc/LocalIpcBus.h"
#include "ai/GeminiClient.h"

class JDMixAIEngineAudioProcessor : public juce::AudioProcessor
{
public:
    JDMixAIEngineAudioProcessor();
    ~JDMixAIEngineAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    using juce::AudioProcessor::processBlock;
    using juce::AudioProcessor::processBlockBypassed;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "JD Mix AI Engine"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    jdmix::ipc::LocalIpcBus& getLocalIpc() { return localIpc; }
    jdmix::analysis::SpectrumAnalyzer& getSpectrum() { return spectrumAnalyzer; }
    jdmix::analysis::LufsMeter& getLufsMeter() { return lufsMeter; }
    jdmix::analysis::MirFeatureExtractor& getMirExtractor() { return mirExtractor; }
    jdmix::dsp::Stage06_PitchStudio& getPitchStudio() { return stage06Pitch; }
    jdmix::ai::GeminiClient& getAiClient() { return aiClient; }

    float getRmsInput() const noexcept { return rmsInputLevel.load(std::memory_order_relaxed); }
    float getRmsOutput() const noexcept { return rmsOutputLevel.load(std::memory_order_relaxed); }
    float getGainReduction() const noexcept { return gainReduction.load(std::memory_order_relaxed); }

private:
    juce::AudioProcessorValueTreeState apvts;

    // FL Studio Variable Block FIFO & Latency PDC
    jdmix::core::VariableBlockFifo<float> variableFifo;
    jdmix::core::LatencyManager latencyManager;

    // DSP Stages (8-Table Chain)
    jdmix::dsp::Stage00_Restoration          stage00Restore;
    jdmix::dsp::Stage01_ClipGainPreRider     stage01Gain;
    jdmix::dsp::Stage02_SubtractiveDynEq     stage02SubEq;
    jdmix::dsp::Stage03_DeEsserPass1         stage03DeEssPre;
    jdmix::dsp::Stage04_SerialParallelComp   stage04Dynamics;
    jdmix::dsp::Stage05_DeEsserPass2         stage05DeEssPost;
    jdmix::dsp::Stage06_PitchStudio          stage06Pitch;
    jdmix::dsp::Stage07_HarmonicSaturator    stage07Saturator;
    jdmix::dsp::Stage08_AdditiveToneEq       stage08AddEq;
    jdmix::dsp::Stage09_SpatialSends         stage09Spatial;
    jdmix::dsp::Stage10_11_StemMasterLimiter stage1011Limiter;

    // Real-Time Analysis & Telemetry
    jdmix::analysis::LufsMeter lufsMeter;
    jdmix::analysis::SpectrumAnalyzer spectrumAnalyzer;
    jdmix::analysis::MirFeatureExtractor mirExtractor;

    // Tier 1 Local Shared-Memory IPC Bus
    jdmix::ipc::LocalIpcBus localIpc;
    juce::Uuid instanceUuid;

    // Tier 2 Cloud AI Engine
    jdmix::ai::GeminiClient aiClient;

    // Atomic GUI meters
    std::atomic<float> rmsInputLevel { 0.0f };
    std::atomic<float> rmsOutputLevel { 0.0f };
    std::atomic<float> gainReduction { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JDMixAIEngineAudioProcessor)
};
