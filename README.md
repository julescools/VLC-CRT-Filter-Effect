CRT Scanline Plugin for VLC
The first native CRT scanline video filter ever made for VLC media player.
VLC has over 4 billion downloads and is the world's most popular open-source media player, yet it has never had a native CRT shader plugin. Emulator communities have long enjoyed CRT simulation through RetroArch and mpv shader stacks, but VLC's plugin architecture made this a gap no one filled — until now.
Features

Authentic CRT scanline simulation — cosine-wave brightness modulation on the luma plane mimics the gaussian beam profile of real CRT phosphor lines.
Resolution-aware auto-scaling — scanline spacing scales relative to 480p (NTSC reference), and darkness scales relative to 1080p. A 360p video gets fine, subtle lines; a 1080p video gets the full effect. The visual density stays consistent regardless of source resolution.
Zero-cost bypass — setting darkness to 0 short-circuits the filter entirely, passing frames through with no processing overhead.
Live adjustment via Lua extension — a companion control panel (View → CRT Scanline Controller) lets you adjust darkness, spacing, and blend mode in real time during playback without restarting VLC.
Presets — Subtle, Classic, and Heavy presets for quick switching between looks.
Smooth and hard modes — smooth blend uses a cosine wave for natural phosphor falloff; hard mode produces sharp alternating bright/dark bands.

Parameters
ParameterRangeDefaultDescriptioncrtscanline-darkness0–10035Scanline intensity at 1080p reference (auto-reduced for lower resolutions)crtscanline-spacing1–202Scanline period in pixels at 480p reference (auto-scaled to video resolution)crtscanline-blendon/offonSmooth cosine-wave blending vs hard alternating lines
Technical Details

Architecture: Native VLC video filter module (C99), compiled as a standalone DLL. Not a shader injector, overlay, or external tool — runs inside VLC's filter pipeline.
Pixel processing: Operates on planar YUV frames (I420, J420, YV12, I422, etc.). Modulates the Y (luma) plane per-row; chroma planes pass through unchanged.
Compatibility: VLC 3.0.x on Windows 64-bit. The SDK headers are from VLC 3.0.23; ABI-compatible with any 3.0.x install.
Build requirements: Visual Studio 2022/2026 with C++ desktop workload, VLC 3.0.x SDK (from the win64 zip or 7z archive). Compiles with MSVC using /std:c11.
Limitation: Requires software decoding (--avcodec-hw=none). VLC 3.0.x hardware decoding passes opaque D3D11 textures that CPU-side filters cannot access. This is a known VLC architectural constraint affecting all CPU video filters.
