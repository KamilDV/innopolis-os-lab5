#!/bin/sh
set -eu
cd "$(dirname "$0")"
n=${1:-}
case "$n" in 1|2|3) ;; *) echo "Usage: $0 n (1..3)" >&2; exit 1;; esac
${CC:-cc} -O2 -std=c11 -Wall -Wextra publisher.c -o publisher
${CC:-cc} -O2 -std=c11 -Wall -Wextra subscriber.c -o subscriber
if command -v gnome-terminal >/dev/null 2>&1; then
  gnome-terminal -- sh -c "cd '$PWD'; ./publisher '$n'; exec sh" &
  i=1; while [ "$i" -le "$n" ]; do
    gnome-terminal -- sh -c "cd '$PWD'; ./subscriber '$i'; exec sh" &
    i=$((i + 1))
  done
elif command -v xterm >/dev/null 2>&1; then
  xterm -e sh -c "cd '$PWD'; ./publisher '$n'; exec sh" &
  i=1; while [ "$i" -le "$n" ]; do
    xterm -e sh -c "cd '$PWD'; ./subscriber '$i'; exec sh" &
    i=$((i + 1))
  done
elif command -v osascript >/dev/null 2>&1; then
  osascript - "$PWD" "$n" <<'APPLESCRIPT'
on run argv
  set workdir to item 1 of argv
  set countSubscribers to (item 2 of argv) as integer
  tell application "Terminal"
    do script "cd " & quoted form of workdir & "; ./publisher " & countSubscribers
    repeat with i from 1 to countSubscribers
      do script "cd " & quoted form of workdir & "; ./subscriber " & i
    end repeat
    activate
  end tell
end run
APPLESCRIPT
else
  echo "Open separate terminals in $PWD and run ./publisher $n and ./subscriber 1..$n" >&2
fi
