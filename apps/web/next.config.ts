import type { NextConfig } from "next";
import path from "node:path";
import { withBotId } from "botid/next/config";
import { withContentCollections } from "@content-collections/next";

const deploymentId =
	process.env.CF_PAGES_COMMIT_SHA ??
	process.env.VERCEL_GIT_COMMIT_SHA ??
	process.env.GITHUB_SHA ??
	"local";

const nextConfig: NextConfig = {
	serverExternalPackages: ["@huggingface/transformers", "onnxruntime-common", "onnxruntime-web"],
	compiler: {
		removeConsole: process.env.NODE_ENV === "production",
	},
	reactStrictMode: true,
	productionBrowserSourceMaps: true,
	output: "standalone",
	turbopack: {
		root: path.resolve(__dirname, "../.."),
	},
	env: {
		NEXT_PUBLIC_DEPLOYMENT_ID: deploymentId,
	},
	webpack: (config) => {
		config.resolve.alias = {
			...config.resolve.alias,
			"opencut-wasm": path.resolve(__dirname, "src/native/opencut-wasm-compat.ts"),
		};
		return config;
	},
	async headers() {
		return [
			{
				source: "/service-worker.js",
				headers: [
					{
						key: "Cache-Control",
						value: "no-cache, no-store, must-revalidate",
					},
				],
			},
			{
				source: "/wasm/:path*",
				headers: [
					{
						key: "Cache-Control",
						value: "public, max-age=0, must-revalidate",
					},
				],
			},
		];
	},
	images: {
		remotePatterns: [
			{
				protocol: "https",
				hostname: "plus.unsplash.com",
			},
			{
				protocol: "https",
				hostname: "images.unsplash.com",
			},
			{
				protocol: "https",
				hostname: "images.marblecms.com",
			},
			{
				protocol: "https",
				hostname: "lh3.googleusercontent.com",
			},
			{
				protocol: "https",
				hostname: "avatars.githubusercontent.com",
			},
			{
				protocol: "https",
				hostname: "api.iconify.design",
			},
			{
				protocol: "https",
				hostname: "api.simplesvg.com",
			},
			{
				protocol: "https",
				hostname: "api.unisvg.com",
			},
			{
				protocol: "https",
				hostname: "cdn.brandfetch.io",
			},
		],
	},
};

export default withContentCollections(withBotId(nextConfig));
