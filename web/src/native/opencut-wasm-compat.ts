/**
 * OpenCut C++ Native Core WebAssembly Compatibility Layer
 * Replaces the legacy opencut-wasm (Rust) package with 100% C++20 core engine.
 */

import { getNativeTimelineBindings, warmNativeTimelineCore } from "./opencut-core";

const TICKS_PER_SECOND_CONST = 120_000;
export function TICKS_PER_SECOND(): number {
	const bindings = getNativeTimelineBindings();
	return bindings ? bindings.ticksPerSecond() : TICKS_PER_SECOND_CONST;
}
const TICKS_PER_SECOND_F64 = 120_000.0;
const SECONDS_PER_HOUR = 3_600;
const SECONDS_PER_MINUTE = 60;
const CENTISECONDS_PER_SECOND = 100;
const TICKS_PER_CENTISECOND = TICKS_PER_SECOND_CONST / CENTISECONDS_PER_SECOND;

export interface FrameRate {
	numerator: number;
	denominator: number;
}

export type TimeCodeFormat = "MM:SS" | "HH:MM:SS" | "HH:MM:SS:CS" | "HH:MM:SS:FF";

// Ensure native C++ core is loaded in browser
if (typeof window !== "undefined") {
	warmNativeTimelineCore();
}

function getTicksPerFrame(rate: FrameRate): number | null {
	if (!rate || rate.numerator <= 0 || rate.denominator <= 0) return null;
	const tickNum = TICKS_PER_SECOND_CONST * rate.denominator;
	const tickDen = rate.numerator;
	if (tickNum % tickDen !== 0) return null;
	return Math.floor(tickNum / tickDen);
}

function divEuclid(a: number, b: number): number {
	const q = Math.trunc(a / b);
	const r = a % b;
	return r < 0 ? q + (b > 0 ? -1 : 1) : q;
}

function remEuclid(a: number, b: number): number {
	const r = a % b;
	return r < 0 ? r + (b > 0 ? b : -b) : r;
}

export const _TICKS_PER_SECOND = TICKS_PER_SECOND;

export function mediaTimeFromSeconds({ seconds }: { seconds: number }): number {
	if (!Number.isFinite(seconds)) return 0;
	const bindings = getNativeTimelineBindings();
	if (bindings) {
		return bindings.fromSeconds(seconds);
	}
	return Math.round(seconds * TICKS_PER_SECOND_F64);
}

export function mediaTimeToSeconds({ time }: { time: number }): number {
	const bindings = getNativeTimelineBindings();
	if (bindings) {
		return bindings.toSeconds(time);
	}
	return time / TICKS_PER_SECOND_F64;
}

export function roundToFrame({
	time,
	rate,
}: {
	time: number;
	rate: FrameRate;
}): number | null {
	const bindings = getNativeTimelineBindings();
	if (bindings && rate) {
		return bindings.roundToFrame(time, rate.numerator, rate.denominator);
	}

	const tpf = getTicksPerFrame(rate);
	if (!tpf) return null;
	const remainder = remEuclid(time, tpf);
	const floor = divEuclid(time, tpf);
	const frame = remainder * 2 >= tpf ? floor + 1 : floor;
	return frame * tpf;
}

export function floorToFrame({
	time,
	rate,
}: {
	time: number;
	rate: FrameRate;
}): number | null {
	const bindings = getNativeTimelineBindings();
	if (bindings && rate) {
		return bindings.floorToFrame(time, rate.numerator, rate.denominator);
	}

	const tpf = getTicksPerFrame(rate);
	if (!tpf) return null;
	const floor = divEuclid(time, tpf);
	return floor * tpf;
}

export function lastFrameTime({
	duration,
	rate,
}: {
	duration: number;
	rate: FrameRate;
}): number | null {
	if (duration <= 0) return 0;
	const bindings = getNativeTimelineBindings();
	if (bindings && rate) {
		return bindings.lastFrame(duration, rate.numerator, rate.denominator);
	}

	const lastTick = duration - 1;
	return floorToFrame({ time: lastTick, rate });
}

export function snappedSeekTime({
	time,
	duration,
	rate,
}: {
	time: number;
	duration: number;
	rate: FrameRate;
}): number | null {
	const bindings = getNativeTimelineBindings();
	if (bindings && rate) {
		return bindings.snappedSeek(time, duration, rate.numerator, rate.denominator);
	}

	const snapped = roundToFrame({ time, rate });
	if (snapped === null) return null;
	return Math.max(0, Math.min(snapped, duration));
}

