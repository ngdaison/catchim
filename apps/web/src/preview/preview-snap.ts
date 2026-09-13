export interface SnapLine {
	type: "horizontal" | "vertical";
	position: number;
}

const ROTATION_SNAP_STEP_DEGREES = 90;
const ROTATION_SNAP_THRESHOLD_DEGREES = 5;
export const MIN_SCALE = 0.01;
export const SNAP_THRESHOLD_SCREEN_PIXELS = 8;

export interface SnapResult {
	snappedPosition: { x: number; y: number };
	activeLines: SnapLine[];
}

type ScaleEdge = "left" | "right" | "top" | "bottom";

export interface ScaleEdgePreference {
	left?: boolean;
	right?: boolean;
	top?: boolean;
	bottom?: boolean;
}

function hasPreferredEdge({
	preferredEdges,
	edge,
}: {
	preferredEdges?: ScaleEdgePreference;
	edge: ScaleEdge;
}): boolean {
	return preferredEdges?.[edge] === true;
}


export function snapPosition({
	proposedPosition,
	canvasSize,
	elementSize,
	rotation = 0,
	snapThreshold,
}: {
	proposedPosition: { x: number; y: number };
	canvasSize: { width: number; height: number };
	elementSize: { width: number; height: number };
	rotation?: number;
	snapThreshold: { x: number; y: number };
}): SnapResult {
	const centerX = 0;
	const centerY = 0;
	const left = -canvasSize.width / 2;
	const right = canvasSize.width / 2;
	const top = -canvasSize.height / 2;
	const bottom = canvasSize.height / 2;

	const rotRad = (rotation * Math.PI) / 180;
	const cosR = Math.abs(Math.cos(rotRad));
	const sinR = Math.abs(Math.sin(rotRad));
	const halfWidth = (elementSize.width * cosR + elementSize.height * sinR) / 2;
	const halfHeight = (elementSize.width * sinR + elementSize.height * cosR) / 2;
	const activeLines: SnapLine[] = [];

	const verticalTargets = [centerX, left, right];
	const horizontalTargets = [centerY, top, bottom];

	let closestDistX = snapThreshold.x;
	let x = proposedPosition.x;
	let activeLineX: SnapLine | null = null;

	const xPoints = [
		proposedPosition.x,
		proposedPosition.x - halfWidth,
		proposedPosition.x + halfWidth,
	];
	const xOffsets = [0, halfWidth, -halfWidth];

	for (const targetX of verticalTargets) {
		for (let i = 0; i < 3; i++) {
			const dist = Math.abs(xPoints[i] - targetX);
			if (dist <= closestDistX) {
				closestDistX = dist;
				x = targetX + xOffsets[i];
				activeLineX = { type: "vertical", position: targetX };
			}
		}
	}

	let closestDistY = snapThreshold.y;
	let y = proposedPosition.y;
	let activeLineY: SnapLine | null = null;

	const yPoints = [
		proposedPosition.y,
		proposedPosition.y - halfHeight,
		proposedPosition.y + halfHeight,
	];
	const yOffsets = [0, halfHeight, -halfHeight];

	for (const targetY of horizontalTargets) {
		for (let i = 0; i < 3; i++) {
			const dist = Math.abs(yPoints[i] - targetY);
			if (dist <= closestDistY) {
				closestDistY = dist;
				y = targetY + yOffsets[i];
				activeLineY = { type: "horizontal", position: targetY };
			}
		}
	}

	if (activeLineX) {
		activeLines.push(activeLineX);
	}
	if (activeLineY) {
		activeLines.push(activeLineY);
	}

	return {
		snappedPosition: { x, y },
		activeLines,
	};
}

export interface ScaleSnapResult {
	snappedScale: number;
	activeLines: SnapLine[];
}

