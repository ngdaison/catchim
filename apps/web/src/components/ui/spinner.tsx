import { HugeiconsIcon, type HugeiconsIconProps } from "@hugeicons/react";
import { Loading03Icon } from "@hugeicons/core-free-icons";
import { cn } from "@/utils/ui";
import { useTranslation } from "@/i18n";

function Spinner({ className, ...props }: Omit<HugeiconsIconProps, "icon">) {
	const { t } = useTranslation();

	return (
		<HugeiconsIcon
			icon={Loading03Icon}
			role="status"
			aria-label={t("common.loading")}
			className={cn("size-4 animate-spin", className)}
			{...props}
		/>
	);
}

export { Spinner };