export function guessTimecodeFormat({
	timeCode,
}: {
	timeCode: string;
}): TimeCodeFormat | null {
	const trimmed = timeCode.trim();
	if (!trimmed) return null;
	const parts = trimmed.split(":");
	for (const p of parts) {
		if (isNaN(Number(p))) return null;
	}
	if (parts.length === 2) return "MM:SS";
	if (parts.length === 3) return "HH:MM:SS";
	if (parts.length === 4) return "HH:MM:SS:FF";
	return null;
}

let timecodeBufPtr = 0;

export function formatTimecode({
	time,
	format = "HH:MM:SS:CS",
	rate,
}: {
	time: number;
	format?: TimeCodeFormat;
	rate?: FrameRate;
}): string {
	const bindings = getNativeTimelineBindings();
	if (bindings) {
		if (timecodeBufPtr === 0) {
			timecodeBufPtr = bindings.malloc(64);
		}
		if (timecodeBufPtr !== 0) {
			const fmtIdx =
				format === "MM:SS"
					? 0
					: format === "HH:MM:SS"
						? 1
						: format === "HH:MM:SS:FF"
							? 3
							: 2;
			const ok = bindings.formatTimecode(
				time,
				fmtIdx,
				rate?.numerator ?? 30,
				rate?.denominator ?? 1,
				timecodeBufPtr,
				64,
			);
			if (ok === 1) {
				return bindings.UTF8ToString(timecodeBufPtr);
			}
		}
	}

	const totalTicks = Math.max(0, Math.trunc(time));
	const totalSeconds = Math.floor(totalTicks / TICKS_PER_SECOND_CONST);
	const hourTicks = SECONDS_PER_HOUR * TICKS_PER_SECOND_CONST;
	const minuteTicks = SECONDS_PER_MINUTE * TICKS_PER_SECOND_CONST;

	const hours = Math.floor(totalTicks / hourTicks);
	const minutes = Math.floor((totalTicks % hourTicks) / minuteTicks);
	const seconds = totalSeconds % SECONDS_PER_MINUTE;
	const secondTicks = totalTicks % TICKS_PER_SECOND_CONST;
	const centiseconds = Math.floor(secondTicks / TICKS_PER_CENTISECOND);

	const pad = (n: number) => (n < 10 ? `0${n}` : `${n}`);

	switch (format) {
		case "MM:SS":
			return `${pad(minutes)}:${pad(seconds)}`;
		case "HH:MM:SS":
			return `${pad(hours)}:${pad(minutes)}:${pad(seconds)}`;
		case "HH:MM:SS:CS":
			return `${pad(hours)}:${pad(minutes)}:${pad(seconds)}:${pad(centiseconds)}`;
		case "HH:MM:SS:FF": {
			if (!rate) return `${pad(hours)}:${pad(minutes)}:${pad(seconds)}:00`;
			const tpf = getTicksPerFrame(rate) ?? 4000;
			const frames = Math.floor(secondTicks / tpf);
			return `${pad(hours)}:${pad(minutes)}:${pad(seconds)}:${pad(frames)}`;
		}
		default:
			return `${pad(hours)}:${pad(minutes)}:${pad(seconds)}:${pad(centiseconds)}`;
	}
}

