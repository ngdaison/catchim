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
				throw new Error(`TTS server error: ${response.status}`);
			}

			const blob = await response.blob();
			const audioContext = this.getAudioContext();
			const arrayBuf = await blob.arrayBuffer();
			const decoded = await audioContext.decodeAudioData(arrayBuf);

			// Apply genuine voice character transformation (pitch factor + DSP effects)
			const transformed = await this.applyVoiceTransform(decoded, voice, 1.0, 0);

			const wavBlob = audioBufferToWavBlob(transformed);
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
			utterance.pitch = voice.pitchFactor ?? 1.0;
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
	 * Apply full character voice transformation:
	 * 1. Resampling pitch shift (Chipmunk +68%, Child girl +38%, Grandpa -18%, Titan -38%)
	 * 2. Character-specific DSP effects (Robot ring-mod, Radio bandpass, Grandpa tremor, Titan rumble, Echo)
	 */
	static async applyVoiceTransform(
		buffer: AudioBuffer,
		voice: TtsVoice,
		speed = 1.0,
		userPitch = 0,
	): Promise<AudioBuffer> {
		const basePitchFactor = voice.pitchFactor ?? 1.0;
		// User pitch slider ranges from -40 to +40, mapped to logarithmic multiplier
		const userPitchFactor = Math.pow(2, (userPitch / 100) * 0.75);
		const totalRate = Math.max(0.4, Math.min(3.0, basePitchFactor * userPitchFactor * speed));

		let currentBuffer = buffer;

		// Step 1: High-fidelity pitch/speed resampling via native browser OfflineAudioContext
		if (Math.abs(totalRate - 1.0) > 0.02) {
			const targetLength = Math.max(1, Math.round(buffer.length / totalRate));
			const offlineCtx = new OfflineAudioContext(
				buffer.numberOfChannels,
				targetLength,
				buffer.sampleRate,
			);
			const source = offlineCtx.createBufferSource();
			source.buffer = buffer;
			source.playbackRate.value = totalRate;
			source.connect(offlineCtx.destination);
			source.start(0);
			currentBuffer = await offlineCtx.startRendering();
		}

		const effect = voice.audioEffect ?? "none";
		if (effect === "none") {
			return currentBuffer;
		}

		// Step 2: Apply DSP Character Effects
		const numChannels = currentBuffer.numberOfChannels;
		const length = currentBuffer.length;
		const sampleRate = currentBuffer.sampleRate;

		// Effect: Robot AI (Ring modulation + metallic formant)
		if (effect === "robot") {
			const audioCtx = this.getAudioContext();
			const outBuffer = audioCtx.createBuffer(numChannels, length, sampleRate);
			const carrierFreq = 56;
			for (let ch = 0; ch < numChannels; ch++) {
				const input = currentBuffer.getChannelData(ch);
				const output = outBuffer.getChannelData(ch);
				for (let i = 0; i < length; i++) {
					const carrier = 0.35 + 0.65 * Math.sin((2 * Math.PI * carrierFreq * i) / sampleRate);
					output[i] = input[i] * carrier * 1.45;
				}
			}
			return outBuffer;
		}

		// Effect: Old Grandfather (tremor vibrato + warm lowpass)
		if (effect === "grandpa") {
			const audioCtx = this.getAudioContext();
			const outBuffer = audioCtx.createBuffer(numChannels, length, sampleRate);
			for (let ch = 0; ch < numChannels; ch++) {
				const input = currentBuffer.getChannelData(ch);
				const output = outBuffer.getChannelData(ch);
				for (let i = 0; i < length; i++) {
					const t = i / sampleRate;
					const tremor = 1.0 + 0.16 * Math.sin(2 * Math.PI * 5.2 * t);
					output[i] = input[i] * tremor;
				}
			}
			const offlineCtx = new OfflineAudioContext(numChannels, length, sampleRate);
			const source = offlineCtx.createBufferSource();
			source.buffer = outBuffer;
			const lp = offlineCtx.createBiquadFilter();
			lp.type = "lowpass";
			lp.frequency.value = 4200;
			source.connect(lp);
			lp.connect(offlineCtx.destination);
			source.start(0);
			return await offlineCtx.startRendering();
		}

		// Effect: Chipmunk (Highpass 380Hz + high shelf boost)
		if (effect === "chipmunk") {
			const offlineCtx = new OfflineAudioContext(numChannels, length, sampleRate);
			const source = offlineCtx.createBufferSource();
			source.buffer = currentBuffer;
			const hp = offlineCtx.createBiquadFilter();
			hp.type = "highpass";
			hp.frequency.value = 380;
			const hs = offlineCtx.createBiquadFilter();
			hs.type = "highshelf";
			hs.frequency.value = 3500;
			hs.gain.value = 4;
			source.connect(hp);
			hp.connect(hs);
			hs.connect(offlineCtx.destination);
			source.start(0);
			return await offlineCtx.startRendering();
		}

		// Effect: Child Girl (Anime cute girl presence filter)
		if (effect === "child-girl") {
			const offlineCtx = new OfflineAudioContext(numChannels, length, sampleRate);
			const source = offlineCtx.createBufferSource();
			source.buffer = currentBuffer;
			const peak = offlineCtx.createBiquadFilter();
			peak.type = "peaking";
			peak.frequency.value = 2800;
			peak.gain.value = 4.5;
			source.connect(peak);
			peak.connect(offlineCtx.destination);
			source.start(0);
			return await offlineCtx.startRendering();
		}

		// Effect: Child Boy (Playful kid presence filter)
		if (effect === "child-boy") {
			const offlineCtx = new OfflineAudioContext(numChannels, length, sampleRate);
			const source = offlineCtx.createBufferSource();
			source.buffer = currentBuffer;
			const peak = offlineCtx.createBiquadFilter();
			peak.type = "peaking";
			peak.frequency.value = 2200;
			peak.gain.value = 3.5;
			source.connect(peak);
			peak.connect(offlineCtx.destination);
			source.start(0);
			return await offlineCtx.startRendering();
		}

		// Effect: Deep Titan / Monster (Heavy bass boost + saturation)
		if (effect === "deep-bass") {
			const offlineCtx = new OfflineAudioContext(numChannels, length, sampleRate);
			const source = offlineCtx.createBufferSource();
			source.buffer = currentBuffer;
			const bass = offlineCtx.createBiquadFilter();
			bass.type = "lowshelf";
			bass.frequency.value = 130;
			bass.gain.value = 12; // +12dB deep rumble

			const shaper = offlineCtx.createWaveShaper();
			const n = 256;
			const curve = new Float32Array(n);
			for (let i = 0; i < n; i++) {
				const x = (i * 2) / n - 1;
				curve[i] = Math.tanh(x * 1.35);
			}
			shaper.curve = curve;

			source.connect(bass);
			bass.connect(shaper);
			shaper.connect(offlineCtx.destination);
			source.start(0);
			return await offlineCtx.startRendering();
		}

		// Effect: Radio / Police dispatch (vintage bandpass + distortion)
		if (effect === "radio") {
			const offlineCtx = new OfflineAudioContext(numChannels, length, sampleRate);
			const source = offlineCtx.createBufferSource();
			source.buffer = currentBuffer;
			const hp = offlineCtx.createBiquadFilter();
			hp.type = "highpass";
			hp.frequency.value = 480;
			const lp = offlineCtx.createBiquadFilter();
			lp.type = "lowpass";
			lp.frequency.value = 2800;
			const shaper = offlineCtx.createWaveShaper();
			const n = 256;
			const curve = new Float32Array(n);
			for (let i = 0; i < n; i++) {
				const x = (i * 2) / n - 1;
				curve[i] = Math.tanh(x * 2.2);
			}
			shaper.curve = curve;
			source.connect(hp);
			hp.connect(lp);
			lp.connect(shaper);
			shaper.connect(offlineCtx.destination);
			source.start(0);
			return await offlineCtx.startRendering();
		}

		// Effect: Demon (Low-shelf boost + 28Hz ring mod)
		if (effect === "demon") {
			const audioCtx = this.getAudioContext();
			const outBuffer = audioCtx.createBuffer(numChannels, length, sampleRate);
			const carrierFreq = 28;
			for (let ch = 0; ch < numChannels; ch++) {
				const input = currentBuffer.getChannelData(ch);
				const output = outBuffer.getChannelData(ch);
				for (let i = 0; i < length; i++) {
					const carrier = 0.5 + 0.5 * Math.sin((2 * Math.PI * carrierFreq * i) / sampleRate);
					const sample = input[i] * carrier * 1.5;
					output[i] = Math.tanh(sample * 1.3);
				}
			}
			return outBuffer;
		}

		// Effect: Stadium Arena Echo
		if (effect === "echo") {
			const extraLength = Math.round(sampleRate * 0.8);
			const offlineCtx = new OfflineAudioContext(numChannels, length + extraLength, sampleRate);
			const source = offlineCtx.createBufferSource();
			source.buffer = currentBuffer;

			const delay = offlineCtx.createDelay(1.0);
			delay.delayTime.value = 0.22;
			const feedback = offlineCtx.createGain();
			feedback.gain.value = 0.42;

			source.connect(offlineCtx.destination);
			source.connect(delay);
			delay.connect(feedback);
			feedback.connect(delay);
			delay.connect(offlineCtx.destination);

			source.start(0);
			return await offlineCtx.startRendering();
		}

		// Effect: ASMR (Gentle lowpass rolloff)
		if (effect === "asmr") {
			const offlineCtx = new OfflineAudioContext(numChannels, length, sampleRate);
			const source = offlineCtx.createBufferSource();
			source.buffer = currentBuffer;
			const lp = offlineCtx.createBiquadFilter();
			lp.type = "lowpass";
			lp.frequency.value = 3600;
			source.connect(lp);
			lp.connect(offlineCtx.destination);
			source.start(0);
			return await offlineCtx.startRendering();
		}

		// Effect: Vlogger (Punchy presence boost)
		if (effect === "vlogger") {
			const offlineCtx = new OfflineAudioContext(numChannels, length, sampleRate);
			const source = offlineCtx.createBufferSource();
			source.buffer = currentBuffer;
			const peak = offlineCtx.createBiquadFilter();
			peak.type = "peaking";
			peak.frequency.value = 3200;
			peak.gain.value = 4.0;
			source.connect(peak);
			peak.connect(offlineCtx.destination);
			source.start(0);
			return await offlineCtx.startRendering();
		}

		return currentBuffer;
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

		// 1. Fetch synthesized base audio from /api/tts
		const response = await fetch("/api/tts", {
			method: "POST",
			headers: { "Content-Type": "application/json" },
			body: JSON.stringify({
				text: cleanText,
				voiceId: voice.id,
				edgeVoiceName: voice.edgeVoiceName,
				engine: voice.engine,
				langCode: voice.langCode,
				userPitch: 0,
				userSpeed: 1.0,
			}),
		});

		if (!response.ok) {
			throw new Error(`Không thể kết nối máy chủ tạo giọng nói (${response.status})`);
		}

		const blob = await response.blob();
		const arrayBuffer = await blob.arrayBuffer();
		const audioContext = this.getAudioContext();
		const decodedBuffer = await audioContext.decodeAudioData(arrayBuffer);

		// 2. Apply full character voice transformation (pitch, speed, DSP effect, user controls)
		const processedBuffer = await this.applyVoiceTransform(decodedBuffer, voice, speed, pitch);

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
