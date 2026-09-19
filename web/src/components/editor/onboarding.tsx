"use client";

import { ArrowRightIcon } from "lucide-react";
import { useState } from "react";
import ReactMarkdown from "react-markdown";
import { SOCIAL_LINKS } from "@/site/social";
import { useLocalStorage } from "@/services/storage/use-local-storage";
import { Button } from "../ui/button";
import { Dialog, DialogBody, DialogContent, DialogTitle } from "../ui/dialog";
import { useTranslation } from "@/i18n";

export function Onboarding() {
	const { t } = useTranslation();
	const [step, setStep] = useState(0);
	const [hasSeenOnboarding, setHasSeenOnboarding] = useLocalStorage({
		key: "hasSeenOnboarding",
		defaultValue: false,
	});

	const isOpen = !hasSeenOnboarding;

	const handleNext = () => {
		setStep(step + 1);
	};

	const handleClose = () => {
		setHasSeenOnboarding({ value: true });
	};

	const getStepTitle = () => {
		switch (step) {
			case 0:
				return t("onboarding.titleWelcome");
			case 1:
				return t("onboarding.titleBeta");
			case 2:
				return t("onboarding.titleTesting");
			default:
				return t("onboarding.titleFallback");
		}
	};

	const renderStepContent = () => {
		switch (step) {
			case 0:
				return (
					<div className="space-y-5">
						<div className="space-y-3">
							<Title title={t("onboarding.titleWelcome")} />
							<Description description={t("onboarding.descriptionWelcome")} />
						</div>
						<NextButton onClick={handleNext}>{t("onboarding.next")}</NextButton>
					</div>
				);
			case 1:
				return (
					<div className="space-y-5">
						<div className="space-y-3">
							<Title title={getStepTitle()} />
							<Description description={t("onboarding.descriptionBetaOne")} />
							<Description description={t("onboarding.descriptionBetaTwo")} />
							<Description description={t("onboarding.descriptionBetaRoadmap")} />
						</div>
						<NextButton onClick={handleNext}>{t("onboarding.next")}</NextButton>
					</div>
				);
			case 2:
				return (
					<div className="space-y-5">
						<div className="space-y-3">
							<Title title={getStepTitle()} />
							<Description
								description={t("onboarding.descriptionTesting", {
									discordUrl: SOCIAL_LINKS.discord,
								})}
							/>
						</div>
						<NextButton onClick={handleClose}>
							{t("onboarding.finish")}
						</NextButton>
					</div>
				);
			default:
				return null;
		}
	};

	return (
		<Dialog open={isOpen} onOpenChange={handleClose}>
			<DialogContent className="sm:max-w-[425px]">
				<DialogTitle>
					<span className="sr-only">{getStepTitle()}</span>
				</DialogTitle>
				<DialogBody>{renderStepContent()}</DialogBody>
			</DialogContent>
		</Dialog>
	);
}

function Title({ title }: { title: string }) {
	return <h2 className="text-lg font-bold md:text-xl">{title}</h2>;
}

function Description({ description }: { description: string }) {
	return (
		<div className="text-muted-foreground">
			<ReactMarkdown
				components={{
					p: ({ children }) => <p className="mb-0">{children}</p>,
					a: ({ href, children }) => (
						<a
							href={href}
							target="_blank"
							rel="noopener noreferrer"
							className="text-foreground hover:text-foreground/80 underline"
						>
							{children}
						</a>
					),
				}}
			>
				{description}
			</ReactMarkdown>
		</div>
	);
}

function NextButton({
	children,
	onClick,
}: {
	children: React.ReactNode;
	onClick: () => void;
}) {
	return (
		<Button onClick={onClick} variant="default" className="w-full">
			{children}
			<ArrowRightIcon className="size-4" />
		</Button>
	);
}
