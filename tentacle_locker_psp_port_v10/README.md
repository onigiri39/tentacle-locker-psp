# Tentacle Locker PSP Port v10

WindowsにPSPSDK/WSL/Dockerをインストールせず、GitHub Actions上の公式PSPSDKコンテナでEBOOT.PBPをビルドする版です。

## 一番簡単な使い方

1. GitHubで新しい空のリポジトリを作る。
2. このフォルダの中身をリポジトリにアップロードする。
3. GitHubの **Actions** → **Build PSP EBOOT** → **Run workflow** を押す。
4. ビルドが成功したら、ワークフローの **Artifacts** から `TENTACLE_LOCKER_PSP` を取得する。
5. PSPの `PSP/GAME/TENTACLE_LOCKER_PSP/` に `EBOOT.PBP` と `assets` を置く。

この方式では、手元のWindows PCにPSPSDK、WSL、Docker Desktopは不要です。

## 技術情報

ビルドには公式の `ghcr.io/pspdev/pspsdk:latest` コンテナを使用します。
