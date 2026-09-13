export type VoiceCategory =
	| "all"
	| "vietnamese"
	| "english"
	| "trending"
	| "story"
	| "fun";

export type VoiceGender = "female" | "male" | "child" | "special";

export type VoiceAudioEffect = "none";

export interface TtsVoice {
	id: string;
	name: string;
	language: string; // e.g., 'vi-VN', 'en-US'
	gender: VoiceGender;
	categories: VoiceCategory[];
	description: string;
	avatarGradient: string; // Tailwind gradient classes e.g. "from-pink-500 to-rose-400"
	avatarIcon?: string;
	previewSampleText: string;
	engine: "edge" | "google" | "web-speech";
	edgeVoiceName?: string;
	langCode: string; // 'vi', 'en', etc.
	pitchOffset?: number;
	rateOffset?: number;
	pitchHz?: number; // Base pitch offset in Hz
	rateMultiplier?: number; // Base rate multiplier
	pitchFactor?: number; // Real pitch/resample factor e.g. 1.65 (chipmunk), 1.35 (girl), 0.82 (grandpa), 0.62 (titan)
	audioEffect?: VoiceAudioEffect;
}

export interface TtsSynthesisOptions {
	text: string;
	voice: TtsVoice;
	speed?: number; // 0.5 to 2.0 (default 1.0)
	pitch?: number; // -50 to 50 (default 0)
	volume?: number; // 0 to 1.0 (default 1.0)
}

export interface TtsSynthesisResult {
	audioBuffer: AudioBuffer;
	blobUrl: string;
	durationSeconds: number;
}
