"use client";

import { useEffect } from "react";

const deploymentId = process.env.NEXT_PUBLIC_DEPLOYMENT_ID ?? "local";

export function ServiceWorkerProvider() {
	useEffect(() => {
		const enabled =
			process.env.NODE_ENV === "production" ||
			process.env.NEXT_PUBLIC_ENABLE_SERVICE_WORKER === "1";
		if (!enabled || !("serviceWorker" in navigator)) {
			return;
		}

		const controller = new AbortController();
		const register = () => {
			void navigator.serviceWorker
				.register(
					`/service-worker.js?version=${encodeURIComponent(deploymentId)}`,
				)
				.catch(() => undefined);
		};

		if (document.readyState === "complete") {
			register();
		} else {
			window.addEventListener("load", register, {
				once: true,
				signal: controller.signal,
			});
		}

		return () => controller.abort();
	}, []);

	return null;
}
