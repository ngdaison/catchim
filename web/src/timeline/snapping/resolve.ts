import type { SnapPoint, SnapResult } from "./types";
import type { MediaTime } from "@/wasm";

function isSortedByTime(snapPoints: SnapPoint[]): boolean {
	for (let index = 1; index < snapPoints.length; index += 1) {
		if (snapPoints[index - 1].time > snapPoints[index].time) {
			return false;
		}
	}

	return true;
}

function findLowerBound({
	snapPoints,
	targetTime,
}: {
	snapPoints: SnapPoint[];
	targetTime: number;
}): number {
	let low = 0;
	let high = snapPoints.length;

	while (low < high) {
		const mid = low + Math.floor((high - low) / 2);
		if (snapPoints[mid].time < targetTime) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}

	return low;
}

function resolveTimelineSnapLinear({
	targetTime,
	snapPoints,
	maxSnapDistance,
}: {
	targetTime: MediaTime;
	snapPoints: SnapPoint[];
	maxSnapDistance: number;
}): SnapResult {
	let closestSnapPoint: SnapPoint | null = null;
	let closestDistance = Infinity;

	for (const snapPoint of snapPoints) {
		const distance = Math.abs(targetTime - snapPoint.time);
		if (distance <= maxSnapDistance && distance < closestDistance) {
			closestDistance = distance;
			closestSnapPoint = snapPoint;
		}
	}

	return {
		snappedTime: closestSnapPoint ? closestSnapPoint.time : targetTime,
		snapPoint: closestSnapPoint,
		snapDistance: closestDistance,
	};
}

export function resolveSortedTimelineSnap({
	targetTime,
	snapPoints,
	maxSnapDistance,
}: {
	targetTime: MediaTime;
	snapPoints: SnapPoint[];
	maxSnapDistance: number;
}): SnapResult {
	let closestSnapPoint: SnapPoint | null = null;
	let closestDistance = Infinity;

	const firstCandidateIndex = findLowerBound({
		snapPoints,
		targetTime: targetTime - maxSnapDistance,
	});
	for (
		let index = firstCandidateIndex;
		index < snapPoints.length &&
		snapPoints[index].time <= targetTime + maxSnapDistance;
		index += 1
	) {
		const snapPoint = snapPoints[index];
		const distance = Math.abs(targetTime - snapPoint.time);
		if (distance <= maxSnapDistance && distance < closestDistance) {
			closestDistance = distance;
			closestSnapPoint = snapPoint;
		}
	}

	return {
		snappedTime: closestSnapPoint ? closestSnapPoint.time : targetTime,
		snapPoint: closestSnapPoint,
		snapDistance: closestDistance,
	};
}

export function resolveTimelineSnap({
	targetTime,
	snapPoints,
	maxSnapDistance,
}: {
	targetTime: MediaTime;
	snapPoints: SnapPoint[];
	maxSnapDistance: number;
}): SnapResult {
	if (!isSortedByTime(snapPoints)) {
		return resolveTimelineSnapLinear({
			targetTime,
			snapPoints,
			maxSnapDistance,
		});
	}

	return resolveSortedTimelineSnap({
		targetTime,
		snapPoints,
		maxSnapDistance,
	});
}
