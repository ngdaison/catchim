import type { TtsSynthesisOptions, TtsSynthesisResult, TtsVoice } from "./types";

let currentPreviewAudio: HTMLAudioElement | null = null;
let currentPreviewUtterance: SpeechSynthesisUtterance | null = null;

function writeString(view: DataView, offset: number, str: string): void {
	for (let i = 0; i < str.length; i++) {
		view.setUint8(offset + i, str.charCodeAt(i));
	}
}

/**
 * Convert AudioBuffer into a standard 16-bit PCM WAV Blob
 */
function audioBufferToWavBlob(buffer: AudioBuffer): Blob {
	const numChannels = buffer.numberOfChannels;
	const sampleRate = buffer.sampleRate;
	const format = 1; // PCM
	const bitDepth = 16;
	const bytesPerSample = bitDepth / 8;
	const blockAlign = numChannels * bytesPerSample;
	const dataLength = buffer.length * blockAlign;
	const bufferLength = 44 + dataLength;

	const arrayBuffer = new ArrayBuffer(bufferLength);
	const view = new DataView(arrayBuffer);

	// RIFF header
	writeString(view, 0, "RIFF");
	view.setUint32(4, 36 + dataLength, true);
	writeString(view, 8, "WAVE");

	// fmt chunk
	writeString(view, 12, "fmt ");
	view.setUint32(16, 16, true);
	view.setUint16(20, format, true);
	view.setUint16(22, numChannels, true);
	view.setUint32(24, sampleRate, true);
	view.setUint32(28, sampleRate * blockAlign, true);
	view.setUint16(32, blockAlign, true);
	view.setUint16(34, bitDepth, true);

	// data chunk
	writeString(view, 36, "data");
	view.setUint32(40, dataLength, true);

	let offset = 44;
	const channels: Float32Array[] = [];
	for (let c = 0; c < numChannels; c++) {
		channels.push(buffer.getChannelData(c));
	}

	for (let i = 0; i < buffer.length; i++) {
		for (let c = 0; c < numChannels; c++) {
			let sample = channels[c][i];
			sample = Math.max(-1, Math.min(1, sample));
			const intSample = sample < 0 ? sample * 0x8000 : sample * 0x7fff;
			view.setInt16(offset, intSample, true);
			offset += 2;
		}
	}

	return new Blob([arrayBuffer], { type: "audio/wav" });
}

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
	 * Preview a voice sample with full character pitch, speed, and DSP effects
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
			// Fetch synthesized audio from /api/tts
			const response = await fetch("/api/tts", {
				method: "POST",
				headers: { "Content-Type": "application/json" },
				body: JSON.stringify({
					text: textToSpeak,
					voiceId: voice.id,
					edgeVoiceName: voice.edgeVoiceName,
					engine: voice.engine,
					langCode: voice.langCode,
					userPitch: 0,
					userSpeed: 1.0,
				}),
			});

			if (!response.ok) {
				let msg = `TTS server error: ${response.status}`;
				try {
					const data = await response.json();
					if (data?.error) msg = data.error;
				} catch {}
				throw new Error(msg);
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
			audio.onerror = () => {
				URL.revokeObjectURL(audioUrl);
				if (currentPreviewAudio === audio) currentPreviewAudio = null;
				onEnded?.();
			};
			await audio.play();
		} catch (err) {
			console.warn("[TTS preview failed, falling back to Web Speech]:", err);
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
			utterance.pitch = 1.0;
			utterance.rate = voice.rateMultiplier ?? 1.0;

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
	 * Uses pure natural engine audio with zero DSP effect or artificial distortion
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

		// 1. Fetch synthesized base audio from /api/tts with user speed and pitch
		const response = await fetch("/api/tts", {
			method: "POST",
			headers: { "Content-Type": "application/json" },
			body: JSON.stringify({
				text: cleanText,
				voiceId: voice.id,
				edgeVoiceName: voice.edgeVoiceName,
				engine: voice.engine,
				langCode: voice.langCode,
				pitchHz: voice.pitchHz ?? 0,
				rateMultiplier: voice.rateMultiplier ?? 1.0,
				userPitch: pitch,
				userSpeed: speed,
			}),
		});

		if (!response.ok) {
			let msg = `Không thể kết nối máy chủ tạo giọng nói (${response.status})`;
			try {
				const data = await response.json();
				if (data?.error) msg = data.error;
			} catch {}
			throw new Error(msg);
		}

		const blob = await response.blob();
		const arrayBuffer = await blob.arrayBuffer();
		const audioContext = this.getAudioContext();
		const decodedBuffer = await audioContext.decodeAudioData(arrayBuffer);

		// 2. Apply volume adjustment if needed
		const finalBuffer = (volume !== 1.0)
			? this.applyVolumeToBuffer(audioContext, decodedBuffer, volume)
			: decodedBuffer;

		// 3. Create standard WAV blob so timeline audio element can seek and play reliably
		const finalBlob = audioBufferToWavBlob(finalBuffer);
		const blobUrl = URL.createObjectURL(finalBlob);

		return {
			audioBuffer: finalBuffer,
			blobUrl,
			durationSeconds: finalBuffer.duration,
		};
	}

	private static applyVolumeToBuffer(
		audioContext: AudioContext,
		buffer: AudioBuffer,
		volume: number,
	): AudioBuffer {
		const newBuffer = audioContext.createBuffer(
			buffer.numberOfChannels,
			buffer.length,
			buffer.sampleRate,
		);
		for (let channel = 0; channel < buffer.numberOfChannels; channel++) {
			const sourceData = buffer.getChannelData(channel);
			const targetData = newBuffer.getChannelData(channel);
			for (let i = 0; i < buffer.length; i++) {
				targetData[i] = sourceData[i] * volume;
			}
		}
		return newBuffer;
	}
}
