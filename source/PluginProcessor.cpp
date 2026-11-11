#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace jdmix;

JDMixAIEngineAudioProcessor::JDMixAIEngineAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", core::ParamRegistry::createLayout()),
      localIpc("JDMixAIEngine_SharedMem"),
      aiClient(apvts)
{
    instanceUuid = juce::Uuid();
}

JDMixAIEngineAudioProcessor::~JDMixAIEngineAudioProcessor()
{
    localIpc.unregisterInstance(instanceUuid);
}

void JDMixAIEngineAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    // FL Studio Variable Block FIFO: fixed 1024-sample internal processing frame
    variableFifo.prepare(2, 1024);

    // Set PDC Latency in FL Studio
    setLatencySamples(variableFifo.getLatencySamples());
    latencyManager.prepare(2, variableFifo.getLatencySamples());

    // Prepare DSP Stages
    stage00Restore.prepare(sampleRate, 1024);
    stage01Gain.prepare(sampleRate, 1024);
    stage02SubEq.prepare(sampleRate, 1024);
    stage03DeEssPre.prepare(sampleRate, 1024);
    stage04Dynamics.prepare(sampleRate, 1024);
    stage05DeEssPost.prepare(sampleRate, 1024);
    stage06Pitch.prepare(sampleRate, 1024);
    stage07Saturator.prepare(sampleRate, 1024);
    stage08AddEq.prepare(sampleRate, 1024);
    stage09Spatial.prepare(sampleRate, 1024);
    stage1011Limiter.prepare(sampleRate, 1024);

    // Prepare Analysis
    lufsMeter.prepare(sampleRate, 1024);
    spectrumAnalyzer.prepare(sampleRate);
    mirExtractor.prepare(sampleRate);

    // Register instance on Local IPC Bus
    localIpc.registerInstance(instanceUuid, "Mixer Insert", 0);
}

void JDMixAIEngineAudioProcessor::releaseResources()
{
    variableFifo.reset();
    stage00Restore.reset();
    stage01Gain.reset();
    stage02SubEq.reset();
    stage03DeEssPre.reset();
    stage04Dynamics.reset();
    stage05DeEssPost.reset();
    stage06Pitch.reset();
    stage07Saturator.reset();
    stage08AddEq.reset();
    stage09Spatial.reset();
    stage1011Limiter.reset();
    lufsMeter.reset();
}