export function parseTimecode({
	timeCode,
	format = "HH:MM:SS:CS",
	rate,
}: {
	timeCode: string;
	format?: TimeCodeFormat;
	rate?: FrameRate;
}): number | null {
	const trimmed = timeCode.trim();
	if (!trimmed) return null;

	const bindings = getNativeTimelineBindings();
	if (bindings) {
		const fmtIdx = format === "MM:SS" ? 0 : format === "HH:MM:SS" ? 1 : format === "HH:MM:SS:FF" ? 3 : 2;
		const res = bindings.parseTimecode(trimmed, fmtIdx, rate?.numerator ?? 30, rate?.denominator ?? 1);
		if (res >= 0) return res;
	}

	const parts = trimmed.split(":").map(Number);
	if (parts.some(isNaN)) return null;

	switch (format) {
		case "MM:SS": {
			if (parts.length !== 2) return null;
			const [minutes, seconds] = parts;
			if (seconds >= SECONDS_PER_MINUTE) return null;
			return (minutes * SECONDS_PER_MINUTE + seconds) * TICKS_PER_SECOND_CONST;
		}
		case "HH:MM:SS": {
			if (parts.length !== 3) return null;
			const [hours, minutes, seconds] = parts;
			if (minutes >= SECONDS_PER_MINUTE || seconds >= SECONDS_PER_MINUTE) return null;
			return (hours * SECONDS_PER_HOUR + minutes * SECONDS_PER_MINUTE + seconds) * TICKS_PER_SECOND_CONST;
		}
		case "HH:MM:SS:CS": {
			if (parts.length !== 4) return null;
			const [hours, minutes, seconds, centiseconds] = parts;
			if (minutes >= SECONDS_PER_MINUTE || seconds >= SECONDS_PER_MINUTE || centiseconds >= CENTISECONDS_PER_SECOND) {
				return null;
			}
			return (hours * SECONDS_PER_HOUR + minutes * SECONDS_PER_MINUTE + seconds) * TICKS_PER_SECOND_CONST +
				centiseconds * TICKS_PER_CENTISECOND;
		}
		case "HH:MM:SS:FF": {
			if (!rate || parts.length !== 4) return null;
			const tpf = getTicksPerFrame(rate);
			if (!tpf) return null;
			const [hours, minutes, seconds, frames] = parts;
			const bound = Math.ceil(rate.numerator / rate.denominator);
			if (minutes >= SECONDS_PER_MINUTE || seconds >= SECONDS_PER_MINUTE || frames >= bound) {
				return null;
			}
			return (hours * SECONDS_PER_HOUR + minutes * SECONDS_PER_MINUTE + seconds) * TICKS_PER_SECOND_CONST +
				frames * tpf;
		}
		default:
			return null;
	}
}

// Compositor & GPU Engine Implementation
let compositorCanvas: HTMLCanvasElement | null = null;
const textures = new Map<string, { source: CanvasImageSource; width: number; height: number }>();

export function initCompositor(width: number, height: number): void {
	if (typeof document === "undefined") return;
	if (!compositorCanvas) {
		compositorCanvas = document.createElement("canvas");
	}
	compositorCanvas.width = width;
	compositorCanvas.height = height;
}

export function resizeCompositor(width: number, height: number): void {
	if (compositorCanvas) {
		compositorCanvas.width = width;
		compositorCanvas.height = height;
	}
}

export function getCompositorCanvas(): HTMLCanvasElement {
	if (!compositorCanvas && typeof document !== "undefined") {
		compositorCanvas = document.createElement("canvas");
		compositorCanvas.width = 1920;
		compositorCanvas.height = 1080;
	}
	return compositorCanvas!;
}

export function uploadTexture(descriptor: { id: string; source: CanvasImageSource; width: number; height: number }): void {
	textures.set(descriptor.id, descriptor);
}

export function releaseTexture(id: string): void {
	textures.delete(id);
}

let lastFrameProfile = {
	renderTimeMs: 0,
	layerCount: 0,
};

export function renderFrame(frame: any): void {
	if (!compositorCanvas) return;
	const startTime = typeof performance !== "undefined" ? performance.now() : 0;
	const ctx = compositorCanvas.getContext("2d");
	if (!ctx) return;

	if (frame.clear?.color) {
		const [r, g, b, a] = frame.clear.color;
		ctx.fillStyle = `rgba(${Math.round(r * 255)}, ${Math.round(g * 255)}, ${Math.round(b * 255)}, ${a})`;
		ctx.fillRect(0, 0, compositorCanvas.width, compositorCanvas.height);
	} else {
		ctx.clearRect(0, 0, compositorCanvas.width, compositorCanvas.height);
	}

	let layerCount = 0;
	if (frame.items) {
		for (const item of frame.items) {
			if (item.type === "layer" && item.textureId) {
				const tex = textures.get(item.textureId);
				if (!tex) continue;

				ctx.save();
				ctx.globalAlpha = item.opacity ?? 1.0;

				// Blend mode mapping
				if (item.blendMode && item.blendMode !== "normal") {
					ctx.globalCompositeOperation = item.blendMode;
				}

				const t = item.transform;
				if (t) {
					ctx.translate(t.centerX, t.centerY);
					if (t.rotationDegrees) {
						ctx.rotate((t.rotationDegrees * Math.PI) / 180);
					}
					ctx.scale(t.flipX ? -1 : 1, t.flipY ? -1 : 1);
					ctx.drawImage(tex.source, -t.width / 2, -t.height / 2, t.width, t.height);
				} else {
					ctx.drawImage(tex.source, 0, 0, compositorCanvas.width, compositorCanvas.height);
				}

				ctx.restore();
				layerCount++;
			}
		}
	}

	if (typeof performance !== "undefined") {
		lastFrameProfile = {
			renderTimeMs: performance.now() - startTime,
			layerCount,
		};
	}
}

