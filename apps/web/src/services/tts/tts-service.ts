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
			// Fetch synthesized audio from /api/tts with all voice parameters
			const response = await fetch("/api/tts", {
				method: "POST",
				headers: { "Content-Type": "application/json" },
				body: JSON.stringify({
					text: textToSpeak,
					voiceId: voice.id,
					edgeVoiceName: voice.edgeVoiceName,
					pitchHz: voice.pitchHz ?? 0,
					rateMultiplier: voice.rateMultiplier ?? 1.0,
					engine: voice.engine,
					langCode: voice.langCode,
					userPitch: 0,
					userSpeed: 1.0,
				}),
			});

			if (!response.ok) {
				throw new Error(`TTS server error: ${response.status}`);
			}

			const blob = await response.blob();

			// If voice has special DSP audio effects (robot, radio, deep bass), process via Web Audio
			if (voice.audioEffect && voice.audioEffect !== "none") {
				const audioContext = this.getAudioContext();
				const arrayBuf = await blob.arrayBuffer();
				const decoded = await audioContext.decodeAudioData(arrayBuf);
				const processed = await this.applyAudioEffects(decoded, voice.audioEffect);
				const wavBlob = audioBufferToWavBlob(processed);
				const audioUrl = URL.createObjectURL(wavBlob);
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
			} else {
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
					this.previewViaWebSpeech(voice, textToSpeak, onEnded, onError);
				};

				await audio.play();
			}
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
			utterance.pitch = (voice.pitchHz ? 1 + voice.pitchHz / 100 : 1.0);
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
	 * Apply Web Audio DSP effects like Robot AM ring modulation, Radio filter, and Deep bass
	 */
	static async applyAudioEffects(
		buffer: AudioBuffer,
		effect?: "none" | "robot" | "chipmunk" | "radio" | "deep-bass" | "echo",
	): Promise<AudioBuffer> {
		if (!effect || effect === "none") {
			return buffer;
		}

		const numChannels = buffer.numberOfChannels;
		const length = buffer.length;
		const sampleRate = buffer.sampleRate;

		if (effect === "robot") {
			// Real electronic Robot AM ring-modulation: multiplies audio by 55Hz carrier sine wave
			const audioCtx = this.getAudioContext();
			const outBuffer = audioCtx.createBuffer(numChannels, length, sampleRate);
			const carrierFreq = 55;
			for (let ch = 0; ch < numChannels; ch++) {
				const input = buffer.getChannelData(ch);
				const output = outBuffer.getChannelData(ch);
				for (let i = 0; i < length; i++) {
					const carrier = 0.5 + 0.5 * Math.sin((2 * Math.PI * carrierFreq * i) / sampleRate);
					output[i] = input[i] * carrier * 1.35;
				}
			}
			return outBuffer;
		}

		if (effect === "radio") {
			const offlineCtx = new OfflineAudioContext(numChannels, length, sampleRate);
			const source = offlineCtx.createBufferSource();
			source.buffer = buffer;

			// Vintage radio bandpass: highpass 450Hz + lowpass 3000Hz
			const hp = offlineCtx.createBiquadFilter();
			hp.type = "highpass";
			hp.frequency.value = 450;

			const lp = offlineCtx.createBiquadFilter();
			lp.type = "lowpass";
			lp.frequency.value = 3000;

			// Distortion waveshaper
			const shaper = offlineCtx.createWaveShaper();
			const n = 256;
			const curve = new Float32Array(n);
			for (let i = 0; i < n; i++) {
				const x = (i * 2) / n - 1;
				curve[i] = Math.tanh(x * 1.75);
			}
			shaper.curve = curve;

			source.connect(hp);
			hp.connect(lp);
			lp.connect(shaper);
			shaper.connect(offlineCtx.destination);

			source.start(0);
			return await offlineCtx.startRendering();
		}

		if (effect === "deep-bass") {
			const offlineCtx = new OfflineAudioContext(numChannels, length, sampleRate);
			const source = offlineCtx.createBufferSource();
			source.buffer = buffer;

			const bass = offlineCtx.createBiquadFilter();
			bass.type = "lowshelf";
			bass.frequency.value = 140;
			bass.gain.value = 9; // +9dB deep bass presence

			source.connect(bass);
			bass.connect(offlineCtx.destination);

			source.start(0);
			return await offlineCtx.startRendering();
		}

		return buffer;
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

		// 1. Fetch synthesized audio from /api/tts with full neural parameters
		const response = await fetch("/api/tts", {
			method: "POST",
			headers: { "Content-Type": "application/json" },
			body: JSON.stringify({
				text: cleanText,
				voiceId: voice.id,
				edgeVoiceName: voice.edgeVoiceName,
				pitchHz: voice.pitchHz ?? 0,
				rateMultiplier: voice.rateMultiplier ?? 1.0,
				userPitch: pitch,
				userSpeed: speed,
				engine: voice.engine,
				langCode: voice.langCode,
			}),
		});

		if (!response.ok) {
			throw new Error(`Không thể kết nối máy chủ tạo giọng nói (${response.status})`);
		}

		const blob = await response.blob();
		const arrayBuffer = await blob.arrayBuffer();
		const audioContext = this.getAudioContext();
		const decodedBuffer = await audioContext.decodeAudioData(arrayBuffer.slice(0));

		// 2. Apply DSP audio effects (robot, radio, deep bass)
		const processedBuffer = await this.applyAudioEffects(decodedBuffer, voice.audioEffect);

		// 3. Apply volume adjustment if needed
		const finalBuffer = (volume !== 1.0)
			? this.applyVolumeToBuffer(audioContext, processedBuffer, volume)
			: processedBuffer;

		// 4. Create standard WAV blob so timeline audio element can seek and play reliably
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
