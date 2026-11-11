#version 330 core
in vec2 vUv;
out vec4 fragColor;

uniform float uTime;
uniform float uRmsLevel;        // Coupled to Stage 1 Input RMS (-18 dBFS target)
uniform float uFormantShift;    // Coupled to Stage 6 Formant (-1.0 to +1.0)
uniform float uAiTransforming;  // 0.0 = Idle/Audio-Reactive, 1.0 = AI Processing

float waveRing(vec2 uv, float radius, float freq, float amp, float speed, float phase) {
    float angle = atan(uv.y, uv.x);
    float modulation = sin(angle * freq + uTime * speed + phase) * amp
                     + cos(angle * (freq * 1.5) - uTime * (speed * 0.7)) * (amp * 0.6);
    float dist = length(uv) + modulation;
    return smoothstep(0.022, 0.0, abs(dist - radius));
}

void main() {
    vec2 uv = (vUv - 0.5) * 2.2;
    float r = length(uv);

    // Dark sunken lens background with vignette
    vec3 col = vec3(0.035, 0.04, 0.06) * (1.0 - 0.35 * r);

    float dynAmp = 0.025 + uRmsLevel * 0.06 + uAiTransforming * 0.04;
    float speed  = 1.8 + uAiTransforming * 3.5;

    // 4 Concentric Iridescent Rings (Cyan, Green, Purple/Magenta, Gold/Orange)
    for (int i = 0; i < 4; ++i) {
        float fi = float(i);
        float baseRad = 0.30 + fi * 0.14 + uFormantShift * 0.05;
        float freq    = 5.0 + fi * 2.0;

        // Chromatic RGB separation per ring for neon glass refraction
        float ringR = waveRing(uv, baseRad + 0.008, freq, dynAmp, speed, fi * 1.3);
        float ringG = waveRing(uv, baseRad,         freq, dynAmp, speed, fi * 1.3 + 0.15);
        float ringB = waveRing(uv, baseRad - 0.008, freq, dynAmp, speed, fi * 1.3 + 0.30);

        vec3 layerColor = (i % 2 == 0)
            ? vec3(ringR * 0.2, ringG * 0.85, ringB * 1.0)   // Electric Cyan / Green
            : vec3(ringR * 1.0, ringG * 0.45, ringB * 0.95); // Neon Magenta / Amber-Gold

        col += layerColor * (1.1 - fi * 0.15);
    }

    // Circular lens mask with subtle diagonal glass sheen
    float lensMask = smoothstep(0.98, 0.94, r);
    float glassSheen = smoothstep(0.05, -0.4, uv.x + uv.y * 0.6) * 0.06;
    col = (col + glassSheen) * lensMask;

    fragColor = vec4(col, lensMask);
}