export function snapScale({
	proposedScale,
	position,
	baseWidth,
	baseHeight,
	rotation = 0,
	canvasSize,
	snapThreshold,
	preferredEdges,
}: {
	proposedScale: number;
	position: { x: number; y: number };
	baseWidth: number;
	baseHeight: number;
	rotation?: number;
	canvasSize: { width: number; height: number };
	snapThreshold: { x: number; y: number };
	preferredEdges?: ScaleEdgePreference;
}): ScaleSnapResult {
	const centerX = 0;
	const centerY = 0;
	const left = -canvasSize.width / 2;
	const right = canvasSize.width / 2;
	const top = -canvasSize.height / 2;
	const bottom = canvasSize.height / 2;

	const rotRad = (rotation * Math.PI) / 180;
	const cosR = Math.abs(Math.cos(rotRad));
	const sinR = Math.abs(Math.sin(rotRad));
	const aabbBaseHalfW = (baseWidth * cosR + baseHeight * sinR) / 2;
	const aabbBaseHalfH = (baseWidth * sinR + baseHeight * cosR) / 2;

	const leftEdge = position.x - aabbBaseHalfW * proposedScale;
	const rightEdge = position.x + aabbBaseHalfW * proposedScale;
	const topEdge = position.y - aabbBaseHalfH * proposedScale;
	const bottomEdge = position.y + aabbBaseHalfH * proposedScale;

	let bestScale: number | null = null;
	let bestDist = Infinity;
	let bestEdge: ScaleEdge | null = null;

	function consider(scale: number, dist: number, edge: ScaleEdge) {
		if (Math.abs(scale) <= MIN_SCALE) return;
		if (dist < bestDist) {
			bestDist = dist;
			bestScale = scale;
			bestEdge = edge;
		} else if (dist === bestDist && bestEdge !== null) {
			const preferNew = hasPreferredEdge({ preferredEdges, edge });
			const preferOld = hasPreferredEdge({ preferredEdges, edge: bestEdge });
			if (preferNew && !preferOld) {
				bestDist = dist;
				bestScale = scale;
				bestEdge = edge;
			}
		}
	}

	const verticalTargets = [
		{ position: left, line: { type: "vertical" as const, position: left } },
		{
			position: centerX,
			line: { type: "vertical" as const, position: centerX },
		},
		{ position: right, line: { type: "vertical" as const, position: right } },
	];

	for (const target of verticalTargets) {
		const distanceLeft = Math.abs(leftEdge - target.position);
		if (distanceLeft <= snapThreshold.x) {
			consider((position.x - target.position) / aabbBaseHalfW, distanceLeft, "left");
		}
		const distanceRight = Math.abs(rightEdge - target.position);
		if (distanceRight <= snapThreshold.x) {
			consider((target.position - position.x) / aabbBaseHalfW, distanceRight, "right");
		}
	}

	const horizontalTargets = [
		{ position: top, line: { type: "horizontal" as const, position: top } },
		{
			position: centerY,
			line: { type: "horizontal" as const, position: centerY },
		},
		{
			position: bottom,
			line: { type: "horizontal" as const, position: bottom },
		},
	];

	for (const target of horizontalTargets) {
		const distanceTop = Math.abs(topEdge - target.position);
		if (distanceTop <= snapThreshold.y) {
			consider((position.y - target.position) / aabbBaseHalfH, distanceTop, "top");
		}
		const distanceBottom = Math.abs(bottomEdge - target.position);
		if (distanceBottom <= snapThreshold.y) {
			consider((target.position - position.y) / aabbBaseHalfH, distanceBottom, "bottom");
		}
	}

	if (bestScale === null) {
		return { snappedScale: proposedScale, activeLines: [] };
	}

	const snappedLeft = position.x - aabbBaseHalfW * bestScale;
	const snappedRight = position.x + aabbBaseHalfW * bestScale;
	const snappedTop = position.y - aabbBaseHalfH * bestScale;
	const snappedBottom = position.y + aabbBaseHalfH * bestScale;

	const activeLines: SnapLine[] = [];
	const seenKeys = new Set<string>();

	function addLine({ line }: { line: SnapLine }) {
		const key = `${line.type}-${line.position}`;
		if (!seenKeys.has(key)) {
			seenKeys.add(key);
			activeLines.push(line);
		}
	}

	for (const target of verticalTargets) {
		if (
			(hasPreferredEdge({ preferredEdges, edge: "left" }) &&
				Math.abs(snappedLeft - target.position) <= 1) ||
			(hasPreferredEdge({ preferredEdges, edge: "right" }) &&
				Math.abs(snappedRight - target.position) <= 1) ||
			(!preferredEdges &&
				(Math.abs(snappedLeft - target.position) <= 1 ||
					Math.abs(snappedRight - target.position) <= 1))
		) {
			addLine({ line: target.line });
		}
	}
	for (const target of horizontalTargets) {
		if (
			(hasPreferredEdge({ preferredEdges, edge: "top" }) &&
				Math.abs(snappedTop - target.position) <= 1) ||
			(hasPreferredEdge({ preferredEdges, edge: "bottom" }) &&
				Math.abs(snappedBottom - target.position) <= 1) ||
			(!preferredEdges &&
				(Math.abs(snappedTop - target.position) <= 1 ||
					Math.abs(snappedBottom - target.position) <= 1))
		) {
			addLine({ line: target.line });
		}
	}

	return {
		snappedScale: bestScale,
		activeLines,
	};
}

