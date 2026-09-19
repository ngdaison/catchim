import type { SnapPoint, TimelineSnapPointSource } from "./types";

export function buildTimelineSnapPoints({
	sources,
}: {
	sources: TimelineSnapPointSource[];
}): SnapPoint[] {
	const snapPoints: SnapPoint[] = [];

	for (const source of sources) {
		for (const snapPoint of source()) {
			snapPoints.push(snapPoint);
		}
	}

	return snapPoints;
}

export function buildSortedTimelineSnapPoints({
	sources,
}: {
	sources: TimelineSnapPointSource[];
}): SnapPoint[] {
	return buildTimelineSnapPoints({ sources }).sort(
		(leftPoint, rightPoint) => leftPoint.time - rightPoint.time,
	);
}
