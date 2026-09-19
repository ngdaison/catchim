import type { SceneTracks, TimelineTrack } from "@/timeline";
import type { GroupTrackSection } from "./types";

export interface TrackPlacement {
	trackId: string;
	trackType: TimelineTrack["type"];
	section: GroupTrackSection;
	sectionIndex: number;
	displayIndex: number;
}

export function getDisplayTracks({
	tracks,
}: {
	tracks: SceneTracks;
}): TimelineTrack[] {
	return [...tracks.overlay, tracks.main, ...tracks.audio];
}

export function getTrackPlacementById({
	tracks,
	trackId,
}: {
	tracks: SceneTracks;
	trackId: string;
}): TrackPlacement | null {
	if (tracks.main.id === trackId) {
		return {
			trackId,
			trackType: tracks.main.type,
			section: "main",
			sectionIndex: -1,
			displayIndex: tracks.overlay.length,
		};
	}

	const overlayTrackIndex = tracks.overlay.findIndex(
		(track) => track.id === trackId,
	);
	if (overlayTrackIndex >= 0) {
		return {
			trackId,
			trackType: tracks.overlay[overlayTrackIndex].type,
			section: "overlay",
			sectionIndex: overlayTrackIndex,
			displayIndex: overlayTrackIndex,
		};
	}

	const audioTrackIndex = tracks.audio.findIndex(
		(track) => track.id === trackId,
	);
	if (audioTrackIndex >= 0) {
		return {
			trackId,
			trackType: tracks.audio[audioTrackIndex].type,
			section: "audio",
			sectionIndex: audioTrackIndex,
			displayIndex: tracks.overlay.length + 1 + audioTrackIndex,
		};
	}

	return null;
}

export function getTrackPlacementByDisplayIndex({
	tracks,
	displayIndex,
}: {
	tracks: SceneTracks;
	displayIndex: number;
}): TrackPlacement | null {
	if (displayIndex < 0) {
		return null;
	}

	if (displayIndex < tracks.overlay.length) {
		const track = tracks.overlay[displayIndex];
		return {
			trackId: track.id,
			trackType: track.type,
			section: "overlay",
			sectionIndex: displayIndex,
			displayIndex,
		};
	}

	const mainDisplayIndex = tracks.overlay.length;
	if (displayIndex === mainDisplayIndex) {
		return {
			trackId: tracks.main.id,
			trackType: tracks.main.type,
			section: "main",
			sectionIndex: -1,
			displayIndex,
		};
	}

	const audioTrackIndex = displayIndex - mainDisplayIndex - 1;
	const audioTrack = tracks.audio[audioTrackIndex];
	if (!audioTrack) {
		return null;
	}

	return {
		trackId: audioTrack.id,
		trackType: audioTrack.type,
		section: "audio",
		sectionIndex: audioTrackIndex,
		displayIndex,
	};
}
