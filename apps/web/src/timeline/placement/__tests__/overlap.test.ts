import { describe, expect, test } from "bun:test";
import type { VideoElement } from "@/timeline";
import { canPlaceTimeSpansOnTrack } from "@/timeline/placement/overlap";
import { mediaTime, ZERO_MEDIA_TIME } from "@/wasm";

function buildElement({
	id,
	startTime,
	duration,
}: {
	id: string;
	startTime: number;
	duration: number;
}): VideoElement {
	return {
		id,
		type: "video",
		name: id,
		startTime: mediaTime({ ticks: startTime }),
		duration: mediaTime({ ticks: duration }),
		trimStart: ZERO_MEDIA_TIME,
		trimEnd: ZERO_MEDIA_TIME,
		mediaId: `media-${id}`,
		params: {
			"transform.positionX": 0,
			"transform.positionY": 0,
			"transform.scaleX": 1,
			"transform.scaleY": 1,
			"transform.rotate": 0,
			opacity: 1,
		},
	};
}

describe("canPlaceTimeSpansOnTrack", () => {
	test("allows spans that fit between sorted elements", () => {
		expect(
			canPlaceTimeSpansOnTrack({
				track: {
					elements: [
						buildElement({ id: "a", startTime: 0, duration: 10 }),
						buildElement({ id: "b", startTime: 20, duration: 10 }),
					],
				},
				timeSpans: [
					{
						startTime: mediaTime({ ticks: 10 }),
						duration: mediaTime({ ticks: 10 }),
					},
				],
			}),
		).toBe(true);
	});

	test("rejects spans that overlap a later sorted element", () => {
		expect(
			canPlaceTimeSpansOnTrack({
				track: {
					elements: [
						buildElement({ id: "a", startTime: 0, duration: 10 }),
						buildElement({ id: "b", startTime: 20, duration: 10 }),
					],
				},
				timeSpans: [
					{
						startTime: mediaTime({ ticks: 15 }),
						duration: mediaTime({ ticks: 10 }),
					},
				],
			}),
		).toBe(false);
	});

	test("keeps long earlier elements in the overlap search", () => {
		expect(
			canPlaceTimeSpansOnTrack({
				track: {
					elements: [
						buildElement({ id: "long", startTime: 0, duration: 100 }),
						buildElement({ id: "short", startTime: 90, duration: 1 }),
					],
				},
				timeSpans: [
					{
						startTime: mediaTime({ ticks: 91 }),
						duration: mediaTime({ ticks: 1 }),
					},
				],
			}),
		).toBe(false);
	});

	test("ignores the excluded element", () => {
		expect(
			canPlaceTimeSpansOnTrack({
				track: {
					elements: [
						buildElement({ id: "dragged", startTime: 0, duration: 10 }),
					],
				},
				timeSpans: [
					{
						startTime: mediaTime({ ticks: 0 }),
						duration: mediaTime({ ticks: 10 }),
						excludeElementId: "dragged",
					},
				],
			}),
		).toBe(true);
	});
});
