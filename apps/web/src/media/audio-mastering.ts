import { getNativeTimelineBindings } from "../native/opencut-core";

const MASTER_LIMITER_THRESHOLD_DB = -1;
const MASTER_LIMITER_KNEE_DB = 0;
const MASTER_LIMITER_RATIO = 20;
const MASTER_LIMITER_ATTACK_SECONDS = 0.001;
const MASTER_LIMITER_RELEASE_SECONDS = 0.12;
const MASTER_OUTPUT_HEADROOM = 0.98;

export function getAudioBufferPeak({
	audioBuffer,
}: {
	audioBuffer: AudioBuffer;
}): number {
	const bindings = getNativeTimelineBindings();
	if (bindings) {
		let maxPeak = 0;
		for (let channel = 0; channel < audioBuffer.numberOfChannels; channel++) {
			const channelData = audioBuffer.getChannelData(channel);
			if (channelData.length === 0) continue;
			const byteSize = channelData.length * 4;
			const ptr = bindings.malloc(byteSize);
			if (ptr !== 0) {
				try {
					bindings.HEAPF32.set(channelData, ptr >> 2);
					const chPeak = bindings.computeBufferPeak(ptr, channelData.length);
					if (chPeak > maxPeak) {
						maxPeak = chPeak;
					}
				} finally {
					bindings.free(ptr);
				}
			}
		}
		return maxPeak;
	}

	let peak = 0;

	for (let channel = 0; channel < audioBuffer.numberOfChannels; channel++) {
		const channelData = audioBuffer.getChannelData(channel);
		for (let index = 0; index < channelData.length; index++) {
			const magnitude = Math.abs(channelData[index]);
			if (magnitude > peak) {
				peak = magnitude;
			}
		}
	}

	return peak;
}

export function createAudioMasteringChain({
	audioContext,
	destination,
}: {
	audioContext: AudioContext | OfflineAudioContext;
	destination: AudioNode;
}): {
	input: GainNode;
} {
	const input = audioContext.createGain();
	const limiter = audioContext.createDynamicsCompressor();
	const outputGain = audioContext.createGain();

	limiter.threshold.value = MASTER_LIMITER_THRESHOLD_DB;
	limiter.knee.value = MASTER_LIMITER_KNEE_DB;
	limiter.ratio.value = MASTER_LIMITER_RATIO;
	limiter.attack.value = MASTER_LIMITER_ATTACK_SECONDS;
	limiter.release.value = MASTER_LIMITER_RELEASE_SECONDS;
	outputGain.gain.value = MASTER_OUTPUT_HEADROOM;

	input.connect(limiter);
	limiter.connect(outputGain);
	outputGain.connect(destination);

	return { input };
}

export async function applyAudioMasteringToBuffer({
	audioBuffer,
}: {
	audioBuffer: AudioBuffer;
}): Promise<AudioBuffer> {
	if (getAudioBufferPeak({ audioBuffer }) <= MASTER_OUTPUT_HEADROOM) {
		return audioBuffer;
	}

	const offlineContext = new OfflineAudioContext(
		audioBuffer.numberOfChannels,
		Math.max(1, audioBuffer.length),
		audioBuffer.sampleRate,
	);
	const source = offlineContext.createBufferSource();
	source.buffer = audioBuffer;

	const { input } = createAudioMasteringChain({
		audioContext: offlineContext,
		destination: offlineContext.destination,
	});
	source.connect(input);
	source.start(0);

	const renderedBuffer = await offlineContext.startRendering();
	clampAudioBufferPeak({
		audioBuffer: renderedBuffer,
		maxPeak: MASTER_OUTPUT_HEADROOM,
	});
	return renderedBuffer;
}

function clampAudioBufferPeak({
	audioBuffer,
	maxPeak,
}: {
	audioBuffer: AudioBuffer;
	maxPeak: number;
}): void {
	const bindings = getNativeTimelineBindings();
	if (bindings) {
		for (let channel = 0; channel < audioBuffer.numberOfChannels; channel++) {
			const channelData = audioBuffer.getChannelData(channel);
			if (channelData.length === 0) continue;
			const byteSize = channelData.length * 4;
			const ptr = bindings.malloc(byteSize);
			if (ptr !== 0) {
				try {
					bindings.HEAPF32.set(channelData, ptr >> 2);
					bindings.clampBufferSamples(ptr, channelData.length, maxPeak);
					channelData.set(bindings.HEAPF32.subarray(ptr >> 2, (ptr >> 2) + channelData.length));
				} finally {
					bindings.free(ptr);
				}
			}
		}
		return;
	}

	for (let channel = 0; channel < audioBuffer.numberOfChannels; channel++) {
		const channelData = audioBuffer.getChannelData(channel);
		for (let index = 0; index < channelData.length; index++) {
			channelData[index] = Math.max(
				-maxPeak,
				Math.min(maxPeak, channelData[index]),
			);
		}
	}
}
