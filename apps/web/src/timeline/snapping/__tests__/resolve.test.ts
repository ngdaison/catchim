import { describe, expect, test } from "bun:test";
import { resolveTimelineSnap, type SnapPoint } from "@/timeline/snapping";
import { mediaTime } from "@/wasm";

function snapPoint(time: number): SnapPoint {
	return {
		time: mediaTime({ ticks: time }),
		type: "element-start",
		elementId: `element-${time}`,
		trackId: "track-1",
	};
}

describe("resolveTimelineSnap", () => {
	test("resolves the closest snap point from sorted input", () => {
		const result = resolveTimelineSnap({
			targetTime: mediaTime({ ticks: 103 }),
			snapPoints: [snapPoint(0), snapPoint(100), snapPoint(200)],
			maxSnapDistance: 5,
		});

		expect(result.snappedTime).toBe(100);
		expect(result.snapPoint?.elementId).toBe("element-100");
		expect(result.snapDistance).toBe(3);
	});

	test("falls back correctly for unsorted input", () => {
		const result = resolveTimelineSnap({
			targetTime: mediaTime({ ticks: 198 }),
			snapPoints: [snapPoint(200), snapPoint(0), snapPoint(100)],
			maxSnapDistance: 5,
		});

		expect(result.snappedTime).toBe(200);
		expect(result.snapPoint?.elementId).toBe("element-200");
		expect(result.snapDistance).toBe(2);
	});

	test("does not snap outside the threshold", () => {
		const result = resolveTimelineSnap({
			targetTime: mediaTime({ ticks: 90 }),
			snapPoints: [snapPoint(0), snapPoint(100), snapPoint(200)],
			maxSnapDistance: 5,
		});

		expect(result.snappedTime).toBe(90);
		expect(result.snapPoint).toBeNull();
	});
});
