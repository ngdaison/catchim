"use client";

import { useState, useEffect } from "react";
import { useEditor } from "@/editor/use-editor";
import { formatTimecode } from "opencut-wasm";
import { invokeAction } from "@/actions";
import { EditableTimecode } from "@/components/editable-timecode";
import { Button } from "@/components/ui/button";
import {
	FullScreenIcon,
	PauseIcon,
	PlayIcon,
} from "@hugeicons/core-free-icons";
import { HugeiconsIcon } from "@hugeicons/react";
import { Separator } from "@/components/ui/separator";
import {
	Select,
	SelectTrigger,
	SelectContent,
	SelectItem,
	SelectSeparator,
} from "@/components/ui/select";
import { PREVIEW_ZOOM_PRESETS } from "@/preview/zoom";
import { usePreviewViewport } from "./preview-viewport";
import type { MediaTime } from "@/wasm";
import { useTranslation } from "@/i18n";

export function PreviewToolbar({
	onToggleFullscreen,
}: {
	onToggleFullscreen: () => void;
}) {
	return (
		<div className="flex items-center justify-between px-3 py-1 sm:grid sm:grid-cols-[1fr_auto_1fr] sm:pb-3 sm:pt-5 sm:px-5">
			<TimecodeDisplay />
			<div className="flex items-center">
				<PlayPauseButton />
			</div>
			<div className="justify-self-end flex items-center gap-1.5 sm:gap-2.5">
				<div className="hidden sm:flex items-center gap-2.5">
					<ZoomSelect />
					<Separator orientation="vertical" className="h-4" />
				</div>
				<Button variant="text" size="icon" className="size-8" onClick={onToggleFullscreen}>
					<HugeiconsIcon icon={FullScreenIcon} className="size-4" />
				</Button>
			</div>
		</div>
	);
}

function TimecodeDisplay() {
	const editor = useEditor();
	const totalDuration = useEditor((e) => e.timeline.getTotalDuration());
	const fps = useEditor((e) => e.project.getActive().settings.fps);
	const [currentTime, setCurrentTime] = useState<MediaTime>(() =>
		editor.playback.getCurrentTime(),
	);

	useEffect(() => {
		const unsubscribeUpdate = editor.playback.onUpdate(setCurrentTime);
		const unsubscribeSeek = editor.playback.onSeek(setCurrentTime);
		return () => {
			unsubscribeUpdate();
			unsubscribeSeek();
		};
	}, [editor.playback]);

	return (
		<div className="flex items-center">
			<EditableTimecode
				time={currentTime}
				duration={totalDuration}
				format="HH:MM:SS:FF"
				fps={fps}
				onTimeChange={({ time }) => editor.playback.seek({ time })}
				className="text-center"
			/>
			<span className="text-muted-foreground px-2 font-mono text-xs">/</span>
			<span className="text-muted-foreground font-mono text-xs">
				{formatTimecode({
					time: totalDuration,
					format: "HH:MM:SS:FF",
					rate: fps,
				})}
			</span>
		</div>
	);
}

function ZoomSelect() {
	const { t } = useTranslation();
	const { isAtFit, zoomPercent, fitToScreen, setViewportPercent } =
		usePreviewViewport();

	const displayLabel = isAtFit ? t("preview.fit") : `${zoomPercent}%`;

	const onValueChange = (value: string) => {
		if (value === "fit") {
			fitToScreen();
		} else {
			setViewportPercent({ percent: Number(value) });
		}
	};

	return (
		<Select
			value={isAtFit ? "fit" : String(zoomPercent)}
			onValueChange={onValueChange}
		>
			<SelectTrigger className="tabular-nums">{displayLabel}</SelectTrigger>
			<SelectContent>
				<SelectItem value="fit">{t("preview.fit")}</SelectItem>
				<SelectSeparator />
				{PREVIEW_ZOOM_PRESETS.map((preset) => (
					<SelectItem key={preset} value={String(preset)}>
						{preset}%
					</SelectItem>
				))}
			</SelectContent>
		</Select>
	);
}

function PlayPauseButton() {
	const isPlaying = useEditor((e) => e.playback.getIsPlaying());

	return (
		<Button
			variant="text"
			size="icon"
			onClick={() => invokeAction("toggle-play")}
		>
			<HugeiconsIcon icon={isPlaying ? PauseIcon : PlayIcon} />
		</Button>
	);
}
