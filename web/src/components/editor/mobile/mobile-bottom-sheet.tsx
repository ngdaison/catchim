"use client";

import { useMemo } from "react";
import { Button } from "@/components/ui/button";
import { AssetsPanel } from "@/components/editor/panels/assets";
import { PropertiesPanel } from "@/components/editor/panels/properties";
import { BackgroundContent } from "@/components/editor/panels/assets/views/settings/background";
import {
	useAssetsPanelStore,
	tabs,
} from "@/components/editor/panels/assets/assets-panel-store";
import { useEditor } from "@/editor/use-editor";
import { useTranslation, type TranslationKey } from "@/i18n";
import { Cancel01Icon } from "@hugeicons/core-free-icons";
import { HugeiconsIcon } from "@hugeicons/react";
import { Check } from "lucide-react";
import { cn } from "@/utils/ui";
import type { TCanvasSize } from "@/project/types";

export type MobileSheetType =
	"assets" | "properties" | "ratio" | "background" | null;

interface MobileBottomSheetProps {
	sheetType: MobileSheetType;
	onClose: () => void;
}

const RATIO_PRESETS: Array<{
	id: string;
	label: string;
	descKey?: TranslationKey;
	desc?: string;
	size: TCanvasSize;
	aspectBoxClass: string;
}> = [
	{
		id: "9:16",
		label: "9:16",
		desc: "TikTok / Reels",
		size: { width: 1080, height: 1920 },
		aspectBoxClass: "w-4 h-7",
	},
	{
		id: "16:9",
		label: "16:9",
		desc: "YouTube / TV",
		size: { width: 1920, height: 1080 },
		aspectBoxClass: "w-7 h-4",
	},
	{
		id: "1:1",
		label: "1:1",
		desc: "Instagram",
		size: { width: 1080, height: 1080 },
		aspectBoxClass: "w-5 h-5",
	},
	{
		id: "4:5",
		label: "4:5",
		descKey: "mobile.ratioPortrait",
		size: { width: 1080, height: 1350 },
		aspectBoxClass: "w-5 h-6",
	},
	{
		id: "4:3",
		label: "4:3",
		descKey: "mobile.ratioStandard",
		size: { width: 1440, height: 1080 },
		aspectBoxClass: "w-6 h-5",
	},
	{
		id: "21:9",
		label: "21:9",
		descKey: "mobile.ratioCinema",
		size: { width: 2560, height: 1080 },
		aspectBoxClass: "w-8 h-3.5",
	},
];

export function MobileBottomSheet({
	sheetType,
	onClose,
}: MobileBottomSheetProps) {
	const { t } = useTranslation();
	const { activeTab } = useAssetsPanelStore();
	const editor = useEditor();
	const activeProject = useEditor((e) => e.project.getActiveOrNull());
	const currentCanvasSize = activeProject?.settings.canvasSize;

	const title = useMemo(() => {
		if (sheetType === "assets") {
			return t(tabs[activeTab]?.labelKey ?? "mobile.edit");
		}
		if (sheetType === "properties") {
			return t("mobile.properties");
		}
		if (sheetType === "ratio") {
			return t("mobile.ratio");
		}
		if (sheetType === "background") {
			return t("mobile.background");
		}
		return "";
	}, [sheetType, activeTab, t]);

	if (!sheetType) return null;

	const handleSelectRatio = (presetSize: TCanvasSize) => {
		if (typeof navigator !== "undefined" && navigator.vibrate) {
			navigator.vibrate(10);
		}
		editor.project.updateSettings({
			settings: {
				canvasSize: presetSize,
				canvasSizeMode: "preset",
			},
		});
	};

	return (
		<>
			{/* Semi-transparent backdrop that dismisses on tap */}
			<button
				type="button"
				aria-label={t("mobile.closePanel")}
				className="fixed inset-0 z-40 bg-black/40 backdrop-blur-[1px] transition-opacity"
				onClick={onClose}
			/>

			{/* Sliding Sheet Container */}
			<div
				className={cn(
					"fixed inset-x-0 bottom-[calc(4.75rem+env(safe-area-inset-bottom))] z-50",
					"h-[48dvh] max-h-[500px] min-h-[300px]",
					"flex flex-col overflow-hidden rounded-t-2xl border-t bg-background shadow-2xl",
					"animate-in slide-in-from-bottom-5 duration-200 ease-out",
				)}
			>
				{/* Top bar with drag handle and close button */}
				<div className="flex h-11 shrink-0 items-center justify-between border-b px-3.5">
					<div className="flex items-center gap-2.5">
						<div className="h-1.5 w-10 rounded-full bg-muted-foreground/30" />
						<h3 className="text-sm font-semibold text-foreground tracking-tight">
							{title}
						</h3>
					</div>

					<Button
						variant="ghost"
						size="icon"
						className="size-8 rounded-full active:scale-90 transition-transform"
						aria-label={t("mobile.closePanel")}
						onClick={onClose}
					>
						<HugeiconsIcon icon={Cancel01Icon} className="size-4" />
					</Button>
				</div>

				{/* Sheet Body */}
				<div className="min-h-0 flex-1 overflow-y-auto overflow-x-hidden">
					{sheetType === "assets" && (
						<div className="h-full">
							<AssetsPanel
								showTabBar={false}
								className="h-full rounded-none border-0"
							/>
						</div>
					)}

					{sheetType === "properties" && (
						<div className="h-full">
							<PropertiesPanel />
						</div>
					)}

					{sheetType === "ratio" && (
						<div className="flex flex-col gap-3 p-4">
							<p className="text-xs text-muted-foreground">
								{t("mobile.ratioHint")}
							</p>
							<div className="grid grid-cols-3 gap-2.5">
								{RATIO_PRESETS.map((preset) => {
									const isSelected =
										currentCanvasSize?.width === preset.size.width &&
										currentCanvasSize?.height === preset.size.height;

									return (
										<button
											key={preset.id}
											type="button"
											onClick={() => handleSelectRatio(preset.size)}
											className={cn(
												"relative flex flex-col items-center justify-center gap-2 rounded-xl border p-3 text-center transition-all active:scale-95",
												isSelected
													? "border-primary bg-primary/10 text-primary shadow-sm"
													: "border-border bg-card hover:bg-accent text-card-foreground",
											)}
										>
											{isSelected && (
												<div className="absolute top-1.5 right-1.5 size-4 rounded-full bg-primary flex items-center justify-center text-primary-foreground">
													<Check className="size-2.5 stroke-[3]" />
												</div>
											)}
											<div className="flex size-9 items-center justify-center rounded-md bg-muted/60">
												<div
													className={cn(
														"rounded-[2px] border border-current",
														preset.aspectBoxClass,
													)}
												/>
											</div>
											<div className="flex flex-col">
												<span className="text-xs font-bold leading-tight">
													{preset.label}
												</span>
												<span className="text-[10px] text-muted-foreground leading-tight">
													{preset.descKey ? t(preset.descKey) : preset.desc}
												</span>
											</div>
										</button>
									);
								})}
							</div>
						</div>
					)}

					{sheetType === "background" && (
						<div className="p-4">
							<BackgroundContent />
						</div>
					)}
				</div>
			</div>
		</>
	);
}