export interface AxisSnapResult {
	snappedScale: number;
	/** Infinity when no snap candidate was within threshold */
	snapDistance: number;
	activeLines: SnapLine[];
}

export function snapScaleAxes({
	proposedScaleX,
	proposedScaleY,
	position,
	baseWidth,
	baseHeight,
	rotation = 0,
	canvasSize,
	snapThreshold,
	preferredEdges,
}: {
	proposedScaleX: number;
	proposedScaleY: number;
	position: { x: number; y: number };
	baseWidth: number;
	baseHeight: number;
	rotation?: number;
	canvasSize: { width: number; height: number };
	snapThreshold: { x: number; y: number };
	preferredEdges?: ScaleEdgePreference;
}): { x: AxisSnapResult; y: AxisSnapResult } {
	const canvasLeft = -canvasSize.width / 2;
	const canvasRight = canvasSize.width / 2;
	const canvasTop = -canvasSize.height / 2;
	const canvasBottom = canvasSize.height / 2;

	const rotRad = (rotation * Math.PI) / 180;
	const cosR = Math.abs(Math.cos(rotRad));
	const sinR = Math.abs(Math.sin(rotRad));
	const EPSILON = 1e-6;

	// Current AABB edges at proposed scales
	const currentAabbHalfW = (baseWidth * proposedScaleX * cosR + baseHeight * proposedScaleY * sinR) / 2;
	const currentAabbHalfH = (baseWidth * proposedScaleX * sinR + baseHeight * proposedScaleY * cosR) / 2;
	const currentLeftEdge = position.x - currentAabbHalfW;
	const currentRightEdge = position.x + currentAabbHalfW;
	const currentTopEdge = position.y - currentAabbHalfH;
	const currentBottomEdge = position.y + currentAabbHalfH;

	let bestScaleX: number | null = null;
	let bestDistX = Infinity;
	let bestEdgeX: ScaleEdge | null = null;
	let bestLineX: SnapLine | null = null;

	function considerX(scale: number, dist: number, line: SnapLine, edge: ScaleEdge) {
		if (Math.abs(scale) <= MIN_SCALE) return;
		if (dist < bestDistX) {
			bestDistX = dist;
			bestScaleX = scale;
			bestEdgeX = edge;
			bestLineX = line;
		} else if (dist === bestDistX && bestEdgeX !== null) {
			const preferNew = hasPreferredEdge({ preferredEdges, edge });
			const preferOld = hasPreferredEdge({ preferredEdges, edge: bestEdgeX });
			if (preferNew && !preferOld) {
				bestDistX = dist;
				bestScaleX = scale;
				bestEdgeX = edge;
				bestLineX = line;
			}
		}
	}

	const yContribW = baseHeight * proposedScaleY * sinR;
	const yContribH = baseHeight * proposedScaleY * cosR;

	if (cosR > EPSILON) {
		for (const T of [canvasLeft, 0, canvasRight]) {
			const line: SnapLine = { type: "vertical", position: T };
			const distLeft = Math.abs(currentLeftEdge - T);
			if (distLeft <= snapThreshold.x) {
				considerX((2 * (position.x - T) - yContribW) / (baseWidth * cosR), distLeft, line, "left");
			}
			const distRight = Math.abs(currentRightEdge - T);
			if (distRight <= snapThreshold.x) {
				considerX((2 * (T - position.x) - yContribW) / (baseWidth * cosR), distRight, line, "right");
			}
		}
	}

	if (sinR > EPSILON) {
		for (const T of [canvasTop, 0, canvasBottom]) {
			const line: SnapLine = { type: "horizontal", position: T };
			const distTop = Math.abs(currentTopEdge - T);
			if (distTop <= snapThreshold.y) {
				considerX((2 * (position.y - T) - yContribH) / (baseWidth * sinR), distTop, line, "top");
			}
			const distBottom = Math.abs(currentBottomEdge - T);
			if (distBottom <= snapThreshold.y) {
				considerX((2 * (T - position.y) - yContribH) / (baseWidth * sinR), distBottom, line, "bottom");
			}
		}
	}

	let bestScaleY: number | null = null;
	let bestDistY = Infinity;
	let bestEdgeY: ScaleEdge | null = null;
	let bestLineY: SnapLine | null = null;

	function considerY(scale: number, dist: number, line: SnapLine, edge: ScaleEdge) {
		if (Math.abs(scale) <= MIN_SCALE) return;
		if (dist < bestDistY) {
			bestDistY = dist;
			bestScaleY = scale;
			bestEdgeY = edge;
			bestLineY = line;
		} else if (dist === bestDistY && bestEdgeY !== null) {
			const preferNew = hasPreferredEdge({ preferredEdges, edge });
			const preferOld = hasPreferredEdge({ preferredEdges, edge: bestEdgeY });
			if (preferNew && !preferOld) {
				bestDistY = dist;
				bestScaleY = scale;
				bestEdgeY = edge;
				bestLineY = line;
			}
		}
	}

	const xContribW = baseWidth * proposedScaleX * cosR;
	const xContribH = baseWidth * proposedScaleX * sinR;

	if (sinR > EPSILON) {
		for (const T of [canvasLeft, 0, canvasRight]) {
			const line: SnapLine = { type: "vertical", position: T };
			const distLeft = Math.abs(currentLeftEdge - T);
			if (distLeft <= snapThreshold.x) {
				considerY((2 * (position.x - T) - xContribW) / (baseHeight * sinR), distLeft, line, "left");
			}
			const distRight = Math.abs(currentRightEdge - T);
			if (distRight <= snapThreshold.x) {
				considerY((2 * (T - position.x) - xContribW) / (baseHeight * sinR), distRight, line, "right");
			}
		}
	}

	if (cosR > EPSILON) {
		for (const T of [canvasTop, 0, canvasBottom]) {
			const line: SnapLine = { type: "horizontal", position: T };
			const distTop = Math.abs(currentTopEdge - T);
			if (distTop <= snapThreshold.y) {
				considerY((2 * (position.y - T) - xContribH) / (baseHeight * cosR), distTop, line, "top");
			}
			const distBottom = Math.abs(currentBottomEdge - T);
			if (distBottom <= snapThreshold.y) {
				considerY((2 * (T - position.y) - xContribH) / (baseHeight * cosR), distBottom, line, "bottom");
			}
		}
	}

	return {
		x: {
			snappedScale: bestScaleX !== null ? bestScaleX : proposedScaleX,
			snapDistance: bestDistX,
			activeLines: bestLineX ? [bestLineX] : [],
		},
		y: {
			snappedScale: bestScaleY !== null ? bestScaleY : proposedScaleY,
			snapDistance: bestDistY,
			activeLines: bestLineY ? [bestLineY] : [],
		},
	};
}

export interface RotationSnapResult {
	snappedRotation: number;
	isSnapped: boolean;
}

export function snapRotation({
	proposedRotation,
}: {
	proposedRotation: number;
}): RotationSnapResult {
	const nearestRotationSnap =
		Math.round(proposedRotation / ROTATION_SNAP_STEP_DEGREES) *
		ROTATION_SNAP_STEP_DEGREES;
	const distanceToNearestSnap = Math.abs(
		proposedRotation - nearestRotationSnap,
	);
	if (distanceToNearestSnap <= ROTATION_SNAP_THRESHOLD_DEGREES) {
		return { snappedRotation: nearestRotationSnap, isSnapped: true };
	}
	return { snappedRotation: proposedRotation, isSnapped: false };
}
