ROOT=$(cd "$(dirname "$0")/.."; pwd)

"$ROOT/build/bin/JUCE/Projucer" --resave "$ROOT/MultiDexed.jucer"

cd "$ROOT/Builds/LinuxMakefile"
make CONFIG=Release
