const version = new URL(self.location.href).searchParams.get("version") || "v1";
const cacheName = `opencut-static-${version}`;
const warmAssets = [
	"/manifest.json",
	"/wasm/opencut_core.js",
	"/icons/android-icon-192x192.png",
	"/icons/apple-icon-180x180.png",
];

function isCacheableAsset(request, url) {
	if (request.method !== "GET" || url.origin !== self.location.origin) {
		return false;
	}

	return (
		url.pathname.startsWith("/_next/static/") ||
		url.pathname.startsWith("/wasm/") ||
		url.pathname.startsWith("/fonts/") ||
		url.pathname.startsWith("/icons/")
	);
}

async function cacheAsset(cache, request, response) {
	if (response.ok && response.type === "basic") {
		await cache.put(request, response.clone());
	}
	return response;
}

self.addEventListener("install", (event) => {
	event.waitUntil(
		(async () => {
			const cache = await caches.open(cacheName);
			await Promise.allSettled(
				warmAssets.map(async (asset) => {
					const response = await fetch(asset, { cache: "reload" });
					await cacheAsset(cache, asset, response);
				}),
			);
			await self.skipWaiting();
		})(),
	);
});

self.addEventListener("activate", (event) => {
	event.waitUntil(
		(async () => {
			const cacheNames = await caches.keys();
			await Promise.all(
				cacheNames
					.filter(
						(name) => name.startsWith("opencut-static-") && name !== cacheName,
					)
					.map((name) => caches.delete(name)),
			);
			await self.clients.claim();
		})(),
	);
});

self.addEventListener("fetch", (event) => {
	const { request } = event;
	if (request.headers.has("range")) {
		return;
	}

	const url = new URL(request.url);
	if (!isCacheableAsset(request, url)) {
		return;
	}

	const response = caches.open(cacheName).then(async (cache) => {
		const cached = await cache.match(request);
		const network = fetch(request)
			.then((response) => cacheAsset(cache, request, response))
			.catch((error) => {
				if (cached) {
					return cached;
				}
				throw error;
			});

		return { cached, network };
	});

	event.respondWith(response.then(({ cached, network }) => cached ?? network));
	event.waitUntil(
		response
			.then(({ network }) => network)
			.then(() => undefined)
			.catch(() => undefined),
	);
});