bool JDMixAIEngineAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void JDMixAIEngineAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    if (numSamples == 0)
        return;

    // Track input RMS
    float inRms = buffer.getRMSLevel(0, 0, numSamples);
    if (totalNumInputChannels > 1)
        inRms = juce::jmax(inRms, buffer.getRMSLevel(1, 0, numSamples));
    rmsInputLevel.store(inRms, std::memory_order_relaxed);

    // Push into Variable Block FIFO to handle arbitrary FL Studio buffer chunks
    variableFifo.writeSamples(buffer);

    while (variableFifo.hasBlockReady())
    {
        auto& fixedBlock = variableFifo.getReadyBlock();

        // 1. Stage 0: Restoration
        float denoise = *apvts.getRawParameterValue(core::ParamIDs::prepDenoiseDereverb);
        float breath = *apvts.getRawParameterValue(core::ParamIDs::prepBreathCtrl);
        stage00Restore.process(fixedBlock, denoise, breath);

        // 2. Stage 1: Clip Gain & Pre-Rider
        float clipTrim = *apvts.getRawParameterValue(core::ParamIDs::gainClipTrim);
        float preRider = *apvts.getRawParameterValue(core::ParamIDs::gainPreRider);
        stage01Gain.process(fixedBlock, clipTrim, preRider);

        // 3. Stage 2: Subtractive Dynamic EQ
        float hpfFreq = *apvts.getRawParameterValue(core::ParamIDs::eqSubHpf);
        float mudFreq = *apvts.getRawParameterValue(core::ParamIDs::eqSubMudFreq);
        float mudGain = *apvts.getRawParameterValue(core::ParamIDs::eqSubMudGain);
        float resFreq = *apvts.getRawParameterValue(core::ParamIDs::eqSubResFreq);
        float resGain = *apvts.getRawParameterValue(core::ParamIDs::eqSubResGain);
        stage02SubEq.process(fixedBlock, hpfFreq, mudFreq, mudGain, resFreq, resGain);

        // 4. Stage 3: De-Esser Pass 1 (Pre-Comp)
        float deEssPreThresh = *apvts.getRawParameterValue(core::ParamIDs::dynDeessPreThresh);
        float deEssPreFreq   = *apvts.getRawParameterValue(core::ParamIDs::dynDeessPreFreq);
        stage03DeEssPre.process(fixedBlock, deEssPreThresh, deEssPreFreq);

        // 5. Stage 4: Serial (Opto + FET) & Parallel Compression
        float optoThresh = *apvts.getRawParameterValue(core::ParamIDs::dynCompOptoThresh);
        float optoGain   = *apvts.getRawParameterValue(core::ParamIDs::dynCompOptoGain);
        float fetThresh  = *apvts.getRawParameterValue(core::ParamIDs::dynCompFetThresh);
        int   fetRatio   = static_cast<int>(*apvts.getRawParameterValue(core::ParamIDs::dynCompFetRatio));
        float fetAttack  = *apvts.getRawParameterValue(core::ParamIDs::dynCompFetAttack);
        float fetRelease = *apvts.getRawParameterValue(core::ParamIDs::dynCompFetRelease);
        float parBlend   = *apvts.getRawParameterValue(core::ParamIDs::dynCompParBlend);
        float gr = stage04Dynamics.process(fixedBlock, optoThresh, optoGain, fetThresh, fetRatio, fetAttack, fetRelease, parBlend);
        gainReduction.store(gr, std::memory_order_relaxed);

        // 6. Stage 5: De-Esser Pass 2 (Post-Comp)
        float deEssPostThresh = *apvts.getRawParameterValue(core::ParamIDs::dynDeessPostThresh);
        float deEssPostFreq   = *apvts.getRawParameterValue(core::ParamIDs::dynDeessPostFreq);
        stage05DeEssPost.process(fixedBlock, deEssPostThresh, deEssPostFreq);

        // 7. Stage 6: Pitch Studio & Formant Shift
        float retuneSpeed = *apvts.getRawParameterValue(core::ParamIDs::pitchRetuneSpeed);
        float humanize    = *apvts.getRawParameterValue(core::ParamIDs::pitchHumanize);
        float flexTune    = *apvts.getRawParameterValue(core::ParamIDs::pitchFlexTune);
        float formant     = *apvts.getRawParameterValue(core::ParamIDs::pitchFormantShift);
        float throat      = *apvts.getRawParameterValue(core::ParamIDs::pitchThroatLength);
        int   keyRoot     = static_cast<int>(*apvts.getRawParameterValue(core::ParamIDs::pitchKey));
        int   scale       = static_cast<int>(*apvts.getRawParameterValue(core::ParamIDs::pitchScale));
        int   mode        = static_cast<int>(*apvts.getRawParameterValue(core::ParamIDs::pitchMode));
        stage06Pitch.process(fixedBlock, retuneSpeed, humanize, flexTune, formant, throat, keyRoot, scale, mode);

        // 8. Stage 7: Harmonic Saturation
        int   satType    = static_cast<int>(*apvts.getRawParameterValue(core::ParamIDs::toneSatType));
        float satDrive   = *apvts.getRawParameterValue(core::ParamIDs::toneSatDrive);
        float satMix     = *apvts.getRawParameterValue(core::ParamIDs::toneSatMix);
        stage07Saturator.process(fixedBlock, satType, satDrive, satMix);

        // 9. Stage 8: Additive Tone EQ
        float warmthFreq = *apvts.getRawParameterValue(core::ParamIDs::eqAddWarmthFreq);
        float warmthGain = *apvts.getRawParameterValue(core::ParamIDs::eqAddWarmthGain);
        float presFreq   = *apvts.getRawParameterValue(core::ParamIDs::eqAddPresenceFreq);
        float presGain   = *apvts.getRawParameterValue(core::ParamIDs::eqAddPresenceGain);
        float airFreq    = *apvts.getRawParameterValue(core::ParamIDs::eqAddAirFreq);
        float airGain    = *apvts.getRawParameterValue(core::ParamIDs::eqAddAirGain);
        stage08AddEq.process(fixedBlock, warmthFreq, warmthGain, presFreq, presGain, airFreq, airGain);

        // 10. Stage 9: Parallel Spatial Sends
        float roomSend     = *apvts.getRawParameterValue(core::ParamIDs::sendShortRoom);
        float plateSend    = *apvts.getRawParameterValue(core::ParamIDs::sendPlateSend);
        float platePreDly  = *apvts.getRawParameterValue(core::ParamIDs::sendPlatePredelay);
        float slapSend     = *apvts.getRawParameterValue(core::ParamIDs::sendSlapSend);
        float slapTime     = *apvts.getRawParameterValue(core::ParamIDs::sendSlapTime);
        float throwSend    = *apvts.getRawParameterValue(core::ParamIDs::sendThrow3dSend);
        float throwDucking = *apvts.getRawParameterValue(core::ParamIDs::sendThrowDucking);
        stage09Spatial.process(fixedBlock, roomSend, plateSend, platePreDly, slapSend, slapTime, throwSend, throwDucking);

        // 11. Stage 10 & 11: Stem Master Limiter
        float targetLufs   = *apvts.getRawParameterValue(core::ParamIDs::masterTargetLufs);
        float tpCeiling    = *apvts.getRawParameterValue(core::ParamIDs::masterTruePeakCeil);
        float vocalPocket  = *apvts.getRawParameterValue(core::ParamIDs::masterVocalPocket);
        float monoBassFreq = *apvts.getRawParameterValue(core::ParamIDs::masterMonoBassFreq);
        stage1011Limiter.process(fixedBlock, targetLufs, tpCeiling, vocalPocket, monoBassFreq);

        // Analysis: Meters and Spectrum
        lufsMeter.process(fixedBlock);
        spectrumAnalyzer.pushAudio(fixedBlock);
        mirExtractor.process(fixedBlock, lufsMeter, spectrumAnalyzer);

        variableFifo.blockProcessed();
    }

    variableFifo.readProcessedSamples(buffer);

    // Audio Safety Guard: NaN/Inf protection and soft limiter
    core::SafetyGuard::process(buffer);

    // Track output RMS
    float outRms = buffer.getRMSLevel(0, 0, numSamples);
    if (totalNumOutputChannels > 1)
        outRms = juce::jmax(outRms, buffer.getRMSLevel(1, 0, numSamples));
    rmsOutputLevel.store(outRms, std::memory_order_relaxed);

    // Publish to Tier 1 Local IPC Bus
    std::array<float, ipc::ipcSpectrumBands> spec64;
    spectrumAnalyzer.get64Bands(spec64);
    localIpc.updateTelemetry(instanceUuid, outRms, inRms - outRms,
                             lufsMeter.getShortTermLufs(), spec64.data());
}

void JDMixAIEngineAudioProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // Maintain sample-accurate PDC delay line through bypass
    latencyManager.processBypassed(buffer);
}

juce::AudioProcessorEditor* JDMixAIEngineAudioProcessor::createEditor()
{
    return new JDMixAIEngineAudioProcessorEditor(*this);
}

void JDMixAIEngineAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void JDMixAIEngineAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JDMixAIEngineAudioProcessor();
}
