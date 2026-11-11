#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <string_view>
#include <vector>

namespace jdmix::core
{
    namespace ParamIDs
    {
        // Table 2: Restoration, Gain Staging & Pre-Riding
        inline constexpr const char* prepDenoiseDereverb = "prep_denoise_dereverb";
        inline constexpr const char* prepBreathCtrl      = "prep_breath_ctrl";
        inline constexpr const char* gainClipTrim        = "gain_clip_trim";
        inline constexpr const char* gainPreRider        = "gain_pre_rider";

        // Table 3: Subtractive & Additive EQ
        inline constexpr const char* eqSubHpf            = "eq_sub_hpf";
        inline constexpr const char* eqSubMudFreq        = "eq_sub_mud_freq";
        inline constexpr const char* eqSubMudGain        = "eq_sub_mud_gain";
        inline constexpr const char* eqSubResFreq        = "eq_sub_res_freq";
        inline constexpr const char* eqSubResGain        = "eq_sub_res_gain";
        inline constexpr const char* eqAddWarmthFreq     = "eq_add_warmth_freq";
        inline constexpr const char* eqAddWarmthGain     = "eq_add_warmth_gain";
        inline constexpr const char* eqAddPresenceFreq   = "eq_add_presence_freq";
        inline constexpr const char* eqAddPresenceGain   = "eq_add_presence_gain";
        inline constexpr const char* eqAddAirFreq        = "eq_add_air_freq";
        inline constexpr const char* eqAddAirGain        = "eq_add_air_gain";

        // Table 4: De-Essing, Dynamics & Saturation
        inline constexpr const char* dynDeessPreThresh   = "dyn_deess_pre_thresh";
        inline constexpr const char* dynDeessPreFreq     = "dyn_deess_pre_freq";
        inline constexpr const char* dynCompOptoThresh   = "dyn_comp_opto_thresh";
        inline constexpr const char* dynCompOptoGain     = "dyn_comp_opto_gain";
        inline constexpr const char* dynCompFetThresh    = "dyn_comp_fet_thresh";
        inline constexpr const char* dynCompFetRatio     = "dyn_comp_fet_ratio";
        inline constexpr const char* dynCompFetAttack    = "dyn_comp_fet_attack";
        inline constexpr const char* dynCompFetRelease   = "dyn_comp_fet_release";
        inline constexpr const char* dynCompParBlend     = "dyn_comp_par_blend";
        inline constexpr const char* dynDeessPostThresh  = "dyn_deess_post_thresh";
        inline constexpr const char* dynDeessPostFreq    = "dyn_deess_post_freq";
        inline constexpr const char* toneSatType         = "tone_sat_type";
        inline constexpr const char* toneSatDrive        = "tone_sat_drive";
        inline constexpr const char* toneSatMix          = "tone_sat_mix";

        // Table 5: Pitch, Formant & Tuning
        inline constexpr const char* pitchRetuneSpeed    = "pitch_retune_speed";
        inline constexpr const char* pitchHumanize       = "pitch_humanize";
        inline constexpr const char* pitchFlexTune       = "pitch_flex_tune";
        inline constexpr const char* pitchFormantShift   = "pitch_formant_shift";
        inline constexpr const char* pitchThroatLength   = "pitch_throat_length";
        inline constexpr const char* pitchKey            = "pitch_key";
        inline constexpr const char* pitchScale          = "pitch_scale";
        inline constexpr const char* pitchMode           = "pitch_mode";

        // Table 6: Spatial Sends
        inline constexpr const char* sendShortRoom       = "send_short_room";
        inline constexpr const char* sendPlateSend       = "send_plate_send";
        inline constexpr const char* sendPlatePredelay   = "send_plate_predelay";
        inline constexpr const char* sendSlapSend        = "send_slap_send";
        inline constexpr const char* sendSlapTime        = "send_slap_time";
        inline constexpr const char* sendThrow3dSend     = "send_throw_3d_send";
        inline constexpr const char* sendThrowDucking     = "send_throw_ducking";

