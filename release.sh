#!/bin/sh
set -e

VERSION=$1

if [ -z "$VERSION" ]; then
    echo "usage: ./release.sh <version>"
    exit 1
fi

echo "==> releasing strlib v$VERSION"

# Ensure clean working tree
if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "error: working tree is dirty, commit first"
    exit 1
fi

# Prevent duplicate tag
if git rev-parse "v$VERSION" >/dev/null 2>&1; then
    echo "error: tag v$VERSION already exists"
    exit 1
fi

echo "==> building"

mkdir -p build
rm -f build/*

for f in src/*.c; do
    cc -c "$f" -Iinclude -O2 -std=c11 -Wall -Wextra -o "build/$(basename "$f" .c).o"
done

ar rcs build/libstrlib.a build/*.o

echo "==> tagging"

git tag v$VERSION
git push origin main --tags

echo "==> installing headers"

sudo mkdir -p /usr/include/strlib-$VERSION
sudo cp include/*.h /usr/include/strlib-$VERSION/

echo "==> installing library"

sudo cp build/libstrlib.a /usr/lib/libstrlib-$VERSION.a

echo "==> updating symlinks"

sudo rm -rf /usr/include/strlib
sudo rm -f /usr/lib/libstrlib.a

sudo ln -sfn /usr/include/strlib-$VERSION /usr/include/strlib
sudo ln -sfn /usr/lib/libstrlib-$VERSION.a /usr/lib/libstrlib.a

echo "==> done"