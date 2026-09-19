import { en } from "./locales/en";
import { vi } from "./locales/vi";

export const translations = {
	en,
	vi,
} as const;

export type Language = keyof typeof translations;
export type Messages = typeof en;

export const DEFAULT_LANGUAGE: Language = "vi";
export const LANGUAGE_STORAGE_KEY = "opencut:language";

export const LANGUAGE_OPTIONS: Array<{
	value: Language;
	labelKey: TranslationKey;
}> = [
	{ value: "en", labelKey: "common.english" },
	{ value: "vi", labelKey: "common.vietnamese" },
];

type DotPrefix<
	TPrefix extends string,
	TKey extends string,
> = `${TPrefix}.${TKey}`;

type MessageKeys<TValue, TPrefix extends string = ""> = {
	[TKey in keyof TValue & string]: TValue[TKey] extends string
		? TPrefix extends ""
			? TKey
			: DotPrefix<TPrefix, TKey>
		: MessageKeys<
				TValue[TKey],
				TPrefix extends "" ? TKey : DotPrefix<TPrefix, TKey>
			>;
}[keyof TValue & string];

export type TranslationKey = MessageKeys<Messages>;

export function isLanguage(value: string): value is Language {
	return value in translations;
}
