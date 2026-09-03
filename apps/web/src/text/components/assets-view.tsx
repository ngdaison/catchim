import { DraggableItem } from "@/components/editor/panels/assets/draggable-item";
import { PanelView } from "@/components/editor/panels/assets/views/base-panel";
import { useEditor } from "@/editor/use-editor";
import { DEFAULTS } from "@/timeline/defaults";
import { buildTextElement } from "@/timeline/element-utils";
import type { MediaTime } from "@/wasm";
import { useTranslation } from "@/i18n";

export function TextView() {
	const editor = useEditor();
	const { t } = useTranslation();
	const defaultText = t("assets.textDefault");

	const handleAddToTimeline = ({ currentTime }: { currentTime: MediaTime }) => {
		const activeScene = editor.scenes.getActiveScene();
		if (!activeScene) return;

		const element = buildTextElement({
			raw: {
				...DEFAULTS.text.element,
				name: t("assets.text"),
				params: {
					...DEFAULTS.text.element.params,
					content: defaultText,
				},
			},
			startTime: currentTime,
		});

		editor.timeline.insertElement({
			element,
			placement: { mode: "auto" },
		});
	};

	return (
		<PanelView title={t("assets.text")}>
			<DraggableItem
				name={defaultText}
				preview={
					<div className="bg-accent flex size-full items-center justify-center rounded">
						<span className="text-xs select-none">{defaultText}</span>
					</div>
				}
				dragData={{
					id: "temp-text-id",
					type: DEFAULTS.text.element.type,
					name: t("assets.text"),
					content: defaultText,
				}}
				aspectRatio={1}
				onAddToTimeline={handleAddToTimeline}
				shouldShowLabel={false}
			/>
		</PanelView>
	);
}
