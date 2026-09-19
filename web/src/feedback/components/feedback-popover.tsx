"use client";

import { useState } from "react";
import { useForm, useWatch } from "react-hook-form";
import { toast } from "sonner";
import { ClockIcon } from "lucide-react";
import {
	Dialog,
	DialogContent,
	DialogTitle,
	DialogTrigger,
} from "@/components/ui/dialog";
import { Button } from "@/components/ui/button";
import { Textarea } from "@/components/ui/textarea";
import { Spinner } from "@/components/ui/spinner";
import {
	Form,
	FormField,
	FormItem,
	FormControl,
	clearFormDraft,
} from "@/components/ui/form";
import type { FeedbackEntry } from "../types";
import { useTranslation } from "@/i18n";

const PERSIST_KEY = "feedback-draft";
const HISTORY_KEY = "feedback-history";
const MAX_HISTORY = 20;

interface FeedbackFormValues {
	message: string;
}

function isFeedbackEntry(value: unknown): value is FeedbackEntry {
	return (
		typeof value === "object" &&
		value !== null &&
		"id" in value &&
		"message" in value &&
		"createdAt" in value &&
		typeof value.id === "string" &&
		typeof value.message === "string" &&
		typeof value.createdAt === "string"
	);
}

function readHistory(): FeedbackEntry[] {
	try {
		const stored = localStorage.getItem(HISTORY_KEY);
		if (!stored) return [];

		const parsed: unknown = JSON.parse(stored);
		return Array.isArray(parsed) ? parsed.filter(isFeedbackEntry) : [];
	} catch {
		return [];
	}
}

function writeHistory({ entries }: { entries: FeedbackEntry[] }): void {
	try {
		localStorage.setItem(HISTORY_KEY, JSON.stringify(entries));
	} catch {
		// localStorage may be full or unavailable
	}
}

function useFeedback() {
	const { t } = useTranslation();
	const [entries, setEntries] = useState<FeedbackEntry[]>(readHistory);
	const [isSubmitting, setIsSubmitting] = useState(false);

	async function submit({
		values,
		onSuccess,
	}: {
		values: FeedbackFormValues;
		onSuccess: () => void;
	}) {
		if (isSubmitting) return;
		setIsSubmitting(true);

		try {
			const res = await fetch("/api/feedback", {
				method: "POST",
				headers: { "Content-Type": "application/json" },
				body: JSON.stringify(values),
			});

			if (!res.ok) {
				const data = await res.json().catch(() => null);
				throw new Error(data?.error ?? t("feedback.submitError"));
			}

			const { entry } = await res.json();
			const next = [entry, ...entries].slice(0, MAX_HISTORY);
			setEntries(next);
			writeHistory({ entries: next });
			onSuccess();
			toast.success(t("feedback.sent"));
		} catch (error) {
			toast.error(
				error instanceof Error ? error.message : t("feedback.submitError"),
			);
		} finally {
			setIsSubmitting(false);
		}
	}

	return { entries, isSubmitting, submit };
}

export function FeedbackPopover() {
	const { t } = useTranslation();
	const [open, setOpen] = useState(false);

	return (
		<Dialog open={open} onOpenChange={setOpen}>
			<DialogTrigger asChild>
				<Button variant="outline" className="h-8">
					{t("feedback.button")}
				</Button>
			</DialogTrigger>
			<DialogContent className="w-[calc(100%-2rem)] max-w-sm overflow-hidden p-0">
				<div className="border-b p-3 pr-14">
					<DialogTitle className="text-sm font-medium">
						{t("feedback.button")}
					</DialogTitle>
				</div>
				<FeedbackPopoverContent onClose={() => setOpen(false)} />
			</DialogContent>
		</Dialog>
	);
}

type View = "compose" | "history";

function FeedbackPopoverContent({ onClose }: { onClose: () => void }) {
	const { t } = useTranslation();
	const { entries, isSubmitting, submit } = useFeedback();
	const [view, setView] = useState<View>("compose");

	const form = useForm<FeedbackFormValues>({
		defaultValues: { message: "" },
	});
	const message =
		useWatch({
			control: form.control,
			name: "message",
		}) ?? "";

	async function handleSubmit(values: FeedbackFormValues) {
		await submit({
			values,
			onSuccess: () => {
				form.reset({ message: "" });
				clearFormDraft({ key: PERSIST_KEY });
				onClose();
			},
		});
	}

	if (view === "history") {
		return (
			<div className="flex flex-col">
				<div
					className="max-h-72 overflow-y-auto divide-y"
					style={{
						maskImage:
							"linear-gradient(to bottom, black 80%, transparent 100%)",
					}}
				>
					{entries.map((entry) => (
						<FeedbackEntryItem key={entry.id} entry={entry} />
					))}
				</div>
				<div className="border-t px-3 py-2">
					<button
						type="button"
						onClick={() => setView("compose")}
						className="text-xs text-muted-foreground/60 hover:text-muted-foreground transition-colors"
					>
						← {t("feedback.back")}
					</button>
				</div>
			</div>
		);
	}

	return (
		<div className="flex flex-col">
			<Form persistKey={PERSIST_KEY} {...form}>
				<form
					onSubmit={form.handleSubmit(handleSubmit)}
					className="flex flex-col"
				>
					<FormField
						control={form.control}
						name="message"
						render={({ field }) => (
							<FormItem>
								<FormControl>
									<Textarea
										placeholder={t("feedback.placeholder")}
										className="min-h-[7rem] text-sm p-3 bg-background shadow-none border-none! resize-none"
										{...field}
									/>
								</FormControl>
							</FormItem>
						)}
					/>
					<div className="flex items-center justify-between border-t px-3 py-2">
						{entries.length > 0 ? (
							<button
								type="button"
								onClick={() => setView("history")}
								className="flex items-center gap-1.5 text-xs text-muted-foreground/60 hover:text-muted-foreground transition-colors"
							>
								<ClockIcon className="size-3" />
								{entries.length}
							</button>
						) : (
							<span />
						)}
						<div className="flex gap-2">
							{!message.trim() && (
								<Button
									type="button"
									variant="outline"
									size="sm"
									onClick={onClose}
								>
									{t("common.cancel")}
								</Button>
							)}
							<Button
								type="submit"
								size="sm"
								disabled={isSubmitting || !message.trim()}
							>
								{isSubmitting ? <Spinner /> : t("common.send")}
							</Button>
						</div>
					</div>
				</form>
			</Form>
		</div>
	);
}

function relativeDate({
	iso,
	t,
}: {
	iso: string;
	t: ReturnType<typeof useTranslation>["t"];
}): string {
	const diff = Date.now() - new Date(iso).getTime();
	const mins = Math.floor(diff / 60_000);
	if (mins < 1) return t("feedback.justNow");
	if (mins < 60) return t("feedback.minutesAgo", { count: mins });
	const hrs = Math.floor(mins / 60);
	if (hrs < 24) return t("feedback.hoursAgo", { count: hrs });
	const days = Math.floor(hrs / 24);
	if (days < 7) return t("feedback.daysAgo", { count: days });
	return new Date(iso).toLocaleDateString(undefined, {
		month: "short",
		day: "numeric",
	});
}

function FeedbackEntryItem({ entry }: { entry: FeedbackEntry }) {
	const { t } = useTranslation();

	return (
		<div className="px-3 py-2.5">
			<p className="text-sm text-muted-foreground leading-snug whitespace-pre-wrap break-words">
				{entry.message}
			</p>
			<span className="mt-1 block text-[11px] text-muted-foreground/50">
				{relativeDate({ iso: entry.createdAt, t })}
			</span>
		</div>
	);
}
