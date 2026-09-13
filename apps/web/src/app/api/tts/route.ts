import { type NextRequest, NextResponse } from "next/server";
import { MsEdgeTTS, OUTPUT_FORMAT } from "msedge-tts";

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
 * Fetch MP3 audio chunk from Google Translate TTS
 */
async function fetchGoogleTtsChunk(text: string, lang = "vi", speed = 1.0): Promise<ArrayBuffer> {
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

/**
 * Fallback synthesizer using Google Translate TTS
 */
async function fetchGoogleTts(text: string, lang = "vi", speed = 1.0): Promise<Uint8Array> {
	const chunks = splitTextIntoChunks(text, 160);
	if (chunks.length === 0) {
		throw new Error("Không có văn bản hợp lệ để tổng hợp");
	}

	const audioBuffers: ArrayBuffer[] = [];
	for (const chunk of chunks) {
		const buf = await fetchGoogleTtsChunk(chunk, lang, speed);
		audioBuffers.push(buf);
	}

	const totalLength = audioBuffers.reduce((acc, b) => acc + b.byteLength, 0);
	const combined = new Uint8Array(totalLength);
	let offset = 0;
	for (const b of audioBuffers) {
		combined.set(new Uint8Array(b), offset);
		offset += b.byteLength;
	}
	return combined;
}

/**
 * Synthesize speech using Microsoft Edge Neural TTS with exact pitch and rate
 */
async function synthesizeWithEdge(
	text: string,
	voiceName: string,
	pitchHz: number = 0,
	rateMultiplier: number = 1.0,
): Promise<Buffer> {
	const tts = new MsEdgeTTS();
	await tts.setMetadata(voiceName, OUTPUT_FORMAT.AUDIO_24KHZ_96KBITRATE_MONO_MP3);

	const clampedPitch = Math.max(-120, Math.min(160, Math.round(pitchHz)));
	const pitchStr = clampedPitch >= 0 ? `+${clampedPitch}Hz` : `${clampedPitch}Hz`;
	const clampedRate = Math.max(0.5, Math.min(2.5, Number(rateMultiplier.toFixed(2))));

	const { audioStream } = tts.toStream(text, {
		pitch: pitchStr,
		rate: clampedRate,
	});

	return new Promise<Buffer>((resolve, reject) => {
		const chunks: Buffer[] = [];
		let resolved = false;

		const timer = setTimeout(() => {
			if (!resolved) {
				resolved = true;
				try {
					tts.close();
				} catch {}
				reject(new Error("Edge TTS synthesis timeout (12s)"));
			}
		}, 12000);

		audioStream.on("data", (chunk: Buffer) => {
			chunks.push(chunk);
		});

		audioStream.on("end", () => {
			if (!resolved) {
				resolved = true;
				clearTimeout(timer);
				try {
					tts.close();
				} catch {}
				resolve(Buffer.concat(chunks));
			}
		});

		audioStream.on("error", (err: unknown) => {
			if (!resolved) {
				resolved = true;
				clearTimeout(timer);
				try {
					tts.close();
				} catch {}
				reject(err);
			}
		});
	});
}

export async function POST(req: NextRequest) {
	try {
		const body = await req.json();
		const {
			text,
			voiceId,
			edgeVoiceName,
			pitchHz = 0,
			rateMultiplier = 1.0,
			userPitch = 0,
			userSpeed = 1.0,
			engine = "edge",
			langCode = "vi",
		} = body;

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

		const cleanText = text.trim();

		// Case 1: Specific Google Translate voice requested
		if (engine === "google" || voiceId === "vi-female-google") {
			const googleBuffer = await fetchGoogleTts(cleanText, langCode, userSpeed);
			return new NextResponse(googleBuffer as unknown as BodyInit, {
				status: 200,
				headers: {
					"Content-Type": "audio/mpeg",
					"Content-Length": googleBuffer.byteLength.toString(),
					"Cache-Control": "public, max-age=3600, s-maxage=3600",
				},
			});
		}

		// Case 2: Neural Edge TTS voice
		const voiceName =
			edgeVoiceName ||
			(langCode.startsWith("en") ? "en-US-JennyNeural" : "vi-VN-HoaiMyNeural");

		// User slider pitch ranges from -40% to +40% -> mapped to Hz offset
		const effectivePitchHz = Number(pitchHz) + Number(userPitch) * 1.5;
		// User slider speed ranges from 0.5 to 2.0
		const effectiveRate = Number(rateMultiplier) * Number(userSpeed);

		try {
			const edgeAudio = await synthesizeWithEdge(
				cleanText,
				voiceName,
				effectivePitchHz,
				effectiveRate,
			);

			if (edgeAudio.length > 0) {
				return new NextResponse(edgeAudio as unknown as BodyInit, {
					status: 200,
					headers: {
						"Content-Type": "audio/mpeg",
						"Content-Length": edgeAudio.length.toString(),
						"Cache-Control": "public, max-age=3600, s-maxage=3600",
					},
				});
			}
		} catch (edgeError) {
			console.warn("[Edge TTS failed, falling back to Google TTS]:", edgeError);
		}

		// Fallback to Google Translate TTS if Edge TTS had an issue
		const fallbackBuffer = await fetchGoogleTts(cleanText, langCode, userSpeed);
		return new NextResponse(fallbackBuffer as unknown as BodyInit, {
			status: 200,
			headers: {
				"Content-Type": "audio/mpeg",
				"Content-Length": fallbackBuffer.byteLength.toString(),
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
