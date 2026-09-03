import type { PlacementTimeSpan } from "@/timeline/placement/types";
import type { TimelineElement } from "@/timeline";

const WASM_MODULE_URL = "/wasm/opencut_core.js";
const NATIVE_TIMELINE_ENABLED =
	process.env.NEXT_PUBLIC_OPENCUT_NATIVE_TIMELINE !== "0";
const TRACK_ID = "track";
const OK_STATUS = 0;

export interface FrameRate {
	numerator: number;
	denominator: number;
}

export type TimeCodeFormat = "MM:SS" | "HH:MM:SS" | "HH:MM:SS:CS" | "HH:MM:SS:FF";

interface TrackWithElements {
	elements: TimelineElement[];
}

type CArg = string | number;
type CReturnType = "number" | "string" | null;
type CArgType = "number" | "string";
type CFunction = (...args: CArg[]) => number;

interface EmscriptenModule {
	cwrap: (
		ident: string,
		returnType: CReturnType,
		argTypes: CArgType[],
	) => CFunction;
	_malloc: (size: number) => number;
	_free: (ptr: number) => void;
	HEAPF32: Float32Array;
	HEAPF64: Float64Array;
	UTF8ToString: (ptr: number, maxBytes?: number) => string;
	stringToUTF8: (str: string, outPtr: number, maxBytes: number) => void;
}

type EmscriptenFactory = (
	options?: Record<string, unknown>,
) => EmscriptenModule | Promise<EmscriptenModule>;

export interface NativeTimelineBindings {
	create: () => number;
	destroy: (timeline: number) => void;
	addTrack: (timeline: number, trackId: string) => number;
	insertClip: (
		timeline: number,
		trackId: string,
		clipId: string,
		start: number,
		duration: number,
	) => number;
	canPlace: (
		timeline: number,
		trackId: string,
		start: number,
		duration: number,
		excludeClipId: string,
	) => number;
	snap: (
		timeline: number,
		targetTime: number,
		playhead: number,
		threshold: number,
		outSnappedPtr: number,
		outDeltaPtr: number,
	) => number;
	ticksPerSecond: () => number;
	fromSeconds: (seconds: number) => number;
	toSeconds: (ticks: number) => number;
	roundToFrame: (ticks: number, fpsNum: number, fpsDen: number) => number;
	floorToFrame: (ticks: number, fpsNum: number, fpsDen: number) => number;
	lastFrame: (duration: number, fpsNum: number, fpsDen: number) => number;
	snappedSeek: (time: number, duration: number, fpsNum: number, fpsDen: number) => number;
	formatTimecode: (ticks: number, format: number, fpsNum: number, fpsDen: number, outBuf: number, outLen: number) => number;
	parseTimecode: (str: string, format: number, fpsNum: number, fpsDen: number) => number;
	evaluateFade: (offset: number, duration: number, fadeIn: number, fadeOut: number) => number;
	evaluateMaskAlpha: (px: number, py: number, maskType: number, cx: number, cy: number, sx: number, sy: number, rot: number, feather: number, inverted: number) => number;
	solveBezier: (time: number, t0: number, t1: number, t2: number, t3: number) => number;
	evaluateBezierPoint: (progress: number, p0: number, p1: number, p2: number, p3: number) => number;
	evaluateChannel: (keyTimesPtr: number, keyValuesPtr: number, keyInterpPtr: number, numKeys: number, evalTime: number) => number;
	mapTimelineToSourceSpeed: (timelineOffset: number, timelineDuration: number, speedRatiosPtr: number, speedMultipliersPtr: number, numPoints: number, constantSpeed: number) => number;
	findAvailableGap: (timeline: number, trackId: string, duration: number, minStart: number) => number;
	applyRipple: (timeline: number, trackId: string, afterTime: number, deltaTicks: number) => number;
	sceneCreate: () => number;
	sceneDestroy: (scene: number) => void;
	sceneAddItem: (scene: number, id: string, type: number, zIndex: number, opacity: number, blendMode: number, cx: number, cy: number, w: number, h: number, rot: number, flipX: number, flipY: number, assetId: string) => void;
	sceneBuildDisplayList: (scene: number, viewportW: number, viewportH: number, enableCulling: number) => number;
	parseSubtitleTimestamp: (ts: string) => number;
	formatSubtitleTimestamp: (ticks: number, outBuf: number, outLen: number) => number;
	measureLineHeight: (fontSize: number, lineHeightRatio: number) => number;
	breakLinesCount: (text: string, maxWidth: number, avgCharWidth: number) => number;
	pointInRotatedRect: (px: number, py: number, cx: number, cy: number, w: number, h: number, rot: number) => number;
	testSnapAxis: (sourceVal: number, targetVal: number, threshold: number, outSnapped: number, outDelta: number) => number;
	malloc: (size: number) => number;
	free: (ptr: number) => void;
	UTF8ToString: (ptr: number, maxBytes?: number) => string;
	HEAPF64: Float64Array;
}

