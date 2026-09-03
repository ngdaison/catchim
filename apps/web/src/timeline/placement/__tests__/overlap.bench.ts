import { bench, describe } from "bun:test";
import type { VideoElement } from "@/timeline";
import { canPlaceTimeSpansOnTrack } from "@/timeline/placement/overlap";
import { mediaTime, ZERO_MEDIA_TIME } from "@/wasm";

function buildElement(index: number): VideoElement {
	return {
		id: `element-${index}`,
		type: "video",
		name: `Element ${index}`,
		startTime: mediaTime({ ticks: index * 100 }),
		duration: mediaTime({ ticks: 50 }),
		trimStart: ZERO_MEDIA_TIME,
		trimEnd: ZERO_MEDIA_TIME,
		mediaId: `media-${index}`,
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

const track = {
	elements: Array.from({ length: 10_000 }, (_, index) => buildElement(index)),
};

describe("timeline placement overlap performance", () => {
	bench("place 1000 spans on a sorted 10000 element track", () => {
		for (let index = 0; index < 1_000; index += 1) {
			canPlaceTimeSpansOnTrack({
				track,
				timeSpans: [
					{
						startTime: mediaTime({ ticks: index * 1_000 + 75 }),
						duration: mediaTime({ ticks: 10 }),
					},
				],
			});
		}
	});
});