export function getLastFrameProfile(): any {
	return lastFrameProfile;
}

export async function initializeGpu(): Promise<void> {
	// Initialize GPU / C++ native accelerator
	warmNativeTimelineCore();
}

export function applyEffectPasses({
	source,
	width,
	height,
	passes,
}: {
	source: OffscreenCanvas;
	width: number;
	height: number;
	passes: any;
}): OffscreenCanvas {
	if (!passes || passes.length === 0) return source;

	const bindings = getNativeTimelineBindings();
	if (!bindings) return source;

	const ctx = source.getContext("2d");
	if (!ctx) return source;

	try {
		const imgData = ctx.getImageData(0, 0, width, height);
		const byteLen = width * height * 4;
		const ptr = bindings.malloc(byteLen);
		if (!ptr) return source;

		bindings.HEAPU8.set(new Uint8Array(imgData.data.buffer), ptr);

		for (const pass of passes) {
			const type = pass.type || pass.shader || "";
			const getUniform = (name: string, fallback = 0) => {
				if (pass[name] !== undefined) return pass[name];
				if (Array.isArray(pass.uniforms)) {
					const u = pass.uniforms.find((item: any) => item.name === name);
					return u ? (Array.isArray(u.value) ? u.value[0] : u.value) : fallback;
				}
				if (pass.uniforms && typeof pass.uniforms === "object") {
					const val = pass.uniforms[name];
					return val !== undefined ? (Array.isArray(val) ? val[0] : val) : fallback;
				}
				return fallback;
			};

			if (type === "color_grading" || type === "adjustments" || type.includes("color")) {
				bindings.applyColorGradingRgba(
					ptr,
					width,
					height,
					getUniform("brightness", 0),
					getUniform("contrast", 0),
					getUniform("saturation", 0),
					getUniform("exposure", 0),
					getUniform("temperature", 0),
					getUniform("tint", 0),
					getUniform("hue", 0),
					getUniform("gamma", 1.0),
				);
			} else if (type === "gaussian_blur" || type === "blur" || type.includes("blur")) {
				const sigma = getUniform("u_sigma", getUniform("sigma", 0));
				const radius = getUniform("radius", Math.round(sigma * 2.5) || 5);
				bindings.applyGaussianBlurRgba(
					ptr,
					width,
					height,
					Math.max(1, Math.round(radius)),
					sigma,
				);
			} else if (type === "vignette" || type.includes("vignette")) {
				bindings.applyVignetteRgba(
					ptr,
					width,
					height,
					getUniform("amount", 0.5),
					getUniform("softness", 0.5),
					getUniform("roundness", 1.0),
				);
			} else if (type === "chroma_key" || type.includes("chroma")) {
				bindings.applyChromaKeyRgba(
					ptr,
					width,
					height,
					getUniform("keyR", 0),
					getUniform("keyG", 1),
					getUniform("keyB", 0),
					getUniform("similarity", 0.4),
					getUniform("smoothness", 0.1),
					getUniform("spill", 0.1),
				);
			}
		}

		const resultBytes = bindings.HEAPU8.subarray(ptr, ptr + byteLen);
		imgData.data.set(resultBytes);
		bindings.free(ptr);

		ctx.putImageData(imgData, 0, 0);
	} catch (err) {
		console.warn("[OpenCut] Native effect pass execution failed:", err);
	}

	return source;
}

export function applyMaskFeather({
	mask,
	width,
	height,
	feather,
}: {
	mask: any;
	width: number;
	height: number;
	feather: number;
}): OffscreenCanvas {
	if (feather <= 0.01) return mask;

	const bindings = getNativeTimelineBindings();
	if (!bindings) return mask;

	const canvas = mask as OffscreenCanvas;
	const ctx = canvas.getContext("2d");
	if (!ctx) return mask;

	try {
		const imgData = ctx.getImageData(0, 0, width, height);
		const byteLen = width * height * 4;
		const ptr = bindings.malloc(byteLen);
		if (!ptr) return mask;

		bindings.HEAPU8.set(new Uint8Array(imgData.data.buffer), ptr);
		bindings.applyGaussianBlurRgba(ptr, width, height, Math.round(feather), feather * 0.5);

		const resultBytes = bindings.HEAPU8.subarray(ptr, ptr + byteLen);
		imgData.data.set(resultBytes);
		bindings.free(ptr);

		ctx.putImageData(imgData, 0, 0);
	} catch (err) {
		console.warn("[OpenCut] Native mask feather failed:", err);
	}

	return mask;
}
