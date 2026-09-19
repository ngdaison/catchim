export const LANGUAGES = [
	{ code: "en", name: "English", nameVi: "Tiếng Anh" },
	{ code: "es", name: "Spanish", nameVi: "Tiếng Tây Ban Nha" },
	{ code: "it", name: "Italian", nameVi: "Tiếng Ý" },
	{ code: "fr", name: "French", nameVi: "Tiếng Pháp" },
	{ code: "de", name: "German", nameVi: "Tiếng Đức" },
	{ code: "pt", name: "Portuguese", nameVi: "Tiếng Bồ Đào Nha" },
	{ code: "ru", name: "Russian", nameVi: "Tiếng Nga" },
	{ code: "ja", name: "Japanese", nameVi: "Tiếng Nhật" },
	{ code: "vi", name: "Vietnamese", nameVi: "Tiếng Việt" },
	{ code: "zh", name: "Chinese", nameVi: "Tiếng Trung" },
] as const;

export type Language = (typeof LANGUAGES)[number];
export type LanguageCode = Language["code"];
