#include "MirFeatureExtractor.h"
#include <sstream>
#include <iomanip>
#include <numeric>
#include <cmath>

namespace jdmix::analysis
{
    void MirFeatureExtractor::prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        snapshot.spectrum32.fill(-100.0f);
    }

    void MirFeatureExtractor::process(const juce::AudioBuffer<float>& buffer,
                                      const LufsMeter& lufs,
                                      const SpectrumAnalyzer& spectrum)
    {
        snapshot.lufsIntegrated = lufs.getIntegratedLufs();
        snapshot.lufsShortTerm  = lufs.getShortTermLufs();
        snapshot.truePeakDb     = lufs.getMaxTruePeak();

        // Crest Factor / PSR = Peak - Short-term LUFS
        snapshot.crestFactorPsr = juce::jlimit(0.0f, 30.0f, snapshot.truePeakDb - snapshot.lufsShortTerm);

        // Fetch 32-band spectrum
        spectrum.get32Bands(snapshot.spectrum32);

        // Compute Spectral Centroid & Spectral Tilt
        double weightedFreqSum = 0.0;
        double totalLinearPower = 0.0;

        for (int i = 0; i < SpectrumAnalyzer::numBands32; ++i)
        {
            float centerFreq = 20.0f * std::pow(1000.0f, (i + 0.5f) / SpectrumAnalyzer::numBands32);
            float linearPower = juce::Decibels::decibelsToGain(snapshot.spectrum32[i]);

            weightedFreqSum += centerFreq * linearPower;
            totalLinearPower += linearPower;
        }

        if (totalLinearPower > 1e-6)
            snapshot.spectralCentroidHz = static_cast<float>(weightedFreqSum / totalLinearPower);
        else
            snapshot.spectralCentroidHz = 1000.0f;

        // Spectral Tilt: difference between low (100 Hz, ~band 5) and high (10 kHz, ~band 27)
        float lowEnergyDb = snapshot.spectrum32[5];
        float highEnergyDb = snapshot.spectrum32[27];
        snapshot.spectralTiltDb = highEnergyDb - lowEnergyDb;

        // Sibilance Ratio: Energy between 5 kHz and 9 kHz (bands ~23 to ~26) vs total energy
        double sibilancePower = 0.0;
        for (int i = 23; i <= 26 && i < SpectrumAnalyzer::numBands32; ++i)
            sibilancePower += juce::Decibels::decibelsToGain(snapshot.spectrum32[i]);

        snapshot.sibilanceRatio = (totalLinearPower > 1e-6)
            ? static_cast<float>(sibilancePower / totalLinearPower)
            : 0.05f;

        // Mud Energy Ratio: 200 Hz to 400 Hz (bands ~9 to ~13)
        double mudPower = 0.0;
        for (int i = 9; i <= 13 && i < SpectrumAnalyzer::numBands32; ++i)
            mudPower += juce::Decibels::decibelsToGain(snapshot.spectrum32[i]);

        snapshot.mudEnergyRatio = (totalLinearPower > 1e-6)
            ? static_cast<float>(mudPower / totalLinearPower)
            : 0.15f;

        updateClassification();
    }

    void MirFeatureExtractor::updateClassification()
    {
        // Acoustic classification heuristics
        if (snapshot.spectralCentroidHz > 3000.0f && snapshot.sibilanceRatio > 0.15f)
        {
            snapshot.suggestedRole = "Lead Vocal";
        }
        else if (snapshot.spectralCentroidHz < 350.0f)
        {
            snapshot.suggestedRole = "808 / Bass";
        }
        else if (snapshot.crestFactorPsr > 14.0f && snapshot.spectralCentroidHz > 800.0f)
        {
            snapshot.suggestedRole = "Drums (Bus)";
        }
        else
        {
            snapshot.suggestedRole = "Synth";
        }
    }

    MirAcousticSnapshot MirFeatureExtractor::getLatestSnapshot() const
    {
        return snapshot;
    }

    std::string MirAcousticSnapshot::toJsonString() const
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{\n";
        ss << "  \"lufsIntegrated\": " << lufsIntegrated << ",\n";
        ss << "  \"lufsShortTerm\": " << lufsShortTerm << ",\n";
        ss << "  \"truePeakDb\": " << truePeakDb << ",\n";
        ss << "  \"crestFactorPsr\": " << crestFactorPsr << ",\n";
        ss << "  \"spectralCentroidHz\": " << spectralCentroidHz << ",\n";
        ss << "  \"spectralTiltDb\": " << spectralTiltDb << ",\n";
        ss << "  \"sibilanceRatio\": " << sibilanceRatio << ",\n";
        ss << "  \"mudEnergyRatio\": " << mudEnergyRatio << ",\n";
        ss << "  \"detectedKey\": \"" << detectedKey << "\",\n";
        ss << "  \"detectedScale\": \"" << detectedScale << "\",\n";
        ss << "  \"suggestedRole\": \"" << suggestedRole << "\",\n";
        ss << "  \"spectrum32\": [";
        for (size_t i = 0; i < spectrum32.size(); ++i)
        {
            ss << spectrum32[i];
            if (i + 1 < spectrum32.size()) ss << ", ";
        }
        ss << "]\n}";
        return ss.str();
    }
}
