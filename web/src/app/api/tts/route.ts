import { type NextRequest, NextResponse } from "next/server";
import { MsEdgeTTS, OUTPUT_FORMAT } from "msedge-tts";
import fs from "node:fs";
import path from "node:path";

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

/**
 * Helper to locate models/voice folder across development and production layouts
 */
function getVoiceModelDir(): string {
	const candidates = [
		path.join(process.cwd(), "models", "voice"),
		path.join(process.cwd(), "apps", "web", "models", "voice"),
		"d:\\DATA\\source\\catchim\\apps\\web\\models\\voice",
	];
	for (const candidate of candidates) {
		if (fs.existsSync(candidate)) {
			return candidate;
		}
	}
	return candidates[0];
}

interface VoiceMetadata {
	id?: string;
	name?: string;
	description?: string;
	language?: string;
	langCode?: string;
	modelPth?: string;
	modelCkpt?: string;
	sampleAudio?: string;
	promptText?: string;
	promptLang?: string;
}

let cachedSovitsWeights: string | null = null;
let cachedGptWeights: string | null = null;

/**
 * Synthesize audio via local Python GPT-SoVITS api_v2 server
 */
async function synthesizeWithGptSovits(
	text: string,
	speed: number = 1.0,
): Promise<{ buffer: Buffer; contentType: string }> {
	const voiceDir = getVoiceModelDir();
	const infoPath = path.join(voiceDir, "info.json");

	let meta: VoiceMetadata = {
		modelPth: "model.pth",
		modelCkpt: "model.ckpt",
		sampleAudio: "sample.wav",
		promptText: "Xin chào các bạn, đây là giọng đọc nhân bản AI từ GPT-SoVITS.",
		promptLang: "auto",
		langCode: "auto",
	};

	if (fs.existsSync(infoPath)) {
		try {
			const parsed = JSON.parse(fs.readFileSync(infoPath, "utf-8"));
			meta = { ...meta, ...parsed };
		} catch (e) {
			console.warn("[GPT-SoVITS] Không thể đọc info.json, dùng cấu hình mặc định:", e);
		}
	}

	const pthFile = path.resolve(voiceDir, meta.modelPth || "model.pth");
	const ckptFile = path.resolve(voiceDir, meta.modelCkpt || "model.ckpt");
	const sampleFile = path.resolve(voiceDir, meta.sampleAudio || "sample.wav");

	const missingFiles: string[] = [];
	if (!fs.existsSync(pthFile)) missingFiles.push(path.basename(pthFile));
	if (!fs.existsSync(ckptFile)) missingFiles.push(path.basename(ckptFile));
	if (!fs.existsSync(sampleFile)) missingFiles.push(path.basename(sampleFile));

	if (missingFiles.length > 0) {
		throw new Error(
			`Chưa tìm thấy file mô hình: ${missingFiles.join(", ")} trong thư mục apps/web/models/voice/. Vui lòng đặt các file (.pth, .ckpt, sample.wav) vào thư mục này để kích hoạt giọng clone.`
		);
	}

	const gptSovitsBaseUrl = process.env.GPT_SOVITS_URL || "http://127.0.0.1:9880";

	// Verify or switch SoVITS weights if changed
	if (cachedSovitsWeights !== pthFile) {
		try {
			const setSovitsRes = await fetch(
				`${gptSovitsBaseUrl}/set_sovits_weights?weights_path=${encodeURIComponent(pthFile)}`,
				{ method: "GET" }
			);
			if (!setSovitsRes.ok) {
				const errData = await setSovitsRes.text();
				throw new Error(`Lỗi nạp trọng số SoVITS: ${errData}`);
			}
			cachedSovitsWeights = pthFile;
		} catch (err: unknown) {
			if (err instanceof TypeError || (err instanceof Error && err.message.includes("fetch failed"))) {
				throw new Error(
					`Không thể kết nối đến máy chủ GPT-SoVITS tại ${gptSovitsBaseUrl}. Vui lòng khởi chạy server Python (chạy script: apps/web/scripts/start-gpt-sovits.bat hoặc lệnh: python api_v2.py -p 9880).`
				);
			}
			throw err;
		}
	}

	// Verify or switch GPT weights if changed
	if (cachedGptWeights !== ckptFile) {
		try {
			const setGptRes = await fetch(
				`${gptSovitsBaseUrl}/set_gpt_weights?weights_path=${encodeURIComponent(ckptFile)}`,
				{ method: "GET" }
			);
			if (!setGptRes.ok) {
				const errData = await setGptRes.text();
				throw new Error(`Lỗi nạp trọng số GPT: ${errData}`);
			}
			cachedGptWeights = ckptFile;
		} catch (err: unknown) {
			if (err instanceof TypeError || (err instanceof Error && err.message.includes("fetch failed"))) {
				throw new Error(
					`Không thể kết nối đến máy chủ GPT-SoVITS tại ${gptSovitsBaseUrl}. Vui lòng khởi chạy server Python.`
				);
			}
			throw err;
		}
	}

	// Request TTS inference
	const ttsPayload = {
		text,
		text_lang: meta.langCode || "auto",
		ref_audio_path: sampleFile,
		prompt_text: meta.promptText || "",
		prompt_lang: meta.promptLang || "auto",
		speed_factor: Math.max(0.5, Math.min(2.0, Number(speed) || 1.0)),
		media_type: "wav",
		streaming_mode: false,
	};

	let ttsRes: Response;
	try {
		ttsRes = await fetch(`${gptSovitsBaseUrl}/tts`, {
			method: "POST",
			headers: { "Content-Type": "application/json" },
			body: JSON.stringify(ttsPayload),
		});
	} catch {
		throw new Error(
			`Không thể kết nối đến máy chủ GPT-SoVITS tại ${gptSovitsBaseUrl}. Vui lòng kiểm tra server Python đã chạy chưa.`
		);
	}

	if (!ttsRes.ok) {
		let detail = "";
		try {
			const errJson = await ttsRes.json();
			detail = errJson.message || errJson.Exception || JSON.stringify(errJson);
		} catch {
			detail = await ttsRes.text();
		}
		throw new Error(`GPT-SoVITS tạo âm thanh thất bại (${ttsRes.status}): ${detail}`);
	}

	const arrayBuf = await ttsRes.arrayBuffer();
	return {
		buffer: Buffer.from(arrayBuf),
		contentType: "audio/wav",
	};
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

		// Case 0: Built-in GPT-SoVITS cloned neural voice
		if (engine === "gpt-sovits" || voiceId === "vi-custom-gpt-sovits") {
			try {
				const effectiveSpeed = Number(userSpeed) || 1.0;
				const { buffer, contentType } = await synthesizeWithGptSovits(cleanText, effectiveSpeed);
				return new NextResponse(buffer as unknown as BodyInit, {
					status: 200,
					headers: {
						"Content-Type": contentType,
						"Content-Length": buffer.length.toString(),
						"Cache-Control": "no-store",
					},
				});
			} catch (gptError) {
				console.warn("[GPT-SoVITS Warning]:", gptError instanceof Error ? gptError.message : gptError);
				return NextResponse.json(
					{
						error: gptError instanceof Error ? gptError.message : "GPT-SoVITS synthesis failed",
					},
					{ status: 400 },
				);
			}
		}

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