interface NativeTrackCache {
	bindings: NativeTimelineBindings;
	timeline: number;
	clipIds: string[];
	startTimes: number[];
	durations: number[];
}

let nativeBindings: NativeTimelineBindings | null = null;
let nativeLoadPromise: Promise<NativeTimelineBindings | null> | null = null;
const nativeTrackCacheByElements = new WeakMap<
	TimelineElement[],
	NativeTrackCache
>();
const nativeTrackCacheFinalizer =
	typeof FinalizationRegistry === "undefined"
		? null
		: new FinalizationRegistry<NativeTrackCache>((cache) => {
				cache.bindings.destroy(cache.timeline);
			});

function isBrowser() {
	return typeof window !== "undefined";
}

function isRecord(value: unknown): value is Record<string, unknown> {
	return typeof value === "object" && value !== null;
}

function isEmscriptenFactory(value: unknown): value is EmscriptenFactory {
	return typeof value === "function";
}

async function importEmscriptenFactory(): Promise<EmscriptenFactory> {
	const importedModule = (await import(
		/* webpackIgnore: true */ WASM_MODULE_URL
	)) as unknown;
	const candidate =
		isRecord(importedModule) && "default" in importedModule
			? importedModule.default
			: importedModule;

	if (!isEmscriptenFactory(candidate)) {
		throw new Error("OpenCut native timeline module did not export a factory");
	}

	return candidate;
}

