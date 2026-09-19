"use client";

import {
	createContext,
	useCallback,
	useContext,
	useEffect,
	useMemo,
	type ReactNode,
} from "react";
import { useLocalStorage } from "@/services/storage/use-local-storage";
import {
	DEFAULT_LANGUAGE,
	isLanguage,
	LANGUAGE_STORAGE_KEY,
	translations,
	type Language,
	type TranslationKey,
} from "./translations";

interface I18nContextValue {
	language: Language;
	setLanguage: (language: Language) => void;
	t: (key: TranslationKey, values?: Record<string, string | number>) => string;
}

const I18nContext = createContext<I18nContextValue | null>(null);

function isRecord(value: unknown): value is Record<string, unknown> {
	return Boolean(value) && typeof value === "object";
}

function readMessage({
	language,
	key,
}: {
	language: Language;
	key: TranslationKey;
}) {
	const parts = key.split(".");
	let value: unknown = translations[language];

	for (const part of parts) {
		if (!isRecord(value) || !(part in value)) {
			return key;
		}

		value = value[part];
	}

	return typeof value === "string" ? value : key;
}

function formatMessage({
	message,
	values,
}: {
	message: string;
	values?: Record<string, string | number>;
}) {
	if (!values) return message;

	return message.replace(/\{(\w+)\}/g, (match, name) =>
		name in values ? String(values[name]) : match,
	);
}

export function I18nProvider({ children }: { children: ReactNode }) {
	const [storedLanguage, setStoredLanguage] = useLocalStorage<Language>({
		key: LANGUAGE_STORAGE_KEY,
		defaultValue: DEFAULT_LANGUAGE,
	});

	const language = isLanguage(storedLanguage)
		? storedLanguage
		: DEFAULT_LANGUAGE;

	useEffect(() => {
		document.documentElement.lang = language;
	}, [language]);

	const setLanguage = useCallback(
		(nextLanguage: Language) => {
			setStoredLanguage({ value: nextLanguage });
		},
		[setStoredLanguage],
	);

	const t = useCallback(
		(key: TranslationKey, values?: Record<string, string | number>) =>
			formatMessage({
				message: readMessage({ language, key }),
				values,
			}),
		[language],
	);

	const value = useMemo(
		() => ({ language, setLanguage, t }),
		[language, setLanguage, t],
	);

	return <I18nContext.Provider value={value}>{children}</I18nContext.Provider>;
}

export function useTranslation() {
	const context = useContext(I18nContext);

	if (!context) {
		throw new Error("useTranslation must be used within I18nProvider");
	}

	return context;
}
