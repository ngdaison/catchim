import type { TimelineElement } from "@/timeline";
import type { PlacementTimeSpan } from "./types";
import { canPlaceTimeSpansOnTrackNative } from "@/native/opencut-core";

interface TrackWithElements {
	elements: TimelineElement[];
}

interface ElementOverlapIndex {
	readonly isSortedByStartTime: boolean;
	readonly maxEndThroughIndex: number[];
}

const overlapIndexByElements = new WeakMap<
	TimelineElement[],
	ElementOverlapIndex
>();

function wouldElementOverlap({
	elements,
	startTime,
	endTime,
	excludeElementId,
}: {
	elements: TimelineElement[];
	startTime: number;
	endTime: number;
	excludeElementId?: string;
}): boolean {
	return elements.some((element) => {
		if (excludeElementId && element.id === excludeElementId) {
			return false;
		}

		const elementEnd = element.startTime + element.duration;
		return startTime < elementEnd && endTime > element.startTime;
	});
}

function areElementsSortedByStartTime({
	elements,
}: {
	elements: TimelineElement[];
}): boolean {
	for (let index = 1; index < elements.length; index += 1) {
		if (elements[index - 1].startTime > elements[index].startTime) {
			return false;
		}
	}

	return true;
}

function findFirstElementStartingAtOrAfter({
	elements,
	startTime,
}: {
	elements: TimelineElement[];
	startTime: number;
}): number {
	let low = 0;
	let high = elements.length;

	while (low < high) {
		const mid = low + Math.floor((high - low) / 2);
		if (elements[mid].startTime < startTime) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}

	return low;
}

function wouldSortedElementOverlap({
	elements,
	maxEndThroughIndex,
	startTime,
	endTime,
	excludeElementId,
}: {
	elements: TimelineElement[];
	maxEndThroughIndex: number[];
	startTime: number;
	endTime: number;
	excludeElementId?: string;
}): boolean {
	const candidateIndex = findFirstElementStartingAtOrAfter({
		elements,
		startTime,
	});

	for (let index = candidateIndex - 1; index >= 0; index -= 1) {
		if (maxEndThroughIndex[index] <= startTime) {
			break;
		}
		const element = elements[index];
		if (excludeElementId && element.id === excludeElementId) {
			continue;
		}
		if (element.startTime + element.duration <= startTime) {
			continue;
		}
		return true;
	}

	for (let index = candidateIndex; index < elements.length; index += 1) {
		const element = elements[index];
		if (element.startTime >= endTime) {
			break;
		}
		if (excludeElementId && element.id === excludeElementId) {
			continue;
		}
		return true;
	}

	return false;
}

function buildMaxEndThroughIndex({
	elements,
}: {
	elements: TimelineElement[];
}): number[] {
	const maxEndThroughIndex: number[] = [];
	let maxEnd = -Infinity;

	for (const element of elements) {
		maxEnd = Math.max(maxEnd, element.startTime + element.duration);
		maxEndThroughIndex.push(maxEnd);
	}

	return maxEndThroughIndex;
}

function getElementOverlapIndex({
	elements,
}: {
	elements: TimelineElement[];
}): ElementOverlapIndex {
	const cached = overlapIndexByElements.get(elements);
	if (cached) {
		return cached;
	}

	const isSortedByStartTime = areElementsSortedByStartTime({ elements });
	const index = {
		isSortedByStartTime,
		maxEndThroughIndex: isSortedByStartTime
			? buildMaxEndThroughIndex({ elements })
			: [],
	};
	overlapIndexByElements.set(elements, index);

	return index;
}

export function canPlaceTimeSpansOnTrack({
	track,
	timeSpans,
}: {
	track: TrackWithElements;
	timeSpans: PlacementTimeSpan[];
}): boolean {
	const nativeResult = canPlaceTimeSpansOnTrackNative({ track, timeSpans });
	if (nativeResult !== null) {
		return nativeResult;
	}

	const overlapIndex = getElementOverlapIndex({ elements: track.elements });

	return timeSpans.every(({ startTime, duration, excludeElementId }) => {
		const endTime = startTime + duration;
		return overlapIndex.isSortedByStartTime
			? !wouldSortedElementOverlap({
					elements: track.elements,
					maxEndThroughIndex: overlapIndex.maxEndThroughIndex,
					startTime,
					endTime,
					excludeElementId,
				})
			: !wouldElementOverlap({
					elements: track.elements,
					startTime,
					endTime,
					excludeElementId,
				});
	});
}
