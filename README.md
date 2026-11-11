<div align="center">

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="assets/jdlogo-dark-transparent.png">
  <img src="assets/jdlogo-mark-light.png" alt="Jamal Drenthe" width="160" style="height: auto; max-width: 160px; object-fit: contain;">
</picture>

# 🎛️ JD MIX AI ENGINE
### **FL STUDIO FIRST | Complete AI-Powered Audio Production Suite**
*8-Table DSP Chain · 120 FPS OpenGL/GLSL Shader GUI · Multi-Instance 60 Hz Shared-Memory Bus · Google Gemini AI Integration*

[![Build Windows VST3 & Standalone EXE](https://github.com/JamalDrenthe/jd-mix-ai-engine/actions/workflows/build-windows.yml/badge.svg)](https://github.com/JamalDrenthe/jd-mix-ai-engine/actions/workflows/build-windows.yml)
[![Platform: Windows x64](https://img.shields.io/badge/Platform-Windows%20x64-0078D6?logo=windows&logoColor=white)](https://github.com/JamalDrenthe/jd-mix-ai-engine)
[![DAW: FL Studio First](https://img.shields.io/badge/FL%20Studio-First%20Class%20Host-FF8800?logo=image-line&logoColor=white)](https://www.image-line.com/fl-studio/)
[![Language: C++20](https://img.shields.io/badge/Language-C%2B%2B20-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![Framework: JUCE 8](https://img.shields.io/badge/Framework-JUCE%208.0.4-222222)](https://juce.com/)
[![AI Engine: Google Gemini](https://img.shields.io/badge/AI-Google%20Gemini%20API-4285F4?logo=google&logoColor=white)](https://ai.google.dev/)
[![Backend: Firebase GCP](https://img.shields.io/badge/Cloud-Firebase%20%7C%20GCP%20570228537386-FFCA28?logo=firebase&logoColor=black)](https://firebase.google.com/)
[![License: AGPL v3](https://img.shields.io/badge/License-AGPLv3-34A853.svg)](LICENSE)

---

### 💾 Direct Downloads (Windows x64)

<a href="https://github.com/JamalDrenthe/jd-mix-ai-engine/releases/download/v1.0.0/JDMixAIEngine_Setup_v1.0.0.exe">
  <img src="https://img.shields.io/badge/DOWNLOAD-WINDOWS%20INSTALLER%20(.EXE)-0071e3?style=for-the-badge&logo=windows&logoColor=white&labelColor=000000" height="42">
</a>
&nbsp;
<a href="https://github.com/JamalDrenthe/jd-mix-ai-engine/releases/download/v1.0.0/JD-Mix-AI-Engine.exe">
  <img src="https://img.shields.io/badge/DOWNLOAD-STANDALONE%20(.EXE)-2997ff?style=for-the-badge&logo=windows&logoColor=white&labelColor=000000" height="42">
</a>
&nbsp;
<a href="https://github.com/JamalDrenthe/jd-mix-ai-engine/releases/download/v1.0.0/JDMixAIEngine-Windows-x64-VST3.zip">
  <img src="https://img.shields.io/badge/DOWNLOAD-FL%20STUDIO%20VST3%20(.ZIP)-30D158?style=for-the-badge&logo=vst&logoColor=white&labelColor=000000" height="42">
</a>

<br><br>

| Package | Format | Purpose | Direct Download Link |
| :--- | :---: | :--- | :---: |
| 🎛️ **Full Windows Setup (Recommended)** | `.exe` | Automatic VST3 directory detection, auto updates & uninstaller | [**`JDMixAIEngine_Setup_v1.0.0.exe`**](https://github.com/JamalDrenthe/jd-mix-ai-engine/releases/download/v1.0.0/JDMixAIEngine_Setup_v1.0.0.exe) |
| 🚀 **Standalone Application** | `.exe` | Portable hardware-accelerated desktop executable (no install needed) | [**`JD-Mix-AI-Engine.exe`**](https://github.com/JamalDrenthe/jd-mix-ai-engine/releases/download/v1.0.0/JD-Mix-AI-Engine.exe) |
| 📦 **FL Studio 64-bit VST3 Plugin** | `.zip` | Portable VST3 bundle for manual installation | [**`JDMixAIEngine-Windows-x64-VST3.zip`**](https://github.com/JamalDrenthe/jd-mix-ai-engine/releases/download/v1.0.0/JDMixAIEngine-Windows-x64-VST3.zip) |
| 🔄 **Always Latest Version** | Auto | Automatically redirects to the latest release | [**Latest Release Download**](https://github.com/JamalDrenthe/jd-mix-ai-engine/releases/latest/download/JDMixAIEngine_Setup_v1.0.0.exe) |

---

</div>

## 🌌 Overview

**JD Mix AI Engine** is a cutting-edge, open-source audio production suite specifically engineered from the ground up for **FL Studio (Fruity Loops) 64-bit**. 

Traditional audio plugins frequently glitch, lose sync, or pop inside FL Studio due to its unique variable buffer-chunking engine (which splits audio buffers into odd chunks of 17, 43, 128, or 384 samples around automation breakpoints). **JD Mix AI Engine solves this at the hardware level** using a deterministic C++20 FIFO ring adapter, lock-free seqlock IPC, and a 2026-standard 8-Table DSP chain driven by Google Gemini Structured Outputs.

Whether you're mixing modern Trapsoul vocals, punchy 808s, or commercial master buses, JD Mix AI Engine provides real-time local DSP combined with deep multimodal cloud intelligence.

```
[Audio in FL Studio Mixer Track]
              │
              ▼
[1. Local C++20 MIR Feature Extraction]
  ├── LUFS-I/S, True Peak (4x IRC), Crest Factor (PSR)
  └── 32-band 1/3-octave spectrum & Mud/Sibilance energy ratios
              │
              ▼
[2. Secure Cloud Gateway / Deterministic Offline Fallback]
  └── Google Gemini 2026 Structured Table Plan
              │
              ▼
[3. Client-Side Sanitization & 80 ms Value Ramps]
  └── Anti-pop interpolation curve + A/B/Delta audition slots
              │
              ▼
[4. Active 8-Table DSP Chain in FL Studio Mixer]
```

---

## ⚡ Key Highlights

### 1. 🎚️ FL Studio First Mandate
* **Deterministic Variable-Block FIFO Buffer Adapter**: Eliminates buffer fragmentation and clicks in FL Studio's Fruity Wrapper.
* **Exact Plugin Delay Compensation (PDC)**: Constant, phase-aligned sample latency reporting via `setLatencySamples()`.
* **Latency-Matched Bypassing**: When bypassed, audio passes through a dedicated delay line to keep automation curves and parallel mixer channels perfectly phase-locked.
* **Denormal & NaN Shield**: Instant soft-limiting at +12 dBFS with denormal/NaN flush-to-zero protect guards.

### 2. 🧠 Tier 1 IPC & Tier 2 Cloud AI Engine
* **Tier 1 (60 Hz Lock-Free Shared Memory)**: Inter-instance telemetry via OS memory-mapped files (`CreateFileMapping` on Windows / `shm_open` on POSIX). Mix tracks see each other's frequency curves in real-time to alert you of frequency masking (e.g. vocal vs. 808 clash).
* **Tier 2 (Cloud AI Mix Session)**: Connects to Firebase and the Google Gemini API. Generates full 8-Table parameter recipes matching 2026 commercial standards.
* **Smooth 80 ms Ramps**: Parameters never step instantly; all filter frequencies, gains, and ratios transition smoothly over 80 ms curves to guarantee click-free auditioning.
* **Zero Host Interruption**: Cloud requests execute on dedicated background worker threads, never blocking the real-time audio thread.

### 3. 🎨 120 FPS GPU Hardware-Accelerated Workspaces
Crafted in an obsidian, graphite, and electric neon aesthetic (`#090A0F`, `#00E5FF`, `#A855F7`, `#84CC16`, `#F59E0B`):
* **View 1: EQ Clean & Decay** — 6 top ribbon dials, Hero Arch Dial (3.20 s / -18 dBFS), dual-curve spectrum with layered smoke FFT ghost trails and masking alerts.
* **View 2: Split-Glass Auto-Tune & 12-Note Pill Matrix** — Live YIN pitch detection, cent deviation meter (-100 to +100 cents), chromatic pill keyboard, retune speed (0–400 ms), humanize, flex tune, and formant/throat shifting.
* **View 3: Chromatic Liquid Shader Orb** — Custom GLSL fragment shader with 4 concentric pulsing neon wave rings, audio timeline, and the *"✨ Auto-Analyze & Optimize"* control center.
* **View 4: 3D Wireframe Spatial Sends & Mixer Bay** — 3D interactive wireframe sphere panner on glowing pedestal, draggable START/A/B motion paths, and 100% parallel sends with dynamic ducking.
* **View 5: 48-Sector 360° Polar Azimuth Radar** — 360-degree decibel radar (1–48 sectors, 0 to -80 dB) visualizing multi-track balance and 40-slot snapshot memory matrix.

---

## 🎛️ The 8-Table DSP Chain (2026 Standard)

| Stage | Module Name | Algorithm / Model | 2026 Target Standard | Technical Action |
|:---:|:---|:---|:---:|:---|
| **0** | **AI Stem & Cleanup** | Spectral Gate & Breath Filter | Noise < -80 dBFS | Removes room reflections, mouth clicks, and reduces breaths |
| **1** | **Clip Gain & Pre-Rider** | RMS Target Leveler | -18 dBFS RMS / 6–8 dB Dyn | Levels 15–20 dB dynamic jumps before compressor stages |
| **2** | **Subtractive Dyn EQ** | ZDF Dynamic Bell & Res Notch | HPF 80–120Hz, Cut 285Hz | Cleans chest mud and boxiness dynamically before compression |
| **3** | **De-Esser Pass 1** | Fast Split-Band Pre-Comp | 2–4 dB GR (5–9 kHz) | Prevents loud sibilants from choking downstream compressors |
| **4** | **Serial + Parallel Comp** | LA-2A Opto + 1176 FET + Crush | Opto 3dB + FET 4dB + 12:1 Blend | Slow optical leveling + ultra-fast peak control + parallel body |
| **5** | **De-Esser Pass 2** | Post-Comp Lookahead Shelf | 1–3 dB GR (6.5–11 kHz) | Tames high-frequency harshness pulled up by heavy compression |
| **6** | **Pitch & Formant Studio** | YIN Detector + Formant Shift | 0–400 ms Retune, ±12 st Formant | Real-time pitch correction, throat length modeling & doubling |
| **7** | **Harmonic Saturator** | Tube, Tape & Console (4x OS) | 15–40% Drive (Even/Odd) | Adds musical warmth and harmonics so vocals cut through dense mixes |
| **8** | **Additive Tone EQ** | Pultec Passive Curve Emulation | 160Hz Warmth, 3.5k Pres, 12k Air | Polishes frequency spectrum after dynamics are fully tamed |
| **9** | **Parallel Spatial Sends** | Room, Plate, Slapback (0% Fbk) | Pre-Delay 20–40ms, Slap 80–120ms | Creates 3D depth and stereo width with ducking on dry vocals |
| **10** | **Vocal Pocket Rider** | Sidechain Cross-Track IPC | -6 to -4 LUFS above Beat | Keeps vocal dynamically locked in the mix pocket |
| **11** | **4-Stem Master Limiter** | Mono-Bass X-Over + 4x IRC TP | -14 LUFS, -1.0 dBTP Ceiling | Mono-summed bass (<90 Hz), inter-sample peak protection |

---

## 📦 What's Included

The engine is available in three ready-to-use formats:

| Format | File Name | Description | Direct Link |
|---|---|---|:---:|
| **Windows Installer** | `JDMixAIEngine_Setup_v1.0.0.exe` | **Recommended.** Single-click installer with automatic FL Studio VST3 directory detection, auto update & clean uninstall. | [**⬇️ Download .exe**](https://github.com/JamalDrenthe/jd-mix-ai-engine/releases/download/v1.0.0/JDMixAIEngine_Setup_v1.0.0.exe) |
| **Standalone Desktop App** | `JD-Mix-AI-Engine.exe` | Portable Windows desktop application. Run JD Mix AI Engine directly without opening any DAW! | [**⬇️ Download .exe**](https://github.com/JamalDrenthe/jd-mix-ai-engine/releases/download/v1.0.0/JD-Mix-AI-Engine.exe) |
| **FL Studio VST3 Plugin** | `JDMixAIEngine-Windows-x64-VST3.zip` | 64-bit VST3 bundle for FL Studio, Ableton Live, Studio One, Reaper, and Cubase. | [**⬇️ Download .zip**](https://github.com/JamalDrenthe/jd-mix-ai-engine/releases/download/v1.0.0/JDMixAIEngine-Windows-x64-VST3.zip) |

---

## 🚀 FL Studio Quick Start Guide

### Step 1: Install
Download and run [**`JDMixAIEngine_Setup_v1.0.0.exe`**](https://github.com/JamalDrenthe/jd-mix-ai-engine/releases/download/v1.0.0/JDMixAIEngine_Setup_v1.0.0.exe), or drop `JDMixAIEngine.vst3` directly into:
```
C:\Program Files\Common Files\VST3\
```

### Step 2: Scan in FL Studio
1. Open **FL Studio**.
2. Click **Options > Manage Plugins**.
3. Enable **Verify plugins** and click **Find installed plugins**.
4. `JD Mix AI Engine` will be detected with categories: `Effect`, `Dynamics`, `EQ`, `Mastering`.

### Step 3: Insert on Mixer Channel
1. Open the FL Studio Mixer (`F9`).
2. Add `JD Mix AI Engine` to **Insert Slot 1** on your Lead Vocal or Instrument track.
3. Add a second instance to your **Master Track** for the 4-Stem Master Limiter (-1.0 dBTP).

### Step 4: Run the AI Workflow
1. Start playback in FL Studio.
2. In **View 3 (Chromatic Liquid Orb)**, click **"✨ Auto-Analyze & Optimize"** (or type a prompt like *"Optimize for modern Trapsoul"*).
3. The orb will pulse in electric cyan and purple while calculating.
4. Parameters smoothly ramp into place over 80 ms without any audio dropout.
5. Use the **A / B / Delta** buttons in the top navigation bar to instantly audit the difference!

---

## 🔄 Continuous Update & Clean Uninstall Flow

Because JD Mix AI Engine receives continuous updates and improvements, the suite is engineered with seamless, zero-friction upgrade and uninstall lifecycles:

### 1. In-App Update Notifications
* On launch, the plugin and standalone app query GitHub Releases asynchronously in the background.
* When a newer version is released, a glowing `🚀 Update vX.X.X Available` pill appears in the top header.
* Clicking the pill instantly takes you to the download page without interrupting ongoing FL Studio playback.

### 2. 1-Click In-Place Upgrades
* Run the new `JDMixAIEngine_Setup_v*.exe` or double-click `tools/update.bat`.
* The installer automatically refreshes running FL Studio instances to prevent file-locking.
* **Preset Protection Guarantee**: Your custom vocal presets, mix memories, and snapshot folders (`Documents\Image-Line\FL Studio\Presets\Plugin presets\Effects\VST3\JD Mix AI Engine`) are **never overwritten or erased** during an update.

### 3. Clean Uninstaller
* Easily uninstall anytime via **Windows Settings > Installed apps** or **Control Panel > Programs and Features**.
* The uninstaller safely removes the VST3 bundle, standalone binary, desktop shortcuts, and registry entries.
* **Polite Preservation Prompt**: Before finalizing, the uninstaller asks: *"Wilt u uw opgeslagen gebruikers-presets en snapshots behouden?"* If you choose Yes, your personal sound bank remains intact for future re-installations.

---

## 🛠️ Building from Source

### Prerequisites
* Windows 10/11 x64
* [Visual Studio 2022](https://visualstudio.microsoft.com/) with C++20 desktop development workload
* [CMake 3.24+](https://cmake.org/download/)
* [Ninja](https://github.com/ninja-build/ninja/releases) (optional, recommended)
* [Git](https://git-scm.com/)

### Clone and Build
```powershell
# Clone the repository with submodules (JUCE 8.0.4)
git clone --recursive https://github.com/JamalDrenthe/jd-mix-ai-engine.git
cd jd-mix-ai-engine

# Configure with CMake
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

# Build VST3 plugin, Standalone .exe, and test executables
cmake --build build --config Release --target JDMixAIEngine_VST3 JDMixAIEngine_Standalone FifoFuzzTest DspChainTest

# Run automated verification tests
.\build\FifoFuzzTest.exe
.\build\DspChainTest.exe
```

---

## 🧪 Verification & Automated Testing

The codebase includes automated test suites to ensure rock-solid stability in production DAWs:

```
[Test 1] Running VariableBlockFifo Fuzzing Test for FL Studio...
✓ VariableBlockFifo Fuzzing Passed: Processed 2297 samples in arbitrary chunks (17, 43, 128, 384).
[Test 2] Running DSP Chain & True Peak Limiter Test...
✓ Safety Guard & DSP Limiter verified successfully (Signal clamped safely below -1.0 dBTP ceiling, 0 NaNs).
```

---

## 📁 Repository Structure

```
jd-mix-ai-engine/
├── .github/workflows/          # CI matrix building Windows VST3, Standalone .exe & Installer
├── backend/                    # Firebase Cloud Functions (Gemini API Gateway, Firestore rules)
├── external/JUCE/              # JUCE 8.0.4 framework submodule
├── installer/windows/          # Inno Setup script for automated Windows installation
├── source/
│   ├── ai/                     # Gemini JSON schema, HTTPS client, 80 ms parameter applier
│   ├── analysis/               # LUFS meter (BS.1770-4), 64-band FFT, acoustic MIR extractor
│   ├── core/                   # VariableBlockFifo, LatencyManager, SafetyGuard, ParamRegistry
│   ├── dsp/                    # Stages 00 through 11 (8-Table 2026 Production DSP Chain)
│   ├── gui/                    # LookAndFeel, GLSL shaders, Views 1 through 5
│   ├── ipc/                    # 60 Hz lock-free Named Shared-Memory inter-track bus
│   ├── PluginProcessor.cpp/h   # Main audio processor & APVTS parameter layout
│   └── PluginEditor.cpp/h      # Master UI frame, view switching & 60 FPS repaint timer
├── tests/                      # FifoFuzzTest & DspChainTest suites
├── CMakeLists.txt              # Unified CMake configuration
├── LICENSE                     # GNU Affero General Public License v3 (AGPL-3.0)
└── README.md                   # You are here!
```

---

## 📄 License & Open Source

This project is licensed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**. See the [LICENSE](LICENSE) file for details.

* Designed & Developed by **Jamal Drenthe** (`JD Audio Labs`).
* Built with **JUCE 8**, **C++20**, **OpenGL**, **Firebase**, and **Google Gemini**.

<div align="center">

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="assets/jdlogo-dark-transparent.png">
  <img src="assets/jdlogo-mark-light.png" alt="Jamal Drenthe" width="70" style="height: auto; max-width: 70px; object-fit: contain;">
</picture>

<sub>Crafted with precision by Jamal Drenthe (JD Audio Labs) for FL Studio producers worldwide.</sub>
</div>
