"use client";

import { useEffect, useState } from "react";
import { useRouter } from "next/navigation";
import { Button } from "@/components/ui/button";
import { useEditor } from "@/editor/use-editor";
import { ExportButton } from "@/components/editor/export-button";
import { RenameProjectDialog } from "@/project/components/rename-project-dialog";
import { ThemeToggle } from "@/components/theme-toggle";
import { useTranslation } from "@/i18n";
import {
	DropdownMenu,
	DropdownMenuContent,
	DropdownMenuItem,
	DropdownMenuSeparator,
	DropdownMenuTrigger,
} from "@/components/ui/dropdown-menu";
import {
	ArrowLeft01Icon,
	MoreHorizontalIcon,
	Edit02Icon,
} from "@hugeicons/core-free-icons";
import { HugeiconsIcon } from "@hugeicons/react";
import { Undo2, Redo2 } from "lucide-react";
import { toast } from "sonner";
import { cn } from "@/utils/ui";

export function MobileEditorHeader() {
	const router = useRouter();
	const editor = useEditor();
	const { t, language } = useTranslation();
	const activeProject = useEditor((e) => e.project.getActiveOrNull());
	const [isRenameOpen, setIsRenameOpen] = useState(false);
	const [canUndo, setCanUndo] = useState(false);
	const [canRedo, setCanRedo] = useState(false);

	useEffect(() => {
		const updateHistory = () => {
			setCanUndo(editor.command.canUndo());
			setCanRedo(editor.command.canRedo());
		};

		updateHistory();
		editor.command.registerReactor(updateHistory);
	}, [editor.command]);

	const handleExit = async () => {
		try {
			await editor.project.prepareExit();
			editor.project.closeProject();
		} catch {
			editor.project.closeProject();
		}
		router.push("/projects");
	};

	const handleUndo = () => {
		if (editor.command.canUndo()) {
			if (typeof navigator !== "undefined" && navigator.vibrate) {
				navigator.vibrate(10);
			}
			editor.command.undo();
			setCanUndo(editor.command.canUndo());
			setCanRedo(editor.command.canRedo());
		}
	};

	const handleRedo = () => {
		if (editor.command.canRedo()) {
			if (typeof navigator !== "undefined" && navigator.vibrate) {
				navigator.vibrate(10);
			}
			editor.command.redo();
			setCanUndo(editor.command.canUndo());
			setCanRedo(editor.command.canRedo());
		}
	};

	const handleRenameConfirm = async (newName: string) => {
		if (
			activeProject &&
			newName.trim() &&
			newName !== activeProject.metadata.name
		) {
			try {
				await editor.project.renameProject({
					id: activeProject.metadata.id,
					name: newName.trim(),
				});
			} catch (error) {
				toast.error(
					error instanceof Error
						? error.message
						: t("project.failedToRenameProject"),
				);
			} finally {
				setIsRenameOpen(false);
			}
		}
	};

	const projectName = activeProject?.metadata.name || "";
	const displayProjectName =
		language === "vi" && projectName === "New project"
			? t("assets.projectNameDefault")
			: projectName || "OpenCut";

	const fps = activeProject?.settings.fps?.numerator || 30;

	return (
		<header className="flex h-12 w-full shrink-0 items-center justify-between border-b bg-background/95 px-2.5 backdrop-blur-md">
			{/* Left: Back button & Title */}
			<div className="flex min-w-0 items-center gap-1.5">
				<Button
					variant="ghost"
					size="icon"
					className="size-8 shrink-0 active:scale-95 transition-transform"
					aria-label={t("mobile.back")}
					onClick={handleExit}
				>
					<HugeiconsIcon icon={ArrowLeft01Icon} className="size-5" />
				</Button>

				<button
					type="button"
					onClick={() => setIsRenameOpen(true)}
					className="flex max-w-[120px] items-center gap-1 truncate rounded px-1.5 py-0.5 text-left text-xs font-semibold hover:bg-accent active:scale-95 transition-all"
				>
					<span className="truncate">{displayProjectName}</span>
					<span className="shrink-0 rounded bg-muted px-1 py-0.2 text-[9px] font-normal text-muted-foreground">
						{fps}fps
					</span>
				</button>
			</div>

			{/* Right: Undo, Redo, Export & Menu */}
			<div className="flex items-center gap-1">
				<Button
					variant="ghost"
					size="icon"
					disabled={!canUndo}
					onClick={handleUndo}
					aria-label={t("mobile.undo")}
					className={cn(
						"size-8 text-foreground/80 active:scale-90 transition-transform",
						!canUndo && "opacity-30",
					)}
				>
					<Undo2 className="size-4" />
				</Button>

				<Button
					variant="ghost"
					size="icon"
					disabled={!canRedo}
					onClick={handleRedo}
					aria-label={t("mobile.redo")}
					className={cn(
						"size-8 text-foreground/80 active:scale-90 transition-transform",
						!canRedo && "opacity-30",
					)}
				>
					<Redo2 className="size-4" />
				</Button>

				<div className="scale-90 origin-right">
					<ExportButton />
				</div>

				<DropdownMenu>
					<DropdownMenuTrigger asChild>
						<Button
							variant="ghost"
							size="icon"
							className="size-8 active:scale-95 transition-transform"
							aria-label={t("common.more")}
						>
							<HugeiconsIcon icon={MoreHorizontalIcon} className="size-4" />
						</Button>
					</DropdownMenuTrigger>
					<DropdownMenuContent align="end" className="w-44 z-50">
						<DropdownMenuItem
							onClick={() => setIsRenameOpen(true)}
							icon={<HugeiconsIcon icon={Edit02Icon} className="size-4" />}
						>
							{t("project.renameProject")}
						</DropdownMenuItem>
						<DropdownMenuSeparator />
						<div className="px-2 py-1.5 flex items-center justify-between">
							<span className="text-xs text-muted-foreground">
								{t("assets.settings")}
							</span>
							<ThemeToggle />
						</div>
					</DropdownMenuContent>
				</DropdownMenu>
			</div>

			<RenameProjectDialog
				isOpen={isRenameOpen}
				onOpenChange={setIsRenameOpen}
				onConfirm={handleRenameConfirm}
				projectName={projectName}
			/>
		</header>
	);
}
