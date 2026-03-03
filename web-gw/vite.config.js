import { defineConfig } from 'vite'
import preact from '@preact/preset-vite'
import compression from "vite-plugin-compression";

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    preact(),
    compression({
      algorithm: "gzip",
      ext: ".gz",
      deleteOriginFile: false,
      filter: /\.(js|css|html)$/i,
    }),
  ],
  base: './',
  build: {
    sourcemap: false,
    minify: "esbuild",
    outDir: "dist",
    assetsDir: "assets",
  }
})
