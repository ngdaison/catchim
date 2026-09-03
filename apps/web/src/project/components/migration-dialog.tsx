"use client";

import {
	Dialog,
	DialogContent,
	DialogDescription,
	DialogHeader,
	DialogTitle,
} from "@/components/ui/dialog";
import { useEditor } from "@/editor/use-editor";
import { Loader2 } from "lucide-react";
import { useTranslation } from "@/i18n";

export function MigrationDialog() {
	const { t } = useTranslation();
	const editor = useEditor();
	const migrationState = editor.project.getMigrationState();

	if (!migrationState.isMigrating) return null;

	const fromVersion = migrationState.fromVersion ?? "-";
	const toVersion = migrationState.toVersion ?? "-";
	const title = migrationState.projectName
		? t("project.updatingProject")
		: t("project.updatingProjects");
	const description = migrationState.projectName
		? t("project.updateProjectDescription", {
				name: migrationState.projectName,
				fromVersion,
				toVersion,
			})
		: t("project.updateProjectsDescription", {
				fromVersion,
				toVersion,
			});

	return (
		<Dialog open={true}>
			<DialogContent
				className="sm:max-w-md"
				onPointerDownOutside={(event) => event.preventDefault()}
				onEscapeKeyDown={(event) => event.preventDefault()}
			>
				<DialogHeader>
					<DialogTitle>{title}</DialogTitle>
					<DialogDescription>{description}</DialogDescription>
				</DialogHeader>

				<div className="flex items-center justify-center py-4">
					<Loader2 className="text-muted-foreground size-8 animate-spin" />
				</div>
			</DialogContent>
		</Dialog>
	);
}
