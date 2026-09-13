import { type NextRequest, NextResponse } from "next/server";

export const dynamic = "force-dynamic";

/**
 * Split text into readable chunks under maxLen characters respecting punctuation
 */
function splitTextIntoChunks(text: string, maxLen = 160): string[] {
	const trimmed = text.trim();
	if (trimmed.length <= maxLen) {
		return [trimmed];
	}

	const sentences = trimmed.match(/[^.!?,\n]+[.!?,\n]*|\s+/g) || [trimmed];
	const chunks: string[] = [];
	let currentChunk = "";

	for (const part of sentences) {
		if ((currentChunk + part).length <= maxLen) {
			currentChunk += part;
		} else {
			if (currentChunk.trim().length > 0) {
				chunks.push(currentChunk.trim());
			}
			// If a single word/part is longer than maxLen, hard-slice it
			if (part.length > maxLen) {
				for (let i = 0; i < part.length; i += maxLen) {
					chunks.push(part.slice(i, i + maxLen).trim());
				}
				currentChunk = "";
			} else {
				currentChunk = part;
			}
		}
	}

	if (currentChunk.trim().length > 0) {
		chunks.push(currentChunk.trim());
	}

	return chunks.filter((c) => c.length > 0);
}

/**
 * Fetch MP3 audio for a single chunk from Google TTS
 */
async function fetchTtsChunk(text: string, lang = "vi", speed = 1.0): Promise<ArrayBuffer> {
	// Google TTS speed parameter: 1 = normal, <1 slower
	const encodedText = encodeURIComponent(text);
	const url = `https://translate.google.com/translate_tts?ie=UTF-8&q=${encodedText}&tl=${lang}&client=tw-ob&ttsspeed=${speed < 0.9 ? "0.3" : "1"}`;

	const res = await fetch(url, {
		headers: {
			"User-Agent":
				"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
			Referer: "https://translate.google.com/",
		},
	});

	if (!res.ok) {
		throw new Error(`Google TTS request failed with status: ${res.status}`);
	}

	return res.arrayBuffer();
}

export async function POST(req: NextRequest) {
	try {
		const body = await req.json();
		const { text, langCode = "vi", speed = 1.0 } = body;

		if (!text || typeof text !== "string" || text.trim().length === 0) {
			return NextResponse.json(
				{ error: "Văn bản không được để trống" },
				{ status: 400 },
			);
		}

		if (text.length > 4000) {
			return NextResponse.json(
				{ error: "Văn bản vượt quá giới hạn 4000 ký tự" },
				{ status: 400 },
			);
		}

		const chunks = splitTextIntoChunks(text, 160);
		if (chunks.length === 0) {
			return NextResponse.json(
				{ error: "Không tìm thấy nội dung hợp lệ" },
				{ status: 400 },
			);
		}

		// Fetch all chunks in order
		const audioBuffers: ArrayBuffer[] = [];
		for (const chunk of chunks) {
			try {
				const buf = await fetchTtsChunk(chunk, langCode, speed);
				audioBuffers.push(buf);
			} catch (err) {
				console.error("Failed to fetch chunk:", chunk, err);
			}
		}

		if (audioBuffers.length === 0) {
			return NextResponse.json(
				{ error: "Không thể tạo âm thanh cho văn bản" },
				{ status: 502 },
			);
		}

		// Concatenate all MP3 chunks together
		const totalLength = audioBuffers.reduce((acc, b) => acc + b.byteLength, 0);
		const combined = new Uint8Array(totalLength);
		let offset = 0;
		for (const b of audioBuffers) {
			combined.set(new Uint8Array(b), offset);
			offset += b.byteLength;
		}

		return new NextResponse(combined, {
			status: 200,
			headers: {
				"Content-Type": "audio/mpeg",
				"Content-Length": totalLength.toString(),
				"Cache-Control": "public, max-age=3600, s-maxage=3600",
			},
		});
	} catch (error) {
		console.error("[TTS API Error]:", error);
		return NextResponse.json(
			{
				error: error instanceof Error ? error.message : "Internal TTS server error",
			},
			{ status: 500 },
		);
	}
}
