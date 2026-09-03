"use client";

import { useMemo } from "react";
import { useKeybindingsStore } from "@/actions/keybindings-store";
import {
	ACTIONS,
	type TActionCategory,
	type TActionWithOptionalArgs,
} from "@/actions";
import { useTranslation, type TranslationKey } from "@/i18n";
import {
	getPlatformAlternateKey,
	getPlatformSpecialKey,
} from "@/utils/platform";

export interface KeyboardShortcut {
	id: string;
	keys: string[];
	description: string;
	category: string;
	categoryId: TActionCategory;
	action: TActionWithOptionalArgs;
	icon?: React.ReactNode;
}

const ACTION_LABEL_KEYS: Record<TActionWithOptionalArgs, TranslationKey> = {
	"cancel-interaction": "shortcuts.actionCancelInteraction",
	"copy-selected": "shortcuts.actionCopySelected",
	"delete-selected": "shortcuts.actionDeleteSelected",
	"deselect-all": "shortcuts.actionDeselectAll",
	"duplicate-selected": "shortcuts.actionDuplicateSelected",
	"frame-step-backward": "shortcuts.actionFrameStepBackward",
	"frame-step-forward": "shortcuts.actionFrameStepForward",
	"goto-end": "shortcuts.actionGoToTimelineEnd",
	"goto-start": "shortcuts.actionGoToTimelineStart",
	"jump-backward": "shortcuts.actionJumpBackward",
	"jump-forward": "shortcuts.actionJumpForward",
	"paste-copied": "shortcuts.actionPasteElementsAtPlayhead",
	redo: "shortcuts.actionRedo",
	"seek-backward": "shortcuts.actionSeekBackward",
	"seek-forward": "shortcuts.actionSeekForward",
	"select-all": "shortcuts.actionSelectAllElements",
	split: "shortcuts.actionSplitElementsAtPlayhead",
	"split-left": "shortcuts.actionSplitAndRemoveLeft",
	"split-right": "shortcuts.actionSplitAndRemoveRight",
	"stop-playback": "shortcuts.actionStopPlayback",
	"toggle-bookmark": "shortcuts.actionToggleBookmarkAtPlayhead",
	"toggle-elements-muted-selected":
		"shortcuts.actionMuteUnmuteSelectedElements",
	"toggle-elements-visibility-selected":
		"shortcuts.actionShowHideSelectedElements",
	"toggle-play": "shortcuts.actionPlayPause",
	"toggle-ripple-editing": "shortcuts.actionToggleRippleEditing",
	"toggle-snapping": "shortcuts.actionToggleSnapping",
	"toggle-source-audio": "shortcuts.actionExtractOrRecoverSourceAudio",
	undo: "shortcuts.actionUndo",
};

const CATEGORY_LABEL_KEYS: Record<TActionCategory, TranslationKey> = {
	assets: "shortcuts.categoryAssets",
	controls: "shortcuts.categoryControls",
	editing: "shortcuts.categoryEditing",
	history: "shortcuts.categoryHistory",
	navigation: "shortcuts.categoryNavigation",
	playback: "shortcuts.categoryPlayback",
	selection: "shortcuts.categorySelection",
	timeline: "shortcuts.categoryTimeline",
};

function formatKey({ key }: { key: string }): string {
	return key
		.replace("ctrl", getPlatformSpecialKey())
		.replace("alt", getPlatformAlternateKey())
		.replace("shift", "Shift")
		.replace("left", "←")
		.replace("right", "→")
		.replace("up", "↑")
		.replace("down", "↓")
		.replace("space", "Space")
		.replace("home", "Home")
		.replace("enter", "Enter")
		.replace("end", "End")
		.replace("delete", "Delete")
		.replace("backspace", "Backspace")
		.replace("-", "+");
}

export function useKeyboardShortcutsHelp() {
	const { t } = useTranslation();
	const { keybindings } = useKeybindingsStore();

	const shortcuts = useMemo(() => {
		const actionToKeys = new Map<TActionWithOptionalArgs, string[]>();

		for (const [key, action] of keybindings) {
			const existing = actionToKeys.get(action);
			if (existing) {
				existing.push(formatKey({ key }));
			} else {
				actionToKeys.set(action, [formatKey({ key })]);
			}
		}

		const result: KeyboardShortcut[] = [];
		for (const [action, keys] of actionToKeys) {
			const actionDef = ACTIONS[action];
			if (!actionDef) continue;
			result.push({
				id: action,
				keys,
				description: t(ACTION_LABEL_KEYS[action] ?? "common.more"),
				category: t(CATEGORY_LABEL_KEYS[actionDef.category]),
				categoryId: actionDef.category,
				action,
			});
		}

		return result.sort((a, b) => {
			if (a.categoryId !== b.categoryId) {
				return a.categoryId.localeCompare(b.categoryId);
			}
			return a.description.localeCompare(b.description);
		});
	}, [keybindings, t]);

	return {
		shortcuts,
	};
}