function wrapNativeTimelineBindings(
	module: EmscriptenModule,
): NativeTimelineBindings {
	const create = module.cwrap("oc_timeline_create", "number", []);
	const destroyRaw = module.cwrap("oc_timeline_destroy", null, ["number"]);
	const addTrack = module.cwrap("oc_timeline_add_track", "number", [
		"number",
		"string",
	]);
	const insertClip = module.cwrap("ocw_timeline_insert_clip", "number", [
		"number",
		"string",
		"string",
		"number",
		"number",
	]);
	const canPlace = module.cwrap("ocw_timeline_can_place", "number", [
		"number",
		"string",
		"number",
		"number",
		"string",
	]);
	const snap = module.cwrap("ocw_timeline_snap", "number", [
		"number",
		"number",
		"number",
		"number",
		"number",
		"number",
	]);

	const ticksPerSecond = module.cwrap("oc_time_ticks_per_second", "number", []);
	const fromSeconds = module.cwrap("ocw_time_from_seconds", "number", ["number"]);
	const toSeconds = module.cwrap("ocw_time_to_seconds", "number", ["number"]);
	const roundToFrame = module.cwrap("ocw_time_round_to_frame", "number", ["number", "number", "number"]);
	const floorToFrame = module.cwrap("ocw_time_floor_to_frame", "number", ["number", "number", "number"]);
	const lastFrame = module.cwrap("ocw_time_last_frame", "number", ["number", "number", "number"]);
	const snappedSeek = module.cwrap("ocw_time_snapped_seek", "number", ["number", "number", "number", "number"]);
	const formatTimecode = module.cwrap("ocw_time_format_timecode", "number", ["number", "number", "number", "number", "number", "number"]);
	const parseTimecode = module.cwrap("ocw_time_parse_timecode", "number", ["string", "number", "number", "number"]);
	const evaluateFade = module.cwrap("ocw_audio_evaluate_fade", "number", ["number", "number", "number", "number"]);
	const evaluateMaskAlpha = module.cwrap("ocw_mask_evaluate_alpha", "number", [
		"number", "number", "number", "number", "number", "number", "number", "number", "number", "number"
	]);
	const solveBezier = module.cwrap("ocw_animation_solve_bezier", "number", ["number", "number", "number", "number", "number"]);
	const evaluateBezierPoint = module.cwrap("ocw_animation_evaluate_bezier_point", "number", ["number", "number", "number", "number", "number"]);
	const evaluateChannel = module.cwrap("ocw_animation_evaluate_channel", "number", ["number", "number", "number", "number", "number"]);
	const mapTimelineToSourceSpeed = module.cwrap("ocw_speed_map_timeline_to_source", "number", ["number", "number", "number", "number", "number", "number"]);
	const findAvailableGap = module.cwrap("ocw_timeline_find_available_gap", "number", ["number", "string", "number", "number"]);
	const applyRipple = module.cwrap("ocw_timeline_apply_ripple", "number", ["number", "string", "number", "number"]);
	const sceneCreate = module.cwrap("oc_scene_create", "number", []);
	const sceneDestroyRaw = module.cwrap("oc_scene_destroy", null, ["number"]);
	const sceneAddItem = module.cwrap("ocw_scene_add_item", null, [
		"number", "string", "number", "number", "number", "number",
		"number", "number", "number", "number", "number", "number", "number", "string"
	]);
	const sceneBuildDisplayList = module.cwrap("ocw_scene_build_display_list", "number", ["number", "number", "number", "number"]);
	const parseSubtitleTimestamp = module.cwrap("ocw_subtitles_parse_timestamp", "number", ["string"]);
	const formatSubtitleTimestamp = module.cwrap("ocw_subtitles_format_timestamp", "number", ["number", "number", "number"]);
	const measureLineHeight = module.cwrap("ocw_text_measure_line_height", "number", ["number", "number"]);
	const breakLinesCount = module.cwrap("ocw_text_break_lines_count", "number", ["string", "number", "number"]);
	const pointInRotatedRect = module.cwrap("ocw_geometry_point_in_rotated_rect", "number", ["number", "number", "number", "number", "number", "number", "number"]);
	const testSnapAxis = module.cwrap("ocw_geometry_test_snap", "number", ["number", "number", "number", "number", "number"]);

	return {
		create,
		destroy: (timeline: number) => {
			destroyRaw(timeline);
		},
		addTrack,
		insertClip,
		canPlace,
		snap,
		ticksPerSecond,
		fromSeconds,
		toSeconds,
		roundToFrame,
		floorToFrame,
		lastFrame,
		snappedSeek,
		formatTimecode,
		parseTimecode,
		evaluateFade,
		evaluateMaskAlpha,
		solveBezier,
		evaluateBezierPoint,
		evaluateChannel,
		mapTimelineToSourceSpeed,
		findAvailableGap,
		applyRipple,
		sceneCreate,
		sceneDestroy: (scene: number) => {
			sceneDestroyRaw(scene);
		},
		sceneAddItem: (scene: number, id: string, type: number, zIndex: number, opacity: number, blendMode: number, cx: number, cy: number, w: number, h: number, rot: number, flipX: number, flipY: number, assetId: string) => {
			sceneAddItem(scene, id, type, zIndex, opacity, blendMode, cx, cy, w, h, rot, flipX, flipY, assetId);
		},
		sceneBuildDisplayList,
		parseSubtitleTimestamp,
		formatSubtitleTimestamp,
		measureLineHeight,
		breakLinesCount,
		pointInRotatedRect,
		testSnapAxis,
		malloc: module._malloc,
		free: module._free,
		UTF8ToString: module.UTF8ToString,
		HEAPF64: module.HEAPF64,
	};
}

