import type { SceneTracks } from "@/timeline";
import {
	buildSortedTimelineSnapPoints,
	getTimelineSnapThresholdInTicks,
	resolveSortedTimelineSnap,
	type SnapPoint,
} from "@/timeline/snapping";
import { getElementEdgeSnapPoints } from "@/timeline/element-snap-source";
import { getPlayheadSnapPoints } from "@/timeline/playhead-snap-source";
import { getAnimationKeyframeSnapPointsForTimeline } from "@/timeline/animation-snap-points";
import type { MoveGroup } from "./types";
import { addMediaTime, type MediaTime, subMediaTime } from "@/wasm";

export function snapGroupEdges({
	group,
	anchorStartTime,
	tracks,
	playheadTime,
	zoomLevel,
	snapPoints: cachedSnapPoints,
}: {
	group: MoveGroup;
	anchorStartTime: MediaTime;
	tracks: SceneTracks;
	playheadTime: MediaTime;
	zoomLevel: number;
	snapPoints?: SnapPoint[];
}): {
	snappedAnchorStartTime: MediaTime;
	snapPoint: SnapPoint | null;
} {
	const snapPoints =
		cachedSnapPoints ??
		buildMoveGroupSnapPoints({ group, tracks, playheadTime });
	const maxSnapDistance = getTimelineSnapThresholdInTicks({ zoomLevel });

	let closestSnapDistance = Infinity;
	let snappedAnchorStartTime = anchorStartTime;
	let snapPoint: SnapPoint | null = null;

	for (const member of group.members) {
		const memberStartTime = addMediaTime({
			a: anchorStartTime,
			b: member.timeOffset,
		});
		const memberStartSnap = resolveSortedTimelineSnap({
			targetTime: memberStartTime,
			snapPoints,
			maxSnapDistance,
		});
		if (
			memberStartSnap.snapPoint &&
			memberStartSnap.snapDistance < closestSnapDistance
		) {
			closestSnapDistance = memberStartSnap.snapDistance;
			snappedAnchorStartTime = subMediaTime({
				a: memberStartSnap.snappedTime,
				b: member.timeOffset,
			});
			snapPoint = memberStartSnap.snapPoint;
		}

		const memberEndSnap = resolveSortedTimelineSnap({
			targetTime: addMediaTime({
				a: memberStartTime,
				b: member.duration,
			}),
			snapPoints,
			maxSnapDistance,
		});
		if (
			memberEndSnap.snapPoint &&
			memberEndSnap.snapDistance < closestSnapDistance
		) {
			closestSnapDistance = memberEndSnap.snapDistance;
			snappedAnchorStartTime = subMediaTime({
				a: subMediaTime({
					a: memberEndSnap.snappedTime,
					b: member.duration,
				}),
				b: member.timeOffset,
			});
			snapPoint = memberEndSnap.snapPoint;
		}
	}

	return {
		snappedAnchorStartTime,
		snapPoint,
	};
}

export function buildMoveGroupSnapPoints({
	group,
	tracks,
	playheadTime,
}: {
	group: MoveGroup;
	tracks: SceneTracks;
	playheadTime: MediaTime;
}): SnapPoint[] {
	const excludeElementIds = new Set(
		group.members.map((member) => member.elementId),
	);

	return buildSortedTimelineSnapPoints({
		sources: [
			() => getElementEdgeSnapPoints({ tracks, excludeElementIds }),
			() => getPlayheadSnapPoints({ playheadTime }),
			() =>
				getAnimationKeyframeSnapPointsForTimeline({
					tracks,
					excludeElementIds,
				}),
		],
	});
}
