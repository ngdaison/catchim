"use client";

import { useState } from "react";
import { MobileEditorHeader } from "./mobile-editor-header";
import { MobileBottomNavigation } from "./mobile-bottom-navigation";
import { MobileBottomSheet, type MobileSheetType } from "./mobile-bottom-sheet";
import { PreviewPanel } from "@/preview/components";
import { Timeline } from "@/timeline/components";
import { Button } from "@/components/ui/button";
import { useAssetsPanelStore } from "@/components/editor/panels/assets/assets-panel-store";
import { useTranslation } from "@/i18n";
import type {
	PreviewOverlayControl,
	PreviewOverlayInstance,
} from "@/preview/overlays";
import { Plus } from "lucide-react";

interface MobileEditorLayoutProps {
	overlayControls: PreviewOverlayControl[];
	overlayInstances: PreviewOverlayInstance[];
	onOverlayVisibilityChange: (params: {
		overlayId: string;
		isVisible: boolean;
	}) => void;
}

export function MobileEditorLayout({
	overlayControls,
	overlayInstances,
	onOverlayVisibilityChange,
}: MobileEditorLayoutProps) {
	const { t } = useTranslation();
	const { setActiveTab } = useAssetsPanelStore();
	const [activeSheet, setActiveSheet] = useState<MobileSheetType>(null);

	const handleOpenAddMedia = () => {
		if (typeof navigator !== "undefined" && navigator.vibrate) {
			navigator.vibrate(10);
		}
		setActiveTab("media");
		setActiveSheet("assets");
	};

	return (
		<div className="relative flex h-full w-full flex-col overflow-hidden bg-background select-none">
			{/* 1. Mobile CapCut Header */}
			<MobileEditorHeader />

			{/* 2. Main Content Area */}
			<div className="relative flex min-h-0 flex-1 flex-col overflow-hidden pb-[calc(4.75rem+env(safe-area-inset-bottom))]">
				{/* Top Half: Video Preview Player (~45% - 48% height) */}
				<div className="min-h-0 flex-[1.05] overflow-hidden p-1.5 pb-0">
					<PreviewPanel
						overlayControls={overlayControls}
						overlayInstances={overlayInstances}
						onOverlayVisibilityChange={onOverlayVisibilityChange}
					/>
				</div>

				{/* Quick Action Bar between Preview & Timeline */}
				<div className="flex h-7 shrink-0 items-center justify-between px-3 py-1 text-xs text-muted-foreground">
					<span className="text-[11px] font-medium opacity-70">Trục thời gian</span>
					<Button
						variant="ghost"
						size="sm"
						onClick={handleOpenAddMedia}
						className="h-6 gap-1 px-2 text-[11px] font-medium text-primary hover:bg-primary/10 active:scale-95 transition-all"
					>
						<Plus className="size-3.5 stroke-[2.5]" />
						<span>{t("mobile.addMedia")}</span>
					</Button>
				</div>

				{/* Bottom Half: Touch-optimized Timeline (~52% - 55% height) */}
				<div className="min-h-[140px] flex-[0.95] overflow-hidden px-1.5">
					<Timeline />
				</div>
			</div>

			{/* 3. Slide-Up Bottom Sheet (Live adjustments with video visible) */}
			<MobileBottomSheet
				sheetType={activeSheet}
				onClose={() => setActiveSheet(null)}
			/>

			{/* 4. CapCut Dual-Mode Bottom Navigation Bar */}
			<MobileBottomNavigation
				activeSheet={activeSheet}
				onOpenSheet={(sheet) => setActiveSheet(sheet)}
				onCloseSheet={() => setActiveSheet(null)}
			/>
		</div>
	);
}