export function warmNativeTimelineCore(): void {
	if (!NATIVE_TIMELINE_ENABLED || !isBrowser() || nativeBindings) {
		return;
	}

	nativeLoadPromise ??= importEmscriptenFactory()
		.then((factory) =>
			factory({ locateFile: (path: string) => `/wasm/${path}` }),
		)
		.then((module) => {
			nativeBindings = wrapNativeTimelineBindings(module);
			return nativeBindings;
		})
		.catch(() => null);
}

export function getNativeTimelineBindings(): NativeTimelineBindings | null {
	return nativeBindings;
}

function cacheMatchesElements({
	cache,
	elements,
}: {
	cache: NativeTrackCache;
	elements: TimelineElement[];
}): boolean {
	if (cache.clipIds.length !== elements.length) {
		return false;
	}

	return elements.every(
		(element, index) =>
			cache.clipIds[index] === element.id &&
			cache.startTimes[index] === element.startTime &&
			cache.durations[index] === element.duration,
	);
}

function disposeNativeTrackCache(cache: NativeTrackCache): void {
	nativeTrackCacheFinalizer?.unregister(cache);
	cache.bindings.destroy(cache.timeline);
}

function createNativeTrackCache({
	bindings,
	elements,
}: {
	bindings: NativeTimelineBindings;
	elements: TimelineElement[];
}): NativeTrackCache | null {
	const timeline = bindings.create();
	if (timeline === 0) {
		return null;
	}

	let cache: NativeTrackCache | null = null;
	try {
		if (bindings.addTrack(timeline, TRACK_ID) !== OK_STATUS) {
			return null;
		}

		for (const element of elements) {
			const status = bindings.insertClip(
				timeline,
				TRACK_ID,
				element.id,
				element.startTime,
				element.duration,
			);
			if (status !== OK_STATUS) {
				return null;
			}
		}

		cache = {
			bindings,
			timeline,
			clipIds: elements.map((element) => element.id),
			startTimes: elements.map((element) => element.startTime),
			durations: elements.map((element) => element.duration),
		};
		nativeTrackCacheByElements.set(elements, cache);
		nativeTrackCacheFinalizer?.register(elements, cache, cache);
		return cache;
	} finally {
		if (!cache) {
			bindings.destroy(timeline);
		}
	}
}

function getNativeTrackCache({
	bindings,
	elements,
}: {
	bindings: NativeTimelineBindings;
	elements: TimelineElement[];
}): NativeTrackCache | null {
	const cached = nativeTrackCacheByElements.get(elements);
	if (
		cached &&
		cached.bindings === bindings &&
		cacheMatchesElements({ cache: cached, elements })
	) {
		return cached;
	}

	if (cached) {
		disposeNativeTrackCache(cached);
		nativeTrackCacheByElements.delete(elements);
	}

	return createNativeTrackCache({ bindings, elements });
}

export function canPlaceTimeSpansOnTrackNative({
	track,
	timeSpans,
}: {
	track: TrackWithElements;
	timeSpans: PlacementTimeSpan[];
}): boolean | null {
	if (!NATIVE_TIMELINE_ENABLED || timeSpans.length === 0) {
		return null;
	}

	const bindings = getNativeTimelineBindings();
	if (!bindings) {
		return null;
	}

	try {
		const cache = getNativeTrackCache({
			bindings,
			elements: track.elements,
		});
		if (!cache) {
			return null;
		}

		for (const { startTime, duration, excludeElementId } of timeSpans) {
			const canPlace = cache.bindings.canPlace(
				cache.timeline,
				TRACK_ID,
				startTime,
				duration,
				excludeElementId ?? "",
			);
			if (canPlace === 0) {
				return false;
			}
		}

		return true;
	} catch {
		return null;
	}
}
