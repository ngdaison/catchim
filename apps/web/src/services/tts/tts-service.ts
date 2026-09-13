import type { TtsSynthesisOptions, TtsSynthesisResult, TtsVoice } from "./types";

let currentPreviewAudio: HTMLAudioElement | null = null;
let currentPreviewUtterance: SpeechSynthesisUtterance | null = null;

export class TtsService {
	private static audioContext: AudioContext | null = null;

	private static getAudioContext(): AudioContext {
		if (!this.audioContext || this.audioContext.state === "closed") {
			const AudioContextClass =
				window.AudioContext ||
				(window as unknown as { webkitAudioContext: typeof AudioContext }).webkitAudioContext;
			this.audioContext = new AudioContextClass();
		}
		if (this.audioContext.state === "suspended") {
			this.audioContext.resume().catch(() => {});
		}
		return this.audioContext;
	}

	/**
	 * Stop any currently playing voice preview
	 */
	static stopPreview(): void {
		if (currentPreviewAudio) {
			currentPreviewAudio.pause();
			currentPreviewAudio.currentTime = 0;
			currentPreviewAudio = null;
		}
		if (typeof window !== "undefined" && window.speechSynthesis) {
			window.speechSynthesis.cancel();
			currentPreviewUtterance = null;
		}
	}

	/**
	 * Preview a voice sample with optional custom text
	 */
	static async previewVoice(
		voice: TtsVoice,
		customText?: string,
		onEnded?: () => void,
		onError?: (err: unknown) => void,
	): Promise<void> {
		this.stopPreview();

		const textToSpeak = (customText && customText.trim().length > 0)
			? customText.trim()
			: voice.previewSampleText;

		try {
			// Try server-side TTS endpoint first
			const response = await fetch("/api/tts", {
				method: "POST",
				headers: { "Content-Type": "application/json" },
				body: JSON.stringify({
					text: textToSpeak,
					voiceId: voice.id,
					langCode: voice.langCode,
					pitch: voice.pitchOffset ?? 1.0,
					speed: voice.rateOffset ?? 1.0,
				}),
			});

			if (!response.ok) {
				throw new Error(`TTS server error: ${response.status}`);
			}

			const blob = await response.blob();
			const audioUrl = URL.createObjectURL(blob);
			const audio = new Audio(audioUrl);
			currentPreviewAudio = audio;

			audio.onended = () => {
				URL.revokeObjectURL(audioUrl);
				if (currentPreviewAudio === audio) currentPreviewAudio = null;
				onEnded?.();
			};

			audio.onerror = (e) => {
				URL.revokeObjectURL(audioUrl);
				if (currentPreviewAudio === audio) currentPreviewAudio = null;
				// Fallback to browser Web Speech API
				this.previewViaWebSpeech(voice, textToSpeak, onEnded, onError);
			};

			await audio.play();
		} catch (err) {
			// Fallback to browser Web Speech API on network failure or offline
			this.previewViaWebSpeech(voice, textToSpeak, onEnded, onError);
		}
	}

	private static previewViaWebSpeech(
		voice: TtsVoice,
		text: string,
		onEnded?: () => void,
		onError?: (err: unknown) => void,
	): void {
		if (typeof window === "undefined" || !window.speechSynthesis) {
			onError?.(new Error("Web Speech API not supported"));
			return;
		}

		try {
			window.speechSynthesis.cancel();
			const utterance = new SpeechSynthesisUtterance(text);
			utterance.lang = voice.language;
			utterance.pitch = voice.pitchOffset ?? 1.0;
			utterance.rate = voice.rateOffset ?? 1.0;

			// Match voice by language if possible
			const availableVoices = window.speechSynthesis.getVoices();
			const matched = availableVoices.find(
				(v) => v.lang.toLowerCase() === voice.language.toLowerCase() ||
				       v.lang.toLowerCase().startsWith(voice.langCode),
			);
			if (matched) utterance.voice = matched;

			utterance.onend = () => {
				currentPreviewUtterance = null;
				onEnded?.();
			};
			utterance.onerror = (e) => {
				currentPreviewUtterance = null;
				onError?.(e);
			};

			currentPreviewUtterance = utterance;
			window.speechSynthesis.speak(utterance);
		} catch (e) {
			onError?.(e);
		}
	}

	/**
	 * Synthesize speech audio and return decoded AudioBuffer + Blob URL for timeline insertion
	 */
	static async synthesizeSpeechAudio({
		text,
		voice,
		speed = 1.0,
		pitch = 0,
		volume = 1.0,
	}: TtsSynthesisOptions): Promise<TtsSynthesisResult> {
		const cleanText = text.trim();
		if (!cleanText) {
			throw new Error("Văn bản không được để trống.");
		}

		const effectiveSpeed = (voice.rateOffset ?? 1.0) * speed;
		const effectivePitch = (voice.pitchOffset ?? 1.0) + (pitch / 100);

		// 1. Fetch synthesized audio from /api/tts
		const response = await fetch("/api/tts", {
			method: "POST",
			headers: { "Content-Type": "application/json" },
			body: JSON.stringify({
				text: cleanText,
				voiceId: voice.id,
				langCode: voice.langCode,
				speed: effectiveSpeed,
				pitch: effectivePitch,
			}),
		});

		if (!response.ok) {
			throw new Error(`Không thể kết nối máy chủ tạo giọng nói (${response.status})`);
		}

		const blob = await response.blob();
		const arrayBuffer = await blob.arrayBuffer();
		const audioContext = this.getAudioContext();
		const decodedBuffer = await audioContext.decodeAudioData(arrayBuffer.slice(0));

		// Apply volume or retime if needed
		const finalBuffer = (volume !== 1.0)
			? this.applyVolumeToBuffer(audioContext, decodedBuffer, volume)
			: decodedBuffer;

		const blobUrl = URL.createObjectURL(blob);

		return {
			audioBuffer: finalBuffer,
			blobUrl,
			durationSeconds: finalBuffer.duration,
		};
	}

	private static applyVolumeToBuffer(
		ctx: AudioContext,
		buffer: AudioBuffer,
		volume: number,
	): AudioBuffer {
		const out = ctx.createBuffer(
			buffer.numberOfChannels,
			buffer.length,
			buffer.sampleRate,
		);
		for (let ch = 0; ch < buffer.numberOfChannels; ch++) {
			const inData = buffer.getChannelData(ch);
			const outData = out.getChannelData(ch);
			for (let i = 0; i < buffer.length; i++) {
				outData[i] = inData[i] * volume;
			}
		}
		return out;
	}
}