        // Table 7: Stem Mastering & Metering
        inline constexpr const char* masterTargetLufs    = "master_target_lufs";
        inline constexpr const char* masterTruePeakCeil  = "master_true_peak_ceil";
        inline constexpr const char* masterVocalPocket   = "master_vocal_pocket";
        inline constexpr const char* masterMonoBassFreq  = "master_mono_bass_freq";

        // Global Meta & AI
        inline constexpr const char* trackRole           = "track_role";
        inline constexpr const char* aiIntensity         = "ai_intensity";
        inline constexpr const char* currentView         = "current_view";
    }

    class ParamRegistry
    {
    public:
        static juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
        {
            std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

            // Table 2
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::prepDenoiseDereverb, 1 }, "De-Noise/Reverb",
                juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::prepBreathCtrl, 1 }, "Breath Control",
                juce::NormalisableRange<float>(-36.0f, 0.0f, 0.5f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::gainClipTrim, 1 }, "Clip Trim",
                juce::NormalisableRange<float>(-36.0f, 36.0f, 0.1f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::gainPreRider, 1 }, "Pre-Rider Target",
                juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

            // Table 3: Subtractive & Additive EQ
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::eqSubHpf, 1 }, "Sub HPF Freq",
                juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f, 0.5f), 90.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::eqSubMudFreq, 1 }, "Sub Mud Freq",
                juce::NormalisableRange<float>(120.0f, 800.0f, 1.0f, 0.5f), 285.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::eqSubMudGain, 1 }, "Sub Mud Gain",
                juce::NormalisableRange<float>(-18.0f, 6.0f, 0.1f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::eqSubResFreq, 1 }, "Sub Res Freq",
                juce::NormalisableRange<float>(1500.0f, 6000.0f, 1.0f, 0.5f), 3200.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::eqSubResGain, 1 }, "Sub Res Gain",
                juce::NormalisableRange<float>(-18.0f, 6.0f, 0.1f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::eqAddWarmthFreq, 1 }, "Add Warmth Freq",
                juce::NormalisableRange<float>(80.0f, 350.0f, 1.0f, 0.5f), 160.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::eqAddWarmthGain, 1 }, "Add Warmth Gain",
                juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::eqAddPresenceFreq, 1 }, "Add Presence Freq",
                juce::NormalisableRange<float>(1800.0f, 6000.0f, 1.0f, 0.5f), 3500.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::eqAddPresenceGain, 1 }, "Add Presence Gain",
                juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::eqAddAirFreq, 1 }, "Add Air Freq",
                juce::NormalisableRange<float>(6000.0f, 20000.0f, 10.0f, 0.5f), 12000.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::eqAddAirGain, 1 }, "Add Air Gain",
                juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

            // Table 4: Dynamics & Saturation
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::dynDeessPreThresh, 1 }, "De-Ess Pre Thresh",
                juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::dynDeessPreFreq, 1 }, "De-Ess Pre Freq",
                juce::NormalisableRange<float>(4000.0f, 10000.0f, 10.0f, 0.5f), 7000.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::dynCompOptoThresh, 1 }, "Opto Comp Thresh",
                juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::dynCompOptoGain, 1 }, "Opto Make-up Gain",
                juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::dynCompFetThresh, 1 }, "FET Comp Thresh",
                juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID { ParamIDs::dynCompFetRatio, 1 }, "FET Ratio",
                juce::StringArray { "4:1", "8:1", "12:1", "20:1" }, 0));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::dynCompFetAttack, 1 }, "FET Attack",
                juce::NormalisableRange<float>(0.05f, 20.0f, 0.05f, 0.4f), 0.5f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::dynCompFetRelease, 1 }, "FET Release",
                juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 0.5f), 50.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::dynCompParBlend, 1 }, "Parallel Comp Blend",
                juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::dynDeessPostThresh, 1 }, "De-Ess Post Thresh",
                juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::dynDeessPostFreq, 1 }, "De-Ess Post Freq",
                juce::NormalisableRange<float>(6000.0f, 12000.0f, 10.0f, 0.5f), 8500.0f));

            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID { ParamIDs::toneSatType, 1 }, "Sat Type",
                juce::StringArray { "Tube", "Tape", "Console" }, 0));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::toneSatDrive, 1 }, "Sat Drive",
                juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::toneSatMix, 1 }, "Sat Mix",
                juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 30.0f));

            // Table 5: Pitch & Tuning
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::pitchRetuneSpeed, 1 }, "Retune Speed",
                juce::NormalisableRange<float>(0.0f, 400.0f, 0.5f, 0.4f), 25.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::pitchHumanize, 1 }, "Humanize",
                juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 40.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::pitchFlexTune, 1 }, "Flex Tune",
                juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 25.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::pitchFormantShift, 1 }, "Formant Shift",
                juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::pitchThroatLength, 1 }, "Throat Length",
                juce::NormalisableRange<float>(50.0f, 150.0f, 0.5f), 100.0f));

            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID { ParamIDs::pitchKey, 1 }, "Key",
                juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 6)); // Default F#

            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID { ParamIDs::pitchScale, 1 }, "Scale",
                juce::StringArray { "Chromatic", "Major", "Natural Minor", "Harmonic Minor" }, 0));

            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID { ParamIDs::pitchMode, 1 }, "Pitch Mode",
                juce::StringArray { "Low Latency (Live)", "High Quality (HQ)" }, 0));

            // Table 6: Spatial Sends
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::sendShortRoom, 1 }, "Short Room Send",
                juce::NormalisableRange<float>(-60.0f, 6.0f, 0.5f), -14.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::sendPlateSend, 1 }, "Plate Hall Send",
                juce::NormalisableRange<float>(-60.0f, 6.0f, 0.5f), -12.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::sendPlatePredelay, 1 }, "Plate Pre-Delay",
                juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 30.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::sendSlapSend, 1 }, "Slapback Send",
                juce::NormalisableRange<float>(-60.0f, 6.0f, 0.5f), -18.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::sendSlapTime, 1 }, "Slapback Delay Time",
                juce::NormalisableRange<float>(40.0f, 200.0f, 1.0f), 100.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::sendThrow3dSend, 1 }, "Throw 3D Send",
                juce::NormalisableRange<float>(-60.0f, 6.0f, 0.5f), -16.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::sendThrowDucking, 1 }, "Throw Auto-Ducking",
                juce::NormalisableRange<float>(0.0f, 18.0f, 0.5f), 8.0f));

            // Table 7: Stem Master & Target Loudness
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::masterTargetLufs, 1 }, "Target LUFS",
                juce::NormalisableRange<float>(-24.0f, -6.0f, 0.1f), -14.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::masterTruePeakCeil, 1 }, "True Peak Ceiling",
                juce::NormalisableRange<float>(-3.0f, 0.0f, 0.1f), -1.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::masterVocalPocket, 1 }, "Vocal Pocket Offset",
                juce::NormalisableRange<float>(-12.0f, 0.0f, 0.1f), -5.0f));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::masterMonoBassFreq, 1 }, "Mono Bass Crossover",
                juce::NormalisableRange<float>(40.0f, 180.0f, 1.0f), 90.0f));

            // Global Meta & AI
            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID { ParamIDs::trackRole, 1 }, "Track Role",
                juce::StringArray { "Lead Vocal", "Backing Vocal", "Ad-Lib", "Harmony",
                                    "Drums (Bus)", "Kick", "Snare", "Hi-Hats",
                                    "808 / Bass", "Synth", "Guitar", "Piano / Keys",
                                    "Vocal Bus", "Instrument Bus", "Master" }, 0));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::aiIntensity, 1 }, "AI Intensity",
                juce::NormalisableRange<float>(0.0f, 150.0f, 1.0f), 100.0f));

            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID { ParamIDs::currentView, 1 }, "Current View",
                juce::StringArray { "View 1: EQ & Decay", "View 2: Pitch Matrix",
                                    "View 3: AI Resynth Orb", "View 4: 3D Space",
                                    "View 5: Stem Master Radar" }, 0));

            return { params.begin(), params.end() };
        }
    };
}
