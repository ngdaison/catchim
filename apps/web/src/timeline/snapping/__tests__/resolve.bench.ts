import { bench, describe } from "bun:test";
import {
	buildSortedTimelineSnapPoints,
	resolveTimelineSnap,
} from "@/timeline/snapping";
import { mediaTime } from "@/wasm";

const SNAP_POINT_COUNT = 10_000;
const QUERY_COUNT = 1_000;
const snapPoints = buildSortedTimelineSnapPoints({
	sources: [
		() =>
			Array.from({ length: SNAP_POINT_COUNT }, (_, index) => ({
				time: mediaTime({ ticks: index * 100 }),
				type: "element-start" as const,
				elementId: `element-${index}`,
				trackId: "track-1",
			})),
	],
});

describe("timeline snapping performance", () => {
	bench("resolve 1000 queries against 10000 sorted snap points", () => {
		for (let index = 0; index < QUERY_COUNT; index += 1) {
			resolveTimelineSnap({
				targetTime: mediaTime({ ticks: index * 997 }),
				snapPoints,
				maxSnapDistance: 24,
			});
		}
	});
});
